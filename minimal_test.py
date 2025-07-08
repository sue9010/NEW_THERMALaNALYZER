print("Script started.", flush=True)
import ctypes
import socket
import threading
import time
import pdb

# --- Constants ---
INVALID_HANDLE_VALUE = ctypes.c_void_p(-1)
IRF_NO_ERROR = 1
AF_INET = socket.AF_INET
SOCK_STREAM = socket.SOCK_STREAM

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

# --- Structures (minimal set for core functions) ---
class IRF_SAVEDATA_T(ctypes.Structure):
    _fields_ = [
        ("reserved1", BYTE * 128),
        ("reserved2", BYTE * 256),
        ("reserved3", BYTE * 128),
    ]

class IRF_IR_CAM_DATA_T(ctypes.Structure):
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

# --- TCAMINFO Class Definition (simplified) ---
class TCAMINFO(ctypes.Structure):
    _fields_ = [
        ("f_run_thread", BOOL),
        ("f_recv_stream", BOOL),
        ("ir_data", IRF_IR_CAM_DATA_T),
    ]

    def __init__(self):
        super().__init__()
        self._h_sdk_instance = UINT(INVALID_HANDLE_VALUE.value)
        self._keep_alive_id_instance = HANDLE()
        self.f_run_thread = False # Initialize directly
        self.f_recv_stream = False # Initialize directly
        # Minimal initialization for ir_data to avoid crashes
        self.ir_data.ir_image = ctypes.cast(
            ctypes.create_string_buffer(100 * ctypes.sizeof(WORD)),
            ctypes.POINTER(WORD),
        )
        self.ir_data.lpNextData = ctypes.cast(
            ctypes.create_string_buffer(100), ctypes.POINTER(BYTE)
        )
        self.ir_data.image_buffer_size = 100
        self.ir_data.dwSize = 0
        self.ir_data.dwPosition = 0
        self.ir_data.msg_type = 0
        self.ir_data.fw_ver = 0
        self.ir_data.core_type = 0
        self.ir_data.h_res = 0
        self.ir_data.v_res = 0
        self.ir_data.PMSGTYPE = 0
        self.ir_data.RCODE = 0
        # No need for full reset_member or uninitial in minimal test

    def is_connect(self):
        return self._h_sdk_instance.value != INVALID_HANDLE_VALUE.value

