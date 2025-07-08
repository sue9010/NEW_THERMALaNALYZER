import ctypes
import os

# --- 1. 기본 타입 매핑 ---
BYTE = ctypes.c_ubyte
WORD = ctypes.c_ushort
DWORD = ctypes.c_ulong
UINT = ctypes.c_uint
LONG = ctypes.c_long
FLOAT = ctypes.c_float
BOOL = ctypes.c_bool # C++ bool 또는 BOOL (int)에 따라 다를 수 있으나, c_bool이 더 명확
HANDLE = ctypes.c_void_p # Generic handle type
LPCSTR = ctypes.c_char_p # Pointer to a constant null-terminated string
LPBYTE = ctypes.POINTER(BYTE) # Pointer to BYTE

# Windows POINT and RECT structures (from windef.h or similar)
class POINT(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_long),
        ("y", ctypes.c_long),
    ]

class RECT(ctypes.Structure):
    _fields_ = [
        ("left", ctypes.c_long),
        ("top", ctypes.c_long),
        ("right", ctypes.c_long),
        ("bottom", ctypes.c_long),
    ]

# --- 2. DLL 로드 ---
# 현재 작업 디렉토리가 SDK/SDK/ 이므로, bin64/ThermalCamSDK_x64.dll 의 경로는 다음과 같습니다.
dll_path = os.path.abspath("E:/Coding_practice/new_ThermalAnalyzer/SDK/SDK/bin64/ThermalCamSDK_x64.dll")
try:
    thermal_sdk = ctypes.WinDLL(dll_path)
    print(f"DLL '{dll_path}' 로드 성공.")
except OSError as e:
    print(f"DLL 로드 실패: {e}")
    print("DLL 경로를 확인하거나, 64비트 파이썬 환경인지 확인하세요.")
    exit()

# --- 3. Enumeration Definition 매핑 ---
# C enum은 보통 int로 처리되므로 ctypes.c_int를 사용합니다.
# 값은 헤더 파일에 정의된 대로 사용합니다.

class IRF_TEMP_MODE_T(ctypes.c_int):
    _IRF_CELSIUS = 0
    _IRF_FAHRENHEIT = 1
    _IRF_KELVIN = 2

class IRF_AUTOMATIC_TYPE_T(ctypes.c_int):
    _IRF_AUTO = 0
    _IRF_MANUAL = 1

class IRF_DYNAMIC_RANGE_T(ctypes.c_int):
    _IRF_LOW_RANGE = 0
    _IRF_MIDDLE_RANGE = 1
    _IRF_MEDICAL_RANGE = 2

class IRF_AUTO_RANGE_INPUT_METHOD_T(ctypes.c_int):
    _IRF_MIN_MAX = 0
    _IRF_BRIGHTNESS_RATE = 1
    _IRF_SD_RATE = 2
    _IRF_AUTO_BRIGHT = 3
    _IRF_ENHANCE_HIST = 4

class IRF_AUTO_RANGE_OUTPUT_METHOD_T(ctypes.c_int):
    _IRF_LINEAR = 0
    _IRF_NON_LINEAR = 1
    _IRF_TPE = 2
    _IRF_APE = 3
    _IRF_SAPE = 4

class IRF_IMAGE_FILTER_T(ctypes.c_int):
    _IRF_FILTER_NONE = 0
    _IRF_MEDIAN = 1
    _IRF_SOFTEN_SLIGHTLY = 2
    _IRF_SOFTEN_STRONG = 3
    _IRF_SHARPENING_SLIGHTLY = 4
    _IRF_SHARPENING_STRONG = 5
    _IRF_BOXBLUR = 6
    _IRF_FAST_GAUSSIAN = 7
    _IRF_FAST_STACK_BLUR = 8
    _IRF_BI_LATERAL = 9

class IRF_MESSAGE_TYPE_T(ctypes.c_int):
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

