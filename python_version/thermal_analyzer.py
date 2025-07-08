import ctypes
import os

import cv2
import numpy as np

# --- 사용자 설정 ---
# 사용할 카메라 모델: CG320
# 해상도: 384x288
IP_ADDRESS = "192.168.0.101"
PORT = "15001"  # 기본 포트, 필요시 설명서 확인 후 수정
DLL_PATH = r"E:\Coding_practice\new_ThermalAnalyzer\SDK\SDK\bin64\ThermalCamSDK_x64.dll"
WIDTH = 384
HEIGHT = 288
# --- 설정 종료 ---

# 1. SDK DLL 로드
if not os.path.exists(DLL_PATH):
    print(f"오류: DLL 파일을 찾을 수 없습니다. 경로를 확인하세요: {DLL_PATH}")
    exit()
try:
    sdk = ctypes.CDLL(DLL_PATH)
except OSError as e:
    print(f"오류: DLL을 로드하는 중 문제가 발생했습니다: {e}")
    exit()

# 3. SDK 자료구조 정의 (ThermalCamSDK.h 기반)
# pragma pack(1) 지시어에 맞춰 1바이트 크기로 정렬
# 이 구조체들은 SDK와의 메모리 호환을 위해 매우 중요합니다.


class ALRMCFG(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("f_enable", ctypes.c_uint8),
        ("output_mask", ctypes.c_uint8),
        ("cond", ctypes.c_uint8),
        ("reserved", ctypes.c_uint8),
        ("temp", ctypes.c_int16),
        ("delay", ctypes.c_uint16),
    ]


class CG_ROI(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode", ctypes.c_uint8),
        ("temp_type", ctypes.c_uint8),
        ("x", ctypes.c_uint16),
        ("y", ctypes.c_uint16),
        ("w", ctypes.c_uint16),
        ("h", ctypes.c_uint16),
        ("f_corr", ctypes.c_uint8),
        ("em", ctypes.c_uint8),
        ("tr", ctypes.c_uint8),
        ("at", ctypes.c_int16),
        ("reserved", ctypes.c_uint8),
    ]


class CG_ISO(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode_mask", ctypes.c_uint8),
        ("tran_mask", ctypes.c_uint8),
        ("max_temp", ctypes.c_int16),
        ("min_temp", ctypes.c_int16),
        ("above_color", ctypes.c_uint32),
        ("below_color", ctypes.c_uint32),
        ("between_color", ctypes.c_uint32),
    ]


class IRF_SAVEDATA_T_Reserved1_Union_CG(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("cg_roi", CG_ROI * 10),
    ]


class IRF_SAVEDATA_T_Reserved1_Union_CG_ISO(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("cg_iso", CG_ISO * 2),
    ]


class IRF_SAVEDATA_T_Reserved1_Struct(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("limit9", ctypes.c_uint8),
        ("enable_high", ctypes.c_uint8),
        ("correction", ctypes.c_uint8),
        ("emissivity", ctypes.c_uint8),
        ("transmission", ctypes.c_uint8),
        ("atmosphere", ctypes.c_int16),
        ("roi_union", IRF_SAVEDATA_T_Reserved1_Union_CG),
        ("iso_union", IRF_SAVEDATA_T_Reserved1_Union_CG_ISO),
        ("hdmi_list", ctypes.c_uint8 * 8),
        ("support_temp_mode", ctypes.c_uint16),
        ("f_support_motor_focus", ctypes.c_uint8),
        ("f_support_motor_zoom", ctypes.c_uint8),
        ("meas_dist_min", ctypes.c_uint16),
        ("meas_dist_max", ctypes.c_uint16),
        ("meas_temp", (ctypes.c_int16 * 2) * 2),
        ("meas_name", (ctypes.c_uint8 * 10) * 2),
    ]


class IRF_SAVEDATA_T_Reserved1_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved2", ctypes.c_uint8 * 256),
        ("data", IRF_SAVEDATA_T_Reserved1_Struct),
    ]


class IRF_SAVEDATA_T_Reserved3_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved3", ctypes.c_uint8 * 128),
    ]