# --- ThermalCameraSDK Class Definition (simplified) ---
class ThermalCameraSDK:
    def __init__(self, dll_path):
        self.thermal_sdk = None
        try:
            print(f"Attempting to load DLL from: {dll_path}", flush=True)
            self.thermal_sdk = ctypes.WinDLL(dll_path)
            print("ThermalCamSDK_x64.dll loaded successfully.", flush=True)
            self._define_sdk_prototypes()
        except OSError as e:
            print(f"Error loading DLL: {e}", flush=True)

    def _define_sdk_prototypes(self):
        self.thermal_sdk.OpenConnect.argtypes = [
            ctypes.POINTER(UINT),
            ctypes.POINTER(HANDLE),
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self.thermal_sdk.OpenConnect.restype = SHORT

        self.thermal_sdk.CloseConnect.argtypes = [ctypes.POINTER(UINT), HANDLE]
        self.thermal_sdk.CloseConnect.restype = SHORT

        self.thermal_sdk.GetIRImages.argtypes = [
            UINT,
            ctypes.POINTER(HANDLE),
            ctypes.POINTER(IRF_IR_CAM_DATA_T),
        ]
        self.thermal_sdk.GetIRImages.restype = SHORT
        print("All SDK function prototypes defined.", flush=True)

    def open_connect(self, h_sdk_ptr, keep_alive_id_ptr, ip_addr_bytes, port_num_bytes, af_inet, sock_stream):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.OpenConnect(h_sdk_ptr, keep_alive_id_ptr, ip_addr_bytes, port_num_bytes, af_inet, sock_stream)

    def close_connect(self, h_sdk_ptr, keep_alive_id_val):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.CloseConnect(h_sdk_ptr, keep_alive_id_val)

    def get_ir_images(self, h_sdk, keep_alive_id_ptr, ir_data_ptr):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetIRImages(h_sdk.value, keep_alive_id_ptr, ir_data_ptr)

    def get_image_lut(self, p_palette_lut, lp, is_invert):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetImageLUT(p_palette_lut, lp, is_invert)

    def get_image_cg(self, p_ir_tmp_buf, h_sdk, size, level_ptr, span_ptr, agc_ctrl_ptr):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetImageCG(p_ir_tmp_buf, h_sdk.value, size, level_ptr, span_ptr, agc_ctrl_ptr)

    def send_camera_message(self, h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.SendCameraMessage(h_sdk.value, keep_alive_id_ptr, msg_type, pmsg_type, rcode)

    def send_message_to_camera(self, h_sdk, keep_alive_id_ptr, msg_type, pmsg_type, rcode, rcode2, rcode3, rcode4):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.SendMessageToCamera(h_sdk.value, keep_alive_id_ptr, msg_type, pmsg_type, rcode, rcode2, rcode3, rcode4)

    def get_point_temp_cg(self, h_sdk, ir_info, temp_corr_params, point):
        if not self.thermal_sdk:
            return -1
        return self.thermal_sdk.GetPointTempCG(h_sdk.value, ir_info, temp_corr_params, point)

    def is_dll_loaded(self):
        return self.thermal_sdk is not None

# --- ConnectCamera function (simplified) ---
def ConnectCamera(tcam_info: TCAMINFO, ip_address: str, port: int, sdk_instance: ThermalCameraSDK) -> bool:
    c_ip_addr = ip_address.encode("ascii")
    c_port_num = str(port).encode("ascii")

    res = sdk_instance.open_connect(
        ctypes.byref(tcam_info._h_sdk_instance),
        ctypes.byref(tcam_info._keep_alive_id_instance),
        c_ip_addr,
        c_port_num,
        AF_INET,
        SOCK_STREAM,
    )
    print(f"OpenConnect returned: {res}", flush=True)

    if res != IRF_NO_ERROR:
        print(f"OpenConnect 실패: {res}", flush=True)
        return False

    print(f"Successfully connected to camera. Handle: {tcam_info._h_sdk_instance.value}", flush=True)
    tcam_info.f_run_thread = True
    return True

# --- DisconnectCamera function (simplified) ---
def DisconnectCamera(tcam_info: TCAMINFO, sdk_instance: ThermalCameraSDK) -> bool:
    if tcam_info._h_sdk_instance.value == INVALID_HANDLE_VALUE.value:
        print("Camera not connected.", flush=True)
        return False

    tcam_info.f_run_thread = False # Stop the thread if it was running

    res = sdk_instance.close_connect(
        ctypes.byref(tcam_info._h_sdk_instance), tcam_info._keep_alive_id_instance.value
    )

    if res != IRF_NO_ERROR:
        print(f"Failed to disconnect from camera. Error code: {res}", flush=True)
        return False

    print("Successfully disconnected from camera.", flush=True)
    return True

# --- RECEIVE_PROC (simplified for minimal test) ---
def RECEIVE_PROC(tcam_info, sdk_instance):
    print("Receive thread started.", flush=True)
    if tcam_info.is_connect():
        print(f"RECEIVE_PROC: Handle before GetIRImages: {tcam_info._h_sdk_instance.value}", flush=True)
        res = sdk_instance.get_ir_images(
            tcam_info._h_sdk_instance, # Pass HANDLE by value
            ctypes.byref(tcam_info._keep_alive_id_instance),
            ctypes.byref(tcam_info.ir_data),
        )
        print(f"GetIRImages returned: {res}", flush=True)
        if res != IRF_NO_ERROR:
            print(f"GetIRImages failed with error: {res}", flush=True)
    print("Receive thread stopped.", flush=True)


# --- Main Execution ---
if __name__ == "__main__":
    dll_path = r"E:/Coding_practice/new_ThermalAnalyzer/SDK/SDK/bin64/ThermalCamSDK_x64_D.dll"
    sdk_instance = ThermalCameraSDK(dll_path)

    if not sdk_instance.is_dll_loaded():
        print("DLL not loaded. Cannot run example usage.", flush=True)
    else:
        tcam_info_instance = TCAMINFO()
        try:
            camera_ip = "192.168.0.101"
            camera_port = 15001

            print("\n--- Attempting to Connect ---", flush=True)
            if ConnectCamera(tcam_info_instance, camera_ip, camera_port, sdk_instance):
                print("\n--- Connected. Calling GetIRImages ---\n", flush=True)
                # Directly call RECEIVE_PROC for a single iteration to test GetIRImages
                RECEIVE_PROC(tcam_info_instance, sdk_instance)

                print("\n--- Attempting to Disconnect ---\n", flush=True)
                DisconnectCamera(tcam_info_instance, sdk_instance)
            else:
                print("Connection failed. Please check camera IP/port and connection.\n", flush=True)
        finally:
            # tcam_info_instance.uninitial() # Not needed for minimal test
            print("\nProgram finished.\n", flush=True)
