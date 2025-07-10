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
        uic.loadUi("thermal_viewer.ui", self)

        self.thermal_cam = ThermalCam()
        self.frame_queue = asyncio.Queue()
        self.stream_task = None

        self._setup_ui()
        self._connect_signals()

        # Initialize status bar
        self.statusBar()

    # region Setup Methods
    def _setup_ui(self):
        """Initializes UI elements, sets default values, and configures widget properties."""
        # Default edge color (green) and thickness
        self.thermal_cam.edge_color = (0, 255, 0) # BGR format for OpenCV
        self.thermal_cam.edge_thickness = 1
        self.thermal_cam.super_resolution_enabled = False # Default to off
        
        # Configure Super Resolution buttons
        self.super_resolution_button.setCheckable(True)
        self.super_resolution_button.setChecked(False)
        self.super_resolution_button.setText("SR Bicubic Off")

        self.super_resolution_button2.setCheckable(True)
        self.super_resolution_button2.setChecked(False)
        self.super_resolution_button2.setText("SR SRCNN Off")

        # Initialize SR method in thermal_cam to None
        self.thermal_cam.set_sr_method_none()

        # Configure edge_thickness_spinbox and slider
        self.edge_thickness_spinbox.setMinimum(1)
        self.edge_thickness_spinbox.setMaximum(10)
        self.edge_thickness_spinbox.setValue(self.thermal_cam.edge_thickness)

        self.edge_thickness_slider.setMinimum(1)
        self.edge_thickness_slider.setMaximum(10)
        self.edge_thickness_slider.setValue(self.thermal_cam.edge_thickness)

        # Initialize new slider values for auto edge mode
        self.ema_alpha_slider.setMinimum(0)
        self.ema_alpha_slider.setMaximum(1000)
        self.ema_alpha_slider.setValue(int(self.thermal_cam.ema_alpha * 1000))

        self.max_edge_slider.setMinimum(0)
        self.max_edge_slider.setMaximum(100)
        self.max_edge_slider.setValue(int(self.thermal_cam.max_edge_percentage * 100))

        self.threshold_adjustment_step_slider.setMinimum(1)
        self.threshold_adjustment_step_slider.setMaximum(20)
        self.threshold_adjustment_step_slider.setValue(self.thermal_cam.threshold_adjustment_step)

        # Bilateral Filter controls
        self.bilateral_filter_button.setCheckable(True)
        self.bilateral_filter_button.setChecked(False)
        self.bilateral_filter_button.setText("Bilateral Filter Off")

        self.bilateral_d_slider.setMinimum(1)
        self.bilateral_d_slider.setMaximum(20)
        self.bilateral_d_slider.setValue(9) # Default value
        self.bilateral_d_spinbox.setMinimum(1)
        self.bilateral_d_spinbox.setMaximum(20)
        self.bilateral_d_spinbox.setValue(9) # Default value

        self.bilateral_sigma_color_slider.setMinimum(1)
        self.bilateral_sigma_color_slider.setMaximum(200)
        self.bilateral_sigma_color_slider.setValue(75) # Default value
        self.bilateral_sigma_color_spinbox.setMinimum(1)
        self.bilateral_sigma_color_spinbox.setMaximum(200)
        self.bilateral_sigma_color_spinbox.setValue(75) # Default value

        self.bilateral_sigma_space_slider.setMinimum(1)
        self.bilateral_sigma_space_slider.setMaximum(200)
        self.bilateral_sigma_space_slider.setValue(75) # Default value
        self.bilateral_sigma_space_spinbox.setMinimum(1)
        self.bilateral_sigma_space_spinbox.setMaximum(200)
        self.bilateral_sigma_space_spinbox.setValue(75) # Default value

        self._update_ui_state()

    def _connect_signals(self):
        """Connects UI element signals to corresponding slots."""
        self.connect_button.clicked.connect(self.connect_camera)
        self.disconnect_button.clicked.connect(self.disconnect_camera)

        # Canny edge thresholds
        self.t1_slider.valueChanged.connect(lambda value: self._update_canny_threshold(1, value))
        self.t1_spinbox.valueChanged.connect(lambda value: self._update_canny_threshold(1, value))
        self.t2_slider.valueChanged.connect(lambda value: self._update_canny_threshold(2, value))
        self.t2_spinbox.valueChanged.connect(lambda value: self._update_canny_threshold(2, value))

        # Edge mode and color
        self.edge_mode_button.clicked.connect(self.toggle_edge_mode)
        self.edge_auto_manual_button.clicked.connect(self.toggle_edge_auto_manual)
        self.edge_color_button.clicked.connect(self.select_edge_color)
        self.edge_thickness_slider.valueChanged.connect(self.update_edge_thickness)
        self.edge_thickness_spinbox.valueChanged.connect(self.update_edge_thickness)

        # AGC controls
        self.agc_mode_button.clicked.connect(self.toggle_agc_mode)
        self.agc_min_slider.valueChanged.connect(lambda value: self._update_agc_value('min', value))
        self.agc_min_spinbox.valueChanged.connect(lambda value: self._update_agc_value('min', value))
        self.agc_max_slider.valueChanged.connect(lambda value: self._update_agc_value('max', value))
        self.agc_max_spinbox.valueChanged.connect(lambda value: self._update_agc_value('max', value))

        # Super Resolution controls
        self.super_resolution_button.clicked.connect(lambda: self.on_sr_button_clicked(self.super_resolution_button))
        self.super_resolution_button2.clicked.connect(lambda: self.on_sr_button_clicked(self.super_resolution_button2))

        # Auto edge mode parameters
        self.ema_alpha_slider.valueChanged.connect(self.update_ema_alpha)
        self.max_edge_slider.valueChanged.connect(self.update_max_edge_percentage)
        self.threshold_adjustment_step_slider.valueChanged.connect(self.update_threshold_adjustment_step)

        # Bilateral Filter controls
        self.bilateral_filter_button.clicked.connect(self.toggle_bilateral_filter)
        self.bilateral_d_slider.valueChanged.connect(self.update_bilateral_d)
        self.bilateral_d_spinbox.valueChanged.connect(self.update_bilateral_d)
        self.bilateral_sigma_color_slider.valueChanged.connect(self.update_bilateral_sigma_color)
        self.bilateral_sigma_color_spinbox.valueChanged.connect(self.update_bilateral_sigma_color)
        self.bilateral_sigma_space_slider.valueChanged.connect(self.update_bilateral_sigma_space)
        self.bilateral_sigma_space_spinbox.valueChanged.connect(self.update_bilateral_sigma_space)

        # Main update timer
        self.update_timer = QTimer(self)
        self.update_timer.timeout.connect(self.update_image_display)
        self.update_timer.start(50)

        # Status bar messages for sliders
        self.t1_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("Canny 엣지 감지 하위 임계값: 이 값을 높이면 더 강한 엣지만 감지됩니다."))
        self.t2_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("Canny 엣지 감지 상위 임계값: 이 값을 높이면 더 강한 엣지만 감지됩니다."))
        self.edge_thickness_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("엣지 두께: 엣지 선의 두께를 조절합니다. 값이 클수록 선이 두꺼워집니다."))
        self.agc_min_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("수동 AGC 최소 온도: 이 온도 이하의 픽셀은 검은색에 가깝게 표시됩니다."))
        self.agc_max_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("수동 AGC 최대 온도: 이 온도 이상의 픽셀은 흰색에 가깝게 표시됩니다."))
        self.ema_alpha_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("EMA 알파 값 (엣지 감지 평활화): 값이 낮을수록 엣지 감지 임계값 변화가 부드러워집니다."))
        self.max_edge_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("최대 엣지 픽셀 비율: 화면에 표시될 엣지 픽셀의 최대 비율을 설정합니다. 이 비율을 초과하면 엣지 감지 임계값이 자동으로 높아집니다."))
        self.threshold_adjustment_step_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("임계값 조정 단계: 자동 엣지 감지 시 임계값이 한 번에 조정되는 폭을 설정합니다."))
        self.bilateral_d_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("양방향 필터 직경 (d): 필터링에 사용되는 주변 픽셀의 크기입니다. 값이 클수록 더 넓은 영역의 노이즈가 제거되지만, 엣지가 약간 흐려질 수 있습니다."))
        self.bilateral_sigma_color_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("양방향 필터 색상 시그마 (sigmaColor): 색상(온도) 유사성 기준입니다. 값이 클수록 색상 차이가 큰 픽셀도 노이즈로 간주하여 제거합니다. 엣지 보존에 영향을 줍니다."))
        self.bilateral_sigma_space_slider.valueChanged.connect(lambda value: self._show_message_in_statusbar("양방향 필터 공간 시그마 (sigmaSpace): 공간적 거리 기준입니다. 값이 클수록 멀리 떨어진 픽셀도 필터링에 영향을 줍니다. 이미지가 더 부드러워집니다."))
    # endregion

    # region Connection Management
    @asyncSlot()
    async def connect_camera(self):
        try:
            ip_address = self.ip_address_input.text()
            self.thermal_cam.host = ip_address
            
            await self.thermal_cam.connect()
            print(f"Camera connected successfully to {ip_address}.")

            if self.stream_task and not self.stream_task.done():
                print("Stream already running.")
            else:
                self.stream_task = asyncio.create_task(self.thermal_cam.start_stream_continuous(self.frame_queue))
                print("Stream started.")

            self._update_ui_state()
            
            fw_version = self.thermal_cam.get_firmware_version()
            cam_info = self.thermal_cam.get_camera_info()
            print(f"Firmware Version: {fw_version}")
            print("Camera Info:")
            for key, value in cam_info.items():
                print(f"  {key}: {value}")

            self.image_label.setText("Streaming...")

        except Exception as e:
            print(f"Failed to connect to camera: {e}")
            self._update_ui_state()

    @asyncSlot()
    async def disconnect_camera(self):
        if self.stream_task:
            self.stream_task.cancel()
            try:
                await self.stream_task
            except asyncio.CancelledError:
                pass
            self.stream_task = None
            print("Stream stopped.")

        await self.thermal_cam.disconnect()
        
        self._update_ui_state()
        self.image_label.clear()
        self.image_label.setText("No Image")
        print("Camera disconnected.")
    # endregion

    # region Image Display
    def update_image_display(self):
        try:
            raw_frame = None
            # 큐에 쌓인 모든 프레임을 소진하고 가장 최신 프레임만 가져옵니다.
            while not self.frame_queue.empty():
                raw_frame = self.frame_queue.get_nowait()
            
            if raw_frame is None:
                return # 처리할 프레임이 없으면 종료

            celsius_image = self.thermal_cam._convert_raw_to_celsius(raw_frame)
            
            edged_image, display_values = self.thermal_cam._draw_edges_on_image(celsius_image)
            
            if self.thermal_cam.agc_mode == 'auto':
                self.agc_min_label.setText(f"Min Temp: {display_values['agc_min']:.1f}°C")
                self.agc_max_label.setText(f"Max Temp: {display_values['agc_max']:.1f}°C")

            if self.thermal_cam.edge_mode == 'auto' and self.thermal_cam.edge_detection_enabled:
                self.t1_label.setText(f"Threshold 1: {display_values['edge_t1']}")
                self.t2_label.setText(f"Threshold 2: {display_values['edge_t2']}")

            if len(edged_image.shape) == 2:
                edged_image = cv2.cvtColor(edged_image, cv2.COLOR_GRAY2BGR)

            height, width, channel = edged_image.shape
            bytes_per_line = 3 * width
            q_image = QImage(edged_image.data, width, height, bytes_per_line, QImage.Format_BGR888)
            
            pixmap = QPixmap.fromImage(q_image)
            self.image_label.setPixmap(pixmap.scaled(self.image_label.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation))

        except asyncio.QueueEmpty:
            pass
        except Exception as e:
            print(f"Error updating image display: {e}")
    # endregion

    # region UI State & Updates
    def _update_ui_state(self):
        is_connected = self.thermal_cam.is_connected
        self.connect_button.setEnabled(not is_connected)
        self.disconnect_button.setEnabled(is_connected)

        edges_on = self.edge_mode_button.isChecked()
        is_manual_edge = not self.edge_auto_manual_button.isChecked()
        is_manual_agc = self.agc_mode_button.isChecked()
        bilateral_filter_on = self.bilateral_filter_button.isChecked()

        self.edge_auto_manual_button.setEnabled(edges_on and is_connected)
        
        manual_edge_controls_enabled = edges_on and is_manual_edge and is_connected
        self.t1_label.setEnabled(manual_edge_controls_enabled)
        self.t1_slider.setEnabled(manual_edge_controls_enabled)
        self.t1_spinbox.setEnabled(manual_edge_controls_enabled)
        self.t2_label.setEnabled(manual_edge_controls_enabled)
        self.t2_slider.setEnabled(manual_edge_controls_enabled)
        self.t2_spinbox.setEnabled(manual_edge_controls_enabled)

        auto_edge_controls_enabled = edges_on and not is_manual_edge and is_connected
        self.ema_alpha_slider.setEnabled(auto_edge_controls_enabled)
        self.max_edge_slider.setEnabled(auto_edge_controls_enabled)
        self.threshold_adjustment_step_slider.setEnabled(auto_edge_controls_enabled)

        manual_agc_controls_enabled = is_manual_agc and is_connected
        self.agc_min_label.setEnabled(manual_agc_controls_enabled)
        self.agc_min_slider.setEnabled(manual_agc_controls_enabled)
        self.agc_min_spinbox.setEnabled(manual_agc_controls_enabled)
        self.agc_max_label.setEnabled(manual_agc_controls_enabled)
        self.agc_max_slider.setEnabled(manual_agc_controls_enabled)
        self.agc_max_spinbox.setEnabled(manual_agc_controls_enabled)

        # Bilateral Filter controls
        bilateral_controls_enabled = bilateral_filter_on and is_connected
        self.bilateral_d_label.setEnabled(bilateral_controls_enabled)
        self.bilateral_d_slider.setEnabled(bilateral_controls_enabled)
        self.bilateral_d_spinbox.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_color_label.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_color_slider.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_color_spinbox.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_space_label.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_space_slider.setEnabled(bilateral_controls_enabled)
        self.bilateral_sigma_space_spinbox.setEnabled(bilateral_controls_enabled)

    # endregion

    # region Control Handlers
    def _synchronize_widget_value(self, current_value, target_widgets, is_float=False, multiplier=1):
        """Synchronizes the value across multiple widgets (e.g., slider, spinbox, label)."""
        for widget in target_widgets:
            if isinstance(widget, (type(self.t1_slider), type(self.t1_spinbox))): # QSlider or QSpinBox
                if is_float:
                    # For float values, convert back to int for slider/spinbox
                    display_value = int(current_value * multiplier)
                else:
                    display_value = current_value

                if widget.value() != display_value:
                    widget.blockSignals(True)
                    widget.setValue(display_value)
                    widget.blockSignals(False)
            elif isinstance(widget, type(self.t1_label)): # QLabel
                # This part is now handled directly in the update methods for clarity
                pass

    def _update_canny_threshold(self, threshold_type, value):
        if threshold_type == 1:
            self.thermal_cam.canny_threshold1 = value
            label = self.t1_label
            # slider = self.t1_slider # Not needed for synchronization here
            # spinbox = self.t1_spinbox # Not needed for synchronization here
        else: # threshold_type == 2
            self.thermal_cam.canny_threshold2 = value
            label = self.t2_label
            # slider = self.t2_slider # Not needed for synchronization here
            # spinbox = self.t2_spinbox # Not needed for synchronization here
        
        label.setText(f"Threshold {threshold_type}: {value}")
        # self._synchronize_widget_value(value, [slider, spinbox]) # Handled by direct connection

    def select_edge_color(self):
        color = QColorDialog.getColor(QColor(*self.thermal_cam.edge_color[::-1]), self, "Select Edge Color")
        if color.isValid():
            self.thermal_cam.edge_color = (color.blue(), color.green(), color.red())
            self.edge_color_button.setStyleSheet(f"background-color: {color.name()};")

    def update_edge_thickness(self, value):
        self.thermal_cam.edge_thickness = value
        self._synchronize_widget_value(value, [self.edge_thickness_slider, self.edge_thickness_spinbox])

    def on_sr_button_clicked(self, clicked_button):
        if clicked_button == self.super_resolution_button:
            if clicked_button.isChecked():
                self.thermal_cam.set_sr_method_bicubic()
                self.super_resolution_button2.setChecked(False)
                self.super_resolution_button.setText("SR Bicubic On")
                self.super_resolution_button2.setText("SR SRCNN Off")
            else:
                self.thermal_cam.set_sr_method_none()
                self.super_resolution_button.setText("SR Bicubic Off")
        elif clicked_button == self.super_resolution_button2:
            if clicked_button.isChecked():
                self.thermal_cam.set_sr_method_espcn()
                self.super_resolution_button.setChecked(False)
                self.super_resolution_button2.setText("SR ESPCN On")
                self.super_resolution_button.setText("SR Bicubic Off")
                self.thermal_cam.save_sr_debug_images = True
            else:
                self.thermal_cam.set_sr_method_none()
                self.super_resolution_button2.setText("SR ESPCN Off")
        self.thermal_cam.super_resolution_enabled = self.super_resolution_button.isChecked() or self.super_resolution_button2.isChecked()

    def update_ema_alpha(self, value):
        self.thermal_cam.ema_alpha = value / 1000.0
        self.ema_alpha_label.setText(f"EMA Alpha: {self.thermal_cam.ema_alpha:.3f}")

    def update_max_edge_percentage(self, value):
        self.thermal_cam.max_edge_percentage = value / 100.0
        self.max_edge_label.setText(f"Max Edge %: {self.thermal_cam.max_edge_percentage:.2f}")

    def update_threshold_adjustment_step(self, value):
        self.thermal_cam.threshold_adjustment_step = value
        self.threshold_adjustment_step_label.setText(f"Adj Step: {self.thermal_cam.threshold_adjustment_step}")

    def toggle_edge_mode(self, checked):
        self.thermal_cam.edge_detection_enabled = checked
        self.edge_mode_button.setText("Set Edge Off" if checked else "Set Edge On")
        if checked:
            self.thermal_cam.reset_ema_state()
        self._update_ui_state()

    def toggle_edge_auto_manual(self, checked):
        self.thermal_cam.edge_mode = 'auto' if checked else 'manual'
        self.edge_auto_manual_button.setText("Set Manual Edge" if checked else "Set Auto Edge")
        if checked:
            self.thermal_cam.reset_ema_state()
        self._update_ui_state()

    def toggle_agc_mode(self, checked):
        self.thermal_cam.agc_mode = 'manual' if checked else 'auto'
        self.agc_mode_button.setText("Set Auto AGC" if checked else "Set Manual AGC")
        self._update_ui_state()

    def _update_agc_value(self, agc_type, value):
        if isinstance(value, int):
            value = value / 10.0

        if agc_type == 'min':
            self.thermal_cam.manual_agc_min = value
            label = self.agc_min_label
        else: # agc_type == 'max'
            self.thermal_cam.manual_agc_max = value
            label = self.agc_max_label
        
        label.setText(f"{agc_type.title()} Temp: {value:.1f}°C")

    def toggle_bilateral_filter(self, checked):
        self.thermal_cam.bilateral_filter_enabled = checked
        self.bilateral_filter_button.setText("Bilateral Filter On" if checked else "Bilateral Filter Off")
        self._update_ui_state()

    def update_bilateral_d(self, value):
        self.thermal_cam.bilateral_d = value
        self.bilateral_d_label.setText(f"Bilateral D: {value}")
        self._synchronize_widget_value(value, [self.bilateral_d_slider, self.bilateral_d_spinbox])

    def update_bilateral_sigma_color(self, value):
        self.thermal_cam.bilateral_sigma_color = value
        self.bilateral_sigma_color_label.setText(f"Sigma Color: {value}")
        self._synchronize_widget_value(value, [self.bilateral_sigma_color_slider, self.bilateral_sigma_color_spinbox])

    def update_bilateral_sigma_space(self, value):
        self.thermal_cam.bilateral_sigma_space = value
        self.bilateral_sigma_space_label.setText(f"Sigma Space: {value}")
        self._synchronize_widget_value(value, [self.bilateral_sigma_space_slider, self.bilateral_sigma_space_spinbox])

    # endregion

    # region Utility Methods
    def _show_message_in_statusbar(self, message, timeout=2000):
        self.statusBar().showMessage(message, timeout)
    # endregion

    def closeEvent(self, event):
        asyncio.run(self.disconnect_camera())
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)

    viewer = ThermalViewerApp()
    viewer.show()

    with loop:
        loop.run_forever()