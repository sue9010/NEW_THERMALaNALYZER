import ctypes
from ctypes import (
    POINTER,
    Structure,
    c_char_p,
    c_int,
    c_short,
    c_void_p,
    c_uint,
    c_ushort,
    c_ubyte,
    c_long,
    c_float,
    c_uint32,
)
import time
import threading
import queue

# Constants from SDK
AF_INET = 2
SOCK_STREAM = 1
IRF_NO_ERROR = 1

# Message Type Enums
_IRF_NONE = -1
_IRF_ACK = 0
_IRF_NAK = 1
_IRF_ALIVE = 2
_IRF_STREAM_ON = 3
_IRF_STREAM_OFF = 4
_IRF_STREAM_DATA = 5
_IRF_REQ_CAM_DATA = 7
_IRF_CAM_DATA = 8
_IRF_SET_CAM_DATA = 10
_IRF_SET_USER_PALETTE = 11
_IRF_REQ_SYS_INFO = 12
_IRF_SYS_INFO = 13


# Structures from SDK
class IRF_SAVEDATA_T(Structure):
    _pack_ = 1
    _fields_ = [
        ("crc", c_uint32),
        ("ver", c_ubyte),
        ("sensor", c_ubyte),
        ("tv", c_ubyte),
        ("temp_mode", c_ubyte),
        # The rest of the struct is complex. For now, we just pad it out.
        # Total size is 512 bytes.
        ("padding", c_ubyte * (512 - 8))
    ]

class IRF_IR_CAM_DATA_T(Structure):
    _pack_ = 1
    _fields_ = [
        ("ir_image", POINTER(c_ushort)),
        ("image_buffer_size", c_uint32),
        ("lpNextData", POINTER(c_ubyte)),
        ("dwSize", c_uint32),
        ("dwPosition", c_uint32),
        ("msg_type", c_int),
        ("save_data", IRF_SAVEDATA_T),
        ("fw_ver", c_uint32),
        ("core_type", c_uint32),
        ("h_res", c_ushort),
        ("v_res", c_ushort),
        ("PMSGTYPE", c_ushort),
        ("RCODE", c_ushort),
        ("reserved", c_uint32 * 10),
    ]