class IRF_SAVEDATA_T_Main_Struct(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("crc", ctypes.c_uint32),
        ("ver", ctypes.c_uint8),
        ("sensor", ctypes.c_uint8),
        ("tv", ctypes.c_uint8),
        ("temp_mode", ctypes.c_uint8),
        ("id", ctypes.c_uint8),
        ("baudrate", ctypes.c_uint8),
        ("level", ctypes.c_int16),
        ("span", ctypes.c_uint16),
        ("agc", ctypes.c_uint8),
        ("invert", ctypes.c_uint8),
        ("mirror", ctypes.c_uint8),
        ("flip", ctypes.c_uint8),
        ("colorbar", ctypes.c_uint8),
        ("showinfo", ctypes.c_uint8),
        ("indicator", ctypes.c_uint8),
        ("unit", ctypes.c_uint8),
        ("dhcp", ctypes.c_uint8),
        ("color", ctypes.c_uint8),
        ("alpha", ctypes.c_uint8),
        ("zoom", ctypes.c_uint8),
        ("sharp", ctypes.c_uint8),
        ("noise", ctypes.c_uint8),
        ("nuc", ctypes.c_uint16),
        ("econt", ctypes.c_uint8),
        ("ipaddr", ctypes.c_uint32),
        ("netmask", ctypes.c_uint32),
        ("gateway", ctypes.c_uint32),
        ("dns", ctypes.c_uint32),
        ("alarm1_func", ctypes.c_uint8),
        ("alarm1_cond", ctypes.c_uint8),
        ("alarm1_value", ctypes.c_int16),
        ("alarm2_func", ctypes.c_uint8),
        ("alarm2_cond", ctypes.c_uint8),
        ("alarm2_value", ctypes.c_int16),
        ("down_filter", ctypes.c_uint8),
        ("show_center", ctypes.c_uint8),
        ("show_spot", ctypes.c_uint8),
        ("show_correction", ctypes.c_uint8),
        ("show_isotherm", ctypes.c_uint8),
        ("alarm1_duration", ctypes.c_uint8),
        ("alarm2_duration", ctypes.c_uint8),
        # ROI for CX model (placeholder)
        ("roi", (ctypes.c_uint8 * 11) * 2),
        ("f_disp_icon", ctypes.c_uint8),
        ("brightness", ctypes.c_uint8),
        ("contrast", ctypes.c_uint8),
        ("f_edge_enhance", ctypes.c_uint8),
        ("nuc_mode", ctypes.c_uint8),
        ("nuc_time", ctypes.c_uint8),
        ("nuc_thres", ctypes.c_uint16),
        ("agc_man_max", ctypes.c_uint16),
        ("agc_man_min", ctypes.c_uint16),
        ("srl_protocol", ctypes.c_uint8),
        ("scn0_l_margin", ctypes.c_uint16),
        ("scn0_r_margin", ctypes.c_uint16),
        ("scn0_t_margin", ctypes.c_uint16),
        ("scn0_b_margin", ctypes.c_uint16),
        ("scn1_l_margin", ctypes.c_uint16),
        ("scn1_r_margin", ctypes.c_uint16),
        ("scn1_t_margin", ctypes.c_uint16),
        ("scn1_b_margin", ctypes.c_uint16),
        ("hdmi_mode", ctypes.c_uint8),
        ("alarm1_type", ctypes.c_uint8),
        ("alarm1_mode", ctypes.c_uint8),
        ("alarm1_dura", ctypes.c_uint8),
        ("alarm1_remote_ctrl", ctypes.c_uint8),
        ("alarm2_type", ctypes.c_uint8),
        ("alarm2_mode", ctypes.c_uint8),
        ("alarm2_dura", ctypes.c_uint8),
        ("alarm2_remote_ctrl", ctypes.c_uint8),
        ("zero_offset", ctypes.c_int16),
        ("measure_distance", ctypes.c_uint16),
        ("nr1_strength", ctypes.c_uint8),
        ("nr2_strength", ctypes.c_uint8),
        ("ee_strength", ctypes.c_uint8),
    ]


class IRF_SAVEDATA_T_Main_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved1", ctypes.c_uint8 * 128),
        ("data", IRF_SAVEDATA_T_Main_Struct),
    ]


class IRF_SAVEDATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("main_union", IRF_SAVEDATA_T_Main_Union),
        ("reserved1_union", IRF_SAVEDATA_T_Reserved1_Union),
        ("reserved3_union", IRF_SAVEDATA_T_Reserved3_Union),
    ]


