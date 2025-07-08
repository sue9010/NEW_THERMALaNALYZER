import sys
import time
import cv2
import os
import queue
import asyncio
import numpy as np
import zeroconf_discovery
from datetime import datetime
from ipaddressEdit import IPAddressEdit
from videoLabel import VideoLabel
from PyQt5.QtWidgets import (QApplication,
                             QMainWindow,
                             QWidget,
                             QLabel,
                             QFileDialog,
                             QHBoxLayout,
                             QVBoxLayout,
                             QListWidget,
                             QPushButton,
                             QMessageBox,
                             QComboBox)
from PyQt5.QtCore import Qt, QObject, QEvent, QSize, QTimer, pyqtSignal
from PyQt5.QtGui import QIcon, QImage, QFontMetrics
from concurrent.futures import ThreadPoolExecutor
from qasync import QEventLoop, asyncSlot
import thermalcamera_lib as tlib
import threading
from agcControl import AgcControl
import resources_rc  # noqa: F401



SERVICE_NAME="_cox-cam._tcp.local."


class ElidedLabel(QLabel):
    def __init__(self, text="", parent=None):
        super().__init__(text, parent)
        self._full_text = ""
        self.setText(text)
    
    def setText(self, text):
        self._full_text = text
        self.updateText()
        self.setToolTip(text)
    
    def resizeEvent(self, event):
        self.updateText()
        return super().resizeEvent(event)

    def updateText(self):
        fm = QFontMetrics(self.font())
        elided = fm.elidedText(self._full_text,
                               Qt.TextElideMode.ElideMiddle,
                               self.width())
        super().setText(elided)
    
    def text(self):
        return self._full_text


class ClickToExpandFilter(QObject):
    def __init__(self, combo):
        super().__init__()
        self.combo = combo
        
    def eventFilter(self, obj, event):
        if event.type() == QEvent.Type.MouseButtonPress:
            self.combo.showPopup()
            return True
        return False


