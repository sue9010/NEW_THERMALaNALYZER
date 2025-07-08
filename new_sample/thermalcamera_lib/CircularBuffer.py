import numpy as np
import threading
from . import define_protocol as dp


class CircularBuffer:
    def __init__(self, buff_cnt, size, dtype):
        self.buff_cnt = buff_cnt
        self.frames = [np.zeros(size, dtype=dtype) for _ in range(buff_cnt)]
        self.widx = 0
        self.ridx = -1
        self.size = size
        self.lock = threading.Lock()

    def add_data(self, frame):
        with self.lock:
            self.frames[self.widx] = frame.copy()
            self.ridx = self.widx
            self.widx = (self.widx + 1) % self.buff_cnt

    def get_data(self):
        with self.lock:
            if self.ridx == -1:
                return None
            return self.frames[self.ridx]

    def clear(self):
        with self.lock:
            self.ridx = -1
            self.widx = 0
            for f in self.frames:
                f[:] = 0




class FrameBuffer:
    def __init__(self, width, height):
        self.gray       = np.zeros((height, width), dtype=np.uint8)
        self.gray_scale = np.zeros(256, dtype=np.float32)
        self.temp_buff  = np.zeros((height, width), dtype=np.float32)
        self.level      = np.zeros((height, width), dtype=np.uint16)
        self.image      = np.zeros((height, width, 3), dtype=np.uint8)
        self.raw        = np.zeros(((height + 2), width), dtype=np.uint16)
        self.tail       = dp.TPKT_RawDataTail()
        self.agc_min    = 0.0
        self.agc_max    = 0.0
    
    def clear(self):
        self.gray.fill(0)
        self.gray_scale.fill(0)
        self.temp_buff.fill(0)
        self.level.fill(0)
        self.image.fill(0)
        self.raw.fill(0)
        self.tail = dp.TPKT_RawDataTail()
        self.agc_min = 0.0
        self.agc_max = 0.0


class RingBuffer:
    def __init__(self, width, height, buff_count):
        self.buff_count = buff_count
        self.width      = width
        self.height     = height
        self.buffers    = [FrameBuffer(width, height) for _ in range(buff_count)]
        self.idx        = -1
    
    
    def next(self):
        if self.idx == -1:
            self.idx = 0
            return self.idx, self.buffers[self.idx]
        else:
            self.idx = (self.idx + 1) % self.buff_count
            return self.idx, self.buffers[self.idx]
    
    
    def get(self, idx):
        return self.buffers[idx]
    
    
    def clear(self):
        [buf.clear() for buf in self.buffers]
        self.idx = 0