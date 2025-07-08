
// ThermalCamDllSampleDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ThermalCamDllSample.h"
#include "ThermalCamDllSampleDlg.h"
#include "afxdialogex.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CThermalCamDllSampleDlg 대화 상자




CThermalCamDllSampleDlg::CThermalCamDllSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CThermalCamDllSampleDlg::IDD, pParent)
	, m_Alarm_Func1(0)
	, m_AlarmCond1(0)
	, m_AlarmVal1(0)
	, m_bPaletteInverter(FALSE)
	, m_bImageMirror(FALSE)
	, m_bAutoAdjust(FALSE)
	, fDlgLevelValue(20.0f)
	, fDlgSpanValue(10.0f)
	, m_TempScale(_T(""))
	, m_bPaletteInv(FALSE)
	, m_bImgMirror(FALSE)
	, m_bImgFlip(FALSE)
	, m_fImageLevel(0)
	, m_fImageSpan(0)
	, m_bSpotChk(FALSE)
	, m_bRoiChk(FALSE)
	, m_DlgSpotX(0)
	, m_DlgSpotY(0)
	, m_fDlgSpotValue(0)
	, m_DlgRoiX(0)
	, m_DlgRoiY(0)
	, m_DlgRoiW(100)
	, m_DlgRoiH(100)
	, m_fRoiAvgVal(0)
	, m_fRoiStdVal(0)
	, m_fRoiHighVal(0)
	, m_fRoiLowVal(0)
	, m_DlgRoiHighX(0)
	, m_DlgRoiHighY(0)
	, m_DlgRoiLowX(0)
	, m_DlgRoiLowY(0)
	, gaussianMapTable(NULL)
	, gaussianKernel(NULL)
	, gaussianSize(3)
	, bilateral_sigD(0.1f)
	, bilateral_sigR(0.5f)
	, m_nFilterSelect(0)
	, bCommStatus(FALSE)
	, bGaussianKernelSizeUpdatedFlag(TRUE)
	, file_handle(0)
	, bSuspendThread(false)
	, totSizeIRStream(0)
	, pFileHeader(NULL)
	, pFileAddHeader(NULL)
	, curPosition(0)
	, bDisplayStart(false)
	, period(1.0f)
	, savedTime(0)
	, m_bPlayLoop(true)
	, m_hPlayThread(0)
	, m_thread_handle(NULL)
	, bSaveOver(false)
	, bRecordLedNone(false)
//	, nSaveCond(0)
	, pFileSaveHeader(NULL)
	, pFileAddSaveHeader(NULL)
	, bRecording(false)
	, frameCount(0)
	, record_handle(NULL)
	, m_f_LiveImage(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hThread = NULL;	// socket thread
	m_image = NULL;
	tempLUT = NULL;
	bDisplayStatus = FALSE;
	BitsPixel = 0;
	imageX = 0;
	imageY = 0;
	palette = NULL;

	m_hComm = INVALID_HANDLE_VALUE;
	ir_image = NULL;
	//imageInfo.bitPixel = 16;
	imageInfo.xSize = 320;	// image x size : 384
	imageInfo.ySize = 240;
	old_image_size = imageInfo.xSize * imageInfo.ySize;
	m_cam_data.ir_image = new unsigned short[imageInfo.xSize*imageInfo.ySize*IRF_MAX_IR_BUFFER_SIZE];
	m_cam_data.image_buffer_size = imageInfo.xSize*imageInfo.ySize;
	m_cam_data.lpNextData = new BYTE[IRF_BUFFER_SIZE];
	m_cam_data.dwSize = 0L;
	m_cam_data.dwPosition = 0L;
	memset(&cpy_cam_data, 0x00, sizeof(IRF_IR_CAM_DATA_T));

	autoRangeMethod.autoScale = _IRF_AUTO;
	autoRangeMethod.inputMethod = (IRF_AUTO_RANGE_INPUT_METHOD_T)2;		// _IRF_SD_RATE;
	autoRangeMethod.SD_Rate = 5.0f;		// 5.0f;	// Parameter of Standard Deviation method. (Mu-SD_Rate*SD ~ Mu+SD_Rate*SD)
	autoRangeMethod.B_Rate = 0.01f;		// 0.01f;	// 1% of input range.
	autoRangeMethod.outputMethod = (IRF_AUTO_RANGE_OUTPUT_METHOD_T)0;		// _IRF_LINEAR;
	autoRangeMethod.intercept = 0;		// 0;	// Parameter of linear method. (0 <= intercept <= 255)
	autoRangeMethod.gamma = 1.0f;		// 1;		// Parameter of non-linear method. (0.5 <= gamma <= 25)
	autoRangeMethod.plateau = 100;		// 100;	// Plateau is a threshold for Plateau Equalization.
	autoRangeMethod.epsilon = 0.5f;		// 0.5f;	// Using Adaptive Plateau Equalization. (0 <= epsilon <= 1)
	autoRangeMethod.psi = 0.3f;		// 0.3f;		// Using Adaptive Plateau Equalization. (0 <= psi <= 1)
	autoRangeMethod.prevPalteau = 0;		// 0;

	corrPara.atmTemp = 20.0f;
	corrPara.atmTrans = 1.0f;
	corrPara.emissivity = 0.95f;


	IPAddress = _T("192.168.0.5");

	bConnect = FALSE;
	bReady = FALSE;

	strPort = _T("15001");

	m_bAutoAdjust = TRUE;

	//criticalComm.

	memset(&m_RoiRect, 0, sizeof(RECT));
	memset(&m_spotPoint, 0, sizeof(POINT));
	
	pFileHeader = new IRF_IR_FILE_HEADER_T;	// file header.
	pFileAddHeader = new IRF_IR_DATA_HEADER_T;
	
	pFileSaveHeader = new IRF_IR_FILE_HEADER_T;
	pFileAddSaveHeader = new IRF_IR_DATA_HEADER_T;
	UpperCnt=2;
	LowerCnt=2;

}
void CThermalCamDllSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCREEN, m_screen);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPCtrl);
	DDX_Radio(pDX, IDC_FUNC_OFF, m_Alarm_Func1);
	DDX_Radio(pDX, IDC_COND_ABOVE, m_AlarmCond1);
	DDX_Text(pDX, IDC_ALARM1_VAL, m_AlarmVal1);
	DDX_Control(pDX, IDC_PALETTE_COMBO, m_Palette);
	DDX_Text(pDX, IDC_LEVEL_EDIT, fDlgLevelValue);
	DDX_Text(pDX, IDC_TEMPSCALE_STATIC, m_TempScale);
	DDX_Check(pDX, IDC_INVERTPAL_CHECK, m_bPaletteInv);
	DDX_Check(pDX, IDC_MIRROR_CHECK, m_bImgMirror);
	DDX_Check(pDX, IDC_FLIP_CHK, m_bImgFlip);
	DDX_Control(pDX, IDC_TEMPRANGE_COMBO_V2, m_TempCombo);
	DDX_Control(pDX, IDC_TEMPRANGE_COMBO_V2, m_TempCombo);
	DDX_Text(pDX, IDC_SPAN_EDIT, fDlgSpanValue);
	DDX_Check(pDX, IDC_SPOT_CHECK, m_bSpotChk);
	DDX_Check(pDX, IDC_ROI_CHECK, m_bRoiChk);
	DDX_Text(pDX, IDC_SPOT_X_EDIT, m_DlgSpotX);
	DDX_Text(pDX, IDC_SPOT_Y_EDIT, m_DlgSpotY);
	DDX_Text(pDX, IDC_SPOT_VAL_EDIT, m_fDlgSpotValue);
	DDX_Text(pDX, IDC_ROI_X_EDIT, m_DlgRoiX);
	DDX_Text(pDX, IDC_ROI_Y_EDIT, m_DlgRoiY);
	DDX_Text(pDX, IDC_ROI_W_EDIT, m_DlgRoiW);
	DDX_Text(pDX, IDC_ROI_H_EDIT, m_DlgRoiH);
	DDX_Text(pDX, IDC_ROI_AVG_EDIT, m_fRoiAvgVal);
	DDX_Text(pDX, IDC_ROI_STD_EDIT, m_fRoiStdVal);
	DDX_Text(pDX, IDC_ROI_HIGH_VAL_EDIT, m_fRoiHighVal);
	DDX_Text(pDX, IDC_ROI_LOW_VAL_EDIT, m_fRoiLowVal);
	DDX_Text(pDX, IDC_ROI_HIGH_X_EDIT, m_DlgRoiHighX);
	DDX_Text(pDX, IDC_ROI_HIGH_Y_EDIT, m_DlgRoiHighY);
	DDX_Text(pDX, IDC_ROI_LOW_X_EDIT, m_DlgRoiLowX);
	DDX_Text(pDX, IDC_ROI_LOW_Y_EDIT, m_DlgRoiLowY);
	DDX_Radio(pDX, IDC_FILTER_NONE_RADIO, m_nFilterSelect);
	DDX_Control(pDX, IDC_FAST_G_SLIDER, m_fastGSlider);
	DDX_Control(pDX, IDC_FAST_S_SLIDER, m_fastSSlider);
	DDX_Control(pDX, IDC_BOXBLUR_SLIDER, m_boxBlurSlider);
	DDX_Control(pDX, IDC_BI_D_SLIDER, m_BiDomainSlider);
	DDX_Control(pDX, IDC_BI_R_SLIDER, m_BiRangeSlider);
}

