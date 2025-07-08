import ctypes
import socket
import threading
import time
import argparse
import configparser
import os

import numpy as np

# --- Constants from ThermalCamSDK.h and SampleCGDlg.h ---
INVALID_HANDLE_VALUE = ctypes.c_void_p(-1)
IRF_NO_ERROR = 1
AF_INET = socket.AF_INET
SOCK_STREAM = socket.SOCK_STREAM

IRBUFSIZE = 640 * 481  # VGA or QVGA
MAX_PALETTE = 13
PALETTE_SIZE = 256 * 3  # 256 levels * 3 bytes (RGB)

# IRF_MESSAGE_TYPE_T constants
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
_IRF_SPOT_STREAM_ON = 14
_IRF_SPOT_STREAM_OFF = 15
_IRF_SPOT_STREAM_DATA = 16
_IRF_REQ_TEMP_TABLE = 19

# SAVEDATA_VERSION and Sensor IDs
SAVEDATA_VERSION = 0x21
QVGA_ID = 0x20
VGA_ID = 0x21

# --- Default configuration values ---
DEFAULT_DLL_PATH = r"E:/Coding_practice/new_ThermalAnalyzer/SDK/SDK/bin64/ThermalCamSDK_x64_D.dll"
DEFAULT_IP_ADDRESS = "192.168.0.101"
DEFAULT_PORT = 15001

# --- Basic ctypes types ---
BYTE = ctypes.c_ubyte
WORD = ctypes.c_ushort
DWORD = ctypes.c_ulong
UINT = ctypes.c_uint
SHORT = ctypes.c_short
INT = ctypes.c_int
LONG = ctypes.c_long
FLOAT = ctypes.c_float
HANDLE = ctypes.c_void_p
BOOL = ctypes.c_bool


# --- Structures ---
class POINT(ctypes.Structure):
    _fields_ = [("x", LONG), ("y", LONG)]


class IRF_IMAGE_INFO_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [("xSize", WORD), ("ySize", WORD)]


class IRF_TEMP_CORRECTION_PAR_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("emissivity", FLOAT),
        ("atmTemp", FLOAT),
        ("atmTrans", FLOAT),
        ("zero_offset", INT),
    ]


class IRF_SAVEDATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("reserved1", BYTE * 128),
        ("reserved2", BYTE * 256),
        ("reserved3", BYTE * 128),
    ]


class IRF_AUTO_RANGE_METHOD_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("autoScale", INT),
        ("inputMethod", INT),
        ("outputMethod", INT),
        ("B_Rate", FLOAT),
        ("SD_Rate", FLOAT),
        ("intercept", BYTE),
        ("gamma", FLOAT),
        ("plateau", UINT),
        ("epsilon", FLOAT),
        ("psi", FLOAT),
        ("prevPalteau", FLOAT),
    ]


class IRF_IR_CAM_DATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("ir_image", ctypes.POINTER(WORD)),
        ("image_buffer_size", DWORD),
        ("lpNextData", ctypes.POINTER(BYTE)),
        ("dwSize", DWORD),
        ("dwPosition", DWORD),
        ("msg_type", INT),
        ("save_data", IRF_SAVEDATA_T),
        ("fw_ver", UINT),
        ("core_type", UINT),
        ("h_res", WORD),
        ("v_res", WORD),
        ("PMSGTYPE", WORD),
        ("RCODE", WORD),
        ("reserved", DWORD * 10),
    ]


