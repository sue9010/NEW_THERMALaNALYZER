import logging
import threading
import numpy as np
import queue
import zstandard as zstd
from enum import Enum
from pathlib import Path
from . import ThermalCamera as tc
from . import define_enums as de
from . import define_protocol as dp



class PLAY_TYPE(Enum):
    PLAY = 0
    STOP = 1


class ThermalReaderException(Exception):
    def __init__(self, message: str):
        super().__init__(message)


class ThermalReader:
    def __init__(self):
        self.running = False
        self.file_path = None
        self.width = 0
        self.height = 0
        self.cam_env = dp.TPKT_CameraEnv()
        self.file_hd = dp.TPKT_RecFileHeader()
        self.data_hd = dp.TPKT_RecDataHeader()
        self.temp_mode = de.TEMPERATURE_MODE.TMODE_NONE
        self.tempLut = {}
        self.lvl_ringbuff = None
        self.temp_buff = None
        self.gray = None
        self.gray_scaled_ranges = None
        self.playctrl_queue = queue.Queue()
        self.stream_queue = queue.Queue()
        self.logger = logging.getLogger()
        self.reader_thread = threading.Thread(target=self.read_file)
        self.error_callback = None
        self.stream_ready = None
        self.frame_count = 0
        self.curr_frame_idx = 0
        self.play_type = PLAY_TYPE.PLAY



    def set_error_callback(self, callback):
        self.error_callback = callback


    def set_stream_ready(self, callback):
        self.stream_ready = callback


    def reader_close(self):
        self.playctrl_queue.put(False)
        if self.reader_thread.is_alive():
            self.reader_thread.join()
        self.running = False

    def is_opened(self):
        return self.running


    def reader_clear(self):
        self.temp_mode = de.TEMPERATURE_MODE.TMODE_NONE
        self.tempLut = {}
        self.lvl_ringbuff = None
        self.temp_buff = None
        self.gray = None
        self.file_hd = dp.TPKT_RecFileHeader()
        self.data_hd = dp.TPKT_RecDataHeader()
        self.frame_count = 0
        self.curr_frame_idx = 0
        self.playctrl_queue = queue.Queue()
        self.stream_queue = queue.Queue()
        self.play_type = PLAY_TYPE.PLAY


    def record_file_open(self, path):
        self.reader_clear()
        self.file_path = path
        if self.reader_thread is not None and self.reader_thread.is_alive():
            self.reader_thread.join()
        self.reader_thread = threading.Thread(target=self.read_file)
        self.reader_thread.start()
        self.running = True


    def next_frame(self, set_frame_idx=None):
        if set_frame_idx is None:
            idx = self.curr_frame_idx
        else:
            idx = set_frame_idx
        
        if self.play_type == PLAY_TYPE.PLAY:
            idx += 1
        self.curr_frame_idx = idx
        if set_frame_idx is None:
            self.playctrl_queue.put(True)
            return self.stream_queue.get()
        else:
            return None


    def get_position(self):
        frm_size = self.width * (self.height + 3) * np.uint16().itemsize
        if self.curr_frame_idx <= 1:
            self.curr_frame_idx = 1
        elif self.frame_count <= self.curr_frame_idx:
            self.curr_frame_idx = self.frame_count
        sidx = (self.curr_frame_idx - 1) * frm_size
        eidx = sidx + frm_size
        return sidx, eidx


    def read_file(self):
        path = Path(self.file_path)
        if path.suffix == '.zst':
            self.with_decompression()
        elif path.suffix == '.crd':
            self.without_decompression()


    def without_decompression(self):
        try:
            with open(self.file_path, 'rb') as file:
                data = file.read()
                file_size = len(data)
                self.get_header_data(data)
                self.init_camera_env()
                w, h = self.width, self.height
                
                if self.stream_ready:
                    self.stream_ready()
                
                record_size = (w * (h + 3)) * self.data_hd.total_frame * np.uint16().itemsize
                file_hd_size = self.file_hd.get_size()
                data_hd_size = self.data_hd.get_size()
                total_size = record_size + file_hd_size + data_hd_size
                if total_size != file_size:
                    raise ThermalReaderException('the data size is incorrect.')
                
                while True:
                    if not self.playctrl_queue.get():
                        break
                    
                    sidx, eidx = self.get_position()
                    frame_data = np.frombuffer(data[sidx:eidx], dtype=np.uint8)
                    s  = w * np.uint16().itemsize
                    e = s + (w * (h + 2)) * np.uint16().itemsize
                    raw_data = frame_data[s : e]
                    bin_lvl, bin_tail = tc.get_rawdata_to_lvl_and_tail(w, h, raw_data)
                    lvl = np.frombuffer(bin_lvl, dtype=np.uint16)
                    tail = dp.TPKT_RawDataTail.from_bytes(bin_tail)
                    
                    self.temp_mode = de.TEMPERATURE_MODE(
                                        tc.get_current_temp_mode(tail))
                    if self.temp_mode == de.TEMPERATURE_MODE.TMODE_NONE:
                        self.gray, self.gray_scaled_ranges = tc.cvtGrayImage(lvl)
                    else:
                        self.temp_buff, self.min_temp, self.max_temp = tc.cvt_lvl_2_temp(lvl,
                                                                                         self.tempLut,
                                                                                         self.temp_mode)
                        self.gray, self.gray_scaled_ranges = tc.cvt_temp_2_gray(self.temp_buff,
                                                                                self.min_temp,
                                                                                self.max_temp)
                    self.stream_queue.put(self.gray)
        except ThermalReaderException as e:
            if self.error_callback:
                self.error_callback(str(e))
            self.stream_queue.put(0)
        except Exception as e:
            if self.error_callback:
                self.error_callback(str(e))
            self.stream_queue.put(0)
        finally:
            pass
        

    def with_decompression(self):
        try:
            dctx = zstd.ZstdDecompressor()
            with open(self.file_path, 'rb') as compressed:
                with dctx.stream_reader(compressed) as reader:
                    decompressed_data = reader.read()
                    file_size = len(decompressed_data)
                    self.get_header_data(decompressed_data)
                    self.init_camera_env()
                    w, h = self.width, self.height
                    
                    if self.stream_ready:
                        self.stream_ready()
                    
                    record_size = (w * (h + 3)) * self.data_hd.total_frame * np.uint16().itemsize
                    file_hd_size = self.file_hd.get_size()
                    data_hd_size = self.data_hd.get_size()
                    total_size = record_size + file_hd_size + data_hd_size
                    if total_size != file_size:
                        raise ThermalReaderException('The data size is incorrect.')
                    
                    while True:
                        if not self.playctrl_queue.get():
                            break
                        
                        sidx, eidx = self.get_position()
                        frame_data = np.frombuffer(decompressed_data[sidx:eidx], dtype=np.uint8)
                        s = w * 2
                        e = s + (w * (h + 1)) * 2
                        raw_data = frame_data[s : e]
                        bin_lvl, bin_tail = tc.get_rawdata_to_lvl_and_tail(w, h, raw_data)
                        lvl = np.frombuffer(bin_lvl, dtype=np.uint16)
                        tail = dp.TPKT_RawDataTail.from_bytes(bin_tail)
                        
                        self.temp_mode = de.TEMPERATURE_MODE(
                                            tc.get_current_temp_mode(tail))
                        if self.temp_mode == de.TEMPERATURE_MODE.TMODE_NONE:
                            self.gray, self.gray_scaled_ranges = tc.cvtGrayImage(lvl)
                        else:
                            self.temp_buff, self.min_temp, self.max_temp = tc.cvt_lvl_2_temp(lvl,
                                                                                             self.tempLut,
                                                                                             self.temp_mode)
                            self.gray, self.gray_scaled_ranges = tc.cvt_temp_2_gray(self.temp_buff,
                                                                                    self.min_temp,
                                                                                    self.max_temp)
                        self.stream_queue.put(self.gray)
        except ThermalReaderException as e:
            if self.error_callback:
                self.error_callback(str(e))
            self.stream_queue.put(0)
        except Exception as e:
            if self.error_callback:
                self.error_callback(str(e))
            self.stream_queue.put(0)
        finally:
            pass


    def get_header_data(self, data: bytes):
        try:
            file_hd_size = self.file_hd.get_size()
            data_hd_size = self.data_hd.get_size()
            bin_data_hd = data[-data_hd_size:]
            bin_file_hd = data[-(data_hd_size + file_hd_size):-data_hd_size]
            self.file_hd = dp.TPKT_RecFileHeader.from_bytes(bin_file_hd)
            self.data_hd = dp.TPKT_RecDataHeader.from_bytes(bin_data_hd)
            self.width, self.height = self.file_hd.width, self.file_hd.height
            self.frame_count = self.data_hd.total_frame
        except Exception:
            raise
    
    
    def init_camera_env(self):
        mode = self.data_hd.camera_env.temp_mode
        support_mask = self.data_hd.camera_env.support_temp_mode
        self.temp_mode = (
            de.TEMPERATURE_MODE(mode)
            if mode
            in (
                de.TEMPERATURE_MODE.TMODE_NORMAL.value,
                de.TEMPERATURE_MODE.TMODE_HIGH.value,
                de.TEMPERATURE_MODE.TMODE_MEDICAL.value,
                )
            else de.TEMPERATURE_MODE.TMODE_NONE
        )
        if self.temp_mode is not de.TEMPERATURE_MODE.TMODE_NONE:
            self.tempLut = tc.create_temp_lut(self.temp_mode,
                                              support_mask,
                                              self.data_hd.camera_env)
        