BEGIN_MESSAGE_MAP(CThermalCamDllSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(MY_MESSAGE1, OnErrorNotificationUserHandler)
	ON_MESSAGE(MY_MESSAGE2, OnDisplayImageHandler)
	ON_MESSAGE(MY_MESSAGE3, OnGetSaveDataHandler)
	ON_BN_CLICKED(IDC_CONNECT_BTN, &CThermalCamDllSampleDlg::OnBnClickedConnectBtn)
	ON_BN_CLICKED(IDC_DISCONNECT_BTN, &CThermalCamDllSampleDlg::OnBnClickedDisconnectBtn)
	ON_BN_CLICKED(IDC_LIVE_BTN, &CThermalCamDllSampleDlg::OnBnClickedLiveBtn)
	ON_BN_CLICKED(IDC_FREEZE_BTN, &CThermalCamDllSampleDlg::OnBnClickedFreezeBtn)
	ON_BN_CLICKED(IDC_ALARM_SET_BTN, &CThermalCamDllSampleDlg::OnBnClickedAlarmSetBtn)
	ON_BN_CLICKED(IDC_NUC_BTN, &CThermalCamDllSampleDlg::OnBnClickedNucBtn)
	ON_CBN_SELCHANGE(IDC_PALETTE_COMBO, &CThermalCamDllSampleDlg::OnCbnSelchangePaletteCombo)
	ON_BN_CLICKED(IDC_INVERTPAL_CHECK, &CThermalCamDllSampleDlg::OnBnClickedInvertpalCheck)
	ON_BN_CLICKED(IDC_MIRROR_CHECK, &CThermalCamDllSampleDlg::OnBnClickedMirrorCheck)
	ON_BN_CLICKED(IDC_FLIP_CHK, &CThermalCamDllSampleDlg::OnBnClickedFlipChk)
	ON_BN_CLICKED(IDC_AUTOADJUST_CHECK, &CThermalCamDllSampleDlg::OnBnClickedAutoadjustCheck)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_TEMPRANGE_COMBO_V2, &CThermalCamDllSampleDlg::OnCbnSelchangeTemprangeComboV2)
	ON_BN_CLICKED(IDC_SPOT_CHECK, &CThermalCamDllSampleDlg::OnBnClickedSpotCheck)
	ON_BN_CLICKED(IDC_ROI_CHECK, &CThermalCamDllSampleDlg::OnBnClickedRoiCheck)
	ON_BN_CLICKED(IDC_SPOT_APPLY_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedSpotApplyButton)
	ON_BN_CLICKED(IDC_ROI_APPLY_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedRoiApplyButton)
	ON_BN_CLICKED(IDC_FILTER_NONE_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterNoneRadio)
	ON_BN_CLICKED(IDC_FILTER_MEDIAN_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterMedianRadio)
	ON_BN_CLICKED(IDC_FILTER_SOFT1_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterSoft1Radio)
	ON_BN_CLICKED(IDC_FILTER_SOFT2_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterSoft2Radio)
	ON_BN_CLICKED(IDC_FILTER_SHARP1_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterSharp1Radio)
	ON_BN_CLICKED(IDC_FILTER_SHARP2_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterSharp2Radio)
	ON_BN_CLICKED(IDC_FILTER_BI_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterBiRadio)
	ON_BN_CLICKED(IDC_FILTER_FASTG_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterFastgRadio)
	ON_BN_CLICKED(IDC_FILTER_FASTS_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterFastsRadio)
	ON_BN_CLICKED(IDC_FILTER_BOX_RADIO, &CThermalCamDllSampleDlg::OnBnClickedFilterBoxRadio)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_FAST_G_SLIDER, &CThermalCamDllSampleDlg::OnNMReleasedcaptureFastGSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_FAST_S_SLIDER, &CThermalCamDllSampleDlg::OnNMReleasedcaptureFastSSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BOXBLUR_SLIDER, &CThermalCamDllSampleDlg::OnNMReleasedcaptureBoxblurSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BI_D_SLIDER, &CThermalCamDllSampleDlg::OnNMReleasedcaptureBiDSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BI_R_SLIDER, &CThermalCamDllSampleDlg::OnNMReleasedcaptureBiRSlider)
	ON_BN_CLICKED(IDC_SET_LEVEL_SPAN_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedSetLevelSpanButton)
	ON_BN_CLICKED(IDC_FILE_OPEN_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedFileOpenButton)
	ON_BN_CLICKED(IDC_START_RECORD_IR_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedRecordIrButton)
	ON_BN_CLICKED(IDC_STOP_RECORD_IR_BUTTON, &CThermalCamDllSampleDlg::OnBnClickedStopRecordIrButton)
	ON_BN_CLICKED(IDC_BTN_SAVE_OFFSET, &CThermalCamDllSampleDlg::OnBnClickedBtnSaveOffset)
	ON_BN_CLICKED(IDC_BTN_LOAD_OFFSET, &CThermalCamDllSampleDlg::OnBnClickedBtnLoadOffset)
END_MESSAGE_MAP()


// CThermalCamDllSampleDlg 메시지 처리기

BOOL CThermalCamDllSampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	CThermalCamDllSampleApp *		pApp;

	TCHAR				tc_text[256];
	

	pApp = (CThermalCamDllSampleApp *)(AfxGetApp());

#if defined(_M_X64)
	::_stprintf_s( tc_text, _T("ThermalCamDllSample-64Bits, (Version : %s)"), pApp->m_szFileVersion.GetString() );
#elif defined(_M_IX86)
	::_stprintf_s( tc_text, _T("ThermalCamDllSample-32Bits, (Version : %s)"), pApp->m_szFileVersion.GetString() );
#else
	DebugBreak();