class IRF_PALETTE_TYPE_T(ctypes.c_int):
    YELLOW_COLOR_MAP = 0
    RAINBOW_COLOR_MAP = 1
    RAIN900_COLOR_MAP = 2
    RAIN10_COLOR_MAP = 3
    MIDGREY_COLOR_MAP = 4
    MIDGREEN_COLOR_MAP = 5
    MEDICAL_COLOR_MAP = 6
    IRON10_COLOR_MAP = 7
    IRON_COLOR_MAP = 8
    GREYRED_COLOR_MAP = 9
    GREY10_COLOR_MAP = 10
    GREY_COLOR_MAP = 11
    GLOWBOW_COLOR_MAP = 12

class IRF_CAM_PALETTE_TYPE_T(ctypes.c_int):
    GREY = 0
    RAINBOW = 1
    IRON = 2
    GREYRED = 3
    GLOWBOW = 4
    YELLOW = 5
    MIDGREY = 6
    MIDGREEN = 7

class IRF_CAM_MODEL_TYPE_T(ctypes.c_int):
    CX320 = 0
    CX640 = 1
    CG320 = 2
    CG640 = 3
    CZ1024 = 4

# --- 4. Structure Definition 매핑 ---
# #pragma pack(1) 지시자에 따라 _pack_ = 1 설정

# IRF_IMAGE_INFO_T
class IRF_IMAGE_INFO_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("xSize", WORD), # uint16_t
        ("ySize", WORD), # uint16_t
    ]

# IRF_TEMP_CORRECTION_PAR_T
class IRF_TEMP_CORRECTION_PAR_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("emissivity", FLOAT),
        ("atmTemp", FLOAT),
        ("atmTrans", FLOAT),
        ("zero_offset", ctypes.c_int),
    ]

# IRF_IR_FILE_HEADER_T
class IRF_IR_FILE_HEADER_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("ver", BYTE),
        ("image_info", IRF_IMAGE_INFO_T),
        ("temp_correction", IRF_TEMP_CORRECTION_PAR_T),
    ]

# IRF_AUTO_RANGE_METHOD_T
class IRF_AUTO_RANGE_METHOD_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("autoScale", IRF_AUTOMATIC_TYPE_T),
        ("inputMethod", IRF_AUTO_RANGE_INPUT_METHOD_T),
        ("outputMethod", IRF_AUTO_RANGE_OUTPUT_METHOD_T),
        ("B_Rate", FLOAT),
        ("SD_Rate", FLOAT),
        ("intercept", BYTE), # uint8_t
        ("gamma", FLOAT),
        ("plateau", UINT), # uint32_t
        ("epsilon", FLOAT),
        ("psi", FLOAT),
        ("prevPalteau", FLOAT),
    ]

# IRF_NUMERIC_INFO_T
class IRF_NUMERIC_INFO_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("min", FLOAT),
        ("max", FLOAT),
        ("avg", FLOAT),
        ("std", FLOAT),
    ]

# IRF_SET_USER_PALETTE
class IRF_SET_USER_PALETTE(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("Info", BYTE * 7),
        ("Index", BYTE),
        ("Data", BYTE * 1024),
        ("pngLength", UINT), # uint32_t
        ("pngData", BYTE * 8192),
    ]

# ALRMCFG (ALRMCFG, * ALRMCFG_PTR)
class ALRMCFG(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("f_enable", BYTE), # uint8_t
        ("output_mask", BYTE), # uint8_t
        ("cond", BYTE), # uint8_t
        ("reserved", BYTE), # uint8_t
        ("temp", ctypes.c_short), # int16_t
        ("delay", WORD), # uint16_t
    ]