# --- TCAMINFO Class Definition ---
class TCAMINFO(ctypes.Structure):
    _fields_ = [
        ("p_owner", HANDLE),
        ("ip_addr", BYTE * 4),
        ("port", WORD),
        ("f_run_thread", BOOL),
        ("f_recv_stream", BOOL),
        ("ir_data", IRF_IR_CAM_DATA_T),
        ("agc_ctrl", IRF_AUTO_RANGE_METHOD_T),
        ("ir_size", IRF_IMAGE_INFO_T),
        ("ir_palette", INT),
        ("f_invert", BOOL),
        ("f_mirror", BOOL),
        ("f_flip", BOOL),
        ("f_agc_mode", BOOL),
        ("agc_level", FLOAT),
        ("agc_span", FLOAT),
        ("temp_mode", INT),
        ("p_ir_tmp_buf", ctypes.POINTER(BYTE)),
        ("p_ir_img_buf", ctypes.POINTER(BYTE)),
        ("p_ir_temp_buf", ctypes.POINTER(FLOAT)),
        ("p_palette_lut", (ctypes.POINTER(BYTE) * 2) * MAX_PALETTE),
    ]

    def __init__(self):
        super().__init__()
        self._h_sdk_instance = HANDLE(
            INVALID_HANDLE_VALUE.value
        )  # Managed outside ctypes.Structure
        self._keep_alive_id_instance = HANDLE() # New: Managed outside ctypes.Structure
        self.initial()

    def initial(self):
        self.p_owner = None
        self._h_sdk_instance.value = INVALID_HANDLE_VALUE.value
        self._keep_alive_id_instance.value = 0
        self.ip_addr = (BYTE * 4)(0, 0, 0, 0)
        self.port = 0
        self.f_run_thread = False

        self.f_recv_stream = False

        # Keep references to buffers allocated via ctypes to avoid GC
        # cleaning them up while still in use by the SDK.
        self._ir_image_buf = ctypes.create_string_buffer(
            IRBUFSIZE * ctypes.sizeof(WORD)
        )
        self.ir_data.ir_image = ctypes.cast(
            self._ir_image_buf,
            ctypes.POINTER(WORD),
        )
        self.ir_data.image_buffer_size = IRBUFSIZE
        self._next_data_buf = ctypes.create_string_buffer(8192)
        self.ir_data.lpNextData = ctypes.cast(
            self._next_data_buf, ctypes.POINTER(BYTE)
        )

        self._tmp_buf = ctypes.create_string_buffer(IRBUFSIZE)
        self.p_ir_tmp_buf = ctypes.cast(self._tmp_buf, ctypes.POINTER(BYTE))
        self._img_buf = ctypes.create_string_buffer(IRBUFSIZE * 4)
        self.p_ir_img_buf = ctypes.cast(self._img_buf, ctypes.POINTER(BYTE))
        self._temp_buf = ctypes.create_string_buffer(
            IRBUFSIZE * ctypes.sizeof(FLOAT)
        )
        self.p_ir_temp_buf = ctypes.cast(self._temp_buf, ctypes.POINTER(FLOAT))

        for i in range(MAX_PALETTE):
            buf0 = ctypes.create_string_buffer(PALETTE_SIZE)
            buf1 = ctypes.create_string_buffer(PALETTE_SIZE)
            self.p_palette_lut[i][0] = ctypes.cast(buf0, ctypes.POINTER(BYTE))
            self.p_palette_lut[i][1] = ctypes.cast(buf1, ctypes.POINTER(BYTE))
            # store buffers to keep them alive
            setattr(self, f"_palette_buf_{i}_0", buf0)
            setattr(self, f"_palette_buf_{i}_1", buf1)

        self.reset_member()
        self.reset_ir_data()

    def uninitial(self):
        self.ir_data.ir_image = None
        self.ir_data.lpNextData = None
        self.p_ir_tmp_buf = None
        self.p_ir_img_buf = None
        self.p_ir_temp_buf = None
        self._ir_image_buf = None
        self._next_data_buf = None
        self._tmp_buf = None
        self._img_buf = None
        self._temp_buf = None
        for i in range(MAX_PALETTE):
            self.p_palette_lut[i][0] = None
            self.p_palette_lut[i][1] = None
            setattr(self, f"_palette_buf_{i}_0", None)
            setattr(self, f"_palette_buf_{i}_1", None)

    def reset_member(self):
        self.p_owner = None
        self._h_sdk_instance.value = INVALID_HANDLE_VALUE.value
        self._keep_alive_id_instance.value = 0
        self.ip_addr = (BYTE * 4)(0, 0, 0, 0)
        self.port = 0
        self.f_run_thread = False

        self.f_recv_stream = False
        self.ir_size.xSize = 0
        self.ir_size.ySize = 0
        self.ir_palette = 0
        self.f_invert = False
        self.f_mirror = False
        self.f_flip = False
        self.f_agc_mode = True
        self.agc_level = 0.0
        self.agc_span = 0.0
        self.temp_mode = 0

    def reset_ir_data(self):
        ctypes.memset(self.ir_data.ir_image, 0, IRBUFSIZE * ctypes.sizeof(WORD))
        ctypes.memset(self.ir_data.lpNextData, 0, 8192)
        self.ir_data.dwSize = 0
        self.ir_data.dwPosition = 0
        ctypes.memset(self.p_ir_tmp_buf, 0, IRBUFSIZE)
        ctypes.memset(self.p_ir_img_buf, 0, IRBUFSIZE * 4)
        ctypes.memset(self.p_ir_temp_buf, 0, IRBUFSIZE * ctypes.sizeof(FLOAT))

        self.agc_ctrl.autoScale = 0
        self.agc_ctrl.inputMethod = 2
        self.agc_ctrl.SD_Rate = 5.0
        self.agc_ctrl.B_Rate = 0.01
        self.agc_ctrl.outputMethod = 0
        self.agc_ctrl.intercept = 0
        self.agc_ctrl.gamma = 1.0
        self.agc_ctrl.plateau = 100
        self.agc_ctrl.epsilon = 0.5
        self.agc_ctrl.psi = 0.3
        self.agc_ctrl.prevPalteau = 0.0

    def is_connect(self):
        return self._h_sdk_instance.value != INVALID_HANDLE_VALUE.value


