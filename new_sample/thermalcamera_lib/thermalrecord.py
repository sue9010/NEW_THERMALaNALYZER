import threading
import copy
import numpy as np
import os
import logging
import queue
import zstandard as zstd
from enum import Enum
from datetime import datetime
from pathlib import Path
from . import CircularBuffer as cb
from . import define_enums as de
from . import define_protocol as dp



class SAVE_UNIT_TYPE(Enum):
    UNIT_CAPACITY                           = 0
    UNIT_TIME_SEC                           = 1


class ThermalRecord:
    def __init__(self, path: str | Path, host: str):
        if isinstance(path, str):
            self.base_path = Path(path)
        elif isinstance(path, Path):
            self.base_path = path
        else:
            self.base_path = Path.cwd()
        self.host = host
        self.file_hd = dp.TPKT_RecFileHeader()
        self.data_hd = dp.TPKT_RecDataHeader()
        self.full_path = Path()
        self.file_name = None
        self.path = None
        self.frame_count = 0
        self.start_time = None
        self.total_size = 0
        self.width = 0
        self.height = 0
        self.ringbuff = None
        self.running = True
        self.worker_queue = queue.Queue()
        self.thread = None
        self.logger = logging.getLogger("myLogger")
        self.save_unit_type = SAVE_UNIT_TYPE.UNIT_TIME_SEC
        self.save_param = 10
        self.is_compressed = False


    def clear(self):
        self.worker_queue = queue.Queue()
        self.width = 0
        self.height = 0
        self.total_size = 0
        self.frame_count = 0
        if self.ringbuff is not None:
            self.ringbuff.clear()


    def set_save_unit_type(self, unit_type: SAVE_UNIT_TYPE, param: int):
        self.save_unit_type = unit_type
        self.save_param = param


    def record_stop(self):
        self.running = False
        self.worker_queue.put(False)
        if self.thread is not None and self.thread.is_alive():
            self.thread.join()
        self.clear()
        self.logger.info("ThermalRecorder's resource cleanup completed. It has been successfully terminated.")


    def set_camera_env(self, file_hd: dp.TPKT_RecFileHeader, temp_mode: de.TEMPERATURE_MODE, env: dp.TPKT_CameraEnv):
        self.width = file_hd.width
        self.height = file_hd.height
        self.file_hd = copy.deepcopy(file_hd)
        self.data_hd.temp_mode = (255
                                  if temp_mode == de.TEMPERATURE_MODE.TMODE_NONE
                                  else temp_mode.value)
        self.data_hd.camera_env = copy.deepcopy(env)
        self.ringbuff = cb.CircularBuffer(5, self.width * (self.height + 2), dtype=np.uint16)
        self.check_path()
        self.running = True
        self.thread = threading.Thread(target=self.record_write_file)
        self.thread.start()


    def record_data(self, data):
        self.ringbuff.add_data(data)
        self.worker_queue.put(True)
        #self.logger.info('1')


    def check_path(self):
        today_dir = self.base_path.joinpath(datetime.now().strftime('%Y%m%d'))
        if not today_dir.exists():
            Path.mkdir(today_dir)
        self.path = today_dir
        now = datetime.now()
        self.file_name = f"{self.host.replace('.', '_')}_rec_{now.strftime('%Y%m%d_%H%M%S')}_{now.microsecond // 1000:03d}.crd"
        self.full_path = self.path.joinpath(self.file_name)


    def record_write_file(self):
        if self.is_compressed:
            self.zst_compressed()
        else:
            self.no_compressed()


    def no_compressed(self):
        front_buff = (np.zeros(self.width, dtype=np.uint16)).tobytes()
        
        while self.running:
            self.logger.info('** Start record file writing.')
            self.total_size = 0
            self.frame_count = 0
            self.start_time = datetime.now()
            try:
                with open(self.full_path, 'wb') as file:
                    while not self.is_file_close_required():
                        # 종료 및 중지 신호 감지
                        if not self.worker_queue.get():
                            break
                        
                        write_data = front_buff + self.ringbuff.get_data().tobytes()
                        file.write(write_data)
                        self.frame_count += 1
                        self.total_size = file.tell()
                    
                    # 파일기록 제한조건을 넘었다면
                    # 마지막에 file, data 헤더를 붙인다.add()
                    self.data_hd.total_frame = self.frame_count
                    self.data_hd.date_time = [
                            int(self.start_time.strftime('%Y')),
                            int(self.start_time.strftime('%m')),
                            int(self.start_time.strftime('%d')),
                            int(self.start_time.strftime('%H')),
                            int(self.start_time.strftime('%M')),
                            int(self.start_time.strftime('%S'))
                        ]
                    tail_header = self.file_hd.to_bytes() + self.data_hd.to_bytes()
                    file.write(tail_header)
                    self.total_size = file.tell()
            except Exception as e:
                self.logger.error({e})
            finally:
                # 파일 기록 완료.
                self.logger.info('** Created file information')
                self.logger.info(f'     FileName: {str(self.full_path.relative_to(Path.cwd()))}')
                self.logger.info(f'     FileSize: {self.total_size:,} Bytes')
                self.logger.info(f'     FrameCount: {self.frame_count}')
                self.logger.info(f'     Elasped: {int((datetime.now() - self.start_time).total_seconds())} sec')
                self.logger.info('** File write operation completed. with ztd Compression ')
                self.logger.info('********************************************************')
                self.check_path()


    def zst_compressed(self):
        front_buff = np.zeros(self.width, dtype=np.uint16)
        while self.running:
            zst_compressed_file = self.full_path.with_name(self.full_path.name + '.zst')
            self.logger.info('** Start zstd compression with record file writing.')
            self.total_size = 0
            self.frame_count = 0
            self.start_time = datetime.now()
            cctx = zstd.ZstdCompressor()
            
            try:
                with open(zst_compressed_file, 'wb') as fout:
                    with cctx.stream_writer(fout) as compressor:
                        while not self.is_file_close_required():
                            
                            # 종료 및 중지 신호 감지
                            if not self.worker_queue.get():
                                break
                            
                            write_data = front_buff.tobytes() + self.ringbuff.get_data().tobytes()
                            compressor.write(write_data)
                            self.frame_count += 1
                            self.total_size = fout.tell()
                            #self.logger.info(f'       2      [fnt:{self.frame_count}], [{int((datetime.now() - self.start_time).total_seconds())} sec]')
                    
                        # 파일기록 제한조건을 넘었다면
                        # 마지막에 file, data 헤더를 붙인다.
                        self.data_hd.total_frame = self.frame_count
                        self.data_hd.date_time = [
                            int(self.start_time.strftime('%Y')),
                            int(self.start_time.strftime('%m')),
                            int(self.start_time.strftime('%d')),
                            int(self.start_time.strftime('%H')),
                            int(self.start_time.strftime('%M')),
                            int(self.start_time.strftime('%S'))
                        ]
                        tail_header = self.file_hd.to_bytes() + self.data_hd.to_bytes()
                        compressor.write(tail_header)
            except Exception as e:
                self.logger.error({e})
            finally:
                # 파일 기록 완료.
                self.logger.info('** Created file information')
                self.logger.info(f'     FileName: {str(zst_compressed_file.relative_to(Path.cwd()))}')
                self.logger.info(f'     FileSize: {os.path.getsize(zst_compressed_file):,} Bytes')
                self.logger.info(f'     FrameCount: {self.frame_count}')
                self.logger.info(f'     Elasped: {int((datetime.now() - self.start_time).total_seconds())} sec')
                self.logger.info('** File write operation completed. with ztd Compression ')
                self.logger.info('********************************************************')
                self.check_path()


    def is_file_close_required(self) -> bool:
        match self.save_unit_type:
            case SAVE_UNIT_TYPE.UNIT_CAPACITY:
                parameter = 1024 * 1024 * self.save_param
                return parameter <= self.total_size
            case SAVE_UNIT_TYPE.UNIT_TIME_SEC:
                diff = datetime.now() - self.start_time
                return self.save_param <= diff.total_seconds()
        return False

