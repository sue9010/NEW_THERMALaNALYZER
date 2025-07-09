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

        # Initialize new slider values
        self.ema_alpha_slider.setMinimum(0)
        self.ema_alpha_slider.setMaximum(1000)
        self.ema_alpha_slider.setValue(int(self.thermal_cam.ema_alpha * 1000))

        self.max_edge_slider.setMinimum(0)
        self.max_edge_slider.setMaximum(100)
        self.max_edge_slider.setValue(int(self.thermal_cam.max_edge_percentage * 100))

        self.threshold_adjustment_step_slider.setMinimum(1)
        self.threshold_adjustment_step_slider.setMaximum(20)
        self.threshold_adjustment_step_slider.setValue(self.thermal_cam.threshold_adjustment_step)

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

        # Main update timer
        self.update_timer = QTimer(self)
        self.update_timer.timeout.connect(self.update_image_display)
        self.update_timer.start(50)
    # endregion
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
            raw_frame = self.frame_queue.get_nowait()
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
                if is_float:
                    widget.setText(f"{widget.objectName().replace('_label', '').replace('_', ' ').title()}: {current_value:.1f}°C")
                else:
                    widget.setText(f"{widget.objectName().replace('_label', '').replace('_', ' ').title()}: {current_value}")

    def _update_canny_threshold(self, threshold_type, value):
        if threshold_type == 1:
            self.thermal_cam.canny_threshold1 = value
            label = self.t1_label
            slider = self.t1_slider
            spinbox = self.t1_spinbox
        else: # threshold_type == 2
            self.thermal_cam.canny_threshold2 = value
            label = self.t2_label
            slider = self.t2_slider
            spinbox = self.t2_spinbox
        
        label.setText(f"Threshold {threshold_type}: {value}")
        self._synchronize_widget_value(value, [slider, spinbox])

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
            slider = self.agc_min_slider
            spinbox = self.agc_min_spinbox
        else: # agc_type == 'max'
            self.thermal_cam.manual_agc_max = value
            label = self.agc_max_label
            slider = self.agc_max_slider
            spinbox = self.agc_max_spinbox
        
        label.setText(f"{agc_type.title()} Temp: {value:.1f}°C")
        self._synchronize_widget_value(value, [slider, spinbox], is_float=True, multiplier=10)
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