#endif

	::SetWindowText( this->GetSafeHwnd(), tc_text );



	// Palette Setting.
	m_Palette.AddString(_T("Yellow"));
	m_Palette.AddString(_T("Rainbow"));
	m_Palette.AddString(_T("Rain900"));
	m_Palette.AddString(_T("Rain10"));
	m_Palette.AddString(_T("Midgrey"));
	m_Palette.AddString(_T("Midgreen"));
	m_Palette.AddString(_T("Medical"));
	m_Palette.AddString(_T("Iron10"));
	m_Palette.AddString(_T("Iron"));
	m_Palette.AddString(_T("Greyred"));
	m_Palette.AddString(_T("Grey10"));
	m_Palette.AddString(_T("Grey"));
	m_Palette.AddString(_T("Glowbow"));
	m_Palette.SetCurSel(11);

	// Gray Image
	if(m_image == NULL)		
		m_image = new unsigned char[imageInfo.xSize*imageInfo.ySize];

	// Make palette.
	// Allocate memory if not allocated and get palette information from ThermalCamDll
	if(palette == NULL)
		palette = new unsigned char[256*3];	// Number of red, green, blue and temporary is 256, individually.

	if(GetImageLUT(palette, GREY_COLOR_MAP, FALSE) != IRF_NO_ERROR)
	{
		MessageBox(_T("Palette File Open Error!!"), _T("Error Message"), MB_OK);
	}	

	m_IPCtrl.SetAddress( 192, 168, 1, 1 );	/* Get IP address from IP address control.*/

	((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
	((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);


	// Dynamic Temperature Range Setting.
	CString outTempRangeH;
	CString outTempRangeM;
	
	outTempRangeM.Format("-20 ~ 120 'C");
	outTempRangeH.Format("-20 ~ 650 'C");
	m_TempCombo.AddString(outTempRangeM);
	m_TempCombo.AddString(outTempRangeH);
	m_TempCombo.SetCurSel(0);


	CheckDlgButton(IDC_AUTOADJUST_CHECK, TRUE);
	autoRangeMethod.autoScale = _IRF_AUTO;
	
	((CEdit*)GetDlgItem(IDC_LEVEL_EDIT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_SPAN_EDIT))->EnableWindow(FALSE);	
	((CButton*)GetDlgItem(IDC_SET_LEVEL_SPAN_BUTTON))->EnableWindow(FALSE);	

	((CEdit*)GetDlgItem(IDC_SPOT_X_EDIT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_SPOT_Y_EDIT))->EnableWindow(FALSE);

	((CEdit*)GetDlgItem(IDC_ROI_X_EDIT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_ROI_Y_EDIT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_ROI_W_EDIT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_ROI_H_EDIT))->EnableWindow(FALSE);
	/*
	((CButton*)(GetDlgItem(IDC_FILTER_NONE_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_MEDIAN_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_SOFT1_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_SOFT2_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_SHARP1_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_SHARP2_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_BI_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_FASTG_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_FASTS_RADIO)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FILTER_BOX_RADIO)))->EnableWindow(FALSE);
	*/
	//Set default value spot/roi var.
	OnBnClickedSpotApplyButton();	
	OnBnClickedRoiApplyButton();
	
	// Set Filter Slider/Static Value default
	m_fastGSlider.SetRange(1,10);
	m_fastGSlider.SetPos(1);
	m_fastSSlider.SetRange(3,10);
	m_fastSSlider.SetPos(3);
	m_boxBlurSlider.SetRange(3,10);
	m_boxBlurSlider.SetPos(3);
	m_BiDomainSlider.SetRange(1,500);
	m_BiDomainSlider.SetPos(1);
	m_BiRangeSlider.SetRange(5,2500);
	m_BiRangeSlider.SetPos(5);


	CString strBuf;
	strBuf.Format("%d",m_fastGSlider.GetPos());
	SetDlgItemTextA(IDC_FAST_G_STATIC,strBuf);
	strBuf.Format("%d",m_fastSSlider.GetPos());
	SetDlgItemTextA(IDC_FAST_S_STATIC,strBuf);
	strBuf.Format("%d",m_boxBlurSlider.GetPos());
	SetDlgItemTextA(IDC_BOXBLUR_STATIC,strBuf);
	strBuf.Format("%.1f",(float)m_BiDomainSlider.GetPos() / 10.0);
	SetDlgItemTextA(IDC_BI_D_STATIC,strBuf);
	strBuf.Format("%.1f",(float)m_BiRangeSlider.GetPos()/10.0);
	SetDlgItemTextA(IDC_BI_R_STATIC,strBuf);

	UpdateData(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}
void CThermalCamDllSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CThermalCamDllSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CThermalCamDllSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CThermalCamDllSampleDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_f_LiveImage = FALSE;

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CloseSocketThreadProc();
	
	if(file_handle != NULL)	
		CloseFileThreadProc();

	// 파일 스트림이 열려있으면 닫는다.;
	if( file_handle != NULL)
	{
		CloseIRCamDataCG( &cpy_cam_data );

		if ( CloseIRStream(file_handle) != IRF_NO_ERROR )
		{
			MessageBox(_T("File close error!"), _T("Error Message"), MB_OK);		// file close error.
		}
		else
			file_handle = NULL;
	}
	if(m_cam_data.ir_image != NULL)	delete [] m_cam_data.ir_image;
	if(m_cam_data.lpNextData != NULL)	delete [] m_cam_data.lpNextData;

	// delete allocated memory
	if(ir_image != NULL)	delete [] ir_image;
	if(m_image != NULL)		delete [] m_image;
	if(palette != NULL)		delete [] palette;
	if(tempLUT != NULL)		delete [] tempLUT;
	if(gaussianMapTable != NULL)	delete [] gaussianMapTable;
	if(pFileHeader != NULL) delete pFileHeader;
	if(pFileAddHeader != NULL) delete pFileAddHeader;
	if(pFileSaveHeader != NULL) delete pFileSaveHeader;
	if(pFileAddSaveHeader != NULL) delete pFileAddSaveHeader;
}

// 사용자 메시지 (WM_USER + 1)
LRESULT CThermalCamDllSampleDlg::OnErrorNotificationUserHandler(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case IRF_CAMERA_CONNECTION_ERROR:
		// 오류가 발생할 때 Image Dialog의 활성;
		MessageBox(_T("Connection error from a camera!"), _T("Error Notification"), MB_OK);
		CloseSocketThreadProc();
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		break;
	case IRF_BUFFER_ALLOCATION_ERROR:
		MessageBox(_T("Failed a buffer allocation!"), _T("Error Notification"), MB_OK);
		CloseSocketThreadProc();
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		break;
	case IRF_CAMERA_DISCONNECTION:	// Disconnected from a camera
		if(bConnect)
		{
			MessageBox(_T("Disconnected signal from a camera!"), _T("Error Notification"), MB_OK);
			CloseSocketThreadProc();
			((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
			((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
			((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
			((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		}
		break;
	case IRF_NAK:
		if(wParam == 2)
		{			
			MessageBox(_T("Camera's packet ID isn't equal to PC's packet ID.\n Please update your PC software!"), _T("Error Notification"), MB_OK);
		}
		else if(wParam == 1)
		{
			MessageBox(_T("Unknown command!"), _T("Error Notification"), MB_OK);
		}
		CloseSocketThreadProc();
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		break;
	default:
		CloseSocketThreadProc();
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		break;
	}

	return 0;
}

// User Message (WM_USER + 2)
LRESULT CThermalCamDllSampleDlg::OnDisplayImageHandler(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;
	// check destroyed
	if ( FALSE == m_f_LiveImage ) {
		return 0;
	}

	LiveImage();

	return 0;
}


// User Message (WM_USER + 3)
LRESULT CThermalCamDllSampleDlg::OnGetSaveDataHandler(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;
	if(m_cam_data.save_data.alarm1_func > 4)	m_Alarm_Func1 = 0;
	else									m_Alarm_Func1 = m_cam_data.save_data.alarm1_func;

	if(m_cam_data.save_data.alarm1_cond > 1)	m_AlarmCond1 = 0;
	else									m_AlarmCond1 = m_cam_data.save_data.alarm1_cond;

	m_AlarmVal1 = (int)(m_cam_data.save_data.alarm1_value*0.1);

	UpdateData(FALSE);

	return 0;
}

// Initialized Image Display
short CThermalCamDllSampleDlg::InitLive()
{
	// Get display pixel bits from device
	CDC *pDC = GetDC();
	BitsPixel = pDC->GetDeviceCaps(BITSPIXEL);
	ReleaseDC(pDC);

	return IRF_NO_ERROR;
}

// Play IR images
void CThermalCamDllSampleDlg::LiveImage()
{
	bDisplayStatus = TRUE;	// display status : on
	if (autoRangeMethod.autoScale == _IRF_MANUAL)	//Get Dlg's Level & Span
	{
		m_fImageLevel = curLevel;
		m_fImageSpan = curSpan;
	}

	if(ir_image == NULL) {
		ir_image = new unsigned short[imageInfo.xSize*imageInfo.ySize];	// (320 * 240 * 2 bytes)/frame.
	}

	if ( NULL == cpy_cam_data.ir_image ) {
		cpy_cam_data.ir_image = ir_image;
	}

	// Change from 16 bits thermal data to 8 bit image.
	GetImage( m_image, (HANDLE)&cpy_cam_data, imageInfo.xSize * imageInfo.ySize, tempLUT, &m_fImageLevel, &m_fImageSpan, &autoRangeMethod );
	TRACE( _T("%f %f\r\n"), m_fImageLevel, m_fImageSpan );

	// Before Image Display
	// if filter selected
	if (m_nFilterSelect != 0)
	{
		switch(m_nFilterSelect)
		{
		case 1:		//soft slightly
			ApplyImageFilter(m_image,imageInfo.xSize,imageInfo.ySize,_IRF_SOFTEN_SLIGHTLY);
			break;
		case 2:		//soft strong
			ApplyImageFilter(m_image,imageInfo.xSize,imageInfo.ySize,_IRF_SOFTEN_STRONG);
			break;
		case 3:		//median
			ApplyImageFilter(m_image,imageInfo.xSize,imageInfo.ySize,_IRF_MEDIAN);
			break;
		case 4:		//sharp slightly
			ApplyImageFilter(m_image,imageInfo.xSize,imageInfo.ySize,_IRF_SHARPENING_SLIGHTLY);
			break;
		case 5:		//sharp strong
			ApplyImageFilter(m_image,imageInfo.xSize,imageInfo.ySize,_IRF_SHARPENING_STRONG);
			break;
		case 6:		//Fast Gaussian
			if (bGaussianKernelSizeUpdatedFlag == TRUE)
			{
				bGaussianKernelSizeUpdatedFlag = FALSE;
				if(gaussianMapTable != NULL)		// Deallocation of dynamic variables when entering repetitive
					delete [] gaussianMapTable;
				if(gaussianKernel != NULL)
					delete [] gaussianKernel;
				
				gaussianKernel = new int[gaussianSize];			// gaussianSize = 1,3,5,7....
				gaussianMapTable = new int[gaussianSize*256];

				memset(gaussianKernel,0,sizeof(int)*gaussianSize);			//always needs to be initialized.
				memset(gaussianMapTable,0,sizeof(int)*gaussianSize*256);	//either.

				GetGaussianKernel(gaussianKernel, gaussianMapTable, gaussianSize/2);	// Before "FastGaussianBlur ()", this func. must be called.
			}
			if((gaussianMapTable != NULL) && (gaussianKernel != NULL)){		// To avoid memory access violation.
				FastGaussianBlur(m_image, imageInfo.xSize, imageInfo.ySize, gaussianKernel, gaussianMapTable, gaussianSize/2);
			}
			break;
		case 7:		//Fast Stack Blur
			FastStackBlur(m_image,imageInfo.xSize,imageInfo.ySize,gaussianSize/2);
			break;
		case 8:		//Box Blur
			BoxBlur(m_image,imageInfo.xSize,imageInfo.ySize,gaussianSize/2);
			break;
		case 9:		//BilateralFilter
			BilateralFilter(m_image,imageInfo.xSize,imageInfo.ySize,bilateral_sigD,bilateral_sigR,1);
			break;
		}
	}

	CDC *pDC = m_screen.GetDC();
	ASSERT(pDC != NULL);

	CRect rect;		
	m_screen.GetClientRect(&rect);

	// set clipping area.
	CRgn rgn;
	rgn.CreateRectRgn(0, 0, rect.Width(), rect.Height());
	pDC->SelectClipRgn(&rgn);

	//Actual Drawing
	ImageDisplay(pDC, imageX, imageY, imageInfo.xSize, imageInfo.ySize, (float)(rect.Width()/(float)imageInfo.xSize), (float)(rect.Height()/(float)imageInfo.ySize));
	ReleaseDC(pDC);

	//
	bDisplayStatus = FALSE;	// display status : off

}

// Image Display
void CThermalCamDllSampleDlg::ImageDisplay(CDC * dc, int x, int y, int width, int height, double StretchFactorX, double StrectchFactorY)
{
	CBitmap *bm = NULL;
	CDC *bdc = NULL;

	if(bm==NULL)
	{
		bm = new CBitmap;
		bm->CreateBitmap(width, height, 1, BitsPixel, NULL);
	}

	if(bdc==NULL)
	{
		bdc = new CDC;
		bdc->CreateCompatibleDC(dc);
		bdc->SelectObject(bm);
	}

	if (BitsPixel==32) {	// 32 bits	
		unsigned int *h = new unsigned int[width*height];
		memset(h,RGB(160,160,160),sizeof(unsigned int) * width * height);
		GetGrayToPaletteImage(m_image, h, (uint16_t)width, (uint16_t)height, palette, BitsPixel, m_bImgMirror, m_bImgFlip);	// ThermalCamDll
		bm->SetBitmapBits(4*width*height,h);	
		delete [] h;
	}
	else if (BitsPixel==16) {	// 16 bits
		unsigned short *h = new unsigned short[width*height];		
		memset(h,RGB(160,160,160),sizeof(unsigned short) * width * height);
		GetGrayToPaletteImage(m_image, h, (uint16_t)width, (uint16_t)height, palette, BitsPixel, m_bImgMirror, m_bImgFlip);	

		bm->SetBitmapBits(2*width*height, h);     
		delete [] h;
	}
	else if (BitsPixel==24) {		// 24 bits
		unsigned char *h = new unsigned char[3*width*height];
		memset(h,RGB(160,160,160),sizeof(unsigned char) * 3 * width * height);
		GetGrayToPaletteImage(m_image, h, (uint16_t)width, (uint16_t)height, palette, BitsPixel, m_bImgMirror, m_bImgFlip);

		bm->SetBitmapBits(3*width*height,h);
		delete [] h;
	}

	dc->SetStretchBltMode(HALFTONE);

	dc->StretchBlt(x, y, (int)(width*StretchFactorX), (int)(height*StrectchFactorY), bdc, 0, 0, width, height, SRCCOPY);

	bm->DeleteObject();
	bdc->DeleteDC();
	delete bdc;
	delete bm;
}

// 팔레트별 LUT생성
void CThermalCamDllSampleDlg::MakePalette(IRF_PALETTE_TYPE_T type, BOOL paletteInv)
{
	paletteInv;
	if(palette == NULL)
		palette = new unsigned char[256*3];	// Number of red, green, blue and temporary is 256, individually.

	if(GetImageLUT(palette, type, m_bPaletteInv ? true : false) != IRF_NO_ERROR)
	{
		MessageBox(_T("Palette File Open Error!!"), _T("Error Message"), MB_OK);
	}	
}


///////////////////////////////////////////////////////////////////////////////
// WatchComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//      Starts Socket Communication Working thread
// PARAMETERS:
//      None
///////////////////////////////////////////////////////////////////////////////
BOOL CThermalCamDllSampleDlg::WatchComm()
{
	if (NULL == m_hThread)			// Is Thread started?
	{
		if (m_hComm != INVALID_HANDLE_VALUE)		// Is Socket valid?
		{
			HANDLE hThread;
			UINT uiThreadId = 0;
			hThread = (HANDLE)_beginthreadex(NULL,  // Security attributes
				0,					// stack
				SocketThreadProc,   // Thread proc
				this,				// Thread param
				CREATE_SUSPENDED,   // creation mode
				&uiThreadId);		// Thread ID

			if ( INVALID_HANDLE_VALUE != hThread)
			{
				//SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
				ResumeThread( hThread );
				m_hThread = hThread;
				return TRUE;
			}
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// SocketThreadProc
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//     Socket Thread function.  This function is the main thread for socket
//     communication - Asynchronous mode.
// PARAMETERS:
//     LPVOID pParam : Thread parameter - a CSocketComm pointer
// NOTES:
///////////////////////////////////////////////////////////////////////////////
UINT WINAPI CThermalCamDllSampleDlg::SocketThreadProc(LPVOID pParam)
{
	CThermalCamDllSampleDlg* pThis = reinterpret_cast<CThermalCamDllSampleDlg*>( pParam );
	_ASSERTE( pThis != NULL );

	pThis->Run();

	return 1L;
} 


void CThermalCamDllSampleDlg::SetTemperatureTable(int temp_mode)
{
	IRF_TEMP_TABLE_T temp_table = receivedTempTable.sTemp_table.temp_table;
	IRF_TEMP_OFFSET_TABLE_T offset_table = receivedTempTable.sTemp_table.temp_offset_table;

	int index = 0;
	int engine_out = 0, old_out = 0;
	float temperature = 0, old_temp = 0;
	int temp_index = -20;
	unsigned int *engine_out_table;
	int *temperature_table;

	if(temp_mode == 0)
	{
		index = (temp_table.normal_max - temp_table.normal_min) + 1;
		engine_out_table = temp_table.normal_table;
		temperature_table = offset_table.normal_table;
	}
	else
	{
		index = (int)((temp_table.high_max - temp_table.high_min)/5.0) + 1;
		engine_out_table = temp_table.high_table;
		temperature_table = offset_table.high_table;
	}

	for(int i=0; i<index; i++)
	{
		temperature = (float)(temp_index + temperature_table[i]*0.1);
		engine_out = engine_out_table[i];

		if(i == 0)
		{
			for(int k=0; k<=engine_out; k++)
				tempLUT[k] = temperature;
		}
		else
		{
			int deltE = engine_out - old_out;				// 엔진출력 차이;
			float deltT = temperature - old_temp;			// 온도 차이;
			float deltV = (float)(deltT/(float)(deltE));	// 단위 엔진출력에 대한 온도값;

			int k = 1;
			for(int m=old_out+1; m<=engine_out; m++)
			{
				tempLUT[m] = old_temp + deltV*k++;
			}
		}
		
		if(i == index-1)	// last position
		{
			for(int k=engine_out+1; k<65536; k++)
				tempLUT[k] = temperature;
		}

		old_temp = temperature;
		old_out = engine_out;

		if(temp_mode == 0)
			temp_index++;
		else
			temp_index = temp_index + 5;
	}
}

void CThermalCamDllSampleDlg::DoRecvSysInfo( IRF_IR_CAM_DATA_T& cam_data ) 
{
	// set temperature offset
	short err = 0 ;
	cpy_cam_data = cam_data;
	if ((cpy_cam_data.fw_ver > 13031202) && (cpy_cam_data.save_data.sensor == 0)) {
		// request temperature offset for CX320 
		err = SendCameraMessage(m_hComm, &keepAliveID, _IRF_REQ_TEMP_TABLE, 0, 0);
		if ( err != IRF_NO_ERROR ) return ;
		
		Sleep(1000); // 절차상의 문제로 대기해야 함
		
		if ( NULL == tempLUT ) {
			tempLUT = new float[IRF_IMAGE_LEVEL];
		}

		err = GetCamTempTable( m_hComm, &keepAliveID, &receivedTempTable );
		if ( err != IRF_NO_ERROR ) return ;

		SetTemperatureTable( cpy_cam_data.save_data.temp_mode );
	}
	else {
		// default temperature offset 
		if ( NULL == tempLUT ) {
			tempLUT = new float[IRF_IMAGE_LEVEL];
		}
#if 0
		if ( GetTempMapTable(tempLUT, (IRF_DYNAMIC_RANGE_T)cpy_cam_data.save_data.temp_mode) != IRF_NO_ERROR ) {			
			MessageBox(_T("Failed to get temperature LUT!"), _T("Error Message"), MB_OK);
			tempLUT = NULL;
		}
#endif
	}

	bReady = TRUE;
}

void CThermalCamDllSampleDlg::DoRecvCamData( IRF_IR_CAM_DATA_T& cam_data ) 
{
	short err = 0;

	// request sys info
	if((err = SendMessageToCamera(m_hComm, &keepAliveID, _IRF_REQ_SYS_INFO, 0, 0, 0, 0, 0)) != IRF_NO_ERROR)
	{
		MessageBox(_T("Can't received Sys Info!"), _T("Message Notification"), MB_OK);
	}

	// copy save data
	cpy_cam_data.save_data = cam_data.save_data;

	::memcpy_s( cpy_cam_data.reserved, sizeof(DWORD) * 10, cam_data.reserved, sizeof(DWORD) * 10 );
		
	// set image size
	if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
		if( XGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 1024, 768 );
		}
		else if ( QVGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 384, 288 );
		}
		else if ( VGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 640, 480 );
		}
	}
	else if ( QVGA_ID == cpy_cam_data.save_data.sensor ) {
		SetImageInfo( 384, 288 );
	}
	else if ( VGA_ID == cpy_cam_data.save_data.sensor ) {
		SetImageInfo( 640, 480 );
	}
	else if( XGA_ID == cpy_cam_data.save_data.sensor ) {
		SetImageInfo( 1024, 768 );
	}
	else if ( QVGA_ID_OLD == cpy_cam_data.save_data.sensor ) {
		if ( 0 == cpy_cam_data.save_data.tv ) {
			SetImageInfo(320, 240);
		}
		else {
			SetImageInfo(384, 288);
		}
	}
	else {
		if ( 0 == cpy_cam_data.save_data.tv ) {
			SetImageInfo(640, 480);
		}
		else {
			SetImageInfo(640, 480);
		}
	}
	
	// get model type
	UpdateModelType( cpy_cam_data.save_data.ver, cpy_cam_data.save_data.sensor );

	// OnGetSaveDataHandler
	SendMessage(MY_MESSAGE3, 0, 0);
}


void CThermalCamDllSampleDlg::DoRecvStreamData( IRF_IR_CAM_DATA_T& cam_data ) 
{
	if(!bDisplayStatus)
	{
		// Copy from communication buffer image to display image buffer.
		if ( NULL != ir_image ) {
			bCommStatus = TRUE;		//Live Image와의 임계영역
			memcpy( ir_image, cam_data.ir_image, sizeof( unsigned short ) * imageInfo.xSize * imageInfo.ySize );
			bCommStatus = FALSE;
		}
				
		if( true == bRecording ) {
			SaveIRImages( 1667 );
		}
		SendNotifyMessage( MY_MESSAGE2, 0, 0 );	//Live image 호출
	}
}

///////////////////////////////////////////////////////////////////////////////
// Run
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//      This function runs the main thread loop
//      this implementation can be overloaded.
//      This function calls CSocketComm::OnDataReceived() (Virtual Function)
// PARAMETERS:
// NOTES:
//      You should not wait on the thread to end in this function or overloads
///////////////////////////////////////////////////////////////////////////////
void CThermalCamDllSampleDlg::Run()
{
	short	err = 0;


	if(ir_image == NULL)
		ir_image = new unsigned short[imageInfo.xSize*imageInfo.ySize];	// (320 * 240 * 2 bytes)/frame.

	memset(&cpy_cam_data, 0x00, sizeof(IRF_IR_CAM_DATA_T));
	memset(m_cam_data.lpNextData, 0, IRF_BUFFER_SIZE);
	m_cam_data.dwSize = 0L;
	m_cam_data.dwPosition = 0L;

	SetTimer(0, 2000, 0);

	InitLive();
	
	while( m_hComm != INVALID_HANDLE_VALUE )
	{
		err = GetIRImages(m_hComm, &keepAliveID, &m_cam_data);
		if(err != IRF_NO_ERROR) {
			SendMessage(MY_MESSAGE1, 0, err);	// Notified error.
			break;
		}

		switch ( m_cam_data.msg_type ) {
			case	_IRF_CAM_DATA		:	DoRecvCamData( m_cam_data );		break;
			case	_IRF_SYS_INFO		:	DoRecvSysInfo( m_cam_data );		break;
			case	_IRF_STREAM_DATA	:	DoRecvStreamData( m_cam_data );	break;
		}
	}
}

// Close Socket
void CThermalCamDllSampleDlg::CloseSocketThreadProc()
{	
	if (m_hComm!=INVALID_HANDLE_VALUE)
	{
		CloseConnect(&m_hComm, keepAliveID);
	}

	// Kill Thread
	if (NULL != m_hThread)
	{
		SleepEx(DEFAULT_TIMEOUT, TRUE);
		if (WaitForSingleObject(m_hThread, 1000L) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 1L);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}


void CThermalCamDllSampleDlg::UpdateModelType(int _ver, int _sensor)
{
	if ( SAVEDATA_VER_20 <= _ver ) {		//	CG MODEL
		if ( QVGA_ID == _sensor )			{ ir_model_type = CG320; }
		else if ( VGA_ID == _sensor )		{ ir_model_type = CG640; }
		else if ( XGA_ID == _sensor )		{ ir_model_type = CZ1024; }
	}
	else if ( SAVEDATA_VER_OLD > _ver ) {	//	CX320
		ir_model_type = CX320;
	}
	else {
		if ( QVGA_ID_OLD == _sensor )		{ ir_model_type = CX320; }
		else if ( VGA_ID_OLD == _sensor )	{ ir_model_type = CX640; }
	}
}


void CThermalCamDllSampleDlg::SetImageInfo(unsigned short image_x, unsigned short image_y)
{
	// update struct_ir_image buffer.
	if ((USHORT)cpy_cam_data.image_buffer_size != image_x * image_y)
	{
		delete[] m_cam_data.ir_image;
		delete[] m_image;


		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			m_cam_data.ir_image = new unsigned short[image_x * ( image_y + 1 ) * IRF_MAX_IR_BUFFER_SIZE];
			m_cam_data.image_buffer_size = image_x * ( image_y + 1 );
		}
		else {
			m_cam_data.ir_image = new unsigned short[image_x * image_y * IRF_MAX_IR_BUFFER_SIZE];
			m_cam_data.image_buffer_size = image_x * image_y;
		}

		m_image = new unsigned char[image_x * image_y];
	}


	// update ir_image buffer
	if (ir_image != NULL)
	{
		if (old_image_size != image_x * image_y)
		{
			old_image_size = image_x * image_y;
			delete[] ir_image;
			cpy_cam_data.ir_image = NULL;
			ir_image = NULL;
		}
	}

	imageInfo.xSize = image_x;
	imageInfo.ySize = image_y;


	if (ir_image == NULL)
	{
		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			ir_image = new unsigned short[imageInfo.xSize * ( imageInfo.ySize + 1 ) ];	// (320 * 240 * 2 bytes)/frame.
		}
		else {
			ir_image = new unsigned short[imageInfo.xSize * imageInfo.ySize];	// (320 * 240 * 2 bytes)/frame.
		}
	}

	//	bug fixed... 20150128
	cpy_cam_data.ir_image = ir_image;
}


void CThermalCamDllSampleDlg::OnBnClickedConnectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	// close playing thread
	if(file_handle != NULL)	
		CloseFileThreadProc();

	// 파일 스트림이 열려있으면 닫는다.;
	if( file_handle != NULL)
	{
		CloseIRCamDataCG( &cpy_cam_data );

		if ( CloseIRStream(file_handle) != IRF_NO_ERROR )
		{
			MessageBox(_T("File close error!"), _T("Error Message"), MB_OK);		// file close error.
		}
		else
			file_handle = NULL;
	}

	BYTE add1, add2, add3, add4;
	m_IPCtrl.GetAddress(add1, add2, add3, add4);	/* Get IP address from IP address control.*/
	IPAddress.Format(_T("%d.%d.%d.%d"), add1, add2, add3, add4);
	IPAddress.ReleaseBuffer();

	// Socket creation
	if(OpenConnect(&m_hComm, &keepAliveID, IPAddress, strPort, AF_INET, SOCK_STREAM) == IRF_HANDLE_ERROR)	// TCP
	{		//if Failed
 		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);

		MessageBox(_T("Failed to connect a camera!! \nRetry again."), _T("Error Message"), MB_OK);
		return;
	}
	else
	{		//if succeed
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_START_RECORD_IR_BUTTON)))->EnableWindow(true);

		WatchComm();	// Thread and data receive.
		bConnect = TRUE;
		bReady = FALSE;
		
		int err = 0;
		
		if((err = SendCameraMessage(m_hComm, &keepAliveID, _IRF_REQ_CAM_DATA, 0, 0)) != IRF_NO_ERROR)
		{
			MessageBox(_T("Can't received IR save data!"), _T("Message Notification"), MB_OK);
		}

		return;
	}
}


void CThermalCamDllSampleDlg::OnBnClickedDisconnectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CloseSocketThreadProc();

	((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(TRUE);
	((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
	((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);

	m_fImageLevel	= 0.0f;
	m_fImageSpan	= 0.0f;
}


void CThermalCamDllSampleDlg::OnBnClickedLiveBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if ( FALSE == bReady ) return;

	if(SendCameraMessage(m_hComm, &keepAliveID, _IRF_STREAM_ON, 0, 0) != IRF_NO_ERROR)
	{	//if Failed
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);

		MessageBox(_T("Can't received IR data!"), _T("Message Notification"), MB_OK);
	}
	else
	{	// if Succeed
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(TRUE);
		SetTimer(0,250,NULL);		// Updating Dialogue static text
	}
}


void CThermalCamDllSampleDlg::OnBnClickedFreezeBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(SendCameraMessage(m_hComm, &keepAliveID, _IRF_STREAM_OFF, 0, 0)  != IRF_NO_ERROR)
	{	//if Failed
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(TRUE);

		MessageBox(_T("Can't received IR data!"), _T("Message Notification"), MB_OK);
	}
	else
	{	// if Succeed
		((CButton*)(GetDlgItem(IDC_CONNECT_BTN)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_DISCONNECT_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_LIVE_BTN)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_FREEZE_BTN)))->EnableWindow(FALSE);

		KillTimer(0);
	}
}


void CThermalCamDllSampleDlg::OnBnClickedAlarmSetBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if ( INVALID_HANDLE_VALUE == m_hComm ) return;

	if ( CX320 == ir_model_type || CX640 == ir_model_type ) {
		// CX
		SendMessageToCamera( m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_ALARM1_FUNC, (uint16_t)m_Alarm_Func1, 0, 0, 0 );
		SendMessageToCamera( m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_ALARM1_COND, (uint16_t)m_AlarmCond1, 0, 0, 0 );
		SendMessageToCamera( m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_ALARM1_VAL, (uint16_t)m_AlarmVal1*10, 0, 0, 0 );
		
		MessageBox( _T("Completed Alarm setting."), _T("Information"), MB_OK );
	}
	else if ( CG320 == ir_model_type || CG640 == ir_model_type ) {
		// CG
		MessageBox( _T("Please check CG Sample"), _T("Information"), MB_OK );
	}
}


