import sys
import asyncio
import numpy as np
import cv2
from PyQt5.QtWidgets import QApplication, QMainWindow, QColorDialog
from PyQt5.QtGui import QImage, QPixmap, QColor
from PyQt5.QtCore import Qt, QTimer
from PyQt5 import uic
from qasync import QEventLoop, asyncSlot

from thermal_camera_client import ThermalCam

class ThermalViewerApp(QMainWindow):
    def __init__(self):
        super().__init__()
        
        # .ui 파일에서 UI를 로드합니다.
        uic.loadUi("thermal_viewer.ui", self)

        self.thermal_cam = ThermalCam()
        self.frame_queue = asyncio.Queue()
        self.stream_task = None

        # Default edge color (green) and thickness
        self.thermal_cam.edge_color = (0, 255, 0) # BGR format for OpenCV
        self.thermal_cam.edge_thickness = 1
        self.thermal_cam.super_resolution_enabled = False # Default to off
        self.super_resolution_button.setCheckable(True) # Make the button toggleable
        self.super_resolution_button.setChecked(False) # Initial state
        self.super_resolution_button.setText("SR Off") # Initial text

        # Configure edge_thickness_spinbox and slider
        self.edge_thickness_spinbox.setMinimum(1)
        self.edge_thickness_spinbox.setMaximum(10) # Example max thickness
        self.edge_thickness_spinbox.setValue(self.thermal_cam.edge_thickness)

        self.edge_thickness_slider.setMinimum(1)
        self.edge_thickness_slider.setMaximum(10) # Example max thickness
        self.edge_thickness_slider.setValue(self.thermal_cam.edge_thickness)

        # Initialize new slider values
        self.ema_alpha_slider.setMinimum(0)
        self.ema_alpha_slider.setMaximum(1000) # Represents 0.000 to 1.000
        self.ema_alpha_slider.setValue(int(self.thermal_cam.ema_alpha * 1000))

        self.max_edge_slider.setMinimum(0)
        self.max_edge_slider.setMaximum(100) # Represents 0.00 to 1.00
        self.max_edge_slider.setValue(int(self.thermal_cam.max_edge_percentage * 100))

        self.threshold_adjustment_step_slider.setMinimum(1)
        self.threshold_adjustment_step_slider.setMaximum(20) # Example range
        self.threshold_adjustment_step_slider.setValue(self.thermal_cam.threshold_adjustment_step)

        self.init_connections()
        self.update_edge_controls_state() # 초기 UI 상태 설정

    def init_connections(self):
        # UI 요소의 시그널을 슬롯(메서드)에 연결합니다.
        self.connect_button.clicked.connect(self.connect_camera)
        self.disconnect_button.clicked.connect(self.disconnect_camera)
        self.start_stream_button.clicked.connect(self.start_stream)
        self.stop_stream_button.clicked.connect(self.stop_stream)

        self.t1_slider.valueChanged.connect(self.update_canny_t1)
        self.t1_spinbox.valueChanged.connect(self.update_canny_t1)
        self.t2_slider.valueChanged.connect(self.update_canny_t2)
        self.t2_spinbox.valueChanged.connect(self.update_canny_t2)

        self.edge_mode_button.clicked.connect(self.toggle_edge_mode)
        self.edge_auto_manual_button.clicked.connect(self.toggle_edge_auto_manual)

        self.agc_mode_button.clicked.connect(self.toggle_agc_mode)
        self.agc_min_slider.valueChanged.connect(self.update_agc_min)
        self.agc_min_spinbox.valueChanged.connect(self.update_agc_min)
        self.agc_max_slider.valueChanged.connect(self.update_agc_max)
        self.agc_max_spinbox.valueChanged.connect(self.update_agc_max)

        # 새 프레임으로 UI를 업데이트하기 위한 타이머
        self.update_timer = QTimer(self)
        self.update_timer.timeout.connect(self.update_image_display)
        self.update_timer.start(30) # 30ms마다 업데이트 (약 33 FPS)

        # Edge Color and Thickness controls
        self.edge_color_button.clicked.connect(self.select_edge_color)
        self.edge_thickness_spinbox.valueChanged.connect(self.update_edge_thickness)
        self.edge_thickness_slider.valueChanged.connect(self.update_edge_thickness)

        # Super Resolution control
        self.super_resolution_button.clicked.connect(self.toggle_super_resolution)

        # New slider connections for auto edge mode
        self.ema_alpha_slider.valueChanged.connect(self.update_ema_alpha)
        self.max_edge_slider.valueChanged.connect(self.update_max_edge_percentage)
        self.threshold_adjustment_step_slider.valueChanged.connect(self.update_threshold_adjustment_step)

    def init_state(self):
        # .ui 파일의 기본값으로 처리되지 않는 위젯의 초기 상태를 설정합니다.
        self.agc_min_label.setEnabled(False)
        self.agc_min_slider.setEnabled(False)
        self.agc_min_spinbox.setEnabled(False)
        self.agc_max_label.setEnabled(False)
        self.agc_max_slider.setEnabled(False)
        self.agc_max_spinbox.setEnabled(False)

    def update_canny_t1(self, value):
        self.thermal_cam.canny_threshold1 = value
        self.t1_label.setText(f"Threshold 1: {value}")
        if self.t1_slider.value() != value:
            self.t1_slider.setValue(value)
        if self.t1_spinbox.value() != value:
            self.t1_spinbox.setValue(value)

    def update_canny_t2(self, value):
        self.thermal_cam.canny_threshold2 = value
        self.t2_label.setText(f"Threshold 2: {value}")
        if self.t2_slider.value() != value:
            self.t2_slider.setValue(value)
        if self.t2_spinbox.value() != value:
            self.t2_spinbox.setValue(value)

    def select_edge_color(self):
        color = QColorDialog.getColor(QColor(*self.thermal_cam.edge_color[::-1]), self, "Select Edge Color") # OpenCV uses BGR, QColor uses RGB
        if color.isValid():
            # Convert QColor (RGB) to OpenCV BGR format
            self.thermal_cam.edge_color = (color.blue(), color.green(), color.red())
            # Optionally, update the button's background color to reflect the selection
            self.edge_color_button.setStyleSheet(f"background-color: {color.name()};")

    def update_edge_thickness(self, value):
        self.thermal_cam.edge_thickness = value

        # Synchronize spinbox and slider
        if self.edge_thickness_spinbox.value() != value:
            self.edge_thickness_spinbox.blockSignals(True)
            self.edge_thickness_spinbox.setValue(value)
            self.edge_thickness_spinbox.blockSignals(False)

        if self.edge_thickness_slider.value() != value:
            self.edge_thickness_slider.blockSignals(True)
            self.edge_thickness_slider.setValue(value)
            self.edge_thickness_slider.blockSignals(False)

        # Optionally, update a label to show current thickness if needed

    def toggle_super_resolution(self):
        checked = self.super_resolution_button.isChecked()
        self.thermal_cam.super_resolution_enabled = checked
        self.super_resolution_button.setText("SR On" if checked else "SR Off")

    def update_ema_alpha(self, value):
        self.thermal_cam.ema_alpha = value / 1000.0
        # Optionally update a label to show the current value
        # self.ema_alpha_label.setText(f"EMA Alpha: {self.thermal_cam.ema_alpha:.3f}")

    def update_max_edge_percentage(self, value):
        self.thermal_cam.max_edge_percentage = value / 100.0
        # Optionally update a label to show the current value
        # self.max_edge_label.setText(f"Max Edge %: {self.thermal_cam.max_edge_percentage:.2f}")

    def update_threshold_adjustment_step(self, value):
        self.thermal_cam.threshold_adjustment_step = value
        # Optionally update a label to show the current value
        # self.threshold_adjustment_step_label.setText(f"Adj Step: {self.thermal_cam.threshold_adjustment_step}")

    def update_edge_controls_state(self):
        edges_on = self.edge_mode_button.isChecked()
        is_manual = not self.edge_auto_manual_button.isChecked()

        self.edge_auto_manual_button.setEnabled(edges_on)
        
        sliders_enabled = edges_on and is_manual
        self.t1_label.setEnabled(sliders_enabled)
        self.t1_slider.setEnabled(sliders_enabled)
        self.t1_spinbox.setEnabled(sliders_enabled)
        self.t2_label.setEnabled(sliders_enabled)
        self.t2_slider.setEnabled(sliders_enabled)
        self.t2_spinbox.setEnabled(sliders_enabled)

        # Enable/disable new sliders based on edge_detection_enabled and auto mode
        auto_edge_sliders_enabled = edges_on and (not is_manual) # Enabled when edge detection is ON and in AUTO mode
        self.ema_alpha_slider.setEnabled(auto_edge_sliders_enabled)
        self.max_edge_slider.setEnabled(auto_edge_sliders_enabled)
        self.threshold_adjustment_step_slider.setEnabled(auto_edge_sliders_enabled)

    def toggle_edge_mode(self, checked):
        self.thermal_cam.edge_detection_enabled = checked
        self.edge_mode_button.setText("Set Edge Off" if checked else "Set Edge On")
        if checked: # Edges are being turned ON
            self.thermal_cam.reset_ema_state() # Reset EMA when turning edges on
        self.update_edge_controls_state()

    def toggle_edge_auto_manual(self, checked):
        self.thermal_cam.edge_mode = 'auto' if checked else 'manual'
        self.edge_auto_manual_button.setText("Set Manual Edge" if checked else "Set Auto Edge")
        if checked: # Switching to Auto Edge
            self.thermal_cam.reset_ema_state() # Reset EMA when switching to auto
        self.update_edge_controls_state()

    def toggle_agc_mode(self, checked):
        if checked:
            self.thermal_cam.agc_mode = 'manual'
            self.agc_mode_button.setText("Set Auto AGC")
            is_manual = True
        else:
            self.thermal_cam.agc_mode = 'auto'
            self.agc_mode_button.setText("Set Manual AGC")
            is_manual = False
        
        print(f"AGC mode set to: {self.thermal_cam.agc_mode}")

        self.agc_min_label.setEnabled(is_manual)
        self.agc_min_slider.setEnabled(is_manual)
        self.agc_min_spinbox.setEnabled(is_manual)
        self.agc_max_label.setEnabled(is_manual)
        self.agc_max_slider.setEnabled(is_manual)
        self.agc_max_spinbox.setEnabled(is_manual)

    def update_agc_min(self, value):
        if isinstance(value, int): # 슬라이더에서 오는 int 값 처리
            value = value / 10.0
        
        self.thermal_cam.manual_agc_min = value
        self.agc_min_label.setText(f"Min Temp: {value:.1f}°C")
        if self.agc_min_slider.value() != int(value * 10):
            self.agc_min_slider.setValue(int(value * 10))
        if self.agc_min_spinbox.value() != value:
            self.agc_min_spinbox.setValue(value)

    def update_agc_max(self, value):
        if isinstance(value, int): # 슬라이더에서 오는 int 값 처리
            value = value / 10.0

        self.thermal_cam.manual_agc_max = value
        self.agc_max_label.setText(f"Max Temp: {value:.1f}°C")
        if self.agc_max_slider.value() != int(value * 10):
            self.agc_max_slider.setValue(int(value * 10))
        if self.agc_max_spinbox.value() != value:
            self.agc_max_spinbox.setValue(value)

    @asyncSlot()
    async def connect_camera(self):
        try:
            await self.thermal_cam.connect()
            self.connect_button.setEnabled(False)
            self.disconnect_button.setEnabled(True)
            self.start_stream_button.setEnabled(True)
            print("Camera connected successfully.")
            fw_version = self.thermal_cam.get_firmware_version()
            cam_info = self.thermal_cam.get_camera_info()
            print(f"Firmware Version: {fw_version}")
            print("Camera Info:")
            for key, value in cam_info.items():
                print(f"  {key}: {value}")

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
            try:
                await self.stream_task
            except asyncio.CancelledError:
                pass # 정상적인 취소
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
            try:
                await self.stream_task
            except asyncio.CancelledError:
                pass # 정상적인 취소
            self.stream_task = None
        self.start_stream_button.setEnabled(True)
        self.stop_stream_button.setEnabled(False)
        print("Stream stopped.")

    def update_image_display(self):
        try:
            raw_frame = self.frame_queue.get_nowait()
            celsius_image = self.thermal_cam._convert_raw_to_celsius(raw_frame)
            
            # _draw_edges_on_image에서 이미지와 함께 현재 값들을 받아옵니다.
            edged_image, display_values = self.thermal_cam._draw_edges_on_image(celsius_image)
            
            # Auto AGC 모드일 때 AGC 라벨 업데이트
            if self.thermal_cam.agc_mode == 'auto':
                self.agc_min_label.setText(f"Min Temp: {display_values['agc_min']:.1f}°C")
                self.agc_max_label.setText(f"Max Temp: {display_values['agc_max']:.1f}°C")
            # Manual AGC 모드일 때는 update_agc_min/max에서 이미 라벨이 업데이트됩니다.

            # Auto Edge 모드이고 Edge Detection이 활성화되어 있을 때 Edge 라벨 업데이트
            if self.thermal_cam.edge_mode == 'auto' and self.thermal_cam.edge_detection_enabled:
                self.t1_label.setText(f"Threshold 1: {display_values['edge_t1']}")
                self.t2_label.setText(f"Threshold 2: {display_values['edge_t2']}")
            # Manual Edge 모드이거나 Edge Detection이 비활성화되어 있을 때는 update_canny_t1/t2에서 업데이트되거나 비활성화됩니다.

            height, width, channel = edged_image.shape
            bytes_per_line = 3 * width
            q_image = QImage(edged_image.data, width, height, bytes_per_line, QImage.Format_BGR888)
            
            pixmap = QPixmap.fromImage(q_image)
            self.image_label.setPixmap(pixmap.scaled(self.image_label.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation))

        except asyncio.QueueEmpty:
            pass
        except Exception as e:
            print(f"Error updating image display: {e}")

    def closeEvent(self, event):
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