# IRF_SAVEDATA_T (복잡한 union과 nested struct 포함)
# union은 ctypes에서 직접 매핑하기 어렵고, 보통 가장 큰 멤버로 처리하거나
# 필요한 멤버만 접근하도록 합니다. 여기서는 union의 첫 번째 struct를 매핑합니다.
# 실제 사용 시에는 어떤 union 멤버가 활성화되는지 SDK 문서를 참조해야 합니다.
class IRF_SAVEDATA_T_ROI_CX(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("flag", BYTE),
        ("x1", WORD),
        ("y1", WORD),
        ("x2", WORD),
        ("y2", WORD),
    ]

class IRF_SAVEDATA_T_CG_ROI(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode", BYTE),
        ("temp_type", BYTE),
        ("x", WORD),
        ("y", WORD),
        ("w", WORD),
        ("h", WORD),
        ("f_corr", BYTE),
        ("em", BYTE),
        ("tr", BYTE),
        ("at", ctypes.c_short),
        ("reserved", BYTE),
    ]

class IRF_SAVEDATA_T_ISO_CX(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("enable", BYTE),
        ("seed_color", UINT),
        ("top", ctypes.c_short),
        ("bottom", ctypes.c_short),
        ("reserved", BYTE * 3),
    ]

class IRF_SAVEDATA_T_CG_ISO(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode_mask", BYTE),
        ("tran_mask", BYTE),
        ("max_temp", ctypes.c_short),
        ("min_temp", ctypes.c_short),
        ("above_color", UINT),
        ("below_color", UINT),
        ("between_color", UINT),
    ]

class IRF_SAVEDATA_T_InnerStruct1(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("crc", UINT),
        ("ver", BYTE),
        ("sensor", BYTE),
        ("tv", BYTE),
        ("temp_mode", BYTE),
        ("id", BYTE),
        ("baudrate", BYTE),
        ("level", ctypes.c_short),
        ("span", WORD),
        ("agc", BYTE),
        ("invert", BYTE),
        ("mirror", BYTE),
        ("flip", BYTE),
        ("colorbar", BYTE),
        ("showinfo", BYTE),
        ("indicator", BYTE),
        ("unit", BYTE),
        ("dhcp", BYTE),
        ("color", BYTE),
        ("alpha", BYTE),
        ("zoom", BYTE),
        ("sharp", BYTE),
        ("noise", BYTE),
        ("nuc", WORD),
        ("econt", BYTE),
        ("ipaddr", UINT),
        ("netmask", UINT),
        ("gateway", UINT),
        ("dns", UINT),
        ("alarm1_func", BYTE),
        ("alarm1_cond", BYTE),
        ("alarm1_value", ctypes.c_short),
        ("alarm2_func", BYTE),
        ("alarm2_cond", BYTE),
        ("alarm2_value", ctypes.c_short),
        ("down_filter", BYTE),
        ("show_center", BYTE),
        ("show_spot", BYTE),
        ("show_correction", BYTE),
        ("show_isotherm", BYTE),
        ("alarm1_duration", BYTE),
        ("alarm2_duration", BYTE),
        ("roi", IRF_SAVEDATA_T_ROI_CX * 2), # Array of 2
        # CG Model specific fields
        ("f_disp_icon", BYTE),
        ("brightness", BYTE),
        ("contrast", BYTE),
        ("f_edge_enhance", BYTE),
        ("nuc_mode", BYTE),
        ("nuc_time", BYTE),
        ("nuc_thres", WORD),
        ("agc_man_max", WORD),
        ("agc_man_min", WORD),
        ("srl_protocol", BYTE),
        ("scn0_l_margin", WORD),
        ("scn0_r_margin", WORD),
        ("scn0_t_margin", WORD),
        ("scn0_b_margin", WORD),
        ("scn1_l_margin", WORD),
        ("scn1_r_margin", WORD),
        ("scn1_t_margin", WORD),
        ("scn1_b_margin", WORD),
        ("hdmi_mode", BYTE),
        ("alarm1_type", BYTE),
        ("alarm1_mode", BYTE),
        ("alarm1_dura", BYTE),
        ("alarm1_remote_ctrl", BYTE),
        ("alarm2_type", BYTE),
        ("alarm2_mode", BYTE),
        ("alarm2_dura", BYTE),
        ("alarm2_remote_ctrl", BYTE),
        ("zero_offset", ctypes.c_short),
        ("measure_distance", WORD),
        ("nr1_strength", BYTE),
        ("nr2_strength", BYTE),
        ("ee_strength", BYTE),
    ]