class IRF_IR_CAM_DATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("ir_image", ctypes.POINTER(ctypes.c_uint16)),
        ("image_buffer_size", ctypes.c_ulong),
        ("lpNextData", ctypes.POINTER(ctypes.c_byte)),
        ("dwSize", ctypes.c_ulong),
        ("dwPosition", ctypes.c_ulong),
        ("msg_type", ctypes.c_int),  # IRF_MESSAGE_TYPE_T is enum, so int
        ("save_data", IRF_SAVEDATA_T),
        ("fw_ver", ctypes.c_uint32),
        ("core_type", ctypes.c_uint32),
        ("h_res", ctypes.c_uint16),
        ("v_res", ctypes.c_uint16),
        ("PMSGTYPE", ctypes.c_uint16),
        ("RCODE", ctypes.c_uint16),
        ("reserved", ctypes.c_ulong * 10),
    ]

# 3. SDK 자료구조 정의 (ThermalCamSDK.h 기반)
# pragma pack(1) 지시어에 맞춰 1바이트 크기로 정렬
# 이 구조체들은 SDK와의 메모리 호환을 위해 매우 중요합니다.


class ALRMCFG(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("f_enable", ctypes.c_uint8),
        ("output_mask", ctypes.c_uint8),
        ("cond", ctypes.c_uint8),
        ("reserved", ctypes.c_uint8),
        ("temp", ctypes.c_int16),
        ("delay", ctypes.c_uint16),
    ]


class CG_ROI(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode", ctypes.c_uint8),
        ("temp_type", ctypes.c_uint8),
        ("x", ctypes.c_uint16),
        ("y", ctypes.c_uint16),
        ("w", ctypes.c_uint16),
        ("h", ctypes.c_uint16),
        ("f_corr", ctypes.c_uint8),
        ("em", ctypes.c_uint8),
        ("tr", ctypes.c_uint8),
        ("at", ctypes.c_int16),
        ("reserved", ctypes.c_uint8),
    ]


class CG_ISO(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("mode_mask", ctypes.c_uint8),
        ("tran_mask", ctypes.c_uint8),
        ("max_temp", ctypes.c_int16),
        ("min_temp", ctypes.c_int16),
        ("above_color", ctypes.c_uint32),
        ("below_color", ctypes.c_uint32),
        ("between_color", ctypes.c_uint32),
    ]


class IRF_SAVEDATA_T_Reserved1_Union_CG(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("cg_roi", CG_ROI * 10),
    ]


class IRF_SAVEDATA_T_Reserved1_Union_CG_ISO(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("cg_iso", CG_ISO * 2),
    ]


class IRF_SAVEDATA_T_Reserved1_Struct(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("limit9", ctypes.c_uint8),
        ("enable_high", ctypes.c_uint8),
        ("correction", ctypes.c_uint8),
        ("emissivity", ctypes.c_uint8),
        ("transmission", ctypes.c_uint8),
        ("atmosphere", ctypes.c_int16),
        ("roi_union", IRF_SAVEDATA_T_Reserved1_Union_CG),
        ("iso_union", IRF_SAVEDATA_T_Reserved1_Union_CG_ISO),
        ("hdmi_list", ctypes.c_uint8 * 8),
        ("support_temp_mode", ctypes.c_uint16),
        ("f_support_motor_focus", ctypes.c_uint8),
        ("f_support_motor_zoom", ctypes.c_uint8),
        ("meas_dist_min", ctypes.c_uint16),
        ("meas_dist_max", ctypes.c_uint16),
        ("meas_temp", (ctypes.c_int16 * 2) * 2),
        ("meas_name", (ctypes.c_uint8 * 10) * 2),
    ]


class IRF_SAVEDATA_T_Reserved1_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved2", ctypes.c_uint8 * 256),
        ("data", IRF_SAVEDATA_T_Reserved1_Struct),
    ]


class IRF_SAVEDATA_T_Reserved3_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved3", ctypes.c_uint8 * 128),
    ]