void CThermalCamDllSampleDlg::OnBnClickedNucBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if ( INVALID_HANDLE_VALUE == m_hComm ) return;

	if ( CX320 == ir_model_type || CX640 == ir_model_type ) {
		SendMessageToCamera( m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_NUC, 7, 0, 0, 0 );
	}
	else if ( CG320 == ir_model_type || CG640 == ir_model_type ) {
		SendMessageToCamera( m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_NUC_ONETIME, 0, 0, 0, 0 );
	}
}


char* CThermalCamDllSampleDlg::UTF8toANSI(char *pszCode)  
{  
	BSTR    bstrWide;  
	char*   pszAnsi;  
	int     nLength;      

	// Get nLength of the Wide Char buffer  
	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1,NULL, NULL);  
	bstrWide = SysAllocStringLen(NULL, nLength);  

	// Change UTF-8 to Unicode (UTF-16)  
	MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1, bstrWide,nLength);    

	// Get nLength of the multi byte buffer   
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);  
	pszAnsi = new char[nLength];    

	// Change from unicode to mult byte  
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);  

	SysFreeString(bstrWide);  

	return pszAnsi;  
}  


void CThermalCamDllSampleDlg::OnCbnSelchangePaletteCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//	UpdateData(TRUE);
	MakePalette((IRF_PALETTE_TYPE_T)m_Palette.GetCurSel(), m_bPaletteInv);
}



