import asyncio
import socket
import time

import cv2  # Add OpenCV import
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
        self.sr_net = None # Initialize SR DNN model
        self.sr_scale_factor = 2 # SRCNN upscaling factor (e.g., 2x, 3x, 4x)
        self.sr_method = "ESPCN" # Default SR method: "ESPCN" or "Bicubic"
        self.received_data = asyncio.Queue()
        self.listener_task = None
        self.temp_lookup_table = None  # Initialize temp_lookup_table
        self.save_sr_debug_images = False # Flag to save SR debug images
        self.canny_threshold1 = 50
        self.canny_threshold2 = 150
        self.agc_mode = "auto"  # 'auto' or 'manual'
        self.manual_agc_min = 20.0
        self.manual_agc_max = 40.0
        self.edge_detection_enabled = True
        self.edge_mode = "auto"  # 'auto' or 'manual'
        self.ema_lower_threshold = 0.0
        self.ema_upper_threshold = 0.0
        self.ema_alpha = 0.001  # Smoothing factor (0.0 to 1.0). Lower = more smoothing. edge 변경 속도 제어
        self.frame_count_for_ema = 0  # To handle the first frame
        self.edge_color = (0, 255, 0)  # Default green, BGR
        self.edge_thickness = 1  # Default 1 pixel
        self.max_edge_percentage = 0.05  # Max 5% of pixels can be edges
        self.threshold_adjustment_step = 5  # How much to adjust thresholds by

        # Bilateral Filter parameters
        self.bilateral_filter_enabled = False
        self.bilateral_d = 9  # Diameter of each pixel neighborhood
        self.bilateral_sigma_color = 75  # Filter sigma in the color space
        self.bilateral_sigma_space = 75  # Filter sigma in the coordinate space

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

    def _create_temp_lookup_table(self):
        if not self.camera_config:
            raise RuntimeError(
                "Camera config not available for temperature lookup table."
            )

        min_lvl = self.camera_config.meas_level[0][0]
        max_lvl = self.camera_config.meas_level[0][1]
        min_temp_raw = self.camera_config.meas_temp[0][0]
        max_temp_raw = self.camera_config.meas_temp[0][1]

        min_temp_celsius = min_temp_raw / 10.0
        max_temp_celsius = max_temp_raw / 10.0

        if max_lvl <= min_lvl:
            raise ValueError("Invalid level range for temperature conversion.")

        self.temp_lookup_table = np.full(65536, np.nan, dtype=np.float32)

        levels = np.arange(min_lvl, max_lvl + 1)
        temperatures = np.linspace(min_temp_celsius, max_temp_celsius, len(levels))

        self.temp_lookup_table[levels] = temperatures

        self.temp_lookup_table[:min_lvl] = min_temp_celsius
        self.temp_lookup_table[max_lvl + 1 :] = max_temp_celsius

        print(
            f"Temperature lookup table created. Range: {min_lvl}-{max_lvl} levels -> {min_temp_celsius:.2f}-{max_temp_celsius:.2f}°C"
        )

    def _convert_raw_to_celsius(self, raw_data: np.ndarray) -> np.ndarray:
        if self.temp_lookup_table is None:
            self._create_temp_lookup_table()  # Create if not already created

        converted_data = self.temp_lookup_table[raw_data.astype(np.uint16)]
        return converted_data

    def _convert_raw_to_grayscale(self, raw_data: np.ndarray) -> np.ndarray:
        # Normalize the raw 16-bit data to 0-255 for 8-bit grayscale image
        # Find the min and max values in the raw data
        min_val = raw_data.min()
        max_val = raw_data.max()

        # Use np.interp for linear interpolation to scale values to 0-255
        # np.interp(x, (x_min, x_max), (0, 255))
        grayscale_image = np.interp(raw_data, (min_val, max_val), (0, 255)).astype(
            np.uint8
        )
        return grayscale_image

    

    def reset_ema_state(self):
        self.ema_lower_threshold = 0.0
        self.ema_upper_threshold = 0.0
        self.frame_count_for_ema = 0

    def _load_espcn_model(self):
        if self.sr_net is None:
            print("Loading ESPCN model...")
            model_path = "models/ESPCN-x2-rgb.onnx" # Path to your ONNX ESPCN model
            try:
                self.sr_net = cv2.dnn.readNet(model_path)
                print(f"ESPCN model loaded successfully from {model_path}")
            except Exception as e:
                print(f"Error loading ESPCN model: {e}. Please ensure the file exists and is valid.")
            print("ESPCN model loaded.")

    def set_sr_method_espcn(self):
        self.sr_method = "ESPCN"
        print("SR method set to ESPCN.")

    def set_sr_method_bicubic(self):
        self.sr_method = "Bicubic"
        print("SR method set to Bicubic interpolation.")

    def set_sr_method_none(self):
        self.sr_method = "None"
        print("SR method set to None.")

    def _draw_edges_on_image(self, celsius_data: np.ndarray) -> tuple[np.ndarray, dict]:
        display_values = {
            "agc_min": None,
            "agc_max": None,
            "edge_t1": None,
            "edge_t2": None,
        }

        # Normalize Celsius data to 0-255 based on AGC mode
        if self.agc_mode == "auto":
            # For Auto AGC, use Contrast Limited Adaptive Histogram Equalization (CLAHE)
            # for superior real-time contrast enhancement.

            # First, normalize the full-range float data to a standard 8-bit image.
            # This preserves the dynamic range of the current scene.
            base_gray_image = cv2.normalize(
                celsius_data, None, 0, 255, cv2.NORM_MINMAX
            ).astype(np.uint8)

            # Create and cache a CLAHE object for efficiency.
            if not hasattr(self, "clahe"):
                self.clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))

            # Apply CLAHE to enhance local contrast.
            normalized_celsius = self.clahe.apply(base_gray_image)

            # Apply Super Resolution if enabled and a method is selected
            if self.super_resolution_enabled:
                # Save image before SR if debug flag is set
                if self.save_sr_debug_images:
                    # Ensure it's 3-channel for saving as JPG
                    before_sr_image_display = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                    cv2.imwrite("sr_before.jpg", before_sr_image_display)
                    print("Saved sr_before.jpg")

                if self.sr_method == "ESPCN":
                    self._load_espcn_model()
                    if self.sr_net:
                        input_for_sr = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                        blob = cv2.dnn.blobFromImage(input_for_sr, 1.0/255.0, (0, 0), (0, 0, 0), True, False) # 픽셀 값 정규화 및 채널 변경
                        self.sr_net.setInput(blob)
                        
                        super_resolved_output = self.sr_net.forward()
                        print(f"[Debug] super_resolved_output shape: {super_resolved_output.shape}")
                        
                        super_resolved_image = super_resolved_output[0, :, :, :]
                        # Transpose from (C, H, W) to (H, W, C) for OpenCV
                        super_resolved_image = np.transpose(super_resolved_image, (1, 2, 0))
                        # Rescale from [0, 1] float to [0, 255] uint8
                        super_resolved_image = np.clip(super_resolved_image * 255, 0, 255).astype(np.uint8)

                        # Check if the super_resolved_image is mostly black
                        if np.mean(super_resolved_image) < 10: # Threshold for "mostly black"
                            print("Warning: ESPCN output is mostly black. Falling back to original image.")
                            normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                        else:
                            normalized_celsius = super_resolved_image.copy()
                        
                        print(f"[Debug] After SR, normalized_celsius shape: {normalized_celsius.shape}, dtype: {normalized_celsius.dtype}")
                    else:
                        print("ESPCN model not loaded. Cannot apply ESPCN. Falling back to no SR.")
                        normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                elif self.sr_method == "Bicubic":
                    print("Applying bicubic interpolation for Super Resolution.")
                    scale_factor = self.sr_scale_factor
                    new_width = int(normalized_celsius.shape[1] * scale_factor)
                    new_height = int(normalized_celsius.shape[0] * scale_factor)
                    normalized_celsius = cv2.resize(
                        normalized_celsius,
                        (new_width, new_height),
                        interpolation=cv2.INTER_CUBIC,
                    )
                    normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                else:
                    print("Super Resolution is enabled but no valid method selected (SRCNN/Bicubic). No SR applied.")
                    normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
            else: # SR not enabled, ensure normalized_celsius is 3-channel for consistency
                normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)

            # Save image after SR if debug flag is set
            if self.save_sr_debug_images:
                cv2.imwrite("sr_after.jpg", normalized_celsius)
                print("Saved sr_after.jpg")
                self.save_sr_debug_images = False # Reset flag after saving

            # Capture min/max values for display
            display_values["agc_min"] = celsius_data.min()
            display_values["agc_max"] = celsius_data.max()
        else:  # Manual AGC
            # Manual AGC: clip to a user-defined min/max temp range, then normalize.
            clipped_data = np.clip(
                celsius_data, self.manual_agc_min, self.manual_agc_max
            )
            # Handle the case where min and max are the same to avoid division by zero.
            if self.manual_agc_max <= self.manual_agc_min:
                normalized_celsius = np.full_like(clipped_data, 128, dtype=np.uint8)
            else:
                # Linearly scale the clipped data to the 0-255 range.
                normalized_celsius = (
                    255
                    * (clipped_data - self.manual_agc_min)
                    / (self.manual_agc_max - self.manual_agc_min)
                ).astype(np.uint8)

            # Apply Super Resolution if enabled and a method is selected
            if self.super_resolution_enabled:
                # Save image before SR if debug flag is set
                if self.save_sr_debug_images:
                    before_sr_image_display = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                    cv2.imwrite("sr_before.jpg", before_sr_image_display)
                    print("Saved sr_before.jpg")

                if self.sr_method == "ESPCN":
                    self._load_espcn_model()
                    if self.sr_net:
                        input_for_sr = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                        blob = cv2.dnn.blobFromImage(input_for_sr, 1.0/255.0, (0, 0), (0, 0, 0), True, False) # 픽셀 값 정규화 및 채널 변경
                        self.sr_net.setInput(blob)
                        
                        super_resolved_output = self.sr_net.forward()
                        print(f"[Debug] super_resolved_output shape: {super_resolved_output.shape}")
                        
                        super_resolved_image = super_resolved_output[0, :, :, :]
                        # Transpose from (C, H, W) to (H, W, C) for OpenCV
                        super_resolved_image = np.transpose(super_resolved_image, (1, 2, 0))
                        # Rescale from [0, 1] float to [0, 255] uint8
                        super_resolved_image = np.clip(super_resolved_image * 255, 0, 255).astype(np.uint8)

                        if np.mean(super_resolved_image) < 10: # Threshold for "mostly black"
                            print("Warning: ESPCN output is mostly black. Falling back to original image.")
                            normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                        else:
                            normalized_celsius = super_resolved_image.copy()
                        
                        print(f"[Debug] After SR, normalized_celsius shape: {normalized_celsius.shape}, dtype: {normalized_celsius.dtype}")
                    else:
                        print("ESPCN model not loaded. Cannot apply ESPCN. Falling back to no SR.")
                        normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                elif self.sr_method == "Bicubic":
                    print("Applying bicubic interpolation for Super Resolution.")
                    scale_factor = self.sr_scale_factor
                    new_width = int(normalized_celsius.shape[1] * scale_factor)
                    new_height = int(normalized_celsius.shape[0] * scale_factor)
                    normalized_celsius = cv2.resize(
                        normalized_celsius,
                        (new_width, new_height),
                        interpolation=cv2.INTER_CUBIC,
                    )
                    normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
                else:
                    print("Super Resolution is enabled but no valid method selected (SRCNN/Bicubic). No SR applied.")
                    normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)
            else: # SR not enabled, ensure normalized_celsius is 3-channel for consistency
                normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)

            # Save image after SR if debug flag is set
            if self.save_sr_debug_images:
                cv2.imwrite("sr_after.jpg", normalized_celsius)
                print("Saved sr_after.jpg")
                self.save_sr_debug_images = False # Reset flag after saving

            # Capture min/max values for display
            display_values["agc_min"] = self.manual_agc_min
            display_values["agc_max"] = self.manual_agc_max

        # Apply Bilateral Filter if enabled
        if self.bilateral_filter_enabled:
            # Ensure the image is 8-bit and 3-channel for bilateral filter if it's not already
            # Bilateral filter works on 8-bit images. If normalized_celsius is float, convert.
            # If it's 1-channel, convert to 3-channel for consistent processing.
            if normalized_celsius.dtype != np.uint8:
                normalized_celsius = normalized_celsius.astype(np.uint8)
            if len(normalized_celsius.shape) == 2:
                normalized_celsius = cv2.cvtColor(normalized_celsius, cv2.COLOR_GRAY2BGR)

            normalized_celsius = cv2.bilateralFilter(
                normalized_celsius,
                self.bilateral_d,
                self.bilateral_sigma_color,
                self.bilateral_sigma_space,
            )

        colored_image = normalized_celsius

        # Apply edge detection if enabled
        if self.edge_detection_enabled:
            # Determine Canny thresholds based on mode
            if self.edge_mode == "auto":
                # Auto mode: calculate thresholds based on image median
                v = np.median(normalized_celsius)
                sigma = 0.33
                lower_current = int(max(0, (1.0 - sigma) * v))
                upper_current = int(min(255, (1.0 + sigma) * v))

                # Apply Exponential Moving Average (EMA) for smoothing
                if self.frame_count_for_ema == 0:  # First frame, initialize EMA
                    self.ema_lower_threshold = float(lower_current)
                    self.ema_upper_threshold = float(upper_current)
                else:
                    self.ema_lower_threshold = (
                        self.ema_alpha * lower_current
                        + (1 - self.ema_alpha) * self.ema_lower_threshold
                    )
                    self.ema_upper_threshold = (
                        self.ema_alpha * upper_current
                        + (1 - self.ema_alpha) * self.ema_upper_threshold
                    )

                self.frame_count_for_ema += 1

                # Use smoothed thresholds for Canny
                lower = int(self.ema_lower_threshold)
                upper = int(self.ema_upper_threshold)

                display_values["edge_t1"] = lower
                display_values["edge_t2"] = upper
            else:  # Manual mode
                lower = self.canny_threshold1
                upper = self.canny_threshold2
                display_values["edge_t1"] = lower
                display_values["edge_t2"] = upper

            # Convert to grayscale for Canny edge detection if it's a 3-channel image
            if len(normalized_celsius.shape) == 3 and normalized_celsius.shape[2] == 3:
                gray_for_canny = cv2.cvtColor(normalized_celsius, cv2.COLOR_BGR2GRAY)
            else:
                gray_for_canny = normalized_celsius # Already grayscale or 1-channel

            # Apply a small Gaussian blur (3x3 kernel) to suppress noise before Canny.
            blurred_for_canny = cv2.GaussianBlur(gray_for_canny, (3, 3), 0)

            # Apply Canny edge detection
            edges = cv2.Canny(blurred_for_canny, lower, upper)

            # 1. 현재 엣지 픽셀 비율 계산
            current_edge_percentage = np.count_nonzero(edges) / edges.size

            # 2. EMA 임계값 동적 조정
            if current_edge_percentage > self.max_edge_percentage:
                # 엣지 수가 너무 많으면 임계값 증가 (더 엄격하게)
                self.ema_lower_threshold += self.threshold_adjustment_step
                self.ema_upper_threshold += self.threshold_adjustment_step
            elif current_edge_percentage < self.max_edge_percentage / 2:
                # 엣지 수가 너무 적으면 임계값 감소 (더 많은 엣지 감지)
                self.ema_lower_threshold -= self.threshold_adjustment_step
                self.ema_upper_threshold -= self.threshold_adjustment_step

            # 임계값 유효 범위 (0-255) 보장
            self.ema_lower_threshold = max(0, min(255, self.ema_lower_threshold))
            self.ema_upper_threshold = max(0, min(255, self.ema_upper_threshold))

            # 디버깅을 위한 출력
            print(
                f"Edge Density: {current_edge_percentage:.4f}, Adjusted Canny Thresholds: ({int(self.ema_lower_threshold)}, {int(self.ema_upper_threshold)})"
            )

            # 3. 다음 프레임에 적용 (lower, upper 변수가 이미 EMA 값을 사용하도록 설정되어 있으므로,
            #    여기서 조정된 self.ema_lower_threshold와 self.ema_upper_threshold가 다음 프레임에 반영됨)

            # Use edges directly for contour finding (thinning removed for performance)
            contours, _ = cv2.findContours(
                edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
            )

            # Draw contours with anti-aliasing (LINE_AA) to make them appear smoother
            cv2.drawContours(
                colored_image,
                contours,
                -1,
                self.edge_color,
                self.edge_thickness,
                lineType=cv2.LINE_AA,
            )

        print(f"[Debug] Before return, colored_image shape: {colored_image.shape}, dtype: {colored_image.dtype}")
        return colored_image, display_values

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

        # Extract raw image data
        raw_image_data = np.frombuffer(body[:image_size], dtype=np.uint16).reshape(
            height, width
        )

        print(
            f"Successfully received thermal image data: {len(body)} bytes ({width}x{height})"
        )
        return raw_image_data, width, height

    async def start_stream_continuous(self, frame_queue: asyncio.Queue):
        print("\nStarting continuous thermal image stream...")
        await self._send_message(MESSAGE_TYPE._IRF_STREAM_ON)

        try:
            while self.is_connected:
                header, body = await self._wait_for_response(
                    MESSAGE_TYPE._IRF_STREAM_DATA
                )

                width = self.image_dimensions["width"]
                height = self.image_dimensions["height"]

                image_size = width * height * 2  # 16-bit data

                if len(body) < image_size:
                    print(
                        f"Warning: Incomplete image frame received. Expected {image_size}, got {len(body)}"
                    )
                    continue

                raw_image_data = np.frombuffer(
                    body[:image_size], dtype=np.uint16
                ).reshape(height, width)

                # Put the raw image data into the queue for the UI to process
                await frame_queue.put(raw_image_data)

        except asyncio.CancelledError:
            print("Continuous stream cancelled.")
        except Exception as e:
            print(f"Error during continuous stream: {e}")
        finally:
            await self._send_message(MESSAGE_TYPE._IRF_STREAM_OFF)
            print("Continuous stream stopped.")