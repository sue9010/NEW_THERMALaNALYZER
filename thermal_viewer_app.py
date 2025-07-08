import sys
import asyncio
import numpy as np
import cv2
from PyQt5.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QHBoxLayout, QWidget, QLabel
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import Qt, QTimer
from qasync import QEventLoop, asyncSlot

from thermal_camera_client import ThermalCam, MESSAGE_TYPE

class ThermalViewerApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Thermal Camera Viewer")
        self.setGeometry(100, 100, 800, 600)

        self.thermal_cam = ThermalCam() # Initialize ThermalCam client
        self.frame_queue = asyncio.Queue() # Queue for receiving frames from camera client
        self.stream_task = None

        self.init_ui()

    def init_ui(self):
        # Main widget and layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # Image display
        self.image_label = QLabel("No Image")
        self.image_label.setAlignment(Qt.AlignCenter)
        # Initial size can be a placeholder, actual size will be set after connection
        self.image_label.setFixedSize(640, 480) # Placeholder size
        self.image_label.setStyleSheet("background-color: black; color: white;")
        main_layout.addWidget(self.image_label)

        # Control buttons
        control_layout = QHBoxLayout()
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_camera)
        control_layout.addWidget(self.connect_button)

        self.disconnect_button = QPushButton("Disconnect")
        self.disconnect_button.clicked.connect(self.disconnect_camera)
        self.disconnect_button.setEnabled(False)
        control_layout.addWidget(self.disconnect_button)

        self.start_stream_button = QPushButton("Start Stream")
        self.start_stream_button.clicked.connect(self.start_stream)
        self.start_stream_button.setEnabled(False)
        control_layout.addWidget(self.start_stream_button)

        self.stop_stream_button = QPushButton("Stop Stream")
        self.stop_stream_button.clicked.connect(self.stop_stream)
        self.stop_stream_button.setEnabled(False)
        control_layout.addWidget(self.stop_stream_button)

        main_layout.addLayout(control_layout)

        # Timer for updating UI with new frames
        self.update_timer = QTimer(self)
        self.update_timer.timeout.connect(self.update_image_display)
        self.update_timer.start(30) # Update every 30ms (approx 33 FPS)

    @asyncSlot()
    async def connect_camera(self):
        try:
            await self.thermal_cam.connect()
            self.connect_button.setEnabled(False)
            self.disconnect_button.setEnabled(True)
            self.start_stream_button.setEnabled(True)
            print("Camera connected successfully.")
            # Display initial camera info
            fw_version = self.thermal_cam.get_firmware_version()
            cam_info = self.thermal_cam.get_camera_info()
            print(f"Firmware Version: {fw_version}")
            print("Camera Info:")
            for key, value in cam_info.items():
                print(f"  {key}: {value}")

            # Set image label size based on actual camera resolution
            self.image_label.setFixedSize(self.thermal_cam.image_dimensions["width"], self.thermal_cam.image_dimensions["height"])
            self.image_label.setText("Ready to stream")

        except Exception as e:
            print(f"Failed to connect to camera: {e}")
            self.connect_button.setEnabled(True)
            self.disconnect_button.setEnabled(False)
            self.start_stream_button.setEnabled(False)

    @asyncSlot()
    async def disconnect_camera(self):
        if self.stream_task:
            self.stream_task.cancel()
            await self.stream_task # Wait for the task to finish cancelling
            self.stream_task = None
        await self.thermal_cam.disconnect()
        self.connect_button.setEnabled(True)
        self.disconnect_button.setEnabled(False)
        self.start_stream_button.setEnabled(False)
        self.stop_stream_button.setEnabled(False)
        self.image_label.clear()
        self.image_label.setText("No Image")
        print("Camera disconnected.")

    @asyncSlot()
    async def start_stream(self):
        if not self.thermal_cam.is_connected:
            print("Camera not connected.")
            return
        
        if self.stream_task and not self.stream_task.done():
            print("Stream already running.")
            return

        self.start_stream_button.setEnabled(False)
        self.stop_stream_button.setEnabled(True)
        self.stream_task = asyncio.create_task(self.thermal_cam.start_stream_continuous(self.frame_queue))
        print("Stream started.")

    @asyncSlot()
    async def stop_stream(self):
        if self.stream_task:
            self.stream_task.cancel()
            await self.stream_task # Wait for the task to finish cancelling
            self.stream_task = None
        self.start_stream_button.setEnabled(True)
        self.stop_stream_button.setEnabled(False)
        print("Stream stopped.")

    def update_image_display(self):
        try:
            # Try to get a frame from the queue without blocking
            raw_frame = self.frame_queue.get_nowait()
            print(f"[UI Update] Received raw frame. Shape: {raw_frame.shape}, Dtype: {raw_frame.dtype}")
            
            # Convert raw data to Celsius
            celsius_image = self.thermal_cam._convert_raw_to_celsius(raw_frame)
            
            # Draw edges on the Celsius image
            edged_image = self.thermal_cam._draw_edges_on_image(celsius_image)
            
            # Convert numpy array (BGR) to QImage
            height, width, channel = edged_image.shape
            bytes_per_line = 3 * width
            q_image = QImage(edged_image.data, width, height, bytes_per_line, QImage.Format_BGR888)
            
            # Scale QImage to fit QLabel and display
            pixmap = QPixmap.fromImage(q_image)
            self.image_label.setPixmap(pixmap.scaled(self.image_label.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation))
            print("[UI Update] Edged image displayed.")

        except asyncio.QueueEmpty:
            # print("[UI Update] No new frame yet.") # Too noisy
            pass # No new frame yet
        except Exception as e:
            print(f"Error updating image display: {e}")

    def closeEvent(self, event):
        # Schedule disconnect_camera to run on the event loop without blocking closeEvent
        self.disconnect_camera()
        super().closeEvent(event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)

    viewer = ThermalViewerApp()
    viewer.show()

    with loop:
        loop.run_forever()