class IRF_SAVEDATA_T_InnerStruct2(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("limit9", BYTE),
        ("enable_high", BYTE),
        ("correction", BYTE),
        ("emissivity", BYTE),
        ("transmission", BYTE),
        ("atmosphere", ctypes.c_short),
        # Union for spot/cg_roi - using cg_roi as it's more general
        ("cg_roi", IRF_SAVEDATA_T_CG_ROI * 10), # Array of 10
        # Union for iso/cg_iso - using cg_iso as it's more general
        ("cg_iso", IRF_SAVEDATA_T_CG_ISO * 2), # Array of 2
        ("hdmi_list", BYTE * 8),
        ("support_temp_mode", WORD),
        ("f_support_motor_focus", BYTE),
        ("f_support_motor_zoom", BYTE),
        ("meas_dist_min", WORD),
        ("meas_dist_max", WORD),
        ("meas_temp", ctypes.c_short * 2 * 2), # 2D array
        ("meas_level", WORD * 2 * 2), # 2D array
        ("meas_name", BYTE * 2 * 10), # 2D array
    ]

class IRF_SAVEDATA_T_InnerStruct3(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("max_temp", ALRMCFG),
        ("min_temp", ALRMCFG),
        ("avg_temp", ALRMCFG),
        ("ctr_temp", ALRMCFG),
        ("roi_temp", ALRMCFG * 10), # Array of 10
    ]

# Main IRF_SAVEDATA_T structure, handling unions by defining fields for each union member
# If multiple members can be active or it's truly dynamic, you might need a more complex approach.
class IRF_SAVEDATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("data1", IRF_SAVEDATA_T_InnerStruct1),
        ("reserved1", BYTE * 128), # Union member
        ("data2", IRF_SAVEDATA_T_InnerStruct2),
        ("reserved2", BYTE * 256), # Union member
        ("temp_alarm", IRF_SAVEDATA_T_InnerStruct3),
        ("reserved3", BYTE * 128), # Union member
    ]


# IRF_IR_DATA_HEADER_T
class IRF_IR_DATA_HEADER_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("dynamic_range", BYTE), # IRF_DYNAMIC_RANGE_T
        ("year", ctypes.c_int),
        ("month", ctypes.c_int),
        ("day", ctypes.c_int),
        ("hour", ctypes.c_int),
        ("minute", ctypes.c_int),
        ("second", ctypes.c_int),
        ("total_frame", ctypes.c_int),
        ("save_data", IRF_SAVEDATA_T),
        ("reserved", BYTE * 460),
    ]

# IRF_CRD_FILE_HEADER_T
class IRF_CRD_FILE_HEADER_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("fhSignature", BYTE * 3),
        ("fhSize", WORD),
        ("fhReserved1", BYTE),
        ("fhVer", BYTE),
        ("imageInfo", IRF_IMAGE_INFO_T),
        ("tempCorrection", IRF_TEMP_CORRECTION_PAR_T),
        ("dynamicRange", BYTE), # IRF_DYNAMIC_RANGE_T
        ("fcYear", ctypes.c_int),
        ("fcMon", ctypes.c_int),
        ("fcDay", ctypes.c_int),
        ("fcHour", ctypes.c_int),
        ("fcMin", ctypes.c_int),
        ("fcSec", ctypes.c_int),
        ("totalFrame", UINT), # uint32_t
        ("fps", ctypes.c_int),
        ("reserved", BYTE * 968),
    ]