class IRF_SAVEDATA_T_Main_Struct(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("crc", ctypes.c_uint32),
        ("ver", ctypes.c_uint8),
        ("sensor", ctypes.c_uint8),
        ("tv", ctypes.c_uint8),
        ("temp_mode", ctypes.c_uint8),
        ("id", ctypes.c_uint8),
        ("baudrate", ctypes.c_uint8),
        ("level", ctypes.c_int16),
        ("span", ctypes.c_uint16),
        ("agc", ctypes.c_uint8),
        ("invert", ctypes.c_uint8),
        ("mirror", ctypes.c_uint8),
        ("flip", ctypes.c_uint8),
        ("colorbar", ctypes.c_uint8),
        ("showinfo", ctypes.c_uint8),
        ("indicator", ctypes.c_uint8),
        ("unit", ctypes.c_uint8),
        ("dhcp", ctypes.c_uint8),
        ("color", ctypes.c_uint8),
        ("alpha", ctypes.c_uint8),
        ("zoom", ctypes.c_uint8),
        ("sharp", ctypes.c_uint8),
        ("noise", ctypes.c_uint8),
        ("nuc", ctypes.c_uint16),
        ("econt", ctypes.c_uint8),
        ("ipaddr", ctypes.c_uint32),
        ("netmask", ctypes.c_uint32),
        ("gateway", ctypes.c_uint32),
        ("dns", ctypes.c_uint32),
        ("alarm1_func", ctypes.c_uint8),
        ("alarm1_cond", ctypes.c_uint8),
        ("alarm1_value", ctypes.c_int16),
        ("alarm2_func", ctypes.c_uint8),
        ("alarm2_cond", ctypes.c_uint8),
        ("alarm2_value", ctypes.c_int16),
        ("down_filter", ctypes.c_uint8),
        ("show_center", ctypes.c_uint8),
        ("show_spot", ctypes.c_uint8),
        ("show_correction", ctypes.c_uint8),
        ("show_isotherm", ctypes.c_uint8),
        ("alarm1_duration", ctypes.c_uint8),
        ("alarm2_duration", ctypes.c_uint8),
        # ROI for CX model (placeholder)
        ("roi", (ctypes.c_uint8 * 11) * 2),
        ("f_disp_icon", ctypes.c_uint8),
        ("brightness", ctypes.c_uint8),
        ("contrast", ctypes.c_uint8),
        ("f_edge_enhance", ctypes.c_uint8),
        ("nuc_mode", ctypes.c_uint8),
        ("nuc_time", ctypes.c_uint8),
        ("nuc_thres", ctypes.c_uint16),
        ("agc_man_max", ctypes.c_uint16),
        ("agc_man_min", ctypes.c_uint16),
        ("srl_protocol", ctypes.c_uint8),
        ("scn0_l_margin", ctypes.c_uint16),
        ("scn0_r_margin", ctypes.c_uint16),
        ("scn0_t_margin", ctypes.c_uint16),
        ("scn0_b_margin", ctypes.c_uint16),
        ("scn1_l_margin", ctypes.c_uint16),
        ("scn1_r_margin", ctypes.c_uint16),
        ("scn1_t_margin", ctypes.c_uint16),
        ("scn1_b_margin", ctypes.c_uint16),
        ("hdmi_mode", ctypes.c_uint8),
        ("alarm1_type", ctypes.c_uint8),
        ("alarm1_mode", ctypes.c_uint8),
        ("alarm1_dura", ctypes.c_uint8),
        ("alarm1_remote_ctrl", ctypes.c_uint8),
        ("alarm2_type", ctypes.c_uint8),
        ("alarm2_mode", ctypes.c_uint8),
        ("alarm2_dura", ctypes.c_uint8),
        ("alarm2_remote_ctrl", ctypes.c_uint8),
        ("zero_offset", ctypes.c_int16),
        ("measure_distance", ctypes.c_uint16),
        ("nr1_strength", ctypes.c_uint8),
        ("nr2_strength", ctypes.c_uint8),
        ("ee_strength", ctypes.c_uint8),
    ]


class IRF_SAVEDATA_T_Main_Union(ctypes.Union):
    _pack_ = 1
    _fields_ = [
        ("reserved1", ctypes.c_uint8 * 128),
        ("data", IRF_SAVEDATA_T_Main_Struct),
    ]


class IRF_SAVEDATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("main_union", IRF_SAVEDATA_T_Main_Union),
        ("reserved1_union", IRF_SAVEDATA_T_Reserved1_Union),
        ("reserved3_union", IRF_SAVEDATA_T_Reserved3_Union),
    ]


