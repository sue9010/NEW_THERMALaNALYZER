import ctypes
from ctypes import c_void_p, c_char_p, c_int, c_short, POINTER

AF_INET = 2
SOCK_STREAM = 1

class ThermalCam:
    def __init__(self, dll_path="SDK/SDK/bin/ThermalCamSDK.dll"):
        # Load the camera SDK DLL
        # Use WinDLL when available (Windows). Fallback to CDLL for other OSes
        loader = getattr(ctypes, 'WinDLL', ctypes.CDLL)
        self.lib = loader(dll_path)

        # Define argument and return types for functions we use
        self.lib.OpenConnect.argtypes = [POINTER(c_void_p), POINTER(c_void_p), c_char_p, c_char_p, c_int, c_int]
        self.lib.OpenConnect.restype = c_short

        self.lib.CloseConnect.argtypes = [POINTER(c_void_p), c_void_p]
        self.lib.CloseConnect.restype = c_short

        self.handle = c_void_p()
        self.timer_id = c_void_p()

    def connect(self, ip="192.168.0.101", port=15001):
        result = self.lib.OpenConnect(
            ctypes.byref(self.handle),
            ctypes.byref(self.timer_id),
            ip.encode('ascii'),
            str(port).encode('ascii'),
            AF_INET,
            SOCK_STREAM,
        )
        if result != 0:
            raise RuntimeError(f"OpenConnect failed with code {result}")
        print("Connected to camera", ip, port)

    def disconnect(self):
        if self.handle:
            self.lib.CloseConnect(ctypes.byref(self.handle), self.timer_id)
            print("Disconnected")

if __name__ == "__main__":
    cam = ThermalCam()
    cam.connect()
    cam.disconnect()
