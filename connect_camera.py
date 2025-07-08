import asyncio
import socket
import time

import numpy as np

from new_sample.thermalcamera_lib.define_constants import (
    THERMAL_PACKET_HEADER_SIZE,
    THERMAL_PACKET_ID,
)
from new_sample.thermalcamera_lib.define_enums import MESSAGE_TYPE
from new_sample.thermalcamera_lib.define_protocol import (
    TPKT_CameraEnv,
    TPKT_CameraSystemInfo,
    TPKT_GetResolution,
    TPKT_Header,
    TPKT_RawDataTail,
)


class ThermalCam:
    def __init__(self, ip="192.168.0.101", port=15001):
        self.ip = ip
        self.port = port
        self.reader = None
        self.writer = None
        self.is_connected = False
        self.camera_config = None
        self.system_info = None
        self.image_dimensions = {"width": 0, "height": 0}
        self.received_data = asyncio.Queue()
        self.listener_task = None

    async def _listener(self):
        try:
            while self.is_connected:
                header_data = await self.reader.readexactly(THERMAL_PACKET_HEADER_SIZE)
                header = TPKT_Header.from_bytes(header_data)

                if header.ID != THERMAL_PACKET_ID:
                    print(f"Invalid packet ID: {header.ID}")
                    continue

                body_size = header.Length - THERMAL_PACKET_HEADER_SIZE
                body_data = b""
                if body_size > 0:
                    body_data = await self.reader.readexactly(body_size)

                await self.received_data.put((header, body_data))

        except (asyncio.IncompleteReadError, ConnectionResetError) as e:
            print(f"Connection lost: {e}")
            self.is_connected = False
        except Exception as e:
            print(f"An error occurred in the listener: {e}")
            self.is_connected = False

    async def connect(self):
        try:
            self.reader, self.writer = await asyncio.wait_for(
                asyncio.open_connection(self.ip, self.port), timeout=5
            )
            self.is_connected = True
            self.listener_task = asyncio.create_task(self._listener())
            print(f"Connected to {self.ip}:{self.port}")

            # --- Camera Initialization Sequence ---
            print("Requesting Core Info...")
            await self._send_message(MESSAGE_TYPE._IRF_REQ_CORE_INFO)
            header, body = await self._wait_for_response(
                MESSAGE_TYPE._IRF_RES_CORE_INFO
            )
            core_info = TPKT_GetResolution.from_bytes(body)
            self.image_dimensions["width"] = core_info.h_res
            self.image_dimensions["height"] = core_info.v_res
            print(f"Received Core Info: {self.image_dimensions}")

            print("Requesting System Info...")
            await self._send_message(MESSAGE_TYPE._IRF_REQ_SYS_INFO)
            header, body = await self._wait_for_response(MESSAGE_TYPE._IRF_RES_SYS_INFO)
            self.system_info = TPKT_CameraSystemInfo.from_bytes(body)
            print("Received System Info.")

            print("Requesting Camera Env...")
            await self._send_message(MESSAGE_TYPE._IRF_REQ_CAM_ENV)
            header, body = await self._wait_for_response(MESSAGE_TYPE._IRF_RES_CAM_ENV)
            self.camera_config = TPKT_CameraEnv.from_bytes(body)
            print("Received Camera Env.")

        except (asyncio.TimeoutError, ConnectionRefusedError) as e:
            raise RuntimeError(f"Connection failed: {e}")

    async def disconnect(self):
        if self.listener_task:
            self.listener_task.cancel()
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()
        self.is_connected = False
        print("Disconnected.")

    async def _send_message(self, msg_type, body=b""):
        header = TPKT_Header(
            Type=msg_type.value, Length=THERMAL_PACKET_HEADER_SIZE + len(body)
        )
        self.writer.write(header.to_bytes() + body)
        await self.writer.drain()

    async def _wait_for_response(self, target_msg_type, timeout=5):
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                header, body = await asyncio.wait_for(
                    self.received_data.get(), timeout=1
                )
                if header.Type == target_msg_type.value:
                    return header, body
                else:
                    print(f"Ignoring message type: {header.Type}")
            except asyncio.TimeoutError:
                continue
        raise RuntimeError(f"Timed out waiting for message type {target_msg_type.name}")

    def get_firmware_version(self):
        if not self.system_info:
            raise RuntimeError("System info not available.")
        fw_ver = self.system_info.fw_ver
        major = (fw_ver >> 24) & 0xFF
        minor = (fw_ver >> 16) & 0xFF
        patch = fw_ver & 0xFFFF
        return f"{major}.{minor}.{patch}"

    def get_camera_info(self):
        if not self.camera_config:
            raise RuntimeError("Camera config not available.")

        sensor_map = {0x00: "CX320", 0x01: "CX640", 0x20: "CG QVGA", 0x21: "CG VGA"}
        temp_mode_map = {0: "Normal", 1: "High", 2: "Medical"}

        info = {
            "Setup Data Version": self.camera_config.ver,
            "Sensor Type": sensor_map.get(
                self.camera_config.sensor, f"Unknown ({self.camera_config.sensor})"
            ),
            "Temp Mode": temp_mode_map.get(
                self.camera_config.temp_mode,
                f"Unknown ({self.camera_config.temp_mode})",
            ),
        }
        return info

    async def get_thermal_image(self):
        print("\nAttempting to get thermal image...")
        await self._send_message(MESSAGE_TYPE._IRF_STREAM_ON)

        header, body = await self._wait_for_response(MESSAGE_TYPE._IRF_STREAM_DATA)

        await self._send_message(MESSAGE_TYPE._IRF_STREAM_OFF)
        print("Stream stopped.")

        width = self.image_dimensions["width"]
        height = self.image_dimensions["height"]

        # The body of STREAM_DATA contains the raw image data + tail
        tail_size = TPKT_RawDataTail.get_size()
        image_size = width * height * 2  # 16-bit data

        if len(body) < image_size:
            print(
                f"Warning: Incomplete image frame received. Expected {image_size}, got {len(body)}"
            )
            return None, width, height

        image_data = np.frombuffer(body[:image_size], dtype=np.uint16)
        # You might need to reshape it: image_data = image_data.reshape((height, width))

        print(
            f"Successfully received thermal image data: {len(body)} bytes ({width}x{height})"
        )
        return image_data, width, height


