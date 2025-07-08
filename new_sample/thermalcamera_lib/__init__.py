from .thermalcamera import ThermalCamera, ThermalReturn
from .adaptiveAGC import AdaptiveAGC
# from .thermalrecord import ThermalRecord
# from .thermalreader import ThermalReader, PLAY_TYPE
from .define_enums import (MESSAGE_TYPE,
                           TEMPERATURE_MODE,
                           ERROR_CODE,
                           THERMAL_FPS_TYPE,
                           THERMAL_UPDATE_STATUS,
                           THERMAL_PALETTE_TYPE)
from .define_protocol import (TPKT_Header,
                              TPKT_SetParameter,
                              TPKT_GetResolution,
                              TPKT_CameraEnv,
                              TPKT_RawDataTail,
                              TPKT_TempCorrection,
                              TPKT_RecFileHeader,
                              TPKT_RecDataHeader,
                              TPKT_UpdateResult,
                              TPKT_FirmwareUpdate,
                              TPKT_SPIFlashUpdate,)
from .define_colormap import _PALETTE_NAMES

__all__ = [
    "ThermalCamera",
    "ThermalReturn",
    "AdaptiveAGC",
    "MESSAGE_TYPE",
    "TEMPERATURE_MODE",
    "ERROR_CODE",
    "THERMAL_FPS_TYPE",
    "THERMAL_UPDATE_STATUS",
    "THERMAL_PALETTE_TYPE",
    "TPKT_Header",
    "TPKT_SetParameter",
    "TPKT_GetResolution",
    "TPKT_CameraEnv",
    "TPKT_RawDataTail",
    "TPKT_TempCorrection",
    "TPKT_RecFileHeader",
    "TPKT_RecDataHeader",
    "TPKT_UpdateResult",
    "TPKT_FirmwareUpdate",
    "TPKT_SPIFlashUpdate",
    "_PALETTE_NAMES",
]