# IRF_CRD_DATA_HEADER_T
class IRF_CRD_DATA_HEADER_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("dhSeparator", BYTE * 2),
        ("dhSize", BYTE),
        ("dhType", BYTE),
        ("curFrame", UINT), # uint32_t
        ("savedTime", ctypes.c_longlong), # __int64
        ("dataSize", UINT), # uint32_t
        ("reserved", BYTE * 240),
    ]

# IRF_IR_CAM_DATA_T
class IRF_IR_CAM_DATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("ir_image", ctypes.POINTER(WORD)), # uint16_t*
        ("image_buffer_size", DWORD),
        ("lpNextData", LPBYTE),
        ("dwSize", DWORD),
        ("dwPosition", DWORD),
        ("msg_type", IRF_MESSAGE_TYPE_T),
        ("save_data", IRF_SAVEDATA_T),
        ("fw_ver", UINT), # uint32_t
        ("core_type", UINT), # uint32_t
        ("h_res", WORD), # uint16_t
        ("v_res", WORD), # uint16_t
        ("PMSGTYPE", WORD), # uint16_t
        ("RCODE", WORD), # uint16_t
        ("reserved", DWORD * 10),
    ]

# IRF_SYS_INFO_T
class IRF_SYS_INFO_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("MAC", BYTE * 6),
        ("fw_ver", UINT),
        ("cpu_ver", UINT),
        ("fpga_ver", UINT),
        ("engine_ver", UINT),
        ("reserved", BYTE * 42),
    ]

# IRF_RECEIVED_CAM_SYS_T
class IRF_RECEIVED_CAM_SYS_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("sys_info", IRF_SYS_INFO_T),
        ("msg_type", IRF_MESSAGE_TYPE_T),
        ("PMSGTYPE", WORD),
        ("RCODE", WORD),
    ]

# IRF_TEMP_TABLE_T
class IRF_TEMP_TABLE_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("normal_min", ctypes.c_short),
        ("normal_max", ctypes.c_short),
        ("high_min", ctypes.c_short),
        ("high_max", ctypes.c_short),
        ("normal_table", UINT * 256),
        ("high_table", UINT * 256),
    ]

# IRF_TEMP_OFFSET_TABLE_T
class IRF_TEMP_OFFSET_TABLE_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("normal_min", ctypes.c_short),
        ("normal_max", ctypes.c_short),
        ("high_min", ctypes.c_short),
        ("high_max", ctypes.c_short),
        ("normal_table", ctypes.c_int * 256),
        ("high_table", ctypes.c_int * 256),
    ]

# IRF_MIX_TEMP_TABLE_T
class IRF_MIX_TEMP_TABLE_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("temp_table", IRF_TEMP_TABLE_T),
        ("temp_offset_table", IRF_TEMP_OFFSET_TABLE_T),
    ]

# IRF_RECEIVED_TEMP_TABLE_T
class IRF_RECEIVED_TEMP_TABLE_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("msg_type", IRF_MESSAGE_TYPE_T),
        ("sTemp_table", IRF_MIX_TEMP_TABLE_T),
        ("PMSGTYPE", WORD),
        ("RCODE", WORD),
    ]

# --- 5. Function Definition 매핑 ---

# Common Interface
# THERMALSDK_API int16_t PASCAL OpenConnect(HANDLE *pHandle, HANDLE *pTimerID, LPCSTR strDestination, LPCSTR strServiceName, int nProtocol, int nType);
thermal_sdk.OpenConnect.argtypes = [
    ctypes.POINTER(HANDLE),
    ctypes.POINTER(HANDLE),
    LPCSTR,
    LPCSTR,
    ctypes.c_int,
    ctypes.c_int,
]
thermal_sdk.OpenConnect.restype = ctypes.c_short # int16_t

# THERMALSDK_API int16_t PASCAL CloseConnect(HANDLE *handle, HANDLE timerID);
thermal_sdk.CloseConnect.argtypes = [
    ctypes.POINTER(HANDLE),
    HANDLE,
]
thermal_sdk.CloseConnect.restype = ctypes.c_short # int16_t