class IRF_IR_CAM_DATA_T(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("ir_image", ctypes.POINTER(ctypes.c_uint16)),
        ("image_buffer_size", ctypes.c_ulong),
        ("lpNextData", ctypes.POINTER(ctypes.c_byte)),
        ("dwSize", ctypes.c_ulong),
        ("dwPosition", ctypes.c_ulong),
        ("msg_type", ctypes.c_int),  # IRF_MESSAGE_TYPE_T is enum, so int
        ("save_data", IRF_SAVEDATA_T),
        ("fw_ver", ctypes.c_uint32),
        ("core_type", ctypes.c_uint32),
        ("h_res", ctypes.c_uint16),
        ("v_res", ctypes.c_uint16),
        ("PMSGTYPE", ctypes.c_uint16),
        ("RCODE", ctypes.c_uint16),
        ("reserved", ctypes.c_ulong * 10),
    ]

# 2. 함수 프로토타입 정의 (안정성 향상)
# 설명서나 헤더 파일(ThermalCamSDK.h)을 통해 함수의 인자와 반환 타입을 명시하는 것이 좋습니다.
# 아래는 일반적인 추정을 기반으로 하며, 실제와 다를 수 있습니다.
HANDLE = ctypes.c_void_p

# int OpenConnect(HANDLE* pHandle, HANDLE* pTimerID, LPCSTR strDestination, LPCSTR strServiceName, int nFamily, int nSocketType);
sdk.OpenConnect.argtypes = [
    ctypes.POINTER(HANDLE),
    ctypes.POINTER(HANDLE),
    ctypes.c_char_p,
    ctypes.c_char_p,
    ctypes.c_int,
    ctypes.c_int,
]
sdk.OpenConnect.restype = ctypes.c_int16

# int GetIRImages(HANDLE handle, HANDLE* pTimerID, IRF_IR_CAM_DATA_T* pData);
sdk.GetIRImages.argtypes = [HANDLE, ctypes.POINTER(HANDLE), ctypes.POINTER(IRF_IR_CAM_DATA_T)]
sdk.GetIRImages.restype = ctypes.c_int16

# int CloseConnect(HANDLE* pHandle, HANDLE TimerID);
sdk.CloseConnect.argtypes = [ctypes.POINTER(HANDLE), HANDLE]
sdk.CloseConnect.restype = ctypes.c_int16

# int16_t SendCameraMessage(HANDLE handle, HANDLE *pTimerID, IRF_MESSAGE_TYPE_T type, uint16_t PMSGTYPE, uint16_t RCODE);
sdk.SendCameraMessage.argtypes = [
    HANDLE,
    ctypes.POINTER(HANDLE),
    ctypes.c_int,
    ctypes.c_uint16,
    ctypes.c_uint16,
]
sdk.SendCameraMessage.restype = ctypes.c_int16



# 4. 카메라 연결
cam_handle = HANDLE()
timer_id = HANDLE()

print(f"{IP_ADDRESS}:{PORT} 주소로 카메라 연결을 시도합니다...")
res = sdk.OpenConnect(
    ctypes.byref(cam_handle),
    ctypes.byref(timer_id),
    IP_ADDRESS.encode("utf-8"),
    PORT.encode("utf-8"),
    2,  # AF_INET
    1,  # SOCK_STREAM
)

if res != 1:  # 성공 시 1 (IRF_NO_ERROR)을 반환
    print(f"카메라 연결 실패 (에러 코드: {res})")
    print("다음 사항을 확인하세요:")
    print(f"1. 카메라가 {IP_ADDRESS} IP 주소로 설정되었는지 확인")
    print("2. 카메라와 PC가 네트워크로 연결되어 있는지 확인 (ping 등)")
    print("3. 방화벽이 포트 15001를 차단하지 않는지 확인")
    exit()

print("카메라 연결 성공!")

# [중요] 스트림 시작 전, 카메라 정보 동기화를 위한 설정 정보 요청 (Handshaking)
print("[DEBUG] 카메라 설정 정보(CAM_DATA)를 요청합니다...")
res_cam_data = sdk.SendCameraMessage(cam_handle, ctypes.byref(timer_id), 7, 0, 0) # _IRF_REQ_CAM_DATA = 7
print(f"[DEBUG] SendCameraMessage(_IRF_REQ_CAM_DATA) 반환 값: {res_cam_data}")

if res_cam_data != 1:
    print(f"오류: 카메라 설정 정보 요청에 실패했습니다 (코드: {res_cam_data}).")
    sdk.CloseConnect(ctypes.byref(cam_handle), timer_id)
    exit()