class MainWindow(QMainWindow):
    sig_connect_stat = pyqtSignal(bool)
    sig_write_stat = pyqtSignal(str)
    sig_fps_stat = pyqtSignal(float)
    
    def __init__(self):
        super().__init__()
        self.thermal = tlib.ThermalCamera()
        self.thermal_thread = None
        
        self.write_thread = None
        self.write_running = False
        self.write_queue = queue.Queue()
        self.last_tick = None
        
        self.initUI()
        self.signal_connect()
        QTimer.singleShot(0, self.start_discovery)
        self.controlEnable(False)


    def initUI(self):
        self.resize(900, 600)
        self.setWindowTitle('COX ThermalCamera Viewer')
        self.setWindowIcon(QIcon(':cox_cam'))
        
        # 카메라 목록 (콤보)
        self.cb_camera_list = QComboBox(self)
        self.cb_camera_list.setFixedHeight(25)
        self.cb_camera_list.setFixedWidth(250)
        self.cb_camera_list.setEditable(True)
        self.cb_camera_list.lineEdit().setPlaceholderText('📷 접속할 카메라를 선택하세요')
        self.cb_camera_list.lineEdit().setReadOnly(True)
        self.filter = ClickToExpandFilter(self.cb_camera_list)
        self.cb_camera_list.lineEdit().installEventFilter(self.filter)
        
        # ip address (에디트박스)
        self.ip_address = IPAddressEdit()
        
        # 새로고침 (버튼)
        self.btn_refresh = QPushButton("🔁 새로고침")
        self.btn_refresh.setFixedHeight(25)
        self.btn_connect = QPushButton("✅ 연결")
        self.btn_connect.setFixedHeight(25)
        
        # nuc
        self.btn_nuc = QPushButton("NUC")
        self.btn_nuc.setFixedHeight(25)
        
        # palette
        self.cb_palette = QComboBox(self)
        self.cb_palette.setFixedHeight(25)
        self.cb_palette.addItems(tlib._PALETTE_NAMES)
        self.cb_palette.setCurrentIndex(11)
        
        # agc
        self.cb_agc_process = QComboBox(self)
        self.cb_agc_process.setFixedHeight(25)
        self.cb_agc_process.addItem("Adaptive hist")
        self.cb_agc_process.addItem("Percentile")
        self.cb_agc_process.setCurrentIndex(0)
        
        # fps
        self.lb_fps = QLabel()
        self.lb_fps.setFixedHeight(25)
        
        # screen viewer
        self.video = VideoLabel()
        
        # span level bar
        self.spanlevelbar = AgcControl(self.thermal)
        
        # 기록 작업 시작/중지
        self.btn_save_start = QPushButton('🔴 기록 시작')
        self.btn_save_stop = QPushButton('⚫ 기록 중지')
        
        # 기록 대상 확인
        self.btn_basepath = QPushButton('기본경로 설정')
        self.btn_basepath.setFixedHeight(25)
        self.btn_basepath.setFixedWidth(100)
        self.lb_basepath = ElidedLabel(f'{os.getcwd()}')
        self.lb_basepath.setFixedWidth(140)
        self.lb_basepath.setFixedHeight(25)
        
        self.btn_target = QPushButton("저장위치 열기")
        self.btn_target.setFixedWidth(100)
        self.btn_target.setFixedHeight(25)
        self.lb_target = ElidedLabel('-')
        self.lb_target.setFixedWidth(140)
        self.lb_target.setFixedHeight(25)
        
        # 기록 로그
        self.log_list = QListWidget(self)
        self.log_list.setFixedWidth(250)
        
        ####################################################
        # Layout                                           #
        ####################################################
        top_hbox = QHBoxLayout()
        top_hbox.addWidget(self.cb_camera_list)
        top_hbox.addWidget(self.ip_address)
        top_hbox.addWidget(self.btn_refresh)
        top_hbox.addWidget(self.btn_connect)
        top_hbox.addStretch()
        
        sec_top_hbox = QHBoxLayout()
        sec_top_hbox.addWidget(self.btn_nuc)
        sec_top_hbox.addWidget(self.cb_palette)
        sec_top_hbox.addWidget(self.cb_agc_process)
        sec_top_hbox.addWidget(self.lb_fps)
        sec_top_hbox.addSpacing(200)
        
        lvbox = QVBoxLayout()
        rvbox = QVBoxLayout()
        
        lvbox.addLayout(top_hbox)
        lvbox.addLayout(sec_top_hbox)
        lvbox.addWidget(self.video)
        lvbox.addWidget(self.spanlevelbar)
        
        work_top_hbox = QHBoxLayout()
        work_top_hbox.addWidget(self.btn_save_start)
        work_top_hbox.addWidget(self.btn_save_stop)
        
        work_sec_hbox = QHBoxLayout()
        work_sec_hbox.addWidget(self.btn_basepath)
        work_sec_hbox.addWidget(self.lb_basepath)
        
        work_thr_hbox = QHBoxLayout()
        work_thr_hbox.addWidget(self.btn_target)
        work_thr_hbox.addWidget(self.lb_target)
        
        rvbox.addLayout(work_top_hbox)
        rvbox.addLayout(work_sec_hbox)
        rvbox.addLayout(work_thr_hbox)
        rvbox.addWidget(self.log_list)
        
        main_hbox = QHBoxLayout()
        main_hbox.addLayout(lvbox)
        main_hbox.addLayout(rvbox)
        
        # main_layout = QVBoxLayout()
        # main_layout.addLayout(top_hbox)
        # main_layout.addLayout(main_hbox)
        
        widget = QWidget()
        widget.setLayout(main_hbox)
        self.setCentralWidget(widget)


    def controlEnable(self, isconnect):
        self.cb_camera_list.setEnabled(not isconnect)
        self.ip_address.setEnabled(not isconnect)
        self.btn_refresh.setEnabled(not isconnect)
        self.btn_connect.setText("✅ 연결" if not isconnect else "❌ 연결해제")
        self.btn_nuc.setEnabled(isconnect)
        self.cb_palette.setEnabled(isconnect)
        self.cb_agc_process.setEnabled(isconnect)
        self.spanlevelbar.agc_change_signal.connect(self.setAGCType)
        self.btn_save_start.setEnabled(isconnect and not self.write_running)
        self.btn_save_stop.setEnabled(isconnect and self.write_running)
        self.log_list.setEnabled(isconnect)
        self.btn_target.setEnabled(isconnect)


    def closeEvent(self, event):
        if self.write_running:
            self.write_running = False
            self.write_queue.put((None, None))
            self.write_thread.join()
            self.write_thread = None
        if self.thermal.isConnected():
            self.thermal.stopCamera()
            self.thermal_thread.join()
            self.thermal_thread = None
        return super().closeEvent(event)
    
    
    @asyncSlot()
    async def start_discovery(self):
        try:
            self.cb_camera_list.setEnabled(False)
            self.ip_address.setEnabled(False)
            self.btn_refresh.setEnabled(False)
            self.btn_connect.setEnabled(False)
            self.cb_camera_list.clear()
            self.cb_camera_list.lineEdit().setPlaceholderText('🔍 카메라 찾는 중...')
            self.ip_address.setText('')
            self.cam_list = await zeroconf_discovery.discover_camera(SERVICE_NAME, 2)
            for c in self.cam_list:
                self.cb_camera_list.addItem(f'{c["ip"]}  ({c["mac"]})')
            
            self.cb_camera_list.setEnabled(True)
            self.ip_address.setEnabled(True)
            self.btn_refresh.setEnabled(True)
            self.btn_connect.setEnabled(True)
            self.cb_camera_list.setCurrentIndex(-1)
            self.cb_camera_list.lineEdit().setPlaceholderText('📷 접속할 카메라를 선택하세요')
            self.ip_address.setText('')
            self.cb_camera_list.currentIndexChanged.connect(self.on_iplist_change)
        except Exception as e:
            print(f"오류: {e}")
    
    
    def signal_connect(self):
        self.sig_connect_stat.connect(self.on_connect_stat)
        self.sig_write_stat.connect(self.on_write_stat)
        self.sig_fps_stat.connect(self.on_fps_stat)
        self.btn_refresh.clicked.connect(self.start_discovery)
        self.btn_connect.clicked.connect(self.on_btn_connect_clk)
        self.btn_nuc.clicked.connect(self.on_btn_nuc_clk)
        self.cb_palette.currentIndexChanged.connect(self.on_palette_change)
        self.cb_agc_process.currentIndexChanged.connect(self.on_agc_process_change)
        self.btn_save_start.clicked.connect(self.on_btn_savestart_clk)
        self.btn_save_stop.clicked.connect(self.on_btn_savestop_clk)
        self.btn_basepath.clicked.connect(self.on_btn_basepath_clk)
        self.btn_target.clicked.connect(self.on_btn_target_clk)
    
    
    def on_connect_stat(self, isconnect):
        self.controlEnable(isconnect)
    
    
    def on_btn_nuc_clk(self):
        self.thermal.onetimeNUC()
    
    
    def on_palette_change(self, index: int):
        self.thermal.setPaletteType(index)
    
    
    def on_agc_process_change(self, index: int):
        self.thermal.setAGCProcessType(index)
    
    
    def on_iplist_change(self, index: int):
        self.ip_address.setText(self.cam_list[index]["ip"])
        
    
    def on_btn_connect_clk(self):
        if self.cb_camera_list.currentIndex() == -1:
            return
        ip = self.ip_address.text()
        
        if self.thermal.isConnected():
            if self.write_running:
                self.write_running = False
                self.write_queue.put((None, None))
                self.write_thread.join()
                self.write_thread = None
            
            self.thermal.stopCamera()
            self.thermal_thread.join()
            self.thermal_thread = None
            self.log_list.clear()
        else:
            if self.thermal_thread is not None:
                self.thermal_thread.join()
                self.thermal_thread = None
            if self.thermal_thread is None:
                self.thermal_thread = threading.Thread(target=self.thermalFnc,
                                                    args=(ip,))
                self.thermal_thread.start()
    
    
    def thermalFnc(self, ip: str):
        self.thermal.startCamera(ip, 15001)
        ret = self.thermal.wait_for_response()
        if not ret.response:
            self.thermal.stopCamera()
            self.sig_connect_stat.emit(False)
            QMessageBox.critical(self,
                                 "COX ThermalCamera Viewer",
                                 f'"{ip}"카메라 연결에 실패하였습니다.')
            return
        
        # 연결 성공
        self.sig_connect_stat.emit(True)
        self.video.resolution = QSize(self.thermal.cfg.width, self.thermal.cfg.height)
        self.thermal.startStream()
        
        #clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        while True:
            data: tlib.ThermalReturn = self.thermal.get_recv_data()
            
            if not data.response:
                break
            
            fps = data.result_data["fps"]
            frm_buff = data.result_data["frm_buff"]
            temp_buff = frm_buff.temp_buff
            agc_min = frm_buff.agc_min
            agc_max = frm_buff.agc_max
            
            self.sig_fps_stat.emit(fps)
            
            dst = None
            if self.thermal.cfg.pal_type == tlib.define_enums.THERMAL_PALETTE_TYPE.PAL_GRAY:
                dst = frm_buff.gray
            else:
                dst = frm_buff.image
            
            if self.write_running:
                now = time.time()
                if self.last_tick is None:
                    self.write_queue.put((dst.copy(), temp_buff.copy()))
                    self.last_tick = time.time()
                elif 1.0 <= now - self.last_tick:
                    self.write_queue.put((dst.copy(), temp_buff.copy()))
                    self.last_tick = now
            
            hist, bin_edges = np.histogram(temp_buff, bins=256)
            self.spanlevelbar.setData(agc_min, agc_max, hist, bin_edges)
            
            if dst.size == self.thermal.cfg.width * self.thermal.cfg.height:
                self.video.setDisplayData(dst)
            else:
                self.video.setDisplayData(dst, QImage.Format.Format_BGR888)
        
        self.video.drawClear()
        self.spanlevelbar.clear()
        self.sig_connect_stat.emit(False)
    
    
    def setAGCType(self,
                   auto: bool,
                   agc_min: int | float,
                   agc_max: int | float):
        self.thermal.setAGCType(auto, agc_min, agc_max)
    
    
    def on_btn_savestart_clk(self):
        self.log_list.clear()
        self.btn_save_start.setEnabled(False)
        self.btn_save_stop.setEnabled(True)
        self.write_running = True
        self.last_tick = None
        self.write_thread = threading.Thread(target=self.fn_writethread)
        self.write_thread.start()
    
    
    def on_btn_savestop_clk(self):
        self.write_running = False
        self.last_tick = None
        if self.write_thread is not None:
            self.write_queue.put((None, None))
            self.write_thread.join()
            self.write_thread = None
        self.btn_save_start.setEnabled(True)
        self.btn_save_stop.setEnabled(False)
    
    
    def fn_writethread(self):
        ip = self.ip_address.text()
        executor = ThreadPoolExecutor(max_workers=2)
        
        now = datetime.now()
        folder = now.strftime('%Y%m%d_%H%M')
        self.lb_target.setText(f'output\\{folder}')
        path = os.path.join(self.lb_basepath.text(), 'output', folder)
        if not os.path.exists(path):
            os.makedirs(path)
        
        count = 0
        while self.write_running:
            img, tempbuff = self.write_queue.get()
            if img is None or tempbuff is None:
                continue
            
            now = datetime.now()
            suffix = now.strftime('%Y%m%d_%H%M%S')
            count += 1
            csv = os.path.join(path, f'{ip}_{count:05d}_{suffix}.csv')
            jpg = os.path.join(path, f'{ip}_{count:05d}_{suffix}.jpg')
            
            executor.submit(self.save_csv, csv, tempbuff)
            executor.submit(self.save_jpg, jpg, img)
            self.sig_write_stat.emit(f'[{count}] {suffix} CSV, JPG 저장')
        
        # ThreadPoolExecutor 종료 대기
        executor.shutdown(wait=True)
    
    def save_csv(self, file_path: str, data):
        np.savetxt(file_path, data, fmt="%.2f", delimiter=",")
    
    
    def save_jpg(self, file_path: str, data):
        cv2.imwrite(file_path, data)
    
    
    def on_write_stat(self, msg: str):
        self.log_list.addItem(msg)
        if 1000 < self.log_list.count():
            self.log_list.takeItem(0)
        
        last_idx = self.log_list.count() - 1
        last_item = self.log_list.item(last_idx)
        self.log_list.setCurrentItem(last_item)
        self.log_list.scrollToBottom()
        
    
    def on_btn_basepath_clk(self):
        dir = QFileDialog.getExistingDirectory(self, "폴더 선택")
        if dir:
            self.lb_basepath.setText(dir)
    
    
    def on_btn_target_clk(self):
        base = self.lb_basepath.text()
        output = self.lb_target.text()
        path = os.path.join(base, output)
        if not os.path.exists(path):
            return
        os.startfile(path)
    
    
    def on_fps_stat(self, fps: float):
        self.lb_fps.setText(f'FPS: {fps:.2f}')


if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setWindowIcon(QIcon(':smart_car'))
    
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)
    
    win = MainWindow()
    win.show()
    
    with loop:
        loop.run_forever()