import numpy as np
from collections import deque

HIST_BINS = 1024
RAW_MIN_FIXED = 2000
RAW_MAX_FIXED = 14000
ADAPTIVE_HIST_COVERATE_PERCENT = 0.95


class AdaptiveAGC:
    def __init__(self, queue_size=10):
        self.low_queue = deque(maxlen=queue_size)
        self.high_queue = deque(maxlen=queue_size)
        self.prev_min = None
        self.prev_max = None
    
    
    def clear(self):
        self.low_queue.clear()
        self.high_queue.clear()
        self.prev_min = None
        self.prev_max = None
        
    
    def adaptiveHistogramAGC(self,
                             src,
                             gray,
                             adaptive_per: float = 0.97,
                             alpha: float = 0.6):
        
        hist, bin_edges = np.histogram(src, bins=HIST_BINS)
        totpx = src.size
        limit_px = totpx * adaptive_per
        
        best_range_size = float('inf')
        best_start_val = RAW_MIN_FIXED
        best_end_val = RAW_MAX_FIXED
        
        left_idx = 0
        curr_range = 0
        frm_min = np.min(src)
        frm_max = np.max(src)
        
        for right_idx in range(HIST_BINS):
            curr_range += hist[right_idx]
            
            while limit_px <= curr_range:
                start_val = bin_edges[left_idx]
                end_val = bin_edges[right_idx + 1]
                curr_range_size = end_val - start_val
                
                if curr_range_size < best_range_size:
                    best_range_size = curr_range_size
                    best_start_val = start_val
                    best_end_val = end_val
                curr_range -= hist[left_idx]
                left_idx += 1
        
        
        if best_range_size == float('inf'):
            calc_min = max(frm_min, RAW_MIN_FIXED)
            calc_max = min(frm_max, RAW_MAX_FIXED)
        else:
            calc_min = best_start_val
            calc_max = best_end_val
        
        diff_min = calc_min - frm_min
        diff_max = frm_max - calc_max
        
        diff_frm = frm_max - frm_min
        diff_calc = calc_max - calc_min
        
        ratio = diff_frm / diff_calc
        offset_max = diff_max / ratio
        offset_min = diff_min / ratio
     
        dst_min = calc_min - offset_min
        dst_max = calc_max + offset_max
        
        agc_min = dst_min
        agc_max = dst_max
        
        self.low_queue.append(agc_min)
        self.high_queue.append(agc_max)
        
        if self.prev_min is None:
            self.prev_min = agc_min
            self.prev_max = agc_max
        else:
            agc_min = np.mean(self.low_queue)
            agc_max = np.mean(self.high_queue)
            agc_min = self.prev_min * (1 - alpha) + agc_min * alpha
            agc_max = self.prev_max * (1 - alpha) + agc_max * alpha
        
        self.prev_min = agc_min
        self.prev_max = agc_max
        range_val = max(1.0, agc_max - agc_min)
        
        scaled = (src.astype(np.float32) - agc_min) / range_val * 255.0
        gray[:] = np.clip(scaled, 0, 255).astype(np.uint8)
        return agc_min, agc_max
    
    
    
    def percentileStretching(self, src, gray):
        alpha = 0.5
        hist, bin_edges = np.histogram(src, bins=256)
        size = src.size
        
        val1 = int(size * 0.01)
        val2 = int(size * 0.0001)
        sum = 0
        frm_min = np.min(src)
        frm_max = np.max(src)
        agc_min = 0
        agc_max = 0
        
        for idx, value in enumerate(hist):
            if value < val2:
                continue
            sum += value
            if sum < val1:
                continue
            agc_min = bin_edges[idx]
            break
        
        sum = 0
        for idx, value in enumerate(reversed(hist)):
            if value < val2 and sum == 0:
                continue
            
            sum += value
            if sum < val1:
                continue
            agc_max = bin_edges[len(bin_edges) - idx]
            break
        
        if src.dtype in [np.float32, np.float64]:
            agc_min -= 1.5
            agc_max += 1.5
        elif src.dtype == np.uint16:
            if 300 < agc_min - frm_min:
                agc_min -= 300
            else:
                agc_min -= (agc_min - frm_min)
            
            if 300 < frm_max - agc_max:
                agc_max += 300
            else:
                agc_max += (frm_max - agc_max)

        self.low_queue.append(agc_min)
        self.high_queue.append(agc_max)

        if self.prev_min is None:
            self.prev_min = agc_min
            self.prev_max = agc_max
        else:
            agc_min = np.mean(self.low_queue)
            agc_max = np.mean(self.high_queue)
            agc_min = self.prev_min * (1 - alpha) + agc_min * alpha
            agc_max = self.prev_max * (1 - alpha) + agc_max * alpha
        
        self.prev_min = agc_min
        self.prev_max = agc_max
        range_value = max(1.0, agc_max - agc_min)
        
        scaled = (src.astype(np.float32) - agc_min) / range_value * 255.0
        gray[:] = np.clip(scaled, 0, 255).astype(np.uint8)
        return agc_min, agc_max
        
        
        
        