import socket
import sys
import os
import traceback
import asyncio
import numpy as np
import queue
import math
import time
import threading
import logging
from pathlib import Path
from . import CircularBuffer as cb
from . import define_enums as de
from . import define_constants as dc
from . import define_protocol as dp
from . import define_colormap as COLOR_MAP
from . import adaptiveAGC
from functools import reduce


logging.basicConfig(level=logging.INFO)


class ThermalReturn:
    def __init__(self, result: bool):
        self.response = result
        self.result_data = {}
        self.reason = ""


class ThermalUpdateRequest:
    def __init__(self):
        self.mode = de.MESSAGE_TYPE._IRF_NONE
        self.spi_num = 0
        self.spi_addr = 0
        self.file_path = ""



class ThermalCameraException(Exception):
    def __init__(self, message: str, code: de.ERROR_CODE):
        super().__init__(message)
        self.code = code.value
        logging.getLogger('myLogger').error(f'{message}')


class ThermalConfig:
    def __init__(self):
        self.reset()
    
    def reset(self):
        self.width = 0
        self.height = 0
        self.temp_mode = de.TEMPERATURE_MODE.TMODE_NONE
        self.agc_tpye = de.THERMAL_AGC_TYPE.AGC_AUTO
        self.agc_process_type = de.THERMAL_AGC_PROCESS.AGCPRO_ADAPTIVE_HISTOGRAM
        self.agc_min = 0
        self.agc_max = 0
        self.pal_type = de.THERMAL_PALETTE_TYPE.PAL_GRAY
        self.invert = False