class ThermalCam:
    def __init__(self, dll_path="SDK/SDK/bin64/ThermalCamSDK_x64.dll"):
        loader = getattr(ctypes, "WinDLL", ctypes.CDLL)
        self.lib = loader(dll_path)
        self.handle = c_void_p()
        self.timer_id = c_void_p()
        
        self._define_sdk_functions()

        self.message_queue = queue.Queue()
        self.stop_event = threading.Event()
        self.listen_thread = None

    def _define_sdk_functions(self):
        self.lib.OpenConnect.argtypes = [POINTER(c_void_p), POINTER(c_void_p), c_char_p, c_char_p, c_int, c_int]
        self.lib.OpenConnect.restype = c_short
        self.lib.CloseConnect.argtypes = [POINTER(c_void_p), c_void_p]
        self.lib.CloseConnect.restype = c_short
        self.lib.SendCameraMessage.argtypes = [c_void_p, POINTER(c_void_p), c_int, c_ushort, c_ushort]
        self.lib.SendCameraMessage.restype = c_short
        self.lib.GetIRImages.argtypes = [c_void_p, POINTER(c_void_p), POINTER(IRF_IR_CAM_DATA_T)]
        self.lib.GetIRImages.restype = c_short

    def _listen_thread_func(self):
        """This function runs in a background thread and continuously polls for messages."""
        image_x_size = 1024 
        image_y_size = 768
        ir_image_buffer = (c_ushort * (image_x_size * image_y_size))()
        next_data_buffer = (c_ubyte * 8192)()

        while not self.stop_event.is_set():
            cam_data = IRF_IR_CAM_DATA_T()
            cam_data.ir_image = ctypes.cast(ir_image_buffer, POINTER(c_ushort))
            cam_data.lpNextData = ctypes.cast(next_data_buffer, POINTER(c_ubyte))
            cam_data.image_buffer_size = image_x_size * image_y_size

            result = self.lib.GetIRImages(self.handle, ctypes.byref(self.timer_id), ctypes.byref(cam_data))
            
            if result != IRF_NO_ERROR:
                time.sleep(0.5)
                continue

            if cam_data.msg_type != _IRF_NONE:
                new_cam_data = IRF_IR_CAM_DATA_T()
                ctypes.pointer(new_cam_data)[0] = cam_data
                self.message_queue.put(new_cam_data)
            
            time.sleep(0.05)

    def connect(self, ip="192.168.0.101", port=15001):
        result = self.lib.OpenConnect(ctypes.byref(self.handle), ctypes.byref(self.timer_id), ip.encode("ascii"), str(port).encode("ascii"), AF_INET, SOCK_STREAM)
        if result != IRF_NO_ERROR:
            raise RuntimeError(f"OpenConnect failed with code {result}")
        
        self.stop_event.clear()
        self.listen_thread = threading.Thread(target=self._listen_thread_func)
        self.listen_thread.daemon = True
        self.listen_thread.start()
        print(f"Connected to camera and started listener thread. {ip}:{port}")

    def disconnect(self):
        if self.listen_thread and self.listen_thread.is_alive():
            self.stop_event.set()
            self.listen_thread.join(timeout=2)
            print("Listener thread stopped.")

        if self.handle:
            self.lib.CloseConnect(ctypes.byref(self.handle), self.timer_id)
            print("Disconnected")

    def _wait_for_message(self, target_msg_type, timeout=10):
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                cam_data = self.message_queue.get(timeout=1)
                if cam_data.msg_type == target_msg_type:
                    return cam_data
                else:
                    print(f"Ignoring message type: {cam_data.msg_type}")
            except queue.Empty:
                continue
        return None

    def get_firmware_version(self):
        with self.message_queue.mutex:
            self.message_queue.queue.clear()
        result = self.lib.SendCameraMessage(self.handle, ctypes.byref(self.timer_id), _IRF_REQ_CAM_DATA, 0, 0)
        if result != IRF_NO_ERROR:
            raise RuntimeError(f"SendCameraMessage(_IRF_REQ_CAM_DATA) failed with code {result}")
        cam_data = self._wait_for_message(_IRF_CAM_DATA)
        if not cam_data:
            raise RuntimeError("Did not receive CAM_DATA in time.")
        result = self.lib.SendCameraMessage(self.handle, ctypes.byref(self.timer_id), _IRF_REQ_SYS_INFO, 0, 0)
        if result != IRF_NO_ERROR:
            raise RuntimeError(f"SendCameraMessage(_IRF_REQ_SYS_INFO) failed with code {result}")
        sys_info_data = self._wait_for_message(_IRF_SYS_INFO)
        if not sys_info_data:
            raise RuntimeError("Failed to get firmware version in time.")
        major = (sys_info_data.fw_ver >> 24) & 0xFF
        minor = (sys_info_data.fw_ver >> 16) & 0xFF
        patch = sys_info_data.fw_ver & 0xFFFF
        return f"{major}.{minor}.{patch}"

    def get_camera_info(self):
        with self.message_queue.mutex:
            self.message_queue.queue.clear()
        result = self.lib.SendCameraMessage(self.handle, ctypes.byref(self.timer_id), _IRF_REQ_CAM_DATA, 0, 0)
        if result != IRF_NO_ERROR:
            raise RuntimeError(f"SendCameraMessage(_IRF_REQ_CAM_DATA) failed with code {result}")
        cam_data = self._wait_for_message(_IRF_CAM_DATA)
        if not cam_data:
            raise RuntimeError("Did not receive CAM_DATA in time.")
        save_data = cam_data.save_data
        sensor_map = {0x00: "CX320", 0x01: "CX640", 0x20: "CG QVGA", 0x21: "CG VGA"}
        temp_mode_map = {0: "Normal", 1: "High", 2: "Medical"}
        info = {
            "Setup Data Version": save_data.ver,
            "Sensor Type": sensor_map.get(save_data.sensor, f"Unknown ({save_data.sensor})"),
            "Temp Mode": temp_mode_map.get(save_data.temp_mode, f"Unknown ({save_data.temp_mode})")
        }
        return info

if __name__ == "__main__":
    cam = ThermalCam()
    try:
        cam.connect()
        fw_version = cam.get_firmware_version()
        print(f"Firmware Version: {fw_version}")
        
        cam_info = cam.get_camera_info()
        print("\nCamera Info:")
        for key, value in cam_info.items():
            print(f"  {key}: {value}")

    except RuntimeError as e:
        print(f"An error occurred: {e}")
    finally:
        cam.disconnect()