# THERMALSDK_API int16_t PASCAL GetIRImages(HANDLE handle, HANDLE *pTimerID, IRF_IR_CAM_DATA_T* cam_data);
thermal_sdk.GetIRImages.argtypes = [
    HANDLE,
    ctypes.POINTER(HANDLE),
    ctypes.POINTER(IRF_IR_CAM_DATA_T),
]
thermal_sdk.GetIRImages.restype = ctypes.c_short # int16_t

# THERMALSDK_API int16_t PASCAL SendMessageToCamera(HANDLE handle, HANDLE *pTimerID, IRF_MESSAGE_TYPE_T type, uint16_t PMSGTYPE, uint16_t RCODE, DWORD RCODE2, DWORD RCODE3, DWORD RCODE4);
thermal_sdk.SendMessageToCamera.argtypes = [
    HANDLE,
    ctypes.POINTER(HANDLE),
    IRF_MESSAGE_TYPE_T,
    WORD, # uint16_t
    WORD, # uint16_t
    DWORD,
    DWORD,
    DWORD,
]
thermal_sdk.SendMessageToCamera.restype = ctypes.c_short # int16_t

# THERMALSDK_API int16_t PASCAL GetImageLUT(uint8_t *palette, IRF_PALETTE_TYPE_T paletteType, bool bInvert);
thermal_sdk.GetImageLUT.argtypes = [
    LPBYTE, # uint8_t*
    IRF_PALETTE_TYPE_T,
    BOOL, # bool
]
thermal_sdk.GetImageLUT.restype = ctypes.c_short # int16_t

# THERMALSDK_API int16_t PASCAL GetGrayToPaletteImage(uint8_t *from_image, void* to_image, uint16_t width, uint16_t height, uint8_t *palette, int BitsPixel, BOOL bMirror, BOOL bFlip);
thermal_sdk.GetGrayToPaletteImage.argtypes = [
    LPBYTE, # uint8_t*
    ctypes.c_void_p, # void*
    WORD, # uint16_t
    WORD, # uint16_t
    LPBYTE, # uint8_t*
    ctypes.c_int,
    BOOL, # BOOL
    BOOL, # BOOL
]
thermal_sdk.GetGrayToPaletteImage.restype = ctypes.c_short # int16_t

# CX320, CX640 Only Interface (예시)
# THERMALSDK_API float PASCAL GetPointTemp(HANDLE ir_image, IRF_IMAGE_INFO_T image_info, float *tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, POINT pt);
thermal_sdk.GetPointTemp.argtypes = [
    HANDLE,
    IRF_IMAGE_INFO_T,
    ctypes.POINTER(FLOAT), # float*
    IRF_TEMP_CORRECTION_PAR_T,
    POINT,
]
thermal_sdk.GetPointTemp.restype = FLOAT # float

# CG320, CG640 Only Interface (예시)
# THERMALSDK_API float PASCAL GetPointTempCG( HANDLE ir_image, IRF_IMAGE_INFO_T image_info, IRF_TEMP_CORRECTION_PAR_T corrPara, POINT pt );
thermal_sdk.GetPointTempCG.argtypes = [
    HANDLE,
    IRF_IMAGE_INFO_T,
    IRF_TEMP_CORRECTION_PAR_T,
    POINT,
]
thermal_sdk.GetPointTempCG.restype = FLOAT # float

# Temperature Converter (예시)
# THERMALSDK_API float PASCAL ConvertFahToCels(float temp);
thermal_sdk.ConvertFahToCels.argtypes = [FLOAT]
thermal_sdk.ConvertFahToCels.restype = FLOAT

print("\n--- ctypes 매핑 완료 ---")
print("이제 위에서 정의된 클래스와 함수를 사용하여 SDK와 상호작용할 수 있습니다.")
print("예시: thermal_sdk.OpenConnect(...)")