# 카메라가 응답하고 내부적으로 준비할 시간을 줍니다.
print("[DEBUG] 카메라 응답 대기 (0.5초)...")
import time

time.sleep(0.5)

# 이제 영상 데이터 스트림 시작을 요청합니다.
print("[DEBUG] 영상 데이터 스트림 시작을 요청합니다...")
res_stream_on = sdk.SendCameraMessage(cam_handle, ctypes.byref(timer_id), 3, 0, 0)  # _IRF_STREAM_ON = 3
print(f"[DEBUG] SendCameraMessage(_IRF_STREAM_ON) 반환 값: {res_stream_on}")

if res_stream_on != 1:  # IRF_NO_ERROR
    print(f"오류: 영상 스트림 시작 요청에 실패했습니다 (코드: {res_stream_on}).")
    sdk.CloseConnect(ctypes.byref(cam_handle), timer_id)
    exit()

print("[DEBUG] 영상 스트림 시작 요청 성공. 데이터 수신 대기 루프 시작...")


# 5. 영상 수신 및 표시 루프
cam_data = IRF_IR_CAM_DATA_T()
window_name = "Thermal Camera Feed (CG320) - Press 'q' to quit"

try:
    frame_count = 0
    packet_count = 0
    print("--- 데이터 수신 루프 시작 ---")
    while True:
        res = sdk.GetIRImages(cam_handle, ctypes.byref(timer_id), ctypes.byref(cam_data))

        # GetIRImages가 무언가 수신했을 때 (성공 = 1)
        if res == 1:
            packet_count += 1
            msg_type = cam_data.msg_type
            print(f"[DEBUG] 패킷 수신! (카운트: {packet_count}, msg_type: {msg_type})")

            # 스트림 데이터(_IRF_STREAM_DATA = 5)를 수신한 경우
            if msg_type == 5:
                frame_count += 1
                if frame_count % 30 == 0:  # 30프레임마다 로그 출력
                    print(f"[SUCCESS] 영상 프레임 수신 중... (총 {frame_count} 프레임)")

                # 데이터 포인터가 유효한지 확인
                if not cam_data.ir_image:
                    continue

                # ctypes 배열을 NumPy 배열로 변환
                raw_data = np.ctypeslib.as_array(
                    cam_data.ir_image, shape=(HEIGHT * WIDTH,)
                ).copy()

                # 16비트 RAW 데이터를 8비트 그레이스케일로 정규화
                min_val = np.min(raw_data)
                max_val = np.max(raw_data)

                if max_val > min_val:
                    normalized_data = (raw_data - min_val) / (max_val - min_val) * 255
                else:
                    normalized_data = np.zeros_like(raw_data)

                gray_image = normalized_data.astype(np.uint8)
                image_frame = gray_image.reshape((HEIGHT, WIDTH))

                # OpenCV로 화면에 표시
                cv2.imshow(window_name, image_frame)

            elif msg_type == 13:  # _IRF_SYS_INFO
                print("[INFO] 카메라 시스템 정보(SYS_INFO) 패킷을 수신했습니다.")
                # 여기서 수신된 시스템 정보를 활용하는 코드를 추가할 수 있습니다.

            elif msg_type == 8:  # _IRF_CAM_DATA
                print("[INFO] 카메라 설정 정보(CAM_DATA) 패킷을 수신했습니다.")

            else:
                print(f"[INFO] 처리되지 않은 메시지 타입({msg_type})을 수신했습니다.")

        # 'q' 키를 누르면 루프 종료
        if cv2.waitKey(1) & 0xFF == ord("q"):
            print("종료 키 입력됨. 프로그램을 종료합니다.")
            break

        # 루프가 너무 빨리 도는 것을 방지 (CPU 부담 감소)
        time.sleep(0.001)
finally:
    # 6. 스트림 정지 및 연결 해제
    print("영상 데이터 스트림 정지를 요청합니다...")
    sdk.SendCameraMessage(cam_handle, ctypes.byref(timer_id), 4, 0, 0)  # _IRF_STREAM_OFF = 4
    print("카메라 연결을 해제합니다.")
    sdk.CloseConnect(ctypes.byref(cam_handle), timer_id)
    cv2.destroyAllWindows()
    print("프로그램이 종료되었습니다.")
