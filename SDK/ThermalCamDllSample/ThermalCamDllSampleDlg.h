
// ThermalCamDllSampleDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxmt.h"

typedef enum
{
	PLAY,
	STOP,
	PAUSE,
	REWIND,
	FAST_FORWARD,
	BACKWARD_STEP,
	FORWARD_STEP,
	NONE
} IRF_PLAY_TYPE_T;

// CThermalCamDllSampleDlg 대화 상자
class CThermalCamDllSampleDlg : public CDialogEx
{
// 생성입니다.
public:
	CThermalCamDllSampleDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_THERMALCAMDLLSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	char* UTF8toANSI(char *pszCode);
public:
//Comm
	BOOL bConnect;
	HANDLE      m_hThread;      // Thread Comm handle
	HANDLE		m_hComm;		// Socket Comm handle	 
	HANDLE		keepAliveID;	
	LPCTSTR		strPort;
	CIPAddressCtrl m_IPCtrl;
	CString IPAddress;
	IRF_IR_CAM_DATA_T m_cam_data;		// Structure of ir-images to get from communication.
	IRF_IR_CAM_DATA_T cpy_cam_data;		// Structure of ir-images to get from communication.

	IRF_CAM_MODEL_TYPE_T	ir_model_type;		// Camera model type
	BOOL		m_f_LiveImage;

	IRF_RECEIVED_TEMP_TABLE_T receivedTempTable;

	BOOL	bReady;


	BOOL bCommStatus;

	BOOL WatchComm();       // Start Socket thread
	void SetImageInfo(unsigned short image_x, unsigned short image_y);
	static UINT WINAPI SocketThreadProc(LPVOID pParam);
	void CloseSocketThreadProc();	// Closed socket thread processing.
	void UpdateModelType( int _ver, int _sensor );
	afx_msg void OnBnClickedConnectBtn();
	afx_msg void OnBnClickedDisconnectBtn();
	
	void SetTemperatureTable(int temp_mode);

	void DoRecvSysInfo( IRF_IR_CAM_DATA_T& cam_data ) ;
	void DoRecvCamData( IRF_IR_CAM_DATA_T& cam_data ) ;
	void DoRecvStreamData( IRF_IR_CAM_DATA_T& cam_data ) ;