//palette Inverted
void CThermalCamDllSampleDlg::OnBnClickedInvertpalCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	MakePalette((IRF_PALETTE_TYPE_T)m_Palette.GetCurSel(), m_bPaletteInv);
}

// flip vertical
void CThermalCamDllSampleDlg::OnBnClickedMirrorCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}

// flip horizon
void CThermalCamDllSampleDlg::OnBnClickedFlipChk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CThermalCamDllSampleDlg::OnBnClickedAutoadjustCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if(m_bAutoAdjust == TRUE)
	{
		m_bAutoAdjust = FALSE;
		((CEdit*)GetDlgItem(IDC_LEVEL_EDIT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_SPAN_EDIT))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_SET_LEVEL_SPAN_BUTTON))->EnableWindow(TRUE);	
		OnBnClickedSetLevelSpanButton();
		autoRangeMethod.autoScale = _IRF_MANUAL;		// then GetImage(...) Applied
	}
	else if(m_bAutoAdjust == FALSE)
	{
		m_bAutoAdjust = TRUE;
		((CEdit*)GetDlgItem(IDC_LEVEL_EDIT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SPAN_EDIT))->EnableWindow(FALSE);	
		((CButton*)GetDlgItem(IDC_SET_LEVEL_SPAN_BUTTON))->EnableWindow(FALSE);			
		autoRangeMethod.autoScale = _IRF_AUTO;		// then GetImage(...) Applied
	}
	UpdateData(FALSE);
}

void CThermalCamDllSampleDlg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	//	CDialogEx::OnOK();
}

//카메라에 메세지를 전송하여 카메라의 RawData 취득모드를 변환한다.
void CThermalCamDllSampleDlg::OnCbnSelchangeTemprangeComboV2() 
{
	//TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	IRF_DYNAMIC_RANGE_T dynamic_range;

	dynamic_range = (IRF_DYNAMIC_RANGE_T)(m_TempCombo.GetCurSel()); 

	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		if(dynamic_range == _IRF_LOW_RANGE)
		{		
			SendMessageToCamera(m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_TEMP_MODE, 0, 0, 0, 0);	// normal mode.
		}	
		else if(dynamic_range == _IRF_MIDDLE_RANGE)
			SendMessageToCamera(m_hComm, &keepAliveID, _IRF_SET_CAM_DATA, CMD_TEMP_MODE, 1, 0, 0, 0);	// high mode.
	}

	
	
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		int err = 0;

		if((err = SendCameraMessage(m_hComm, &keepAliveID, _IRF_REQ_CAM_DATA, 0, 0)) != IRF_NO_ERROR)
		{
			MessageBox(_T("Can't received IR save data!"), _T("Message Notification"), MB_OK);
		}
	}

	// Get temperature map table.
	if ( tempLUT == NULL) {
		tempLUT = new float[IRF_IMAGE_LEVEL];
	}
