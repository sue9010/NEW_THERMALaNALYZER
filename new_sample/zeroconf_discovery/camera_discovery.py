import asyncio
from zeroconf import Zeroconf, ServiceBrowser, ServiceListener

class CameraListener(ServiceListener):
    def __init__(self):
        self.cameras = []
    
    def convert_properties(self, properties):
        return { key.decode(): value.decode() for key, value in properties.items() }
    
    def add_service(self, zc, type_, name):
        info = zc.get_service_info(type_, name)
        if info.properties:
            camera = self.convert_properties(info.properties)
            self.cameras.append(camera)
    
    def update_service(self, zc, type_, name):
        pass
    
    def remove_service(self, zc, type_, name):
        pass


async def _async_discover_cameras(service_type: str, timeout: int):
    zeroconf = Zeroconf()
    listener = CameraListener()
    browser = ServiceBrowser(zeroconf, service_type, listener)
    print(f"Discovering cameras for {timeout} seconds...")
    await asyncio.sleep(timeout)
    
    browser.cancel()
    zeroconf.close()
    
    return listener.cameras


async def discover_camera(service_type: str, timeout: int):
    return await _async_discover_cameras(service_type, timeout)