	virtual void Run();

//Raw Data & Raw Control
	HANDLE		m_hPlayThread;
	unsigned short* ir_image;
	unsigned char *m_image;	// bitmap image
	unsigned char *palette;
	float		*tempLUT;			// Temperature Map Table.
	IRF_IMAGE_INFO_T imageInfo;		// Image Information.
	int old_image_size;
	IRF_AUTO_RANGE_METHOD_T autoRangeMethod;	// Input and output range setting methods
	CStatic m_screen;	//실제 그려지는 다이얼로그상의 스크린변수
	short InitLive();
	void LiveImage();	// Initialize Image Display.
	void MakePalette(IRF_PALETTE_TYPE_T type, BOOL paletteInv);
	afx_msg LRESULT OnErrorNotificationUserHandler(WPARAM wParam, LPARAM lParam);		// User Message Handler
	afx_msg LRESULT OnDisplayImageHandler(WPARAM wParam, LPARAM lParam);		// User Message Handler
	afx_msg LRESULT OnGetSaveDataHandler(WPARAM wParam, LPARAM lParam);		// User Message Handler
	afx_msg void OnBnClickedLiveBtn();
	afx_msg void OnBnClickedFreezeBtn();

//Image Control
	BOOL bDisplayStatus;			// display status
	int BitsPixel;					// Bits pixel of device graphic
	int imageX;						// start x coordinate in client area.
	int imageY;						// start y coordinate in client area.
	CComboBox m_Palette;
	BOOL m_bPaletteInverter;
	BOOL m_bImageMirror;
	BOOL m_bAutoAdjust;		//CheckBox 변수
	float fDlgLevelValue;
	float fDlgSpanValue;
	CString m_TempScale;
	short minSpan;		// minimum span.
	short maxSpan;		// maximum span.
	short curLevel;		// current level.
	short curSpan;		// current span.
	short tempRangeMin;
	short tempRangeMax;
	BOOL m_bPaletteInv;
	BOOL m_bImgMirror;
	BOOL m_bImgFlip;
	float m_fImageLevel;
	float m_fImageSpan;	
	void ImageDisplay(CDC * dc, int x, int y, int width, int height, double StretchFactorX, double StrectchFactorY);
	afx_msg void OnCbnSelchangePaletteCombo();
	afx_msg void OnBnClickedInvertpalCheck();
	afx_msg void OnBnClickedMirrorCheck();
	afx_msg void OnBnClickedFlipChk();
	afx_msg void OnBnClickedAutoadjustCheck();

//CameraControl
	CComboBox m_TempCombo;
	int m_Alarm_Func1;
	int m_AlarmCond1;
	int m_AlarmVal1;
	afx_msg void OnBnClickedNucBtn();
	afx_msg void OnCbnSelchangeTemprangeComboV2();
	afx_msg void OnBnClickedAlarmSetBtn();

//Spot
	IRF_TEMP_CORRECTION_PAR_T corrPara;
	BOOL m_bSpotChk;
	POINT m_spotPoint;
	int m_DlgSpotX;
	int m_DlgSpotY;
	float m_fDlgSpotValue;
	afx_msg void OnBnClickedSpotCheck();
	afx_msg void OnBnClickedSpotApplyButton();

//ROI
	BOOL m_bRoiChk;
	int m_DlgRoiX;
	int m_DlgRoiY;
	int m_DlgRoiW;
	int m_DlgRoiH;
	RECT m_RoiRect;
	POINT m_RoiHighPoint;
	POINT m_RoiLowPoint;
	float m_fRoiAvgVal;
	float m_fRoiStdVal;
	float m_fRoiHighVal;
	float m_fRoiLowVal;
	int m_DlgRoiHighX;
	int m_DlgRoiHighY;
	int m_DlgRoiLowX;
	int m_DlgRoiLowY;
	afx_msg void OnBnClickedRoiCheck();
	afx_msg void OnBnClickedRoiApplyButton();

//Filter
	int* gaussianMapTable;
	int* gaussianKernel;
	CSliderCtrl m_fastGSlider;
	CSliderCtrl m_fastSSlider;
	CSliderCtrl m_boxBlurSlider;
	CSliderCtrl m_BiDomainSlider;
	CSliderCtrl m_BiRangeSlider;
	int gaussianSize;
	float bilateral_sigD;
	float bilateral_sigR;
	int m_nFilterSelect;
	BOOL bGaussianKernelSizeUpdatedFlag;
	afx_msg void OnBnClickedFilterNoneRadio();
	afx_msg void OnBnClickedFilterMedianRadio();
	afx_msg void OnBnClickedFilterSoft1Radio();
	afx_msg void OnBnClickedFilterSoft2Radio();
	afx_msg void OnBnClickedFilterSharp1Radio();
	afx_msg void OnBnClickedFilterSharp2Radio();
	afx_msg void OnBnClickedFilterBiRadio();
	afx_msg void OnBnClickedFilterFastgRadio();
	afx_msg void OnBnClickedFilterFastsRadio();
	afx_msg void OnBnClickedFilterBoxRadio();
	afx_msg void OnNMReleasedcaptureFastGSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureFastSSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureBoxblurSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureBiDSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureBiRSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSetLevelSpanButton();
	
//file play
	afx_msg void OnBnClickedFileOpenButton();
	BOOL GetSocketStatus(void);
	HANDLE file_handle;
	void CloseFileThreadProc(void);
	IRF_PLAY_TYPE_T playStatus;
	bool bSuspendThread;
	unsigned long totSizeIRStream;
	IRF_IR_FILE_HEADER_T* pFileHeader;
	IRF_IR_DATA_HEADER_T* pFileAddHeader;
	unsigned long curPosition;
	void OperateIRPlayer(IRF_PLAY_TYPE_T action);
	void FileThreadPause(bool bPause);
	bool StartPlayThread(void);
	static UINT WINAPI PlayThreadProc(LPVOID pParam);
	void PlayThreadRun();
	int UpperCnt;
	int LowerCnt;
	short SetPlayType(IRF_PLAY_TYPE_T playType, int* frame_time);
	bool bDisplayStart;
	float period;			// Period.
	void sleep(unsigned int mseconds);
	long long savedTime;
	bool m_bPlayLoop;
	BOOL m_runPlayThread;

//file save
	bool bRecording;
	afx_msg void OnBnClickedRecordIrButton();
	afx_msg void OnBnClickedStopRecordIrButton();
	CWinThread* m_thread_handle;
	bool bSaveOver;
	bool bRecordLedNone;
	void SaveIRStream(void);
	HANDLE record_handle;
	IRF_IR_FILE_HEADER_T* pFileSaveHeader;
	IRF_IR_DATA_HEADER_T* pFileAddSaveHeader;
	IRF_DYNAMIC_RANGE_T GetDynamicTempRange(void);
	short frameCount;
	void SaveIRImages(int intervalTime);
	afx_msg void OnBnClickedBtnSaveOffset();
	afx_msg void OnBnClickedBtnLoadOffset();
};