#if 0
	if(GetTempMapTable(tempLUT, dynamic_range) != IRF_NO_ERROR)
	{			
		MessageBox(_T("Failed to get temperature LUT!"), _T("Error Message"), MB_OK);
		tempLUT = NULL;
	}
#endif
}


void CThermalCamDllSampleDlg::OnBnClickedSpotCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if (m_bSpotChk == TRUE)
	{
		//		m_bSpotChk = FALSE;
		((CEdit*)GetDlgItem(IDC_SPOT_X_EDIT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_SPOT_Y_EDIT))->EnableWindow(TRUE);

	} 
	else if (m_bSpotChk == FALSE)
	{
		//		m_bSpotChk = TRUE;
		((CEdit*)GetDlgItem(IDC_SPOT_X_EDIT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SPOT_Y_EDIT))->EnableWindow(FALSE);
	}
	UpdateData(FALSE);		// then LiveImage Applied
}


void CThermalCamDllSampleDlg::OnBnClickedRoiCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	UpdateData(TRUE);
	if (m_bRoiChk == TRUE)
	{
		//		m_bRoiChk = FALSE;
		((CEdit*)GetDlgItem(IDC_ROI_X_EDIT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_ROI_Y_EDIT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_ROI_W_EDIT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_ROI_H_EDIT))->EnableWindow(TRUE);

	} 
	else if (m_bRoiChk == FALSE)
	{
		//		m_bRoiChk = TRUE;
		((CEdit*)GetDlgItem(IDC_ROI_X_EDIT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_ROI_Y_EDIT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_ROI_W_EDIT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_ROI_H_EDIT))->EnableWindow(FALSE);
	}
	UpdateData(FALSE);		// then LiveImage Applied

}


void CThermalCamDllSampleDlg::OnBnClickedSpotApplyButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_spotPoint.x = m_DlgSpotX;
	m_spotPoint.y = m_DlgSpotY;		// then LiveImage Applied
}


void CThermalCamDllSampleDlg::OnBnClickedRoiApplyButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_DlgRoiX > imageInfo.xSize)
		m_DlgRoiX = imageInfo.xSize - 2;

	if (m_DlgRoiY > imageInfo.ySize)
		m_DlgRoiY = imageInfo.ySize - 2;

	if (m_DlgRoiW == 0)
		m_DlgRoiW = 1;
	if ((m_DlgRoiX + m_DlgRoiW) > imageInfo.xSize)
		m_DlgRoiW = imageInfo.xSize - m_DlgRoiX - 1;

	if (m_DlgRoiH == 0)
		m_DlgRoiH = 1;
	if ((m_DlgRoiY + m_DlgRoiH) > imageInfo.ySize)
		m_DlgRoiH = imageInfo.ySize - m_DlgRoiY - 1;



	m_RoiRect.left = m_DlgRoiX;
	m_RoiRect.top = m_DlgRoiY;
	m_RoiRect.right = m_DlgRoiX + m_DlgRoiW;
	m_RoiRect.bottom = m_DlgRoiY + m_DlgRoiH;		// then LiveImage Applied
	UpdateData(FALSE);

}


void CThermalCamDllSampleDlg::OnBnClickedFilterNoneRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterMedianRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterSoft1Radio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterSoft2Radio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterSharp1Radio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterSharp2Radio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterBiRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterFastgRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterFastsRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	bGaussianKernelSizeUpdatedFlag = TRUE;
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnBnClickedFilterBoxRadio()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);		//get Radio int var.
}


void CThermalCamDllSampleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	switch(nIDEvent){
	case 0:					//Display Text
		CString strBuf;
		strBuf.Format("%.2f ~ %.2f'C", (m_fImageLevel-m_fImageSpan*0.5f), (m_fImageLevel+m_fImageSpan*0.5f));	
			//Calculate Temp. Range
			//min = Level-Span*0.5f, max = Level + Span*0.5f
		SetDlgItemTextA(IDC_TEMPSCALE_STATIC, strBuf);

		if(autoRangeMethod.autoScale == _IRF_AUTO){		//Display Level&Span
			strBuf.Format("%.2f",m_fImageLevel);
			SetDlgItemTextA(IDC_LEVEL_EDIT, strBuf);
			strBuf.Format("%.2f",m_fImageSpan);
			SetDlgItemTextA(IDC_SPAN_EDIT,strBuf);
		}

		if(m_bSpotChk == TRUE){							//Display Spot info.
			float f = GetPointTemp((HANDLE)&cpy_cam_data, imageInfo, tempLUT, corrPara, m_spotPoint);
			strBuf.Format("%.2f",f);
			SetDlgItemTextA(IDC_SPOT_VAL_EDIT,strBuf);
		}

		if (m_bRoiChk == TRUE)							//Display ROI info.
		{
			IRF_NUMERIC_INFO_T info;
			memset(&info,0,sizeof(IRF_NUMERIC_INFO_T));

			if(!bCommStatus){
				m_fDlgSpotValue = GetROITemp((HANDLE)&cpy_cam_data, imageInfo, tempLUT, corrPara, m_RoiRect, &info, &m_RoiLowPoint, &m_RoiHighPoint);

				strBuf.Format("%d",m_RoiHighPoint.x);
				SetDlgItemTextA(IDC_ROI_HIGH_X_EDIT,strBuf);
				strBuf.Format("%d",m_RoiHighPoint.y);
				SetDlgItemTextA(IDC_ROI_HIGH_Y_EDIT,strBuf);
				strBuf.Format("%d",m_RoiLowPoint.x);
				SetDlgItemTextA(IDC_ROI_LOW_X_EDIT,strBuf);
				strBuf.Format("%d",m_RoiLowPoint.y);
				SetDlgItemTextA(IDC_ROI_LOW_Y_EDIT,strBuf);

				strBuf.Format("%.2f",info.max);
				SetDlgItemTextA(IDC_ROI_HIGH_VAL_EDIT,strBuf);
				strBuf.Format("%.2f",info.min);
				SetDlgItemTextA(IDC_ROI_LOW_VAL_EDIT,strBuf);
				strBuf.Format("%.2f",info.avg);
				SetDlgItemTextA(IDC_ROI_AVG_EDIT,strBuf);
				strBuf.Format("%.2f",info.std);
				SetDlgItemTextA(IDC_ROI_STD_EDIT,strBuf);
			}
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CThermalCamDllSampleDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	
	//필터값 디스플레이 업데이트
	//OnNMReleasedcapture..()에서는 실제 함수에 적용
	// 마우스로 스크롤 하는 도중에는 디스플레이 값만 업데이트
	// 마우스 버튼업 될때만 필터값 적용


	CString strBuf;
	if (*pScrollBar == m_fastGSlider)		//Fast Gaussian Filter Value Update
	{
		strBuf.Format("%d",m_fastGSlider.GetPos());
		SetDlgItemTextA(IDC_FAST_G_STATIC,strBuf);
			
	} 
	else if (*pScrollBar == m_fastSSlider)		//Fast Stack Filter Value Update
	{
		strBuf.Format("%d",m_fastSSlider.GetPos());
		SetDlgItemTextA(IDC_FAST_S_STATIC,strBuf);
	} 
	else if (*pScrollBar == m_boxBlurSlider)		//Box Blur Filter Value Update
	{
		strBuf.Format("%d",m_boxBlurSlider.GetPos());
		SetDlgItemTextA(IDC_BOXBLUR_STATIC,strBuf);
	} 
	else if (*pScrollBar == m_BiDomainSlider)		//Bi-Lateral Filter Domain Value Update
	{
		strBuf.Format("%.1f",(float)m_BiDomainSlider.GetPos() / 10.0); //for float input
		SetDlgItemTextA(IDC_BI_D_STATIC,strBuf);
	} 
	else if (*pScrollBar == m_BiRangeSlider)		//Bi-Lateral Filter Range Value Update
	{
		strBuf.Format("%.1f",(float)m_BiRangeSlider.GetPos()/10.0); //for float input
		SetDlgItemTextA(IDC_BI_R_STATIC,strBuf);
	}	
	
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CThermalCamDllSampleDlg::OnNMReleasedcaptureFastGSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	pNMHDR;
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(true);
	if(m_nFilterSelect == 6){
		bGaussianKernelSizeUpdatedFlag = TRUE;
		int sz = m_fastGSlider.GetPos();
		
		gaussianSize = (sz * 2)+1;
	
	}
	*pResult = 0;
}


void CThermalCamDllSampleDlg::OnNMReleasedcaptureFastSSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	pNMHDR;
	if(m_nFilterSelect == 7){
	gaussianSize = m_fastSSlider.GetPos();
	}
	*pResult = 0;
}


void CThermalCamDllSampleDlg::OnNMReleasedcaptureBoxblurSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	pNMHDR;
	if(m_nFilterSelect == 8){
	gaussianSize = m_boxBlurSlider.GetPos();
	}
	*pResult = 0;
}


void CThermalCamDllSampleDlg::OnNMReleasedcaptureBiDSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	pNMHDR;
	if(m_nFilterSelect == 9){
		bilateral_sigD = (float)(m_BiDomainSlider.GetPos() / 10);
	}
	*pResult = 0;
}


void CThermalCamDllSampleDlg::OnNMReleasedcaptureBiRSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	pNMHDR;
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(m_nFilterSelect == 9){
		bilateral_sigR = (float)(m_BiRangeSlider.GetPos() / 10);
	}
	*pResult = 0;
}


void CThermalCamDllSampleDlg::OnBnClickedSetLevelSpanButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(true);
	curLevel = (short)fDlgLevelValue;
	curSpan = (short)fDlgSpanValue;
}