# --- ThermalCameraSDK Class Definition ---
class ThermalCameraSDK:
    def __init__(self, dll_path=DEFAULT_DLL_PATH):
        self.thermal_sdk = None
        try:
            print(f"Attempting to load DLL from: {dll_path}", flush=True)
            self.thermal_sdk = ctypes.WinDLL(dll_path)
            print("ThermalCamSDK_x64.dll loaded successfully.", flush=True)
            self._define_sdk_prototypes()
        except OSError as e:
            print(f"Error loading DLL: {e}", flush=True)

    def _define_sdk_prototypes(self):
        # Define SDK function prototypes
        self.thermal_sdk.OpenConnect.argtypes = [
            ctypes.POINTER(HANDLE),
            ctypes.POINTER(HANDLE),
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self.thermal_sdk.OpenConnect.restype = SHORT

        self.thermal_sdk.CloseConnect.argtypes = [ctypes.POINTER(HANDLE), HANDLE]
        self.thermal_sdk.CloseConnect.restype = SHORT

        self.thermal_sdk.GetIRImages.argtypes = [
            HANDLE,
            ctypes.POINTER(HANDLE),
            ctypes.POINTER(IRF_IR_CAM_DATA_T),
        ]
        self.thermal_sdk.GetIRImages.restype = SHORT

        self.thermal_sdk.GetImageCG.argtypes = [
            ctypes.POINTER(BYTE),
            HANDLE,
            LONG,
            ctypes.POINTER(FLOAT),
            ctypes.POINTER(FLOAT),
            ctypes.POINTER(IRF_AUTO_RANGE_METHOD_T),
        ]
        self.thermal_sdk.GetImageCG.restype = SHORT

        self.thermal_sdk.SendCameraMessage.argtypes = [
            HANDLE,
            ctypes.POINTER(HANDLE),
            INT,
            WORD,
            WORD,
        ]
        self.thermal_sdk.SendCameraMessage.restype = SHORT

        self.thermal_sdk.SendMessageToCamera.argtypes = [
            HANDLE,
            ctypes.POINTER(HANDLE),
            INT,
            WORD,
            WORD,
            DWORD,
            DWORD,
            DWORD,
        ]
        self.thermal_sdk.SendMessageToCamera.restype = SHORT

        self.thermal_sdk.GetPointTempCG.argtypes = [
            HANDLE,
            IRF_IMAGE_INFO_T,
            IRF_TEMP_CORRECTION_PAR_T,
            POINT,
        ]
        self.thermal_sdk.GetPointTempCG.restype = FLOAT
        print("All SDK function prototypes defined.", flush=True)

    def open_connect(self, h_sdk_ptr, keep_alive_id_ptr, ip_addr_bytes, port_num_bytes, af_inet, sock_stream):
        if not self.thermal_sdk:
            return -1  # Indicate error if DLL not loaded
        return self.thermal_sdk.OpenConnect(h_sdk_ptr, keep_alive_id_ptr, ip_addr_bytes, port_num_bytes, af_inet, sock_stream)

    def close_connect(self, h_sdk_ptr, keep_alive_id_val):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.CloseConnect(h_sdk_ptr, keep_alive_id_val)

    def get_ir_images(self, h_sdk, keep_alive_id_ptr, ir_data_ptr):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetIRImages(h_sdk, keep_alive_id_ptr, ir_data_ptr)

    def get_image_lut(self, p_palette_lut, lp, is_invert):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetImageLUT(p_palette_lut, lp, is_invert)

    def get_image_cg(self, p_ir_tmp_buf, h_sdk, size, level_ptr, span_ptr, agc_ctrl_ptr):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetImageCG(p_ir_tmp_buf, h_sdk, size, level_ptr, span_ptr, agc_ctrl_ptr)

    def send_camera_message(self, h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.SendCameraMessage(h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode)

    def send_message_to_camera(self, h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode, rcode2, rcode3, rcode4):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.SendMessageToCamera(h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode, rcode2, rcode3, rcode4)

    def get_point_temp_cg(self, h_sdk, ir_info, temp_corr_params, point):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetPointTempCG(h_sdk, ir_info, temp_corr_params, point)

    def is_dll_loaded(self):
        return self.thermal_sdk is not None


# --- DoRecvACK, DoRecvNAK (Placeholder for now) ---
def DoRecvACK(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK):
    pass

def DoRecvNAK(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK):
    print(f"Received NAK: {tcam_info.ir_data.PMSGTYPE}, {tcam_info.ir_data.RCODE}", flush=True)

# --- DoRecvCamData ---
def DoRecvCamData(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK):
    p_cfg_data = tcam_info.ir_data.save_data

    # Sensor type is stored after the 4-byte CRC and version field.
    sensor_id = p_cfg_data.reserved1[5]
    if sensor_id == QVGA_ID:
        tcam_info.ir_size.xSize = 384
        tcam_info.ir_size.ySize = 288
    elif sensor_id == VGA_ID:
        tcam_info.ir_size.xSize = 640
        tcam_info.ir_size.ySize = 480
    else:
        print(f"Invalid sensor type ({sensor_id}).", flush=True)
        return

    for lp in range(MAX_PALETTE):
        res_normal = sdk_instance.get_image_lut(
            tcam_info.p_palette_lut[lp][0], lp, False
        )
        if res_normal != IRF_NO_ERROR:
            print(f"Failed to get palette {lp} (normal). Error: {res_normal}", flush=True)

        res_invert = sdk_instance.get_image_lut(
            tcam_info.p_palette_lut[lp][1], lp, True
        )
        if res_invert != IRF_NO_ERROR:
            print(f"Failed to get palette {lp} (invert). Error: {res_invert}", flush=True)

    print(
        f"Camera data received. Resolution: {tcam_info.ir_size.xSize}x{tcam_info.ir_size.ySize}", flush=True
    )

# --- DoRecvStreamData ---
def DoRecvStreamData(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK):
    p_ir_cam = tcam_info.ir_data

    # tcam_info.agc_level/span are plain floats. Wrap in ctypes
    # objects so we can pass pointers for GetImageCG.
    level = ctypes.c_float(tcam_info.agc_level)
    span = ctypes.c_float(tcam_info.agc_span)

    res = sdk_instance.get_image_cg(
        tcam_info.p_ir_tmp_buf,
        tcam_info._h_sdk_instance,
        tcam_info.ir_size.xSize * tcam_info.ir_size.ySize,
        ctypes.byref(level),
        ctypes.byref(span),
        ctypes.byref(tcam_info.agc_ctrl),
    )

    # Save updated AGC values back to the structure for later use
    tcam_info.agc_level = level.value
    tcam_info.agc_span = span.value

    if res != IRF_NO_ERROR:
        print(f"Failed GetImageCG. Error: {res}", flush=True)
        return

    # Example: Get temperature at a specific point (e.g., center of the image)
    center_x = tcam_info.ir_size.xSize // 2
    center_y = tcam_info.ir_size.ySize // 2

    target_point = POINT(center_x, center_y)

    temp_corr_params = IRF_TEMP_CORRECTION_PAR_T(
        emissivity=1.0, atmTemp=25.0, atmTrans=1.0, zero_offset=0
    )

    temp_at_point = sdk_instance.get_point_temp_cg(
        tcam_info._h_sdk_instance, tcam_info.ir_size, temp_corr_params, target_point
    )
    # print(f"Temperature at ({center_x}, {center_y}): {temp_at_point:.2f} C", flush=True)

# --- Receive Thread Function ---
def RECEIVE_PROC(tcam_info, sdk_instance):
    print("Receive thread started.", flush=True)
    while tcam_info.f_run_thread:
        if tcam_info.is_connect():
            print(f"RECEIVE_PROC: Handle before GetIRImages: {tcam_info._h_sdk_instance.value}", flush=True)
            res = sdk_instance.get_ir_images(
                tcam_info._h_sdk_instance,
                ctypes.byref(tcam_info._keep_alive_id_instance),
                ctypes.byref(tcam_info.ir_data),
            )
            if res == IRF_NO_ERROR:
                if tcam_info.ir_data.msg_type == _IRF_ACK:
                    DoRecvACK(tcam_info, sdk_instance)
                elif tcam_info.ir_data.msg_type == _IRF_NAK:
                    DoRecvNAK(tcam_info, sdk_instance)
                elif tcam_info.ir_data.msg_type == _IRF_CAM_DATA:
                    DoRecvCamData(tcam_info, sdk_instance)
                elif tcam_info.ir_data.msg_type == _IRF_STREAM_DATA:
                    DoRecvStreamData(tcam_info, sdk_instance)
            else:
                print(f"GetIRImages failed with error: {res}", flush=True)
                if res == -10:
                    tcam_info.f_run_thread = False
                    print("Camera connection error, stopping receive thread.", flush=True)
        time.sleep(0.01)
    print("Receive thread stopped.", flush=True)

# --- ConnectCamera function implementation ---
def ConnectCamera(
    tcam_info: TCAMINFO,
    ip_address: str = DEFAULT_IP_ADDRESS,
    port: int = DEFAULT_PORT,
    sdk_instance: ThermalCameraSDK = None,
) -> bool:
    if sdk_instance is None:
        sdk_instance = ThermalCameraSDK()
    ip_bytes = list(map(int, ip_address.split(".")))
    tcam_info.ip_addr = (BYTE * 4)(*ip_bytes)
    tcam_info.port = port

    c_ip_addr = ip_address.encode("ascii")
    c_port_num = str(port).encode("ascii")

    keep_alive_id_local = tcam_info._keep_alive_id_instance
    res = sdk_instance.open_connect(
        ctypes.byref(tcam_info._h_sdk_instance),
        ctypes.byref(keep_alive_id_local),
        c_ip_addr,
        c_port_num,
        AF_INET,
        SOCK_STREAM,
    )
    tcam_info._keep_alive_id_instance.value = keep_alive_id_local.value

    print(f"OpenConnect returned: {res}", flush=True)

    if res != IRF_NO_ERROR:
        print(f"OpenConnect 실패: {res}", flush=True)
        return False

    print(
        f"Successfully connected to camera. Handle: {tcam_info._h_sdk_instance.value}", flush=True
    )
    time.sleep(0.05)  # Add a small delay after successful connection

    tcam_info.f_run_thread = True
    tcam_info._python_thread_obj = threading.Thread(
        target=RECEIVE_PROC, args=(tcam_info, sdk_instance)
    )
    tcam_info._python_thread_obj.daemon = True
    tcam_info._python_thread_obj.start()
    print("Receive thread started.", flush=True)
    return True

# --- DisconnectCamera function implementation ---
def DisconnectCamera(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK) -> bool:
    if tcam_info._h_sdk_instance.value == INVALID_HANDLE_VALUE.value:
        print("Camera not connected.", flush=True)
        return False

    tcam_info.f_run_thread = False

    # Close the connection first to unblock any pending SDK calls
    res = sdk_instance.close_connect(
        ctypes.byref(tcam_info._h_sdk_instance), tcam_info._keep_alive_id_instance.value
    )

    if res != IRF_NO_ERROR:
        print(f"Failed to disconnect from camera. Error code: {res}", flush=True)
        return False

    # Wait for the receive thread to finish now that the SDK call has returned
    if (
        hasattr(tcam_info, "_python_thread_obj")
        and tcam_info._python_thread_obj
    ):
        tcam_info._python_thread_obj.join(timeout=5)
        if tcam_info._python_thread_obj.is_alive():
            print("Warning: Receive thread did not terminate gracefully.", flush=True)

    tcam_info.reset_member()
    print("Successfully disconnected from camera.", flush=True)
    return True

# --- SendCameraCommand function implementation ---
def SendCameraCommand(
    tcam_info: TCAMINFO, msg_type: int, pmsg_type: int = 0, rcode: int = 0, sdk_instance: ThermalCameraSDK = None
) -> bool:
    if not tcam_info.is_connect():
        print("Camera not connected. Cannot send command.", flush=True)
        return False

    res = sdk_instance.send_camera_message(
        tcam_info._h_sdk_instance,
        ctypes.byref(tcam_info._keep_alive_id_instance),
        msg_type,
        pmsg_type,
        rcode,
    )
    if res != IRF_NO_ERROR:
        print(
            f"Failed to send camera command (type={msg_type}, pmsg={pmsg_type}, rcode={rcode}). Error: {res}", flush=True
        )
        return False
    print(
        f"Camera command sent successfully (type={msg_type}, pmsg={pmsg_type}, rcode={rcode}).", flush=True
    )
    return True

# --- SendCameraConfig function implementation ---
def SendCameraConfig(
    tcam_info: TCAMINFO,
    msg_type: int,
    pmsg_type: int = 0,
    rcode: int = 0,
    rcode2: int = 0,
    rcode3: int = 0,
    rcode4: int = 0,
    sdk_instance: ThermalCameraSDK = None
) -> bool:
    if not tcam_info.is_connect():
        print("Camera not connected. Cannot send config.", flush=True)
        return False

    res = sdk_instance.send_message_to_camera(
        tcam_info._h_sdk_instance,
        ctypes.byref(tcam_info._keep_alive_id_instance),
        msg_type,
        pmsg_type,
        rcode,
        rcode2,
        rcode3,
        rcode4,
    )
    if res != IRF_NO_ERROR:
        print(
            f"Failed to send camera config (type={msg_type}, pmsg={pmsg_type}, rcode={rcode}, rcode2={rcode2}, rcode3={rcode3}, rcode4={rcode4}). Error: {res}", flush=True
        )
        return False
    print(
        f"Camera config sent successfully (type={msg_type}, pmsg={pmsg_type}, rcode={rcode}, rcode2={rcode2}, rcode3={rcode3}, rcode4={rcode4}).", flush=True
    )
    return True

# --- Example Usage ---
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Thermal Analyzer Example")
    parser.add_argument("--config", default="config.ini", help="Path to config file")
    parser.add_argument("--dll", help="Path to ThermalCam SDK DLL")
    parser.add_argument("--ip", help="Camera IP address")
    parser.add_argument("--port", type=int, help="Camera port")
    args = parser.parse_args()

    dll_path = DEFAULT_DLL_PATH
    ip_address = DEFAULT_IP_ADDRESS
    port = DEFAULT_PORT

    if os.path.exists(args.config):
        cfg = configparser.ConfigParser()
        cfg.read(args.config)
        dll_path = cfg.get("camera", "dll_path", fallback=dll_path)
        ip_address = cfg.get("camera", "ip_address", fallback=ip_address)
        port = cfg.getint("camera", "port", fallback=port)

    if args.dll:
        dll_path = args.dll
    if args.ip:
        ip_address = args.ip
    if args.port:
        port = args.port

    sdk_instance = ThermalCameraSDK(dll_path)
    if not sdk_instance.is_dll_loaded():
        print("DLL not loaded. Cannot run example usage.", flush=True)
    else:
        tcam_info_instance = TCAMINFO()
        try:
            if ConnectCamera(tcam_info_instance, ip_address, port, sdk_instance):
                print(
                    f"Connected to {ip_address}:{port}. Waiting for data and processing...",
                    flush=True,
                )

                # Request camera configuration and start streaming
                SendCameraCommand(
                    tcam_info_instance, _IRF_REQ_CAM_DATA, sdk_instance=sdk_instance
                )
                time.sleep(0.5)
                SendCameraCommand(
                    tcam_info_instance, _IRF_STREAM_ON, sdk_instance=sdk_instance
                )

                start_time = time.time()
                while time.time() - start_time < 15:
                    time.sleep(1)

                print("Time elapsed. Disconnecting...", flush=True)
                SendCameraCommand(
                    tcam_info_instance, _IRF_STREAM_OFF, sdk_instance=sdk_instance
                )
                DisconnectCamera(tcam_info_instance, sdk_instance)
            else:
                print("Connection failed. Please check camera IP/port and connection.", flush=True)
        finally:
            tcam_info_instance.uninitial()
            print("Program finished.", flush=True)