class ThermalCamera:
    def __init__(self):
        self.initColormap()
        self.is_connected = False
        self.request_queue = queue.Queue()
        self.event = threading.Event()
        self.running = True
        self.host = None
        self.port = None
        self.cam_env_data = dp.TPKT_CameraEnv()
        self.cam_res_data = dp.TPKT_GetResolution()
        self.cam_sys_data = dp.TPKT_CameraSystemInfo()
        self.cam_tail_data = dp.TPKT_RawDataTail()
        self.cam_update_data = dp.TPKT_UpdateResult()
        self.lvl = None
        self.tempLut = {}
        self.temp_buff = None
        self.gray = None
        self.gray_scaled_ranges = None
        self.lvl_ringbuff = None
        self.ringbuff = None
        self.fps = None
        self.frm_count = 0
        self.stream_start_time = time.time()
        self.logger = logging.getLogger("myLogger")
        self.logger.setLevel(logging.DEBUG)
        self.worker_thread = None
        self.receive_task = None
        self.send_task = None
        self.asyncio_loop = None
        self.tmpBuff = None     # 임시 버퍼
        self.result_queue = queue.Queue()
        self.for_update = False
        self.update_event = threading.Event()
        self.agcProcess = adaptiveAGC.AdaptiveAGC()
        self.cfg = ThermalConfig()


    def initColormap(self):
        self.palette_map = {}
        for i, name in enumerate(COLOR_MAP._PALETTE_NAMES):
            base = i * COLOR_MAP._PALETTE_SIZE
            
            normal_rgb = np.array(COLOR_MAP._PALETTE_COLORMAP[base:base + 256 * 3], dtype=np.uint8)
            invert_rgb = np.array(COLOR_MAP._PALETTE_COLORMAP[base + 256 * 3 : base + 256 * 3 * 2], dtype=np.uint8)
            
            normal_bgr = normal_rgb.reshape((256, 3))[:, ::-1]
            invert_bgr = invert_rgb.reshape((256, 3))[:, ::-1]
            
            self.palette_map[name] = {
                "normal": normal_bgr,
                "invert": invert_bgr,
            }


    def thermalClear(self):
        self.lvl = None
        self.tmpBuff = None     # 임시 버퍼
        self.tempLut = {}
        self.temp_buff = None
        self.gray = None
        self.gray_scaled_ranges = None
        self.cam_env_data = dp.TPKT_CameraEnv()
        self.cam_res_data = dp.TPKT_GetResolution()
        self.cam_sys_data = dp.TPKT_CameraSystemInfo()
        self.cam_tail_data = dp.TPKT_RawDataTail()
        self.cam_update_data = dp.TPKT_UpdateResult()
        self.is_connected = False
        self.result_queue = queue.Queue()
        self.cfg.reset()
        self.agcProcess.clear()


    def isConnected(self):
        return self.is_connected


    def isValidPacketID(self, header: dp.TPKT_Header) -> bool:
        return header.ID == dc.THERMAL_PACKET_ID


    def isValidCamerVersion(self, ver) -> bool:
        return dc.THERMAL_CAM_VERSION <= ver


    def isValidSensor(self, sensor) -> bool:
        return sensor in {dc.THERMAL_QVGA_ID,
                          dc.THERMAL_VGA_ID,
                          dc.THERMAL_XGA_ID}


    def startCamera(self, host: str, port: int, for_update: bool = False):
        self.thermalClear()
        self.for_update = for_update
        self.host = host
        self.port = port
        if self.worker_thread is not None:
            self.worker_thread.join()
            self.worker_thread = None
        self.running = True
        self.worker_thread = threading.Thread(target=asyncio.run, args=(self.main(),))
        self.worker_thread.start()


    async def main(self):
        try:
            self.reader, self.writer = await self.connect_camera()
        except ThermalCameraException as e:
            ret = ThermalReturn(False)
            ret.reason = str(e)
            await asyncio.to_thread(self.result_queue.put, ret)
            return
        
        if self.reader and self.writer:
            if self.for_update is False:
                # Request Camera Environment
                cam_res = self.create_packet(de.MESSAGE_TYPE._IRF_REQ_CORE_INFO)
                cam_sys = self.create_packet(de.MESSAGE_TYPE._IRF_REQ_SYS_INFO)
                cam_env = self.create_packet(de.MESSAGE_TYPE._IRF_REQ_CAM_ENV)

                await asyncio.to_thread(self.request_queue.put, cam_res)
                await asyncio.to_thread(self.request_queue.put, cam_sys)
                await asyncio.to_thread(self.request_queue.put, cam_env)
                self.event.set()
            else:
                ret = ThermalReturn(True)
                await asyncio.to_thread(self.result_queue.put, ret)

            self.asyncio_loop = asyncio.get_event_loop()
            self.receive_task = asyncio.create_task(self.receive_data())
            self.send_task = asyncio.create_task(self.send_data())

            try:
                await asyncio.gather(self.receive_task, self.send_task)

            except ThermalCameraException:
                self.logger.info('Interrupt transmission and reception.')
                self.running = False
                self.event.set()
                self.receive_task.cancel()
                self.send_task.cancel()
            except asyncio.CancelledError:
                self.logger.info('The operation is interrupted by a user request.')
                self.running = False
                self.event.set()
                self.receive_task.cancel()
                self.send_task.cancel()
            except Exception as e:
                self.logger.error(f'Error occurred. msg: {e}')
                _, _, exc_tb = sys.exc_info()
                tb_lines = traceback.extract_tb(exc_tb)
                prj_dir = os.path.abspath(os.path.dirname(__file__))
                filtered_tb_lines = [line for line in tb_lines if prj_dir in line.filename]
                
                tb = filtered_tb_lines[-1]
                relative_dir = Path(tb.filename).relative_to(Path.cwd())
                self.logger.error(f'{relative_dir}, line {tb.lineno}, in {tb.name}')
                #self.logger.error(f'{tb.line} <----')
                
                self.running = False
                self.event.set()
                self.receive_task.cancel()
                self.send_task.cancel()
            finally:
                ret = ThermalReturn(False)
                try:
                    self.logger.info('Request the HOST to disconnect...')
                    ret.reason = "Request the HOST to disconnect..."
                    self.writer.close()
                    await asyncio.wait_for(self.writer.wait_closed(), timeout=2)
                except asyncio.TimeoutError:
                    self.logger.warning("No response from HOST. Timeout occurred.")
                    ret.reason = "No response from HOST. Timeout occurred."
                except Exception as e:
                    self.logger.error(f'An unknown error occurred. msg: {e}')
                    ret.reason = f'An unknown error occurred. msg: {e}'
                finally:
                    self.logger.info("Disconnected complete.")
                    self.receive_task = None
                    self.send_task = None
                    self.is_connected = False
                    await asyncio.to_thread(self.result_queue.put, ret)


    async def connect_camera(self):
        try:
            r, w = await asyncio.wait_for(asyncio.open_connection(self.host, self.port), timeout=5)
            sock = w.get_extra_info("socket")
            if sock:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 8192)
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 8192)
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
                sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 5)
                sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3)
                sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 3)
                self.logger.info("Camera connection successful.")
            return r, w
        except asyncio.TimeoutError:
            raise ThermalCameraException("Camera connection failed. msg: Timeout occurred.",
                                         de.ERROR_CODE.ERR_CAMERA_CONNECTION_TIMEOUT)
        except Exception as e:
            raise ThermalCameraException(f"Camera connection failed. msg: {e}",
                                         de.ERROR_CODE.ERR_CAMERA_CONNECTION_FAILED)


    async def receive_data(self):
        try:
            while self.running:
                bin_header = await self.reader.readexactly(dc.THERMAL_PACKET_HEADER_SIZE)
                header = dp.TPKT_Header.from_bytes(bin_header)
                if not self.isValidPacketID(header):
                    raise ThermalCameraException("The ID of the Header is not valid.",
                                                 de.ERROR_CODE.ERR_PACKET_ID)
                bin_body = await self.reader.readexactly(header.Length - dc.THERMAL_PACKET_HEADER_SIZE)
                self.recv_parsingdata(header.Type, bin_body)
        except Exception:
            raise


    async def send_data(self):
        try:
            while self.running:
                await asyncio.to_thread(self.event.wait)
                while not self.request_queue.empty():
                    msg = await asyncio.to_thread(self.request_queue.get)
                    self.writer.write(msg)
                    await self.writer.drain()
                self.event.clear()
        except Exception as e:
            print(f'error : {e}')
            raise


    def create_binary_packet(self,
                             msg: de.MESSAGE_TYPE,
                             binary: bytes) -> bytes:
        if msg in (de.MESSAGE_TYPE._IRF_REQ_FW_UPDATE,
                   de.MESSAGE_TYPE._IRF_REQ_SPIFLASH_UPDATE):
            header = dp.TPKT_Header(Type=msg.value)
            header.Length += len(binary)
            packet = header.to_bytes() + binary
            return packet
        return None


    def create_packet(self,
                      msg: de.MESSAGE_TYPE,
                      param1: int = 0,
                      param2: int = 0,
                      param3: int = 0,
                      param4: int = 0,
                      param5: int = 0) -> bytes:
        header = dp.TPKT_Header(Type=msg.value)
        if msg == de.MESSAGE_TYPE._IRF_SET_CAM_OPT:
            body = dp.TPKT_SetParameter(Command=param1,
                                        Value1=param2,
                                        Value2=param3,
                                        Value3=param4,
                                        Value4=param5)
            body_packed = body.to_bytes()
            header.Length += len(body_packed)
            packet = header.to_bytes() + body_packed
        elif msg == de.MESSAGE_TYPE._IRF_REQ_SYS_INFO:
            if param2 != 0:
                header.ID = param3
            packet = header.to_bytes()
        else:
            packet = header.to_bytes()
        return packet


    def recv_parsingdata(self, type: int, data: bytes) -> bool:
        match type:
            case de.MESSAGE_TYPE._IRF_RES_CAM_ENV.value:
                self.verifyCameraEnvData(data)
            case de.MESSAGE_TYPE._IRF_RES_CORE_INFO.value:
                self.verifyCameraResData(data)
            case de.MESSAGE_TYPE._IRF_RES_SYS_INFO.value:
                self.verifyCameraSysData(data)
            case de.MESSAGE_TYPE._IRF_STREAM_DATA.value:
                self.verifyCameraStreamData(data)
            case de.MESSAGE_TYPE._IRF_UPDATE_PROGRESS.value:
                self.verifyCameraUpdateStatus(data)
            case _:
                self.logger.warning(
                    f'An undefined header type is received and ignored. '\
                    f'HeaderType[0x{type:02X}], size[{len(data)}]')
                pass


    def verifyCameraUpdateStatus(self, data: bytes):
        if len(data) == dp.TPKT_BinaryUpdateResult.get_size():
            result = dp.TPKT_BinaryUpdateResult.from_bytes(data)
            match result.status:
                # update finish
                case de.THERMAL_UPDATE_STATUS.TU_COMPLETE.value:
                    #print(f'[ FINISH ] status[{result.status}],  type[{result.type}],  op_mode[{result.op_mode}],  op_addr[{result.op_addr}]')
                    #print('update finish!')
                    ret = ThermalReturn(True)
                    ret.result_data = {
                        "status": result.status,
                        "update_type": result.type,
                        "op_mode": result.op_mode,
                        "update_rate": result.op_addr,
                    }
                    self.result_queue.put(ret)
                    self.update_event.set()
        
                # operation?
                case de.THERMAL_UPDATE_STATUS.TU_OPERATION.value:
                    pass
        
                # receive
                case de.THERMAL_UPDATE_STATUS.TU_RECEIVE.value:
                    #print(f'[ RECEIVE ] status[{result.status}],  type[{result.type}],  op_mode[{result.op_mode}],  op_addr[{result.op_addr}]')
                    ret = ThermalReturn(True)
                    ret.result_data = {
                        "status": result.status,
                        "update_type": result.type,
                        "op_mode": result.op_mode,
                        "update_rate": result.op_addr,
                    }
                    self.result_queue.put(ret)
            
                # update
                case de.THERMAL_UPDATE_STATUS.TU_UPDATE.value:
                    #print(f'[ UPDATE ] status[{result.status}],  type[{result.type}],  op_mode[{result.op_mode}],  op_addr[{result.op_addr}]')
                    ret = ThermalReturn(True)
                    ret.result_data = {
                        "status": result.status,
                        "update_type": result.type,
                        "op_mode": result.op_mode,
                        "update_rate": result.op_addr,
                    }
                    self.result_queue.put(ret)
                    
                # error
                case de.THERMAL_UPDATE_STATUS.TU_ERROR.value:
                    print(f'[ ERROR ] status[{result.status}],  type[{result.type}],  op_mode[{result.op_mode}],  op_addr[{result.op_addr}]')
                    ret = ThermalReturn(False)
                    ret.reason = '카메라로부터 "업데이트 오류"를 받았습니다.'
                    self.result_queue.put(ret)
                    self.update_event.set()


    def verifyCameraEnvData(self, data: bytes):
        if len(data) == dp.TPKT_CameraEnv.get_size():
            self.cam_env_data = dp.TPKT_CameraEnv.from_bytes(data)
            if not self.isValidCamerVersion(self.cam_env_data.ver):
                raise ThermalCameraException(
                    f'Not currently supported version, '\
                    f'SupportedVer[0x{dc.THERMAL_CAM_VERSION:02X}], '\
                    f'ResponseVer[0x{self.cam_env_data.ver:02X}]',
                    de.ERROR_CODE.ERR_CAMERA_INVALID_VER)
            if not self.isValidSensor(self.cam_env_data.sensor):
                raise ThermalCameraException(
                    f'Not currently supported sensor, '\
                    f'ResponseSensor[0x{self.cam_env_data.sensor:02X}]',
                    de.ERROR_CODE.ERR_CAMERA_INVALID_SENSOR)
                
            mode = self.cam_env_data.temp_mode
            support_mask = self.cam_env_data.support_temp_mode

            self.cfg.temp_mode = (
                de.TEMPERATURE_MODE(mode)
                if mode
                in (
                    de.TEMPERATURE_MODE.TMODE_NORMAL.value,
                    de.TEMPERATURE_MODE.TMODE_HIGH.value,
                    de.TEMPERATURE_MODE.TMODE_MEDICAL.value,
                )
                else de.TEMPERATURE_MODE.TMODE_NONE
            )
            
            if support_mask == 0:
                self.cfg.temp_mode = de.TEMPERATURE_MODE.TMODE_NONE
            
            if self.cfg.temp_mode is not de.TEMPERATURE_MODE.TMODE_NONE:
                self.tempLut = ThermalCamera.create_temp_lut(self.cfg.temp_mode,
                                                             support_mask,
                                                             self.cam_env_data)
            
            if self.ringbuff is None \
                    or self.ringbuff.width != self.cfg.width \
                    or self.ringbuff.height != self.cfg.height:
                self.ringbuff = cb.RingBuffer(self.cfg.width,
                                              self.cfg.height,
                                              5)
            else:
                self.ringbuff.clear()
            
            
            # if self.lvl_ringbuff is None or self.lvl_ringbuff.size != (self.cfg.height + 2) * self.cfg.width:
            #     self.lvl_ringbuff = cb.CircularBuffer(5, (self.cfg.height + 2) * self.cfg.width, np.uint16)
            # else:
            #     self.lvl_ringbuff.clear()
            self.is_connected = True
            self.tmpBuff = np.zeros(self.cfg.width, dtype=np.uint16).tobytes()
            self.result_queue.put(ThermalReturn(True))
        else:
            raise ThermalCameraException('The packet length for the _IRF_REQ_CAM_ENV '\
                                         'request is incorrect.',
                                         de.ERROR_CODE.ERR_PACKET_SIZE)


    @classmethod
    def create_temp_lut(cls, temp_mode, support_temp_mask, cam_env: dp.TPKT_CameraEnv):
        index = 0
        temp_lut = {}
        for i in range(dc.THERMAL_TEMP_MODE_COUNT):
            if support_temp_mask & (0x0001 << i) == 0:
                continue
            temp_dict = {}
            temp_dict['minlvl'] = cam_env.meas_level[index][0]
            temp_dict['maxlvl'] = cam_env.meas_level[index][1]
            temp_dict['lvl_range'] = temp_dict['maxlvl'] - temp_dict['minlvl']
            temp_dict['mintemp'] = cam_env.meas_temp[index][0] / 10
            temp_dict['maxtemp'] = cam_env.meas_temp[index][1] / 10
            temp_dict['temp_range'] = (temp_dict['maxtemp'] - temp_dict['mintemp'])
            temp_dict['temp_delta'] = (temp_dict['temp_range'] / temp_dict['lvl_range'])
            temp_dict['mode_name'] = cam_env.meas_name[index].decode()
            lut = np.linspace(temp_dict["mintemp"], temp_dict["maxtemp"], temp_dict["lvl_range"] + 1,)
            temp_dict['temp_lut'] = np.full(np.iinfo(np.uint16).max + 1, temp_dict["mintemp"] - 0.1)
            temp_dict['temp_lut'][temp_dict["minlvl"] : temp_dict["maxlvl"] + 1] = lut
            temp_dict['temp_lut'][temp_dict["maxlvl"] + 1 :] = (temp_dict["maxtemp"] + 0.1)
            temp_lut[de.TEMPERATURE_MODE(i)] = temp_dict
            index += 1
        return temp_lut


    def verifyCameraResData(self, data: bytes):
        if len(data) == dp.TPKT_GetResolution.get_size():
            self.cam_res_data = dp.TPKT_GetResolution.from_bytes(data)
            self.cfg.width = self.cam_res_data.h_res
            self.cfg.height = self.cam_res_data.v_res
        else:
            raise ThermalCameraException('The packet length for the _IRF_REQ_CORE_INFO '\
                                         'request is incorrect.',
                                         de.ERROR_CODE.ERR_PACKET_SIZE)


    def verifyCameraSysData(self, data: bytes):
        if len(data) == dp.TPKT_CameraSystemInfo.get_size():
            self.cam_sys_data = dp.TPKT_CameraSystemInfo.from_bytes(data)
        else:
            raise ThermalCameraException('The packet length for the _IRF_REQ_SYS_INFO '\
                                         'request is incorrect.',
                                         de.ERROR_CODE.ERR_PACKET_SIZE)

    @classmethod
    def get_rawdata_to_lvl_and_tail(cls, w, h, raw_data, frm_buffer: cb.FrameBuffer):
        type_size = np.uint16().itemsize
        taildata_size = dp.TPKT_RawDataTail.get_size()
        lvldata_size = w * h * type_size
        frm_buffer.level[:] = np.frombuffer(raw_data[:lvldata_size], dtype=np.uint16).reshape(h, w)
        frm_buffer.tail.__dict__.update(
            dp.TPKT_RawDataTail.from_bytes(
                np.frombuffer(raw_data[lvldata_size : lvldata_size + taildata_size], dtype=np.uint8)).__dict__)


    @classmethod
    def get_current_temp_mode(cls, tail: dp.TPKT_RawDataTail):
        return (int(math.log2(tail.temp_mode))
                if tail.temp_mode != 0
                else -1)


    @classmethod
    def cvt_lvl_2_temp(cls, frm_buff: cb.FrameBuffer, templut, temp_mode):
        frm_buff.temp_buff[:] = templut[temp_mode]['temp_lut'][frm_buff.level]
        return float(np.min(frm_buff.temp_buff)), float(np.max(frm_buff.temp_buff))


    @classmethod
    def cvt_temp_2_gray(cls, temp_buff, min, max):
        gray = np.interp(temp_buff, (min, max), (0, 255)).astype(np.uint8)
        gray_scaled_ranges = np.linspace(min, max, 256)
        return gray, gray_scaled_ranges


    def cvtGrayImage(self, src, gray, gray_scale):
        agc_min = 0
        agc_max = 0
        if self.cfg.agc_tpye == de.THERMAL_AGC_TYPE.AGC_AUTO:
            if self.cfg.agc_process_type == de.THERMAL_AGC_PROCESS.AGCPRO_ADAPTIVE_HISTOGRAM:
                agc_min, agc_max = self.agcProcess.adaptiveHistogramAGC(src, gray)
            elif self.cfg.agc_process_type == de.THERMAL_AGC_PROCESS.AGCPRO_PERCENTILE:
                agc_min, agc_max = self.agcProcess.percentileStretching(src, gray)
            self.cfg.agc_min = agc_min
            self.cfg.agc_max = agc_max
        elif self.cfg.agc_tpye == de.THERMAL_AGC_TYPE.AGC_MANUAL:
            scaled = (src.astype(np.float32) - self.cfg.agc_min) / (self.cfg.agc_max - self.cfg.agc_min) * 255.0
            gray[:] = np.clip(scaled, 0, 255).astype(np.uint8)
        gray_scale[:] = np.linspace(self.cfg.agc_min, self.cfg.agc_max, 256)
        return agc_min, agc_max


    def verifyCameraStreamData(self, data: bytes):
        type_size = np.uint16().itemsize
        total_size = self.cfg.width * (self.cfg.height + 1) * type_size
        total_size2 = self.cfg.width * (self.cfg.height + 2) * type_size
        recv_size = len(data)

        if recv_size in (total_size, total_size2):
            if recv_size == total_size2:
                src = data
            else:
                src = data + self.tmpBuff
            
            # Ringbuff에서 FrameBuffer를 가져온다.
            frm_idx, frm_buff = self.ringbuff.next()
            
            # Raw => Level, TailData 
            # Level: frm_buff.level (np.uint16)
            # tailbuff: frm_buff.tail (np.uint8)
            ThermalCamera.get_rawdata_to_lvl_and_tail(self.cfg.width,
                                                      self.cfg.height,
                                                      src,
                                                      frm_buff)
            
            # Tail에서 현재 온도 모드를 얻음
            # 만약 de.TEMPERATURE_MODE에 존재하지 않는다면
            # TMODE_NORMAL로 고정
            mode = ThermalCamera.get_current_temp_mode(frm_buff.tail)
            self.cfg.temp_mode = de.TEMPERATURE_MODE(mode) \
                                    if mode in de.TEMPERATURE_MODE \
                                    else de.TEMPERATURE_MODE.TMODE_NORMAL
            
            
            # 온도모드 카메라
            if self.cfg.temp_mode is not de.TEMPERATURE_MODE.TMODE_NONE:
                # 온도 변환
                self.min_temp, self.max_temp = ThermalCamera.cvt_lvl_2_temp(frm_buff,
                                                                            self.tempLut,
                                                                            self.cfg.temp_mode)
                
                # gray 이미지 변환
                frm_buff.agc_min, frm_buff.agc_max = self.cvtGrayImage(frm_buff.temp_buff,
                                                                       frm_buff.gray,
                                                                       frm_buff.gray_scale)
            # 보안용 카메라
            else:
                # gray 이미지 변환
                frm_buff.agc_min, frm_buff.agc_max = self.cvtGrayImage(frm_buff.level,
                                                                       frm_buff.gray,
                                                                       frm_buff.gray_scale)

            # palette 적용
            pal = self.palette_map[self.cfg.pal_type.pretty]["normal" if not self.cfg.invert else "invert"]
            pal_arr = np.array(pal, dtype=np.uint8).reshape((256, 3))
            frm_buff.image[:] = pal_arr[frm_buff.gray]

            # framerate 계산
            self.frm_count += 1
            current_time = time.time()
            elapsed_time = current_time - self.stream_start_time
            if 1.0 <= elapsed_time:
                self.fps = self.frm_count / elapsed_time
                self.frm_count = 0
                self.stream_start_time = current_time
            
            # 스트림 관련 데이터 큐에 넣기
            if self.cfg.temp_mode is not de.TEMPERATURE_MODE.TMODE_NONE:
                ret = ThermalReturn(True)
                ret.result_data = {
                    "fps": self.fps,
                    "frm_idx": frm_idx,
                    "frm_buff": frm_buff,
                }
                self.result_queue.put(ret)
            else:
                ret = ThermalReturn(True)
                ret.result_data = {
                    "fps": self.fps,
                    "frm_idx": frm_idx,
                    "frm_buff": frm_buff,
                }
                self.result_queue.put(ret)
        else:
            raise ThermalCameraException('The packet length for the _IRF_STREAM_DATA '\
                                         'request is incorrect.',
                                         de.ERROR_CODE.ERR_PACKET_SIZE)


    def setStreamFPS(self, type: de.THERMAL_FPS_TYPE):
        match type:
            case de.THERMAL_FPS_TYPE.FPS_FULL:
                skip_frame = 0
            case de.THERMAL_FPS_TYPE.FPS_HALF:
                skip_frame = 1
            case de.THERMAL_FPS_TYPE.FPS_1SEC:
                skip_frame = 29
        p = self.create_packet(de.MESSAGE_TYPE._IRF_SET_CAM_OPT, 0xA01, skip_frame)
        self.request_queue.put(p)
        self.event.set()


    def startStream(self):
        p = self.create_packet(de.MESSAGE_TYPE._IRF_STREAM_ON)
        self.request_queue.put(p)
        self.event.set()


    def stopStream(self):
        p = self.create_packet(de.MESSAGE_TYPE._IRF_STREAM_OFF)
        self.request_queue.put(p)
        self.event.set()
        self.frm_count = 0
        self.fps = 0


    def stopCamera(self):
        self.running = False
        if self.receive_task:
            self.asyncio_loop.call_soon_threadsafe(self.receive_task.cancel)
        if self.send_task:
            self.asyncio_loop.call_soon_threadsafe(self.send_task.cancel)
        self.worker_thread.join()
        self.worker_thread = None
        self.receive_task = None
        self.send_task = None
        self.asyncio_loop = None
        self.logger.info("ThermalCamera's resource cleanup completed. It has been successfully terminated.")


    def wait_for_response(self):
        return self.result_queue.get()


    def get_recv_data(self):
        return self.result_queue.get()


    def updateBinary(self, update_info: ThermalUpdateRequest):
        self.update_event.clear()
        ret = ThermalReturn(True)
        try:
            with open(update_info.file_path, "rb") as f:
                f.seek(0, 2)
                data_size = f.tell()
                f.seek(0)
                MAX_FILE_SIZE = 8 * 1024 * 1024
                if MAX_FILE_SIZE < data_size:
                    raise Exception("Error. file size exceeded.")
                
                chunk_size = 0
                body = None
                if update_info.mode == de.MESSAGE_TYPE._IRF_REQ_FW_UPDATE:
                    chunk_size = 1024
                    body = dp.TPKT_FirmwareUpdate(fw_ver=0,
                                                  fw_size=data_size,
                                                  total_block=(data_size + (chunk_size - 1)) // chunk_size,
                                                  block_no=1)
                elif update_info.mode == de.MESSAGE_TYPE._IRF_REQ_SPIFLASH_UPDATE:
                    chunk_size = 2048
                    body = dp.TPKT_SPIFlashUpdate(spi_num=update_info.spi_num,
                                                  write_address=update_info.spi_addr,
                                                  size=0,
                                                  crc=0,
                                                  total_block=(data_size + (chunk_size - 1)) // chunk_size,
                                                  block_no=1,
                                                  total_size=((data_size + (chunk_size - 1)) // chunk_size) * chunk_size)
                
                while chunk := f.read(chunk_size):
                    body.file_block = bytearray(chunk_size)
                    if update_info.mode == de.MESSAGE_TYPE._IRF_REQ_FW_UPDATE:
                        if len(chunk) < chunk_size:
                            chunk += b'\x00' * (chunk_size - len(chunk))
                        body.file_block[:] = b'\x00' * chunk_size
                        body.file_block = chunk
                    elif update_info.mode == de.MESSAGE_TYPE._IRF_REQ_SPIFLASH_UPDATE:
                        if len(chunk) < chunk_size:
                            chunk += b'\xFF' * (chunk_size - len(chunk))
                        body.file_block[:] = b'\xFF' * chunk_size
                        body.file_block = chunk
                        def crc16_byte(crc, byte):
                            index = ((crc >> 8) ^ byte) & 0x00FF
                            return ((crc << 8) ^ dc.CRC16TABLE[index]) & 0xFFFF
                        body.crc = reduce(crc16_byte, chunk, body.crc)
                    binary = body.to_bytes()
                    packet = self.create_binary_packet(update_info.mode, binary)
                    if packet:
                        self.request_queue.put(packet)
                        self.event.set()
                        body.block_no += 1
        except Exception as e:
            print(f'update binary failed. {e}')
            ret.response = False
            ret.reason = str(e)
        finally:
            if ret.response is False:
                self.result_queue.put(ret)
            else:
                self.update_event.wait()
    
    
    def onetimeNUC(self):
        p = self.create_packet(de.MESSAGE_TYPE._IRF_SET_CAM_OPT,
                               dc.TCMD_NUC_ONETIME)
        self.request_queue.put(p)
        self.event.set()


    def setPaletteType(self, palette: de.THERMAL_PALETTE_TYPE | int):
        if isinstance(palette, de.THERMAL_PALETTE_TYPE):
            self.cfg.pal_type = palette
        elif isinstance(palette, int):
            self.cfg.pal_type = de.THERMAL_PALETTE_TYPE(palette)
    
    
    def setAGCType(self, auto, min = 0, max = 0):
        if auto:
            self.cfg.agc_tpye = de.THERMAL_AGC_TYPE.AGC_AUTO
        else:
            self.cfg.agc_tpye = de.THERMAL_AGC_TYPE.AGC_MANUAL
            self.cfg.agc_min = min
            self.cfg.agc_max = max
    
    
    def setAGCProcessType(self, type_: de.THERMAL_AGC_PROCESS | int):
        if isinstance(type_, de.THERMAL_AGC_PROCESS):
            self.cfg.agc_process_type = type_
        elif isinstance(type_, int):
            self.cfg.agc_process_type = de.THERMAL_AGC_PROCESS(type_)
        