async def main():
    cam = ThermalCam()
    try:
        await cam.connect()

        fw_version = cam.get_firmware_version()
        print(f"\nFirmware Version: {fw_version}")

        cam_info = cam.get_camera_info()
        print("\nCamera Info:")
        for key, value in cam_info.items():
            print(f"  {key}: {value}")

        image_data, img_width, img_height = await cam.get_thermal_image()
        if image_data is not None:
            print(f"Image Data Details:")
            print(f"  Type: {type(image_data)}")
            print(f"  Shape: {image_data.shape}")
            print(f"  Dtype: {image_data.dtype}")
            print(f"  Min Value: {image_data.min()}")
            print(f"  Max Value: {image_data.max()}")
            print(f"  First 10 values: {image_data.flatten()[:10]}")

            # Example: Save raw thermal data to a CSV file
            csv_filename = "thermal_image_raw.csv"
            np.savetxt(
                csv_filename,
                image_data.reshape(img_height, img_width),
                delimiter=",",
                fmt="%d",
            )
            print(f"Raw thermal data saved to {csv_filename}")

            # Example: Convert to 8-bit grayscale image and save (requires OpenCV)
            # import cv2
            # normalized_image = cv2.normalize(image_data, None, 0, 255, cv2.NORM_MINMAX)
            # normalized_image = np.uint8(normalized_image)
            # cv2.imwrite("thermal_image_grayscale.png", normalized_image)
            # print(f"Grayscale image saved to thermal_image_grayscale.png")

    except RuntimeError as e:
        print(f"An error occurred: {e}")
    finally:
        await cam.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
