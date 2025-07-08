import struct
from dataclasses import dataclass, field, fields
from struct import pack, unpack
from typing import Optional, List
from .define_constants import THERMAL_PACKET_ID



C_TYPE_MAP = {
    'int8_t':   ('b', 1),
    'uint8_t':  ('B', 1),
    'int16_t':  ('h', 2),
    'uint16_t': ('H', 2),
    'int32_t':  ('i', 4),
    'uint32_t': ('I', 4),
    'int64_t':  ('q', 8),
    'uint64_t': ('Q', 8),
    'float':    ('f', 4),
    'double':   ('d', 8),
    'bytes':    ('s', 1),
}


class BaseStruct:
    @classmethod
    def get_size(cls) -> int:
        fmt = cls.get_format()
        return cls.calcuate_size(fmt)
    
    @classmethod
    def get_format(cls, processed=None) -> str:
        if processed is None:
            processed = set()
            processed.add(cls)
            
        format_parts = []
        
        for field_name, field_value in cls.__dataclass_fields__.items():
            c_type = field_value.metadata.get('ctype', None)
            field_type = field_value.type
            
            if isinstance(c_type, tuple):
                base_type = c_type[0]
                shape = c_type[1:]
                
                if shape:
                    total_size = 1
                    for dim in shape:
                        total_size *= dim
                    
                    if base_type == 'bytes':
                        if len(shape) == 1:
                            format_parts.append(f"{shape[0]}s")
                        else:
                            format_parts.extend(f"{shape[1]}s" for _ in range(shape[0]))
                    else:
                        format_parts.append(f"{total_size}{C_TYPE_MAP.get(base_type)[0]}")
                else:
                    format_parts.append(f"{C_TYPE_MAP.get(base_type)[0]}")
            elif c_type is None:
                if isinstance(field_type, type) and issubclass(field_type, BaseStruct):
                    format_parts.append(field_type.get_format(processed))
                
                if hasattr(field_type, '__origin__') and field_type.__origin__ is list:
                    item_type = field_type.__args__[0]
                    if isinstance(item_type, type) and issubclass(item_type, BaseStruct):
                        if field_value.default_factory:
                            list_length = len(field_value.default_factory())
                            for _ in range(list_length):
                                format_parts.append(item_type.get_format(processed))
            else:
                format_parts.append(f"{C_TYPE_MAP.get(c_type)[0]}")
                
        return f"<{''.join(format_parts)}" if cls in processed else ''.join(format_parts)

    def to_bytes(self) -> bytes:
        fmt = self.get_format()
        src = self.to_tuple()
        return pack(fmt, *src)

    @classmethod
    def from_bytes(cls, data: bytes, index: List = None, unpacked_data = None):
        if index is None:
            index = [0]
            
        field_values = {}
        if unpacked_data is None:
            fmt = cls.get_format()
            unpacked = unpack(fmt, data)
        else:
            unpacked = unpacked_data
        
        for field_name, field_value in cls.__dataclass_fields__.items():
            c_type = field_value.metadata.get('ctype')
            field_type = field_value.type
            
            if isinstance(c_type, tuple):
                base_type = c_type[0]
                shape = c_type[1:]
                
                if base_type == 'bytes':
                    if len(shape) == 1:
                        field_values[field_name] = unpacked[index[0]].rstrip(b'\x00')
                        index[0] += 1
                    else:
                        field_values[field_name] = [val.rstrip(b'\x00') for val in unpacked[index[0] : index[0] + shape[0]]]
                        index[0] += shape[0]
                else:
                    total_count = 1
                    for s in shape:
                        total_count *= s
                    flat_list = list(unpacked[index[0] : index[0] + total_count])
                    if len(shape) == 1:
                        field_values[field_name] = flat_list
                    elif len(shape) == 2:
                        field_values[field_name] = [flat_list[i : i + shape[1]] for i in range(0, total_count, shape[1])]
                    index[0] += total_count
            elif c_type is None:
                if isinstance(field_type, type) and issubclass(field_type, BaseStruct):
                    field_values[field_name] = field_type.from_bytes(data, index, unpacked)
                elif hasattr(field_type, '__origin__') and field_type.__origin__ is list:
                    field_values[field_name] = [item.from_bytes(data, index, unpacked) for item in field_value.default_factory()]
            else:
                field_values[field_name] = unpacked[index[0]]
                index[0] += 1
        
        return cls(**field_values)


    def to_tuple(self):
        result = []
        for field_value in fields(self):
            value = getattr(self, field_value.name)
            c_type = field_value.metadata.get('ctype')
            field_type = field_value.type
            
            if isinstance(value, list) and c_type:
                if isinstance(value[0], list):
                    for sublist in value:
                        result.extend(sublist)
                else:
                    result.extend(value)
            elif c_type is None:
                if isinstance(field_type, type) and issubclass(field_type, BaseStruct):
                    result.extend(value.to_tuple())
                elif hasattr(field_type, '__origin__') and field_type.__origin__ is list:
                    for item in value:
                        result.extend(item.to_tuple())
            else:
                result.append(value)
        return tuple(result)

    def calcuate_size(format_str: str) -> int:
        total_size = 0
        i = 1
        while i < len(format_str):
            char = format_str[i]
            char_size = 0
            if char in 'bBhHiIqQfds':
                j = i - 1
                while 1 <= j:
                    if format_str[j].isdigit():
                        j -= 1
                    else:
                        break
                if j + 1 != i:
                    char_size = int(format_str[j+1:i])
                    total_size += (struct.calcsize(char) * char_size)
                else:
                    total_size += struct.calcsize(char)
            i += 1
        return total_size


