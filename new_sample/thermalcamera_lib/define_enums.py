from enum import Enum


class MESSAGE_TYPE(Enum):
    _IRF_NONE                       = -1
    _IRF_STREAM_ON                  = 0x21
    _IRF_STREAM_OFF                 = 0x22
    _IRF_STREAM_DATA                = 0x23
    _IRF_REQ_CAM_ENV                = 0x24
    _IRF_RES_CAM_ENV                = 0x25
    _IRF_SET_CAM_OPT                = 0x27
    _IRF_REQ_SYS_INFO               = 0x29
    _IRF_RES_SYS_INFO               = 0x30
    _IRF_REQ_FW_UPDATE              = 0x31
    _IRF_REQ_CORE_INFO              = 0xA1
    _IRF_RES_CORE_INFO              = 0xA2
    _IRF_REQ_SPIFLASH_UPDATE        = 0x81
    _IRF_UPDATE_PROGRESS            = 0x82
    _IRF_FPGA_REGIO                 = 0x91


class TEMPERATURE_MODE(Enum):
    TMODE_NONE                  = -1
    TMODE_NORMAL                = 0
    TMODE_HIGH                  = 1
    TMODE_MEDICAL               = 2

    
class ERROR_CODE(Enum):
    ERR_CAMERA_CONNECTION_TIMEOUT           = -100
    ERR_CAMERA_CONNECTION_FAILED            = -101
    ERR_PACKET_ID                           = -102
    ERR_PACKET_SIZE                         = -103
    
    ERR_CAMERA_INVALID_VER                  = -200
    ERR_CAMERA_INVALID_SENSOR               = -201


class THERMAL_FPS_TYPE(Enum):
    FPS_FULL                                = 0
    FPS_HALF                                = 1
    FPS_1SEC                                = 2


class THERMAL_UPDATE_STATUS(Enum):
    TU_COMPLETE                         = 0
    TU_OPERATION                        = 1
    TU_RECEIVE                          = 2
    TU_UPDATE                           = 3
    TU_ERROR                            = 0xFFFFFFFF



class THERMAL_AGC_TYPE(Enum):
    AGC_MANUAL                              = 0
    AGC_AUTO                                = 1


class THERMAL_AGC_PROCESS(Enum):
    AGCPRO_ADAPTIVE_HISTOGRAM               = 0
    AGCPRO_PERCENTILE                       = 1


class THERMAL_PALETTE_TYPE(Enum):
    PAL_YELLOW                      = 0
    PAL_RAINBOW                     = 1
    PAL_RAIN900                     = 2
    PAL_MIDGRAY                     = 3
    PAL_RAIN10                      = 4
    PAL_MIDGREEN                    = 5
    PAL_MEDICAL                     = 6
    PAL_IRON10                      = 7
    PAL_IRON                        = 8
    PAL_GRAYRED                     = 9
    PAL_GRAY10                      = 10
    PAL_GRAY                        = 11
    PAL_GLOWBOW                     = 12
    
    @property
    def pretty(self):
        if self.name.startswith("PAL_"):
            return self.name[4:].capitalize()
        return self.name.capitalize()