void CThermalCamDllSampleDlg::OnBnClickedFileOpenButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if(GetSocketStatus()){
		CloseSocketThreadProc();
		OnBnClickedDisconnectBtn();
	}


	// szFilters is a text string that includes two file name filters:
	// "*.my" for "MyType Files" and "*.*' for "All Files."
	TCHAR szFilters[]= _T("IR Files (*.crd)|*.crd|Jpeg Files (*.jpg)|*.jpg|All Files (*.*)|*.*||");

	// Create an Open dialog; the default file name extension is ".crd".
	CFileDialog fileDlg(TRUE, _T("crd"), _T("*.crd"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);
	
	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.

	if(fileDlg.DoModal() == IDOK)
	{
		CString fileName = fileDlg.GetPathName();
		CString ext = fileDlg.GetFileExt();

		if(ext.Compare(_T("crd")) != 0)
		{
			MessageBox(_T("Failed to load IR file!!"), _T("Error Message"), MB_OK);
			return;
		}

		char buffer[256] = {0};
		wsprintf(buffer, "%s", fileName);

		// 파일 스트림이 열려있으면 닫는다;
		if( file_handle != NULL)
		{
			// 파일 스트림이 닫히기 전에 파일 쓰레드를 중지한다.;
			CloseFileThreadProc();
			CloseIRCamDataCG( &cpy_cam_data );

			if ( CloseIRStream(file_handle) != IRF_NO_ERROR )
			{
				MessageBox(_T("File close error!"), _T("Error Message"), MB_OK);		// file close error.
			}
			else
				file_handle = NULL;
		}
					
		if(LoadIRImage(&file_handle, buffer, (long*)&totSizeIRStream) != IRF_NO_ERROR)
			MessageBox(_T("File load error!"), _T("Error Message"), MB_OK);

		GetIRHeaders(file_handle, pFileHeader, pFileAddHeader, (long*)&curPosition);
		imageInfo = pFileHeader->image_info;
		
		memset(&cpy_cam_data, 0x00, sizeof(IRF_IR_CAM_DATA_T));
		cpy_cam_data.save_data = pFileAddHeader->save_data;
		
		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			if ( IRF_NO_ERROR != CreateIRCamDataCG( &cpy_cam_data ) ) {
				// Error
			}
		}

		// set image size
		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			if ( QVGA_ID == cpy_cam_data.save_data.sensor ) {
				SetImageInfo( 384, 288 );
			}
			else if ( VGA_ID == cpy_cam_data.save_data.sensor ) {
				SetImageInfo( 640, 480 );
			}
			else if( XGA_ID == cpy_cam_data.save_data.sensor ) {
				SetImageInfo( 1024, 768 );
			}
		}
		else if ( QVGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 384, 288 );
		}
		else if ( VGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 640, 480 );
		}
		else if( XGA_ID == cpy_cam_data.save_data.sensor ) {
			SetImageInfo( 1024, 768 );
		}
		else if ( QVGA_ID_OLD == cpy_cam_data.save_data.sensor ) {
			if ( 0 == cpy_cam_data.save_data.tv ) {
				SetImageInfo(320, 240);
			}
			else {
				SetImageInfo(384, 288);
			}
		}
		else {
			if ( 0 == cpy_cam_data.save_data.tv ) {
				SetImageInfo(640, 480);
			}
			else {
				SetImageInfo(640, 480);
			}
		}

		// get model type
		UpdateModelType( cpy_cam_data.save_data.ver, cpy_cam_data.save_data.sensor );
		

		// set temperature offset
		IRF_DYNAMIC_RANGE_T dynamic_range;

		dynamic_range = (IRF_DYNAMIC_RANGE_T)(cpy_cam_data.save_data.temp_mode); 

		if ( tempLUT == NULL) {
			tempLUT = new float[IRF_IMAGE_LEVEL];
		}
#if 0
		if(GetTempMapTable(tempLUT, dynamic_range) != IRF_NO_ERROR)
		{			
			MessageBox(_T("Failed to get temperature LUT!"), _T("Error Message"), MB_OK);
			tempLUT = NULL;
		}
#endif

		OperateIRPlayer(PLAY);
		
		((CButton*)(GetDlgItem(IDC_START_RECORD_IR_BUTTON)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_STOP_RECORD_IR_BUTTON)))->EnableWindow(FALSE);
	}
}


BOOL CThermalCamDllSampleDlg::GetSocketStatus(void)
{
	if(m_hComm == INVALID_HANDLE_VALUE)	return FALSE;
	else								return TRUE;
}


void CThermalCamDllSampleDlg::CloseFileThreadProc(void)
{
	// Kill Thread
	if (NULL != m_hPlayThread)
	{
		DWORD dwExitCode = 0;

		m_runPlayThread = FALSE;

		SleepEx(DEFAULT_TIMEOUT, TRUE);
		
		if (WaitForSingleObject(m_hPlayThread, 200L) == WAIT_TIMEOUT)			
			::GetExitCodeThread (m_hPlayThread, &dwExitCode);

		if(dwExitCode == STILL_ACTIVE)
		{
			TerminateThread(m_hPlayThread, dwExitCode);
			CloseHandle(m_hPlayThread);
		}

		m_hPlayThread = NULL;
		playStatus = NONE;
		bDisplayStatus = false;	// display status : off
		bSuspendThread = false;
	}
}


void CThermalCamDllSampleDlg::OperateIRPlayer(IRF_PLAY_TYPE_T action)
{
	Sleep(10);

	switch(action)
	{
	case PLAY:
		playStatus = PLAY;
		FileThreadPause(FALSE);
		break;
	case PAUSE:
		playStatus = PAUSE;

		FileThreadPause(TRUE);
		break;
	case STOP:
		playStatus = STOP;

		FileThreadPause(TRUE);
		break;
	}
}


void CThermalCamDllSampleDlg::FileThreadPause(bool bPause)
{
	int supendCount = 0;
	
	if(bPause)
	{
		if(m_hPlayThread != NULL)
		{
			bSuspendThread = true;
		}
	}
	else
	{
		if(m_hPlayThread == NULL)
			StartPlayThread();
		else
		{
			//int supendCount;
			do{
				supendCount = ResumeThread(m_hPlayThread);
			}while(supendCount > 0);
		}
	}
}


bool CThermalCamDllSampleDlg::StartPlayThread(void)
{
	
	if (NULL == m_hPlayThread)			// Is Thread started?
	{
		HANDLE hThread;
		UINT uiThreadId = 0;
		hThread = (HANDLE)_beginthreadex(NULL,  // Security attributes
			0,					// stack
			PlayThreadProc,   // Thread proc
			this,				// Thread param
			CREATE_SUSPENDED,   // creation mode
			&uiThreadId);		// Thread ID

		if ( INVALID_HANDLE_VALUE != hThread)
		{
			//SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread( hThread );
			m_hPlayThread = hThread;
			return true;
		}
	}
	return false;
}


// This process is to create thread for play an IR file.
UINT WINAPI CThermalCamDllSampleDlg::PlayThreadProc(LPVOID pParam)
{
	CThermalCamDllSampleDlg* pThis = reinterpret_cast<CThermalCamDllSampleDlg*>( pParam );
	_ASSERTE( pThis != NULL );

	pThis->PlayThreadRun();

	return 1L;
} 

// This process is to create thread for play an IR file.
// Actually, it is statement to display image.
void CThermalCamDllSampleDlg::PlayThreadRun()
{
	short err = 0;
	// Initialize
	if(InitLive() != IRF_NO_ERROR)
		return;
	
	SetTimer(0, 2000, 0);

	int frame_time = 0;	// a time between frames.
	
	UpperCnt = 2;
	LowerCnt = 2;

	m_runPlayThread = TRUE;

	while( TRUE == m_runPlayThread ) {
		err = SetPlayType(playStatus, &frame_time);
		bDisplayStart = true;
		if ( err == IRF_END_OF_FILE ) // end of frame.
		{
			FileThreadPause(TRUE);
		}
		else if ( err == IRF_BEGIN_OF_FILE ) 	// begin of frame.
		{
			FileThreadPause(TRUE);
		}
		else if ( err == IRF_NO_ERROR );
		else
			return;

		if ( !bDisplayStatus ) {
			SendNotifyMessage(MY_MESSAGE2, 0, 0);
		}

		sleep((unsigned int)(frame_time * period * 0.01));
		
		if(bSuspendThread)
		{
			SuspendThread(m_hPlayThread);
			bSuspendThread = false;
		}
	}

	return;
}
short CThermalCamDllSampleDlg::SetPlayType(IRF_PLAY_TYPE_T playType, int* frame_time)
{
	short		err;


	if ( NULL == ir_image ) {
		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			ir_image = new unsigned short[ imageInfo.xSize * ( imageInfo.ySize + 1 ) ];	// (320 * 240 * 2 bytes)/frame.
		}
		else {
			ir_image = new unsigned short[ imageInfo.xSize * imageInfo.ySize ];	// (320 * 240 * 2 bytes)/frame.
		}
	}

	switch( playType )
	{
	case PLAY:
	case FAST_FORWARD:
	case FORWARD_STEP:
		if ( SAVEDATA_VER_20 < cpy_cam_data.save_data.ver ) {
			err = GetIRImageFromStream_v2( file_handle, ir_image, imageInfo.xSize * imageInfo.ySize, totSizeIRStream, &curPosition, frame_time, &savedTime, m_bPlayLoop, FILE_VERSION );
			if ( IRF_NO_ERROR != err ) {
				return -1;
			}
			err = LoadIRCamDataCG( &cpy_cam_data, ir_image, imageInfo.xSize * imageInfo.ySize );
			if ( IRF_NO_ERROR != err ) {
				return -1;
			}
			return IRF_NO_ERROR;
		}
		else {
			err = GetIRImageFromStream_v2( file_handle, ir_image, imageInfo.xSize * imageInfo.ySize, totSizeIRStream, &curPosition, frame_time, &savedTime, m_bPlayLoop, pFileHeader->ver );
			if ( IRF_NO_ERROR != err ) {
				return -1;
			}
			return IRF_NO_ERROR;
		}

		break;
	}

	return 0;
}