@dataclass
class TPKT_Header(BaseStruct):
    ID: Optional[int] = field(default=THERMAL_PACKET_ID, metadata={'ctype': 'uint32_t'})
    Length: Optional[int] = field(default=16, metadata={'ctype': 'uint32_t'})
    Type: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    Reserved: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 6)})


@dataclass
class TPKT_SetParameter(BaseStruct):
    Command: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    Value1: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    Value2: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    Value3: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    Value4: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    Reserved: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 16)})
    

@dataclass
class TPKT_GetResolution(BaseStruct):
    core_type: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    h_res: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    v_res: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    Reserved: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 16)})


@dataclass
class TPKT_CGROI(BaseStruct):
    mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    temp_type: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    x: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    y: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    w: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    h: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    f_corr: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    em: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    tr: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    at: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    reserved: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    
@dataclass
class TPKT_CGISO(BaseStruct):
    mode_mask: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    tran_mask: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    max_temp: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    min_temp: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    above_color: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    below_color: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    between_color: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})


@dataclass
class TPKT_AlarmConfig(BaseStruct):
    f_enable: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    output_mask: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    cond: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    reserved: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    temp: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    delay: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})


@dataclass
class TPKT_CameraSystemInfo(BaseStruct):
    MAC: Optional[int] = field(default=0, metadata={'ctype': ('uint8_t', 6)})
    fw_ver: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    cpu_ver: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    fpga_ver: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    engine_ver: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 42)})


@dataclass
class TPKT_CameraEnv(BaseStruct):
    ############################################################################
    # First Section 128Bytes
    crc: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    ver: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    sensor: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    tv: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    temp_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    id: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    baudrate: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    level: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    span: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    agc: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    invert: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    mirror: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    flip: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    colorbar: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    showinfo: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    indicator: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    unit: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    dhcp: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    color: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alpha: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    zoom: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    sharp: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    noise: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    nuc: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    econt: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    ipaddr: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    netmask: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    gateway: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    dns: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    
    alarm1_func: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_cond: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_value: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    alarm2_func: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_cond: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_value: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    down_filter: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    show_center: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    show_spot: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    show_correction: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    show_isotherm: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_duration: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_duration: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    roi1_flag: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    roi1_x1: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi1_y1: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi1_x2: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi1_y2: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi2_flag: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    roi2_x1: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi2_y1: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi2_x2: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    roi2_y2: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    f_disp_icon: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    brightness: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    contrast: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    f_edge_enhance: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    nuc_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    nuc_time: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    nuc_thres: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    agc_man_max: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    agc_man_min: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    srl_protocol: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    scn0_l_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn0_r_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn0_t_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn0_b_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    scn1_l_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn1_r_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn1_t_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    scn1_b_margin: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    hdmi_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    alarm1_type: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_dura: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm1_remote_ctrl: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    alarm2_type: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_dura: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    alarm2_remote_ctrl: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    zero_offset: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    measure_distance: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    nr1_strength: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    nr2_strength: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    ee_strength: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    reserved1: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 3)})
    ############################################################################
    
    
    ############################################################################
    ## Second Section 256 Bytes
    limit9: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    enable_high: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    correction: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    emissivity: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    transmission: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    atmosphere: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    
    cg_roi: List[TPKT_CGROI] = field(default_factory=lambda: [TPKT_CGROI() for _ in range(10)])
    cg_iso: List[TPKT_CGISO] = field(default_factory=lambda: [TPKT_CGISO() for _ in range(2)])
    
    hdmi_list: List[int] = field(default_factory=lambda: [0] * 8, metadata={'ctype': ('uint8_t', 8)})
    support_temp_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    f_support_motor_focus: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    f_support_motor_zoom: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    
    meas_dist_min: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    meas_dist_max: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    meas_temp: List[List[int]] = field(default_factory=lambda: [[0, 0], [0, 0]], metadata={'ctype': ('int16_t', 2, 2)})
    meas_level: List[List[int]] = field(default_factory=lambda: [[0, 0], [0, 0]], metadata={'ctype': ('uint16_t', 2, 2)})
    
    meas_name: List[bytes] = field(default_factory=lambda: [b'\x00' * 10, b'\x00' * 10], metadata={'ctype': ('bytes', 2, 10)})
    reserved2: Optional[bytes] = field(default=b'\x00', metadata={'ctype': 'bytes'})
    ############################################################################

    ############################################################################
    ## Third Section 128 Bytes
    alarm_max_temp: TPKT_AlarmConfig = field(default_factory=TPKT_AlarmConfig)
    alarm_min_temp: TPKT_AlarmConfig = field(default_factory=TPKT_AlarmConfig)
    alarm_avg_temp: TPKT_AlarmConfig = field(default_factory=TPKT_AlarmConfig)
    alarm_ctr_temp: TPKT_AlarmConfig = field(default_factory=TPKT_AlarmConfig)
    alarm_roi_temp: List[TPKT_AlarmConfig] = field(default_factory=lambda: [TPKT_AlarmConfig() for _ in range(10)])
    reserved3: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 16)})
    ############################################################################


@dataclass
class TPKT_ROIIndicatorPos(BaseStruct):
    max_x: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    max_y: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    min_x: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    min_y: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})


@dataclass
class TPKT_ROIIndicatorLvl(BaseStruct):
    max_lvl: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    min_lvl: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})


@dataclass
class TPKT_RawDataTail(BaseStruct):
    id: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved00: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved01: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved02: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved03: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved04: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    
    radiometric: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    radiometric_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    temp_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    distance_sel: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    user_corr: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    reserved05: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved06: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    reserved07: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    reserved08: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    reserved09: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    reserved: Optional[int] = field(default=0, metadata={'ctype': ('uint16_t', 9)})
    
    roi_pos: List[TPKT_ROIIndicatorPos] = field(default_factory=lambda: [TPKT_ROIIndicatorPos() for _ in range(10)])
    frame_pos: TPKT_ROIIndicatorPos = field(default_factory=TPKT_ROIIndicatorPos)
    roi_lvl: List[TPKT_ROIIndicatorLvl] = field(default_factory=lambda: [TPKT_ROIIndicatorLvl() for _ in range(10)])
    frame_lvl: TPKT_ROIIndicatorLvl = field(default_factory=TPKT_ROIIndicatorLvl)
    roi_sum: List[int] = field(default_factory=lambda: [0] * 10, metadata={'ctype': ('uint32_t', 10)})
    frame_sum: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    roi_tot_px: List[int] = field(default_factory=lambda: [0] * 10, metadata={'ctype': ('uint32_t', 10)})
    frame_tot_px: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    
    ctr_avg: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    reserved10: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved11: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    frame_cnt: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    waittime: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    dwtick: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    sysTime: List[int] = field(default_factory=lambda: [0] * 8, metadata={'ctype': ('uint16_t', 8)})
    
    QuadChReserved0: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    reserved12: List[int] = field(default_factory=lambda: [0] * 158, metadata={'ctype': ('uint16_t', 158)})
    
    time_stamp: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    f_stab_sts: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    dac_gain: Optional[int] = field(default=0, metadata={'ctype': 'int16_t'})
    dac_offset: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    env_ntc_vol: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    tec_ntc_vol: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    tec_org_vol: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    tec_set_vol: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    
    reserved13: List[int] = field(default_factory=lambda: [0] * 22, metadata={'ctype': ('uint16_t', 22)})
    
    sonar_upper: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    sonar_lower: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    sonar_pulse_cnt: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})


@dataclass
class TPKT_TempCorrection(BaseStruct):
    emissivity: Optional[float] = field(default=0.0, metadata={'ctype': 'double'})
    atmTemp: Optional[float] = field(default=0.0, metadata={'ctype': 'double'})
    atmTrans: Optional[float] = field(default=0.0, metadata={'ctype': 'double'})
    zero_offset: Optional[float] = field(default=0.0, metadata={'ctype': 'double'})


@dataclass
class TPKT_RecFileHeader(BaseStruct):
    ver: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    width: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    height: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    temp_corr: TPKT_TempCorrection = field(default_factory=TPKT_TempCorrection)


@dataclass
class TPKT_RecDataHeader(BaseStruct):
    temp_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint8_t'})
    date_time: List[int] = field(default_factory=lambda: [0] * 6, metadata={'ctype': ('int32_t', 6)})
    total_frame: Optional[int] = field(default=0, metadata={'ctype': 'int32_t'})
    camera_env: TPKT_CameraEnv = field(default_factory=TPKT_CameraEnv)
    reserved: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 460)})


@dataclass
class TPKT_BinaryUpdateResult(BaseStruct):
    status: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    type: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    op_mode: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    op_addr: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})


@dataclass
class TPKT_UpdateResult(BaseStruct):
    result: TPKT_BinaryUpdateResult = field(default_factory=TPKT_BinaryUpdateResult)
    msg_type: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    pmsgtype: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    rcode: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})


@dataclass
class TPKT_FirmwareUpdate(BaseStruct):
    fw_ver: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    fw_size: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    total_block: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    block_no: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    file_block: Optional[bytes] = field(default=b'\x00', metadata={'ctype': ('bytes', 1024)})


@dataclass
class TPKT_SPIFlashUpdate(BaseStruct):
    spi_num: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    write_address: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    total_size: Optional[int] = field(default=0, metadata={'ctype': 'uint32_t'})
    size: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    crc: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    total_block: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    block_no: Optional[int] = field(default=0, metadata={'ctype': 'uint16_t'})
    file_block: Optional[bytes] = field(default=b'\xFF', metadata={'ctype': ('bytes', 2048)})