void CThermalCamDllSampleDlg::sleep(unsigned int mseconds)
{
	clock_t goal = mseconds + clock();
	while (goal > clock());
}


void CThermalCamDllSampleDlg::OnBnClickedRecordIrButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_thread_handle = NULL;

	bSaveOver = false;
	bRecordLedNone = false;

	
	// szFilters is a text string that includes two file name filters:
	// "*.my" for "MyType Files" and "*.*' for "All Files."
	CString fileName= "";
	CTime   CurTime = CTime::GetCurrentTime();     // 현재 시스템 시각을 구한다.

	int year = CurTime.GetYear();
	int month = CurTime.GetMonth();
	int day = CurTime.GetDay();
	int hour = CurTime.GetHour();
	int minute = CurTime.GetMinute();
	int second = CurTime.GetSecond();

	bool bAutoPath = true;

	if(bAutoPath)
	{
		// 실행파일 경로;
		char cs_programpath[_MAX_PATH];
		TCHAR programpath[MAX_PATH] = {0};
//		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];

		GetModuleFileName( NULL, programpath, _MAX_PATH);

		sprintf_s(cs_programpath, "%s", programpath);
		_splitpath_s(cs_programpath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, NULL, NULL, NULL);

		CString	SavePath = "C:\\Users\\Public\\Pictures";
		SavePath.Format("%s%s", drive, dir);

		CString CurrentTimeText;

		CurrentTimeText.Format("\\IRS_%04d%02d%02d%02d%02d%02d."
			, year
			, month
			, day
			, hour
			, minute
			, second
			);


		fileName = SavePath + CurrentTimeText + "crd";		
	}
	else
	{
		TCHAR szFilters[]= _T("IR Files (*.crd)|*.crd|");

		// Create an Open dialog; the default file name extension is ".my".
		CFileDialog fileDlg(FALSE, _T("crd"), _T("*.crd"),
			OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilters);

		// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
		// returns IDOK.
		if(fileDlg.DoModal() == IDOK)
		{		
			fileName = fileDlg.GetPathName();
		}
	}	

	if(!fileName.IsEmpty())
	{
		char buffer[256] = {0};
		wsprintf(buffer, "%s", fileName);


		// 파일 스트림이 열려있으면 닫는다;
		if( record_handle != NULL)
		{
			CloseIRCamDataCG( &cpy_cam_data );

			if ( CloseIRStream(record_handle) != IRF_NO_ERROR )
			{				 
				//MessageBox(NULL, _T("File close error!"), _T("Error Message"), MB_OK);		// file close error.
				AfxMessageBox(_T("File open error!"));
			}
			else
				record_handle = NULL;
		}
		

		if ( ir_model_type == CX320 ) {
			pFileSaveHeader->ver = FILE_VERSION_11;
		} else {
			pFileSaveHeader->ver = FILE_VERSION;
		}
		pFileSaveHeader->image_info = imageInfo;
		pFileSaveHeader->temp_correction = corrPara;

		if(GetDynamicTempRange() == _IRF_LOW_RANGE)
			pFileAddSaveHeader->dynamic_range = 0;
		else
			pFileAddSaveHeader->dynamic_range = 1;

		pFileAddSaveHeader->year = year;
		pFileAddSaveHeader->month = month;
		pFileAddSaveHeader->day = day;
		pFileAddSaveHeader->hour = hour;
		pFileAddSaveHeader->minute = minute;
		pFileAddSaveHeader->second = second;

		pFileAddSaveHeader->save_data = cpy_cam_data.save_data;

		// Open for read (will fail if file does not exist)
		if(SaveIRHeader(&record_handle, buffer, pFileSaveHeader, pFileAddSaveHeader) != IRF_NO_ERROR)
		{
			//MessageBox(NULL, _T("File open error!"), _T("Error Message"), MB_OK);	// file open error.
			AfxMessageBox(_T("File open error!"));
			record_handle = NULL;
			bRecording = FALSE;
		}
		else
		{
			bRecording = TRUE;
			frameCount = 0;
		}	


	}
	else
	{
		bRecording = FALSE;		
	}

	if(bRecording){
		((CButton*)(GetDlgItem(IDC_START_RECORD_IR_BUTTON)))->EnableWindow(FALSE);
		((CButton*)(GetDlgItem(IDC_STOP_RECORD_IR_BUTTON)))->EnableWindow(TRUE);
	}
	else{
		((CButton*)(GetDlgItem(IDC_START_RECORD_IR_BUTTON)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_STOP_RECORD_IR_BUTTON)))->EnableWindow(FALSE);
	}
	fileName.Empty();
}


void CThermalCamDllSampleDlg::OnBnClickedStopRecordIrButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	bRecording = FALSE;
		
	if( record_handle != NULL)
	{
		CloseIRCamDataCG( &cpy_cam_data );

		if ( CloseIRStream(record_handle) != IRF_NO_ERROR )
		{
	//		MessageBox(NULL, _T("File close error!"), _T("Error Message"), MB_OK);		// file close error.
				AfxMessageBox(_T("File close error!"));
		}
		else
			record_handle = NULL;
	}
	
		((CButton*)(GetDlgItem(IDC_START_RECORD_IR_BUTTON)))->EnableWindow(TRUE);
		((CButton*)(GetDlgItem(IDC_STOP_RECORD_IR_BUTTON)))->EnableWindow(FALSE);

}


void CThermalCamDllSampleDlg::SaveIRStream(void)
{

}


IRF_DYNAMIC_RANGE_T CThermalCamDllSampleDlg::GetDynamicTempRange(void)
{
	IRF_DYNAMIC_RANGE_T dynamic_range;

	dynamic_range = (IRF_DYNAMIC_RANGE_T)(m_TempCombo.GetCurSel()); 

	return dynamic_range;
}


void CThermalCamDllSampleDlg::SaveIRImages(int intervalTime)
{
	short					err;


	if(!bRecording) return;

	// file header
	pFileSaveHeader->image_info = imageInfo;
	pFileSaveHeader->temp_correction = corrPara;

	
	err = IRF_NO_ERROR;
	if ( CX320 ==ir_model_type || CX640 == ir_model_type ) {
		err = SetIRImageToStream_v2(record_handle, ir_image, imageInfo.xSize*imageInfo.ySize, intervalTime, &frameCount, pFileSaveHeader->ver);
	}else if ( CG320 == ir_model_type || CG640 == ir_model_type ){
		err = SetIRImageToStream_v2(record_handle, ir_image, imageInfo.xSize*imageInfo.ySize, intervalTime, &frameCount, FILE_VERSION );
	}

	switch( err )
	{
	case IRF_BUFFER_ALLOCATION_ERROR:
		OnBnClickedStopRecordIrButton();
//		MessageBox(NULL, _T("IR imabe buffer error!"), _T("Error Message"), MB_OK);
				AfxMessageBox(_T("IR imabe buffer error!"));
		
		break;
	case IRF_IR_IMAGE_WRITE_ERROR:
		OnBnClickedStopRecordIrButton();
//		MessageBox(NULL, _T("IR image write error!"), _T("Error Message"), MB_OK);
				AfxMessageBox(_T("IR image write error!"));
		
		break;
	case IRF_FILE_OPEN_ERROR:
		OnBnClickedStopRecordIrButton();
//		MessageBox(NULL, _T("NO stream!"), _T("Error Message"), MB_OK);
				AfxMessageBox(_T("NO stream!"));
		
		break;
	case IRF_FILE_WRITE_COUNT_OVER:	
		bSaveOver = true;
		OnBnClickedStopRecordIrButton();		
		//MessageBox(NULL, _T("Maximum save time is 161 seconds.\nFile is closed automatically."), _T("Notification Message"), MB_OK);
		break;
	}
}

void CThermalCamDllSampleDlg::OnBnClickedBtnSaveOffset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	if ( NULL == tempLUT ) return ;

	CString fileName= "";
	CTime   CurTime = CTime::GetCurrentTime();     // 현재 시스템 시각을 구한다.

	int year = CurTime.GetYear();
	int month = CurTime.GetMonth();
	int day = CurTime.GetDay();
	int hour = CurTime.GetHour();
	int minute = CurTime.GetMinute();
	int second = CurTime.GetSecond();

	// 실행파일 경로;
	char cs_programpath[_MAX_PATH];
	TCHAR programpath[MAX_PATH] = {0};
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	GetModuleFileName( NULL, programpath, _MAX_PATH);

	sprintf_s(cs_programpath, "%s", programpath);
	_splitpath_s(cs_programpath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, NULL, NULL, NULL);

	CString	SavePath = "C:\\Users\\Public\\Pictures";
	SavePath.Format("%s%s", drive, dir);

	CString CurrentTimeText;

	CurrentTimeText.Format("\\Offset_%04d%02d%02d%02d%02d%02d."
		, year
		, month
		, day
		, hour
		, minute
		, second
		);


	fileName = SavePath + CurrentTimeText + "txt";		

	if ( !fileName.IsEmpty() ) {
		char buffer[256] = {0};
		wsprintf(buffer, "%s", fileName);
	
		ofstream out ( buffer );
		
		for ( int idx = 0 ; idx < 65536 ; idx ++ ) {
			out << tempLUT[ idx ] << "\n";
		}
		out.close();
	}
}


void CThermalCamDllSampleDlg::OnBnClickedBtnLoadOffset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR szFilters[]= _T("txt Files (*.txt)|*.txt|All Files (*.*)|*.*||");

	// Create an Open dialog; the default file name extension is ".txt".
	CFileDialog fileDlg(TRUE, _T("txt"), _T("*.txt"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if ( IDOK == fileDlg.DoModal() ) {
		CString fileName = fileDlg.GetPathName();
				
		char buffer[256] = {0};
		wsprintf( buffer, "%s", fileName );

		ifstream in ( buffer );
		
		for ( int idx = 0 ; idx < 65536 ; idx ++ ) {
			in >> tempLUT[ idx ];
		}
		in.close();
	}
}
