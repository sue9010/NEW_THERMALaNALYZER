
// SampleCGDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SampleCG.h"
#include "SampleCGDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static void		callback_mousemove( POINT _pt )
{
	CSampleCGDlg *		p_main_dlg = (CSampleCGDlg*)(::AfxGetApp()->m_pMainWnd);


	if ( NULL == p_main_dlg )		{ return; }
	if ( 0 > _pt.x || 0 > _pt.y )	{ return; }

	p_main_dlg->EventMouseMove( _pt );
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CSampleCGDlg dialog



CSampleCGDlg::CSampleCGDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSampleCGDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSampleCGDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST_CAMERA, m_ctlListCtrl);
	DDX_Control(pDX, IDC_IPADDR_CAM, m_ctlIPAddress);
	DDX_Control(pDX, IDC_STC_SCREEN, m_ctlScreen);
	DDX_Control(pDX, IDC_CHK_AUTO_RECONNECT, m_autoReconnect);
}

BEGIN_MESSAGE_MAP(CSampleCGDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSampleCGDlg message handlers

BOOL CSampleCGDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	DoInitialize();
	RefreshCameraList();	//	find thermal camera...


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSampleCGDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSampleCGDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSampleCGDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CSampleCGDlg::PreTranslateMessage( MSG* pMsg )
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if ( WM_KEYDOWN == pMsg->message ) {
		switch ( pMsg->wParam ) {
		case	VK_RETURN	:
		case	VK_ESCAPE	:
			{
				::TranslateMessage( pMsg );
				::DispatchMessage( pMsg );
			}
			return ( TRUE );
		}
	}
	else if ( WM_LBUTTONDOWN == pMsg->message ) {
		RECT			rect;


		::GetWindowRect( ::GetDlgItem(this->GetSafeHwnd(), IDC_BTN_NEAR), &rect );
		if ( TRUE == ::PtInRect( &rect, pMsg->pt ) ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 0, 1 );
			m_fFocusNear = TRUE;
		}

		::GetWindowRect( ::GetDlgItem(this->GetSafeHwnd(), IDC_BTN_FAR), &rect );
		if ( TRUE == ::PtInRect( &rect, pMsg->pt ) ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 0, 2 );
			m_fFocusFar = TRUE;
		}

		::GetWindowRect( ::GetDlgItem(this->GetSafeHwnd(), IDC_BTN_TELE), &rect );
		if ( TRUE == ::PtInRect( &rect, pMsg->pt ) ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 1, 1 );
			m_fZoomTele = TRUE;
		}

		::GetWindowRect( ::GetDlgItem(this->GetSafeHwnd(), IDC_BTN_WIDE), &rect );
		if ( TRUE == ::PtInRect( &rect, pMsg->pt ) ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 1, 2 );
			m_fZoomWide = TRUE;
		}
	}
	else if ( WM_LBUTTONUP == pMsg->message ) {
		if ( TRUE == m_fFocusNear ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 0, 0 );
			m_fFocusNear = FALSE;
		}

		if ( TRUE == m_fFocusFar ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 0, 0 );
			m_fFocusFar = FALSE;
		}

		if ( TRUE == m_fZoomTele ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 1, 0 );
			m_fZoomTele = FALSE;
		}

		if ( TRUE == m_fZoomWide ) {
			SendCameraConfig( _IRF_SET_CAM_DATA, CMD_MOTORIZED, 1, 0 );
			m_fZoomWide = FALSE;
		}
	}


	return CDialogEx::PreTranslateMessage( pMsg );
}


LRESULT CSampleCGDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message ) {
	case	WM_COMMAND			:
		{
			if ( 0 == lParam && (0 == wParam || 1 == wParam) ) {
				//	MENU or Accelerator Command
			}
			else {
				//	Control Command
				LRESULT			result;

				WORD			ctrl_id;
				WORD			notify_code;

				HWND			h_wnd;


				ctrl_id		= LOWORD( wParam );
				notify_code	= HIWORD( wParam );

				h_wnd = (HWND)lParam;

				if ( TRUE == DoControlCommand( h_wnd, ctrl_id, notify_code, &result ) ) {
					return ( result );
				}
			}
		}
		break;

	case	WM_NOTIFY					:
		{
			LRESULT			result;

			NMHDR *			p_nmhdr = (NMHDR*)lParam;


			if ( TRUE == DoControlNotify( p_nmhdr, &result ) ) {
				return ( result );
			}
		}
		break;
#ifdef _USED_BONJURESDK_
	case	WM_BONJURE_SERVICE_EVENT	:
		{
			SOCKET				sock = (SOCKET)wParam;
			DNSServiceErrorType err;


			if ( WSAGETSELECTERROR( lParam ) && !HIWORD( lParam ) ) {
				TRACE( _T("bonjure socket error.\r\n") );
				return ( 0 );
			}

			if ( sock != (SOCKET)DNSServiceRefSockFD( m_pServiceRef ) ) {
				TRACE( _T("bonjure invalid socket.\r\n") );
			}

			err = DNSServiceProcessResult( m_pServiceRef );
			if ( kDNSServiceErr_NoError != err ) {
				TRACE( _T("ERROR : DNSServiceProcessResult (%d)\r\n"), err );
			}
		}
		return ( 0 );
#endif
	case	WM_CLOSE					:
		{
			TRACE( _T("WM_CLOSE\r\n") );
		}
		break;

	case	WM_DESTROY					:
		{
			TRACE( _T("WM_DESTROY\r\n") );

			if ( NULL != m_pCamSetupDlg ) {
				m_pCamSetupDlg->DestroyWindow();

				delete m_pCamSetupDlg;
				m_pCamSetupDlg = NULL;
			}

			if ( INVALID_HANDLE_VALUE != m_stTCamInfo.h_sdk ) {
				DisconnectCamera( &m_stTCamInfo );
			}
		}
		break;
	}

	return CDialogEx::WindowProc( message, wParam, lParam );
}


void	CSampleCGDlg::DoInitialize( void )
{
	CSampleCGApp *		pApp;

	TCHAR				tc_text[256];
	

	pApp = (CSampleCGApp *)(AfxGetApp());

	::_stprintf_s( tc_text, _T("SampleCG, (Version : %s)"), pApp->m_szFileVersion.GetString() );

	::SetWindowText( this->GetSafeHwnd(), tc_text );


	//	member variable initialize
#ifdef _USED_BONJURESDK_
	m_pServiceRef		= NULL;
	m_pResolveRef		= NULL;
#endif

	m_pCamSetupDlg = NULL;
	m_pCamSetupDlg = new CCameraSetupDlg;

	if ( NULL != m_pCamSetupDlg ) {
		m_pCamSetupDlg->Create( IDD_DLG_CAMSETUP );
		m_pCamSetupDlg->SetParentDialog( this );
		m_pCamSetupDlg->ShowWindow( SW_HIDE );
	}

	::memset( &m_stRoiCfg[0], 0x00, sizeof(m_stRoiCfg) );
	::memset( &m_stRoiCam[0], 0x00, sizeof(m_stRoiCam) );
	::memset( &m_stRoiSDK[0], 0x00, sizeof(m_stRoiSDK) );

	m_dwStrmBaseTick	= 0;
	m_dwRecvStrmCnt		= 0;
	
	m_stTCamInfo.reset_member();

	//	list control config
	m_ctlListCtrl.SetExtendedStyle( m_ctlListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	m_ctlListCtrl.InsertColumn( 0, _T("MODEL"), LVCFMT_LEFT, 60 );
	m_ctlListCtrl.InsertColumn( 1, _T("NAME"), LVCFMT_LEFT, 50 );
	m_ctlListCtrl.InsertColumn( 2, _T("IP"), LVCFMT_LEFT, 100 );
	m_ctlListCtrl.InsertColumn( 3, _T("MAC"), LVCFMT_LEFT, 120 );

	m_ctlIPAddress.SetAddress( 0, 0, 0, 0 );

	{
		const	TCHAR *			TEMP_MODE[2] = {
									_T(" Normal Mode"),
									_T(" High Mode")
								};

		HWND			h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_TEMPMODE );


		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)TEMP_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)TEMP_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{
		const	TCHAR *			COLOR_PALETTE[13] = {
									_T("Yellow"),		_T("Rainbow"),		_T("Rain900"),
									_T("Rain10"),		_T("Midgrey"),		_T("Midgreen"),
									_T("Medical"),		_T("Iron10"),		_T("Iron"),
									_T("Greyred"),		_T("Grey10"),		_T("Grey"),
									_T("Glowbow")
								};

		HWND			h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_PALETTE );

		int				lp;


		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		for ( lp = 0; lp < 13; lp++ ) {
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)COLOR_PALETTE[lp] );
		}
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 11, 0L );
	}

	{
		const	TCHAR *			STREAM_FPS[30] = {
									_T("FULL"),		_T("1/2"),		_T("1/3"),
									_T("1/4"),		_T("1/5"),		_T("1/6"),
									_T("1/7"),		_T("1/8"),		_T("1/9"),
									_T("1/10"),		_T("1/11"),		_T("1/12"),
									_T("1/13"),		_T("1/14"),		_T("1/15"),
									_T("1/16"),		_T("1/17"),		_T("1/18"),
									_T("1/19"),		_T("1/20"),		_T("1/21"),
									_T("1/22"),		_T("1/23"),		_T("1/24"),
									_T("1/25"),		_T("1/26"),		_T("1/27"),
									_T("1/28"),		_T("1/29"),		_T("1/30")
								};

		HWND			h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_NETFPS );

		int				lp;


		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		for ( lp = 0; lp < 30; lp++ ) {
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STREAM_FPS[lp] );
		}
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{
		const	TCHAR *			ROI_MODE[2] = {
									_T("Camera"),
									_T("SDK")
								};

		TCHAR			txt[64];

		int				lp;

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROINUM );
		for ( lp = 0; lp < 10; lp++ ) {
			::_stprintf_s( txt, _T("ROI%02d"), lp + 1 );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)txt );
		}
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROIMODE );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ROI_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ROI_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{
		RECT			rect;


		m_ctlScreen.GetClientRect( &rect );
		m_ctlScreen.SetActiveScreenRect( &rect );
		m_ctlScreen.SetWindowPos( NULL, 4, 4, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW );

		m_ctlScreen.SetImageFlip( FALSE );
		m_ctlScreen.SetImageMirror( FALSE );
	}

	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_LEVEL ), EM_LIMITTEXT, 8, 0L );
	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_SPAN ), EM_LIMITTEXT, 8, 0L );

	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIX ), EM_LIMITTEXT, 3, 0L );
	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIY ), EM_LIMITTEXT, 3, 0L );
	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIW ), EM_LIMITTEXT, 3, 0L );
	::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIH ), EM_LIMITTEXT, 3, 0L );

	m_fFocusNear	= FALSE;
	m_fFocusFar		= FALSE;
	m_fZoomTele		= FALSE;
	m_fZoomWide		= FALSE;

	DoInitControl( FALSE );
}


void	CSampleCGDlg::DoUninitialize( void )
{
}


void	CSampleCGDlg::DoInitControl( BOOL _f_connect )
{
	HWND			h_ctrl_wnd;


	if ( TRUE == _f_connect ) {
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_CONNECT );
		::SetWindowText( h_ctrl_wnd, _T("Disconnect") );

		::memset( &m_stRoiCfg[0], 0x00, sizeof(m_stRoiCfg) );
		::memset( &m_stRoiCam[0], 0x00, sizeof(m_stRoiCam) );
		::memset( &m_stRoiSDK[0], 0x00, sizeof(m_stRoiSDK) );

		//		Button Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_ONCENUC );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_STREAM );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_INVERT );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_MIRROR );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_FLIP );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_AGCMODE );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("Auto") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_SETLEVEL );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_SETSPAN );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_HOTON );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_COLDON );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_MOUSEON );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_ROISETUP );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_NEAR );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_FAR );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_WIDE );
		::EnableWindow( h_ctrl_wnd, TRUE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_TELE );
		::EnableWindow( h_ctrl_wnd, TRUE );

		//		ComboBox Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_PALETTE );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 11, 0L );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_TEMPMODE );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROINUM );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROIMODE );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_NETFPS );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		//		Edit & Static Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_LEVEL );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0.0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_SPAN );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0.0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_LEVEL );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0.0 'C") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_SPAN );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0.0 'C") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_SCALE );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0.0 'C ~ 0.0 'C") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MAXDATA );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MAXX );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MAXY );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MINDATA );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MINX );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_MINY );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_CTRDATA );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_AVGDATA );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIX );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIY );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIW );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIH );
		::EnableWindow( h_ctrl_wnd, TRUE );
		::SetWindowText( h_ctrl_wnd, _T("0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMAX );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMAXX );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMAXY );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMIN );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMINX );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIMINY );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_ROIAVG );
		::SetWindowText( h_ctrl_wnd, _T("") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_NETFPS );
		::SetWindowText( h_ctrl_wnd, _T("0") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_CAMCTRL );
		::EnableWindow( h_ctrl_wnd, TRUE );
	}
	else {
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_CONNECT );
		::SetWindowText( h_ctrl_wnd, _T("Connect") );

		//		Button Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_ONCENUC );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_STREAM );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );
		
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_INVERT );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_MIRROR );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_FLIP );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_AGCMODE );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("Auto") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_SETLEVEL );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_SETSPAN );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_HOTON );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_COLDON );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CHK_MOUSEON );
		::EnableWindow( h_ctrl_wnd, FALSE );
		::SendMessage( h_ctrl_wnd, BM_SETCHECK, (WPARAM)(int)BST_UNCHECKED, 0L );
		::SetWindowText( h_ctrl_wnd, _T("On") );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_ROISETUP );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_NEAR );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_FAR );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_WIDE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_TELE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		//		ComboBox Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_PALETTE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_TEMPMODE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROINUM );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROIMODE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_NETFPS );
		::EnableWindow( h_ctrl_wnd, FALSE );

		//		Edit & Static Control
		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_LEVEL );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_SPAN );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_LEVEL );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_SPAN );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_STC_SCALE );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIX );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIY );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIW );
		::EnableWindow( h_ctrl_wnd, FALSE );

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDT_ROIH );
		::EnableWindow( h_ctrl_wnd, FALSE );

		m_ctlScreen.Uninitialize();

		h_ctrl_wnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_BTN_CAMCTRL );
		::EnableWindow( h_ctrl_wnd, FALSE );

		if ( NULL != m_pCamSetupDlg && TRUE == m_pCamSetupDlg->IsWindowVisible() ) {
			m_pCamSetupDlg->ShowWindow( SW_HIDE );
		}
	}
}


BOOL	CSampleCGDlg::DoControlCommand( HWND _h_wnd, WORD _ctrl_id, WORD _notify_code, LRESULT* _p_result )
{
	switch ( _ctrl_id ) {
	case	IDC_BTN_CONNECT		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {	//	Connect
					const uint16_t		PORT_NUM = 15001;

					uint8_t				addr[4];


					//	get ip address and make string
					m_ctlIPAddress.GetAddress( addr[0], addr[1], addr[2], addr[3] );

					//	set connection information
					m_stTCamInfo.ip_addr[0] = addr[0];
					m_stTCamInfo.ip_addr[1] = addr[1];
					m_stTCamInfo.ip_addr[2] = addr[2];
					m_stTCamInfo.ip_addr[3] = addr[3];

					m_stTCamInfo.port = PORT_NUM;

					//	set owner window
					m_stTCamInfo.p_owner = this;

					//	process connect camera
					if ( FALSE == ConnectCamera( &m_stTCamInfo ) ) {
						m_stTCamInfo.reset_member();

						return ( TRUE );
					}

					m_fFocusNear	= FALSE;
					m_fFocusFar		= FALSE;
					m_fZoomTele		= FALSE;
					m_fZoomWide		= FALSE;

					DoInitControl( TRUE );
				}
				else {								//	Disconnect
					if ( FALSE == DisconnectCamera( &m_stTCamInfo ) ) {
					}

					m_stTCamInfo.reset_member();

					DoInitControl( FALSE );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_ONCENUC		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					return ( TRUE );
				}

				SendCameraConfig(
								&m_stTCamInfo,
								_IRF_SET_CAM_DATA,
								CMD_NUC_ONETIME
								);

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_SETLEVEL	:
		{
			if ( BN_CLICKED == _notify_code ) {
				TCHAR			value[16] = { NULL, };

				float			level;


				if ( FALSE == m_stTCamInfo.is_connect() ) {
					return ( TRUE );
				}

				::GetDlgItemText( this->GetSafeHwnd(), IDC_EDT_LEVEL, &value[0], 16 );
				level = (float)::_tstof( value );

				m_stTCamInfo.set_agc_level( level );


				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_SETSPAN		:
		{
			if ( BN_CLICKED == _notify_code ) {
				TCHAR			value[16] = { NULL, };

				float			span;


				if ( FALSE == m_stTCamInfo.is_connect() ) {
					return ( TRUE );
				}

				::GetDlgItemText( this->GetSafeHwnd(), IDC_EDT_SPAN, &value[0], 16 );
				span = (float)::_tstof( value );

				m_stTCamInfo.set_agc_span( span );

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_ROISETUP	:
		{
			if ( BN_CLICKED == _notify_code ) {
				int					num;
				uint16_t			x, y, w, h;


				if ( FALSE == m_stTCamInfo.is_connect() ) {
					return ( TRUE );
				}

				num = ::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_ROINUM ), CB_GETCURSEL, 0L, 0L );

				x = (uint16_t)::GetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIX, NULL, FALSE );
				y = (uint16_t)::GetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIY, NULL, FALSE );
				w = (uint16_t)::GetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIW, NULL, FALSE );
				h = (uint16_t)::GetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIH, NULL, FALSE );

				m_stRoiCfg[num].x = x;
				m_stRoiCfg[num].y = y;
				m_stRoiCfg[num].w = w;
				m_stRoiCfg[num].h = h;

				{	//	send camera roi config
					uint8_t			mode;
					uint16_t		rcode;
					uint32_t		rcode2;
					uint32_t		rcode3;


					if ( 0 == x && 0 == y && 0 == w && 0 == h ) {
						m_stRoiCfg[num].f_enable = FALSE;
						mode = 0;
					}
					else if ( 16 >= w || 16 >= h ) {
						m_stRoiCfg[num].f_enable = FALSE;
						mode = 0;

						m_stRoiCfg[num].x = 0;
						m_stRoiCfg[num].y = 0;
						m_stRoiCfg[num].w = 0;
						m_stRoiCfg[num].h = 0;
					}
					else {
						m_stRoiCfg[num].f_enable = TRUE;
						mode = 2;
					}

					num += 11;						//	add base roi number ( ROI01 = 11 ~ ROI10 = 20 )
					rcode	= (num << 8) | mode;	//	0 : off, 1 : spot, 2 : rectangle
					rcode2	= (x << 16) | y;
					rcode3	= (w << 16) | h;

					SendCameraConfig(
									&m_stTCamInfo,
									_IRF_SET_CAM_DATA,
									CMD_ROI_COORCFG,
									rcode,
									rcode2,
									rcode3
									);
				}
				
				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_CAMCTRL	:
		{
			if ( NULL == m_pCamSetupDlg ) {
			}
			else {
				if ( TRUE == m_pCamSetupDlg->IsWindowVisible() ) {
					m_pCamSetupDlg->ShowWindow( SW_HIDE );
				}
				else {
					m_pCamSetupDlg->ShowWindow( SW_SHOW );
				}
			}
		}
		break;

	case	IDC_BTN_CLOSE	:
		{
			CDialogEx::OnOK();
		}
		break;

	case	IDC_CHK_STREAM		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					if ( TRUE == SendCameraCommand( &m_stTCamInfo, _IRF_STREAM_ON ) ) {
						m_stTCamInfo.f_recv_stream = TRUE;

						m_dwStrmBaseTick	= ::GetTickCount();
						m_dwRecvStrmCnt		= 0;

						::SetWindowText( _h_wnd, _T("Off") );
					}
					else {
						Button_SetCheck( _h_wnd, BST_UNCHECKED );
					}
				}
				else {
					if ( TRUE == SendCameraCommand( &m_stTCamInfo, _IRF_STREAM_OFF ) ) {
						m_stTCamInfo.f_recv_stream = FALSE;

						::SetWindowText( _h_wnd, _T("On") );
					}
					else {
						Button_SetCheck( _h_wnd, BST_CHECKED );
					}
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_MIRROR		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					m_stTCamInfo.set_mirror( TRUE );
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					m_stTCamInfo.set_mirror( FALSE );
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_FLIP		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					m_stTCamInfo.set_flip( TRUE );
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					m_stTCamInfo.set_flip( FALSE );
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_INVERT		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					m_stTCamInfo.set_invert( TRUE );
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					m_stTCamInfo.set_invert( FALSE );
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_AGCMODE		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("Auto") );

					m_stTCamInfo.set_agc_mode( TRUE );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					m_stTCamInfo.set_agc_mode( FALSE );
					::SetWindowText( _h_wnd, _T("Manual") );
				}
				else {
					m_stTCamInfo.set_agc_mode( TRUE );
					::SetWindowText( _h_wnd, _T("Auto") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_HOTON		:
		{
			if ( BN_CLICKED == _notify_code ) {
				BOOL		f_hot, f_cold;


				m_ctlScreen.GetDisplayHotCold( f_hot, f_cold );
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					f_hot = FALSE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					f_hot = TRUE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					f_hot = FALSE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_COLDON		:
		{
			if ( BN_CLICKED == _notify_code ) {
				BOOL		f_hot, f_cold;


				m_ctlScreen.GetDisplayHotCold( f_hot, f_cold );
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					f_cold = FALSE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					f_cold = TRUE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					f_cold = FALSE;
					m_ctlScreen.SetDisplayHotCold( f_hot, f_cold );
					m_ctlScreen.SetDisplayHotColdTemp( f_hot, f_cold );
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_MOUSEON		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( FALSE == m_stTCamInfo.is_connect() ) {
					Button_SetCheck( _h_wnd, BST_UNCHECKED );
					::SetWindowText( _h_wnd, _T("On") );

					m_ctlScreen.SetDisplayMouseTemp( FALSE, 0.0f );

					return ( TRUE );
				}

				if ( BST_CHECKED == Button_GetCheck( _h_wnd ) ) {
					::SetWindowText( _h_wnd, _T("Off") );
				}
				else {
					::SetWindowText( _h_wnd, _T("On") );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_PALETTE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT			res;


				res = ::SendMessage( _h_wnd, CB_GETCURSEL, 0L, 0L );
				if ( CB_ERR == res ) {
				}
				else {
					m_stTCamInfo.set_sel_palette( res );
				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_TEMPMODE	:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT			res;


				if ( FALSE == m_stTCamInfo.is_connect() ) {
					::SendMessage( _h_wnd, CB_GETCURSEL, m_stTCamInfo.get_temp_mode(), 0L );

					return ( TRUE );
				}

				res = ::SendMessage( _h_wnd, CB_GETCURSEL, 0L, 0L );
				if ( CB_ERR == res ) {
				}
				else {
					int				msg_res;


					if ( SAVEDATA_VERSION > m_stTCamInfo.ir_data.save_data.ver ) {
						msg_res = ::MessageBox(
											this->GetSafeHwnd(),
											_T("Camera will be rebooted after setting.\nWould you like to proceed?"),
											_T("Information"),
											MB_YESNO | MB_ICONINFORMATION
											);

						if ( IDYES != msg_res ) {
							::SendMessage( _h_wnd, CB_SETCURSEL, (WPARAM)m_stTCamInfo.get_temp_mode(), (LPARAM)0 );
							return ( 0 );
						}
					}

					m_stTCamInfo.set_temp_mode( res );

					SendCameraConfig(
									&m_stTCamInfo,
									_IRF_SET_CAM_DATA,
									CMD_TEMP_MODE,
									(uint16_t)res
									);
					
					SendCameraCommand( &m_stTCamInfo, _IRF_REQ_CAM_DATA );

				}

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NETFPS		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT			res;


				res = ::SendMessage( _h_wnd, CB_GETCURSEL, 0L, 0L );
				if ( CB_ERR == res ) {
				}
				else {
					SendCameraConfig(
									&m_stTCamInfo,
									_IRF_SET_CAM_DATA,
									CMD_NETWORK_FPS,
									(uint16_t)res
									);
				}
			}
		}
		break;

	case	IDC_CMB_ROINUM		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT			res;


				res = ::SendMessage( _h_wnd, CB_GETCURSEL, 0L, 0L );
				if ( CB_ERR == res ) {
				}
				else {
					::SetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIX, m_stRoiCfg[res].x, FALSE );
					::SetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIY, m_stRoiCfg[res].y, FALSE );
					::SetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIW, m_stRoiCfg[res].w, FALSE );
					::SetDlgItemInt( this->GetSafeHwnd(), IDC_EDT_ROIH, m_stRoiCfg[res].h, FALSE );
				}
			}
		}
		break;

#ifdef _USED_BONJURESDK_
	case	IDC_BTN_REFRESH		:
		{
			if ( BN_CLICKED == _notify_code ) {
				RefreshCameraList();

				return ( TRUE );
			}
		}
		break;
#endif
	}

	return ( FALSE );
}


BOOL	CSampleCGDlg::DoControlNotify( NMHDR* _p_nmhdr, LRESULT* _p_result )
{
	*_p_result = 0;

	switch ( _p_nmhdr->idFrom ) {
	case	IDC_LST_CAMERA		:
		{
			if ( NM_DBLCLK == _p_nmhdr->code ) {
				LPNMITEMACTIVATE		p_item = (LPNMITEMACTIVATE)_p_nmhdr;

				TCHAR				ip_addr[32] = { NULL, };
				IN_ADDR				ip;
				

				m_ctlListCtrl.GetItemText( p_item->iItem, 2, ip_addr, 32 );

				::InetPton( AF_INET, ip_addr, &ip );
		
				m_ctlIPAddress.SetAddress( ip.s_net, ip.s_host, ip.s_lh, ip.s_impno );


				return ( TRUE );
			}
		}
		break;
	}

	return ( FALSE );
}


#ifdef _USED_BONJURESDK_

void	CSampleCGDlg::AddResolveCamera( char * _model, char * _ip, char * _mac )
{
	TCHAR			model[32]	= { NULL, };
	TCHAR			ip[32]		= { NULL, };
	TCHAR			mac[32]		= { NULL, };

	TCHAR			temp[64];

	int				lp;


#ifdef	_UNICODE
	::MultiByteToWideChar( CP_ACP, 0, _model, ::strlen(_model), model, 32 );
	::MultiByteToWideChar( CP_ACP, 0, _ip, ::strlen(_ip), ip, 32 );
	::MultiByteToWideChar( CP_ACP, 0, _mac, ::strlen(_mac), mac, 32 );
#else
	::strcpy( model, _model );
	::strcpy( ip, _ip );
	::strcpy( mac, _mac );
#endif

	for ( lp = 0; lp < m_ctlListCtrl.GetItemCount(); lp++ ) {
		m_ctlListCtrl.GetItemText( lp, 2, temp, 64 );

		if ( 0 != ::lstrcmp( ip, temp ) ) { continue; }

		return;
	}

	lp = m_ctlListCtrl.GetItemCount();

	if ( 0 == ::strncmp( "CG3", _model, 3 ) ) {
		::lstrcpy( temp, _T("CG3xx") );
	}
	else if ( 0 == ::strncmp( "CG4", _model, 3 ) ) {
		::lstrcpy( temp, _T("CG6xx") );
	}
	else {
		return;
	}

	m_ctlListCtrl.InsertItem( lp, temp );
	m_ctlListCtrl.SetItemText( lp, 1, model );
	m_ctlListCtrl.SetItemText( lp, 2, ip );
	m_ctlListCtrl.SetItemText( lp, 3, mac );
}


void	CSampleCGDlg::RefreshCameraList( void )
{
	//	Clear List Control
	m_ctlListCtrl.DeleteAllItems();

	CloseServiceDNS();
	CreateServiceDNS();
}


void	CSampleCGDlg::CreateServiceDNS( void )
{
	DNSServiceErrorType			err;

	int				sock_fd;
	int				res;


	if ( NULL != m_pServiceRef ) { return; }

	err = DNSServiceCreateConnection( &m_pServiceRef );
	if ( kDNSServiceErr_NoError != err ) {
		TRACE( _T("ERROR : DNSServiceCreateConnection (%d)\r\n"), err );
	}

	m_pResolveRef = m_pServiceRef;
	err = DNSServiceBrowse(
						&m_pResolveRef,
						kDNSServiceFlagsShareConnection,
						0,
						"_cox-cam._tcp",
						NULL,
						CSampleCGDlg::BrowseReply,
						this
						);
	if ( kDNSServiceErr_NoError != err ) {
		TRACE( _T("ERROR : DNSServiceBrowse (%d)\r\n"), err );
	}

	sock_fd = DNSServiceRefSockFD( m_pServiceRef );
	if ( -1 == sock_fd ) {
		TRACE( _T("ERROR : DNSServiceRefSockFD\r\n") );
	}

	
	res = ::WSAAsyncSelect( sock_fd, this->GetSafeHwnd(), WM_BONJURE_SERVICE_EVENT, (FD_READ | FD_CLOSE) );
	if ( 0 != res ) {
		CloseServiceDNS();
	}
}


void	CSampleCGDlg::CloseServiceDNS( void )
{
	if ( NULL == m_pServiceRef ) { return; }

	DNSServiceRefDeallocate( m_pServiceRef );
	m_pServiceRef = NULL;
}


void	CSampleCGDlg::BrowseResolve( uint32_t _if_idx, const char * _reply_name, const char * _reply_type, const char * _reply_domain )
{
	DNSServiceErrorType			err;
	

	m_pResolveRef = m_pServiceRef;
	err = DNSServiceResolve(
						&m_pResolveRef,
						kDNSServiceFlagsShareConnection,
						_if_idx,
						_reply_name,
						_reply_type,
						_reply_domain,
						CSampleCGDlg::ResolveReply,
						this
						);
	if ( kDNSServiceErr_NoError != err ) {
		TRACE( _T("ERROR : DNSServiceResolve (%d)\r\n"), err );
	}
}


static	int	copy_labels( char *_p_dst, const char * _p_lim, const char ** _pp_srcp, int _labels )
{
	const char *			src = *_pp_srcp;


	while ( '.' != *src || --_labels > 0 ) {
		// Make sure "\." doesn't confuse us
		if ( '\\' == *src )					{ *_p_dst++ = *src++; }
		if ( !*src || _p_dst >= _p_lim )	{ return -1; }
		*_p_dst++ = *src++;

		if ( !*src || _p_dst >= _p_lim )	{ return -1; }
	}

	*_p_dst++	= 0;
	*_pp_srcp	= src + 1;	// skip over final dot

	return 0;
}


void	DNSSD_API	CSampleCGDlg::ResolveReply(
DNSServiceRef				_p_sd_ref, 
DNSServiceFlags				_flags, 
uint32_t					_if_idx,
DNSServiceErrorType			_err_code,
const char *				_full_name, 
const char *				_host_target,
uint16_t					_port,
uint16_t					_txt_len,
const unsigned char *		_txt_record, 
void *						_context
)
{
	CSampleCGDlg *			p_this = reinterpret_cast<CSampleCGDlg*>(_context);

	const unsigned char *	max_txt	= _txt_record + _txt_len;
	const char *			p_src	= _full_name;

	char					name[kDNSServiceMaxDomainName];
	char					type[kDNSServiceMaxDomainName];

	char *					p_copy;
	char					info[64];

	char					model[20]	= { NULL, };
	char					ip[20]		= { NULL, };
	char					mac[20]		= { NULL, };

	uint16_t				port_num;
	

	_ASSERT( NULL != p_this );

	if ( kDNSServiceErr_NoError != _err_code ) {
		TRACE( _T("ERROR : ResolveReply (%d)\r\n"), _err_code );
		p_this->CloseServiceDNS();

		return;
	}

	port_num = ((_port >> 8) & 0x00FF) | ((_port << 8) & 0xFF00);

	if ( 0 != copy_labels( name, name + kDNSServiceMaxDomainName, &p_src, 3 ) ) { return; }		// Fetch name+type

	p_src = _full_name;
	if ( 0 != copy_labels( type, type + kDNSServiceMaxDomainName, &p_src, 1 ) ) { return; }		// Skip first label
	if ( 0 != copy_labels( type, type + kDNSServiceMaxDomainName, &p_src, 2 ) ) { return; }		// Fetch next two labels (service type)

	while ( _txt_record < max_txt ) {
		const unsigned char *const		end = _txt_record + 1 + _txt_record[0];


		::memset( &info[0], 0x00, sizeof(info) );
		p_copy = &info[0];

		_txt_record++;		// Skip over length byte
		while ( _txt_record < end ) {
			if ( *_txt_record == '\\' || *_txt_record == '\"' ) {
				_txt_record++;
				continue;
			}
			
			*p_copy++ = *_txt_record++;
		}

		if ( 0 == ::strncmp( info, "model=", 6 ) ) {
			::strcpy_s( model, &info[6] );
		}
		else if ( 0 == ::strncmp( info, "ip=", 3 ) ) {
			::strcpy_s( ip, &info[3] );
		}
		else if ( 0 == ::strncmp( info, "mac=", 4 ) ) {
			::strcpy_s( mac, &info[4] );
		}
	}

	p_this->AddResolveCamera( model, ip, mac );

	DNSServiceRefDeallocate( _p_sd_ref );
}


void	DNSSD_API	CSampleCGDlg::BrowseReply(
DNSServiceRef				_p_sd_ref, 
const DNSServiceFlags		_flags, 
uint32_t					_if_idx,
DNSServiceErrorType			_err_code,
const char *				_reply_name, 
const char *				_reply_type, 
const char *				_reply_domain, 
void *						_context
)
{
	CSampleCGDlg *				p_this = reinterpret_cast<CSampleCGDlg*>(_context);


	_ASSERT( NULL != p_this );

	if ( 0 == (_flags & kDNSServiceFlagsAdd) ) {
		TRACE( _T("BrowseReply %08X\r\n"), _flags );
		return;
	}

	p_this->BrowseResolve( _if_idx, _reply_name, _reply_type, _reply_domain );
}
#endif


BOOL	CSampleCGDlg::IsConnectCamera( void )
{
	if ( INVALID_HANDLE_VALUE == m_stTCamInfo.h_sdk )  { return ( FALSE ); }
	return ( TRUE );
}


BOOL	CSampleCGDlg::ConnectCamera( TCAMINFO_PTR _p_info )
{
	int16_t				res;


	_ASSERT( NULL != _p_info );

	//	make parameter
	::sprintf_s(
				m_ip_addr,
				"%d.%d.%d.%d",
				_p_info->ip_addr[0], _p_info->ip_addr[1],
				_p_info->ip_addr[2], _p_info->ip_addr[3]
				);
	::sprintf_s( m_port_num, "%d", _p_info->port );


	//	Thermal Camera Connection
	res = OpenConnect(
					&_p_info->h_sdk,				//	handle
					&_p_info->keep_alive_id,		//	timer id
					m_ip_addr,						//	destination ip address
					m_port_num,						//	destination port number
					AF_INET,						//	IPv4
					SOCK_STREAM						//	TCP Stream
					);
	if ( IRF_NO_ERROR != res ) {
		_p_info->keep_alive_id	= 0;

		::MessageBox(
					this->GetSafeHwnd(),
					_T("Failed to connect a camera!! \nRetry again."),
					_T("CG SDK Sample"),
					MB_OK | MB_ICONERROR
					);


		return ( FALSE );
	}

	//	create receive thread
	if ( FALSE == CreateRecvThread( _p_info ) ) {
		::MessageBox(
					this->GetSafeHwnd(),
					_T("Failed to create receive thread!! \nRetry again."),
					_T("CG SDK Sample"),
					MB_OK | MB_ICONERROR
					);

		CloseConnect(
					&_p_info->h_sdk,			//	handle
					_p_info->keep_alive_id		//	timer id
					);
		_p_info->keep_alive_id	= 0;


		return ( FALSE );
	}


	return ( TRUE );
}


BOOL	CSampleCGDlg::DisconnectCamera( TCAMINFO_PTR _p_info )
{
	_ASSERT( NULL != _p_info );

	_p_info->f_run_thread = FALSE;
	if ( INVALID_HANDLE_VALUE != _p_info->h_sdk ) {
		CloseConnect(
					&_p_info->h_sdk,			//	handle
					_p_info->keep_alive_id		//	timer id
					);

		_p_info->keep_alive_id	= 0;
	}

	DestroyRecvThread( _p_info );


	return ( TRUE );
}


BOOL	CSampleCGDlg::CreateRecvThread( TCAMINFO_PTR _p_info )
{
	unsigned int		thread_id;

	
	_ASSERT( NULL != _p_info );

	_p_info->h_thread = (HANDLE)::_beginthreadex(
											NULL,
											0,
											CSampleCGDlg::RECEIVE_PROC,
											_p_info,
											CREATE_SUSPENDED,
											&thread_id
											);

	if ( INVALID_HANDLE_VALUE == _p_info->h_thread ) {
		_p_info->f_run_thread	= FALSE;
		_p_info->h_thread		= NULL;

		return ( FALSE );
	}

	_p_info->f_run_thread = TRUE;

	::ResumeThread( _p_info->h_thread );

	return ( TRUE );
}


BOOL	CSampleCGDlg::DestroyRecvThread( TCAMINFO_PTR _p_info )
{
	BOOL		f_res;
	DWORD		wait_cnt, exit_code;
	

	_ASSERT( NULL != _p_info );

	if ( NULL == _p_info->h_thread ) { return ( TRUE ); }

	wait_cnt = 0;
	f_res = ::GetExitCodeThread( _p_info->h_thread, &exit_code );
	while ( FALSE != f_res ) {
		if ( STILL_ACTIVE == exit_code ) {
			wait_cnt++;
			if ( 50 <= wait_cnt ) {
				::TerminateThread( _p_info->h_thread, 0xFFFFFFFF );
				break;
			}

			// remove message
			while ( TRUE ) {
				MSG		msg; 
				int		wm_msg;

				if ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
					::GetMessage( &msg, NULL, 0, 0 );

					wm_msg = LOWORD( msg.message );
				
					if ( WM_PAINT == wm_msg ) { 
						::TranslateMessage( &msg );
						::DispatchMessage( &msg );
					}
				}
				else {
					break;
				}
			}
			::Sleep( 100 );

			f_res = ::GetExitCodeThread( _p_info->h_thread, &exit_code );

			continue;
		}

		break;
	}

	if ( FALSE == f_res ) {
		::TerminateThread( _p_info->h_thread, 0xFFFFFFFF );
	}

	::CloseHandle( _p_info->h_thread );
	_p_info->h_thread = NULL;


	return ( TRUE );
}


BOOL	CSampleCGDlg::WaitRecvData( TCAMINFO_PTR _p_info )
{
	int16_t				res;

	res = GetIRImages( _p_info->h_sdk, &_p_info->keep_alive_id, &_p_info->ir_data );
	if ( IRF_NO_ERROR != res ) {
		return ( FALSE );
	}

	switch ( _p_info->ir_data.msg_type ) {
	case	_IRF_ACK			:	DoRecvACK( _p_info );			break;
	case	_IRF_NAK			:	DoRecvNAK( _p_info );			break;
	case	_IRF_CAM_DATA		:	DoRecvCamData( _p_info );		break;
	case	_IRF_STREAM_DATA	:	DoRecvStreamData( _p_info );	break;
	default						:
		{
		}
		break;
	}

	return ( TRUE );
}


void	CSampleCGDlg::DoRecvACK( TCAMINFO_PTR _p_info )
{
	TRACE( _T("ACK : %d\r\n"), _p_info->ir_data.PMSGTYPE );
}


void	CSampleCGDlg::DoRecvNAK( TCAMINFO_PTR _p_info )
{
	TRACE( _T("NAK : %d %d\r\n"), _p_info->ir_data.PMSGTYPE, _p_info->ir_data.RCODE );
}


void	CSampleCGDlg::DoRecvCamData( TCAMINFO_PTR _p_info )
{
	IRF_SAVEDATA_T *			p_cfg_data;

	int32_t					lp;
	int16_t					res;


	p_cfg_data = &_p_info->ir_data.save_data;
	if ( SAVEDATA_VER_20 > p_cfg_data->ver ) {
		TRACE( _T("Only CG model can be connected.\r\n") );
		::MessageBox(
					((CSampleCGDlg*)(_p_info->p_owner))->GetSafeHwnd(),
					_T("Only CG model can be connected."),
					_T("CG SDK Sample"),
					MB_OK | MB_ICONERROR
					);

		return;
	}

	if ( QVGA_ID == p_cfg_data->sensor ) {
		_p_info->ir_size.xSize = 384;
		_p_info->ir_size.ySize = 288;
	}
	else if ( VGA_ID == p_cfg_data->sensor ) {
		_p_info->ir_size.xSize = 640;
		_p_info->ir_size.ySize = 480;
	}
	else {
		TRACE( _T("invalid sensor type.\r\n") );
		::MessageBox(
					((CSampleCGDlg*)(_p_info->p_owner))->GetSafeHwnd(),
					_T("Invalid Sensor Resolution."),
					_T("CG SDK Sample"),
					MB_OK | MB_ICONERROR
					);

		return;
	}

	if ( 0 == p_cfg_data->temp_mode ) {
		::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_TEMPMODE ), CB_SETCURSEL, 0, 0L );
	}
	else {
		::SendMessage( ::GetDlgItem( this->GetSafeHwnd(), IDC_CMB_TEMPMODE ), CB_SETCURSEL, 1, 0L );
	}

	m_ctlScreen.RegisterMouseMove( callback_mousemove );
	m_ctlScreen.SetDisplayHotCold( FALSE, FALSE );
	m_ctlScreen.Initialize( _p_info->ir_size.xSize, _p_info->ir_size.ySize, 32 );

	//	load color palette
	for ( lp = 0; lp < MAX_PALETTE; lp++ ) {
		res = GetImageLUT( _p_info->p_palette_lut[lp][0], (IRF_PALETTE_TYPE_T)lp, FALSE );
		if ( IRF_NO_ERROR != res ) { TRACE( _T("failed get palette %d.\r\n"), lp ); }
		res = GetImageLUT( _p_info->p_palette_lut[lp][1], (IRF_PALETTE_TYPE_T)lp, TRUE );
		if ( IRF_NO_ERROR != res ) { TRACE( _T("failed get invert palette %d.\r\n"), lp ); }
	}

	if ( NULL != m_pCamSetupDlg ) {
		m_pCamSetupDlg->SetCameraConfig( p_cfg_data );
		m_pCamSetupDlg->UpdateCameraConfig();
	}
}


void	CSampleCGDlg::DoRecvStreamData( TCAMINFO_PTR _p_info )
{
	IRF_IR_CAM_DATA_T *			p_ir_cam;

	IRF_TEMP_CORRECTION_PAR_T	corr;

	int16_t				res;

	int					lp;

	float				level, span;

	

	_ASSERT( NULL != _p_info );
	p_ir_cam	= &_p_info->ir_data;

	if ( TRUE == _p_info->f_agc_mode ) {
		_p_info->agc_ctrl.autoScale = _IRF_AUTO;
	}
	else {
		_p_info->agc_ctrl.autoScale = _IRF_MANUAL;

		level	= _p_info->agc_level;
		span	= _p_info->agc_span;
	}

	res = GetImageCG(
				_p_info->p_ir_tmp_buf,
				(HANDLE)p_ir_cam,
				_p_info->ir_size.xSize * _p_info->ir_size.ySize,
				&level,
				&span,
				&_p_info->agc_ctrl
				);

	if ( IRF_NO_ERROR != res ) {
		TRACE( _T("failed GetImage.(%d)\r\n"), res );
		return;
	}

	m_dwRecvStrmCnt++;
	if ( 1000 <= (::GetTickCount() - m_dwStrmBaseTick) ) {
		::SetDlgItemInt( GetSafeHwnd(), IDC_STC_NETFPS, m_dwRecvStrmCnt, FALSE );
		m_dwRecvStrmCnt = 0;

		m_dwStrmBaseTick = ::GetTickCount();
	}

	corr.emissivity		= (float)(p_ir_cam->save_data.emissivity / 100.f);
	corr.atmTemp		= (float)(p_ir_cam->save_data.atmosphere / 10.f);
	corr.atmTrans		= (float)(p_ir_cam->save_data.transmission / 100.f);
	corr.zero_offset	= 0;

	//	Raw Data to Temperature Data
	res = GetRawToTempCG(
						(HANDLE)p_ir_cam,
						_p_info->ir_size,
						corr,
						_p_info->get_temperature_buf()
						);
	if ( IRF_NO_ERROR != res ) {
		TRACE( _T("failed GetRawToTempCG.(%d)\r\n"), res );
		return;
	}

	{	//	Camera ROI Information
		TRAWTAIL_PTR				p_tail = NULL;

		IRF_NUMERIC_INFO_T	num_info;
		RECT				roi_rect;

		float				max, min;

		POINT				max_pos;
		POINT				min_pos;

		int					roi_num, roi_mode;

		TCHAR				value[64];


		p_tail = (TRAWTAIL_PTR)_p_info->get_tail_buf();
		//	check tail id
		if ( 0x434F5802 == p_tail->id ) {
			TRAWTAILV2_PTR				p_tail_v2 = (TRAWTAILV2_PTR)_p_info->get_tail_buf();


			max_pos.x = p_tail_v2->frame_pos.max_x;
			max_pos.y = p_tail_v2->frame_pos.max_y;
			::_stprintf_s( value, _T("%d"), max_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXX), value );
			::_stprintf_s( value, _T("%d"), max_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXY), value );

			min_pos.x = p_tail_v2->frame_pos.min_x;
			min_pos.y = p_tail_v2->frame_pos.min_y;
			::_stprintf_s( value, _T("%d"), min_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINX), value );
			::_stprintf_s( value, _T("%d"), min_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINY), value );

			max = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v2->frame_lvl.max_lvl );
			::_stprintf_s( value, _T("%0.1f 'C"), max );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXDATA), value );

			min = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v2->frame_lvl.min_lvl );
			::_stprintf_s( value, _T("%0.1f 'C"), min );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINDATA), value );

			::_stprintf_s( value, _T("%0.1f 'C"), ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v2->ctr_avg ) );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_CTRDATA), value );

			::_stprintf_s( value, _T("%0.1f 'C"), ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, (p_tail_v2->frame_sum / p_tail_v2->frame_tot_px) ) );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_AVGDATA), value );

			m_ctlScreen.SetHotPoint( max_pos, max );
			m_ctlScreen.SetColdPoint( min_pos, min );

			//	set roi camera information
			for ( lp = 0; lp < MAX_ROI; lp++ ) {
				if ( 0 == p_tail_v2->roi_sum[lp] ) {
					::memset( &m_stRoiCam[lp], 0x00, sizeof(m_stRoiCam[lp]) );
					continue;
				}

				m_stRoiCam[lp].max_x = p_tail_v2->roi_pos[lp].max_x;
				m_stRoiCam[lp].max_y = p_tail_v2->roi_pos[lp].max_y;

				m_stRoiCam[lp].min_x = p_tail_v2->roi_pos[lp].min_x;
				m_stRoiCam[lp].min_y = p_tail_v2->roi_pos[lp].min_y;

				m_stRoiCam[lp].max = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v2->roi_lvl[lp].max_lvl );
				m_stRoiCam[lp].min = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v2->roi_lvl[lp].min_lvl );
				m_stRoiCam[lp].avg = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, (p_tail_v2->roi_sum[lp] / p_tail_v2->roi_tot_px[lp]) );;
			}
		}
		else if ( 0x434F5803 == p_tail->id ) {
			TRAWTAILV3_PTR				p_tail_v3 = (TRAWTAILV3_PTR)_p_info->get_tail_buf();


			max_pos.x = p_tail_v3->frame_pos.max_x;
			max_pos.y = p_tail_v3->frame_pos.max_y;
			::_stprintf_s( value, _T("%d"), max_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXX), value );
			::_stprintf_s( value, _T("%d"), max_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXY), value );

			min_pos.x = p_tail_v3->frame_pos.min_x;
			min_pos.y = p_tail_v3->frame_pos.min_y;
			::_stprintf_s( value, _T("%d"), min_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINX), value );
			::_stprintf_s( value, _T("%d"), min_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINY), value );

			max = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v3->frame_lvl.max_lvl );
			::_stprintf_s( value, _T("%0.1f 'C"), max );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MAXDATA), value );

			min = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v3->frame_lvl.min_lvl );
			::_stprintf_s( value, _T("%0.1f 'C"), min );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_MINDATA), value );

			::_stprintf_s( value, _T("%0.1f 'C"), ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v3->ctr_avg ) );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_CTRDATA), value );

			::_stprintf_s( value, _T("%0.1f 'C"), ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, (p_tail_v3->frame_sum / p_tail_v3->frame_tot_px) ) );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_AVGDATA), value );

			m_ctlScreen.SetHotPoint( max_pos, max );
			m_ctlScreen.SetColdPoint( min_pos, min );

			//	set roi camera information
			for ( lp = 0; lp < MAX_ROI; lp++ ) {
				if ( 0 == p_tail_v3->roi_sum[lp] ) {
					::memset( &m_stRoiCam[lp], 0x00, sizeof(m_stRoiCam[lp]) );
					continue;
				}

				m_stRoiCam[lp].max_x = p_tail_v3->roi_pos[lp].max_x;
				m_stRoiCam[lp].max_y = p_tail_v3->roi_pos[lp].max_y;

				m_stRoiCam[lp].min_x = p_tail_v3->roi_pos[lp].min_x;
				m_stRoiCam[lp].min_y = p_tail_v3->roi_pos[lp].min_y;

				m_stRoiCam[lp].max = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v3->roi_lvl[lp].max_lvl );
				m_stRoiCam[lp].min = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, p_tail_v3->roi_lvl[lp].min_lvl );
				m_stRoiCam[lp].avg = ConvertRawToTempCG( (HANDLE)p_ir_cam, corr, (p_tail_v3->roi_sum[lp] / p_tail_v3->roi_tot_px[lp]) );;
			}
		}

		//	set roi sdk information
		for ( lp = 0; lp < MAX_ROI; lp++ ) {
			if ( FALSE == m_stRoiCfg[lp].f_enable ) {
				::memset( &m_stRoiSDK[lp], 0x00, sizeof(m_stRoiSDK[lp]) );
				continue;
			}

			roi_rect.left	= m_stRoiCfg[lp].x;
			roi_rect.right	= m_stRoiCfg[lp].x + m_stRoiCfg[lp].w - 1;

			roi_rect.top	= m_stRoiCfg[lp].y;
			roi_rect.bottom	= m_stRoiCfg[lp].y + m_stRoiCfg[lp].h - 1;

			GetROITempCG(
					(HANDLE)p_ir_cam,
						_p_info->ir_size,
						corr,
						roi_rect,
						&num_info,
						&min_pos,
						&max_pos
						);

			m_stRoiSDK[lp].max_x = (uint16_t)max_pos.x;
			m_stRoiSDK[lp].max_y = (uint16_t)max_pos.y;

			m_stRoiSDK[lp].min_x = (uint16_t)min_pos.x;
			m_stRoiSDK[lp].min_y = (uint16_t)min_pos.y;

			m_stRoiSDK[lp].max = num_info.max;
			m_stRoiSDK[lp].min = num_info.min;
			m_stRoiSDK[lp].avg = num_info.avg;
		}
		
		roi_num		= ::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROINUM), CB_GETCURSEL, 0L, 0L );
		roi_mode	= ::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROIMODE), CB_GETCURSEL, 0L, 0L );
		

		if ( 0 == roi_mode ) {
			max_pos.x = m_stRoiCam[roi_num].max_x;
			max_pos.y = m_stRoiCam[roi_num].max_y;
			::_stprintf_s( value, _T("%d"), max_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAXX), value );
			::_stprintf_s( value, _T("%d"), max_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAXY), value );

			min_pos.x = m_stRoiCam[roi_num].min_x;
			min_pos.y = m_stRoiCam[roi_num].min_y;
			::_stprintf_s( value, _T("%d"), min_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMINX), value );
			::_stprintf_s( value, _T("%d"), min_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMINY), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiCam[roi_num].max );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAX), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiCam[roi_num].min );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMIN), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiCam[roi_num].avg );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIAVG), value );
		}
		else {
			max_pos.x = m_stRoiSDK[roi_num].max_x;
			max_pos.y = m_stRoiSDK[roi_num].max_y;
			::_stprintf_s( value, _T("%d"), max_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAXX), value );
			::_stprintf_s( value, _T("%d"), max_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAXY), value );

			min_pos.x = m_stRoiSDK[roi_num].min_x;
			min_pos.y = m_stRoiSDK[roi_num].min_y;
			::_stprintf_s( value, _T("%d"), min_pos.x );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMINX), value );
			::_stprintf_s( value, _T("%d"), min_pos.y );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMINY), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiSDK[roi_num].max );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMAX), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiSDK[roi_num].min );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIMIN), value );

			::_stprintf_s( value, _T("%0.1f 'C"), m_stRoiSDK[roi_num].avg );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_ROIAVG), value );
		}
	}

	if ( _IRF_AUTO == _p_info->agc_ctrl.autoScale ) {
		_p_info->agc_level	= level;
		_p_info->agc_span	= span;
	}

	{
		TCHAR				value[64];


		::_stprintf_s( value, _T("%0.1f 'C"), level );
		::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_LEVEL), value );

		if ( _IRF_AUTO == _p_info->agc_ctrl.autoScale ) {
			::_stprintf_s( value, _T("%0.1f"), level );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_LEVEL), value );
		}
		
		::_stprintf_s( value, _T("%0.1f 'C"), span );
		::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_SPAN), value );

		if ( _IRF_AUTO == _p_info->agc_ctrl.autoScale ) {
			::_stprintf_s( value, _T("%0.1f"), span );
			::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_SPAN), value );
		}

		::_stprintf_s( value, _T("%0.1f 'C ~ %0.1f 'C"), level - span * 0.5 , level + span * 0.5 );
		::SetWindowText( ::GetDlgItem(GetSafeHwnd(), IDC_STC_SCALE), value );
	}

	{		
		HDC				h_dc = NULL;

		int				bits_pixel;


		h_dc		= ::GetDC( ::GetDlgItem(GetSafeHwnd(), IDC_STC_SCREEN) );
		bits_pixel	= ::GetDeviceCaps( h_dc, BITSPIXEL );

		switch ( bits_pixel ) {
		case	16	:
		case	24	:
		case	32	:
			{
				int						img_bytes;


				img_bytes = _p_info->ir_size.xSize * _p_info->ir_size.ySize * (bits_pixel / 8);

				::memset( _p_info->p_ir_img_buf, 0x00, img_bytes );

				res = GetGrayToPaletteImage(
										_p_info->p_ir_tmp_buf,
										_p_info->p_ir_img_buf,
										(uint16_t)_p_info->ir_size.xSize,
										(uint16_t)_p_info->ir_size.ySize,
										_p_info->get_sel_palette(),
										bits_pixel,
										_p_info->get_mirror(),
										_p_info->get_flip()
										);
				if ( IRF_NO_ERROR != res ) {
					TRACE( _T("failed GetGrayToPaletteImage.(%d)\r\n"), res );
					return;
				}

				m_ctlScreen.SetImageBuf( _p_info->p_ir_img_buf );
			}
			break;

		default		:
			{
				TRACE( _T("ERROR : UNSUPPORT %d Bits Pixel\r\n"), bits_pixel );
			}
			break;
		}

		if ( NULL != h_dc ) {
			::ReleaseDC( ::GetDlgItem(GetSafeHwnd(), IDC_STC_SCREEN), h_dc );
			h_dc = NULL;
		}
	}

}


unsigned int __stdcall CSampleCGDlg::RECEIVE_PROC( void* _param )
{
	TCAMINFO_PTR		p_cam_info	= reinterpret_cast<TCAMINFO_PTR>(_param);
	CSampleCGDlg *		p_owner		= reinterpret_cast<CSampleCGDlg*>(p_cam_info->p_owner);
	
	int16_t				res;

	_ASSERT( NULL != p_cam_info );
	_ASSERT( NULL != p_owner );

	//	reset ir data
	p_cam_info->reset_ir_data();

	//	first request camera data ( config data )
	p_owner->SendCameraCommand( p_cam_info, _IRF_REQ_CAM_DATA );

	while ( p_cam_info->is_run_thread() ) {
		if ( INVALID_HANDLE_VALUE == p_cam_info->h_sdk ) {
			// reconnection check
			if ( FALSE == p_owner->m_autoReconnect.GetCheck() ) break;
						
			Sleep( 10 * 1000 );

			// open connection
			res = OpenConnect( 
				&p_cam_info->h_sdk,				//	handle
				&p_cam_info->keep_alive_id,		//	timer id
				p_owner->m_ip_addr,				//	destination ip address
				p_owner->m_port_num,			//	destination port number
				AF_INET,						//	IPv4
				SOCK_STREAM						//	TCP Stream
				);
			if ( IRF_NO_ERROR != res ) continue ;
			
			//	reset ir data
			p_cam_info->reset_ir_data();

			//	first request camera data ( config data )
			p_owner->SendCameraCommand( p_cam_info, _IRF_REQ_CAM_DATA );

			// send stream on 
			if ( TRUE == p_cam_info->f_recv_stream ) {
				p_owner->SendCameraCommand( p_cam_info, _IRF_STREAM_ON );

				p_owner->m_dwStrmBaseTick	= ::GetTickCount();
				p_owner->m_dwRecvStrmCnt		= 0;
			}


		}

		if ( FALSE == p_owner->WaitRecvData( p_cam_info ) ) {
			TRACE( _T("socket error occur\r\n") );
			// close connection
			CloseConnect( &p_cam_info->h_sdk, p_cam_info->keep_alive_id );
			p_cam_info->keep_alive_id	= 0;
		}
	}
	
	TRACE( _T("receive thread stop.\r\n") );

	p_owner->m_stTCamInfo.reset_member();

	p_owner->DoInitControl( FALSE );


	return ( 0 );
}


BOOL	CSampleCGDlg::SendCameraCommand(
IRF_MESSAGE_TYPE_T	_msg,
uint16_t			_cmd,
uint16_t			_rcode
)
{
	int16_t			res;


	if ( INVALID_HANDLE_VALUE == m_stTCamInfo.h_sdk ) { return ( FALSE ); }

	res = SendCameraMessage(
							m_stTCamInfo.h_sdk,
							&(m_stTCamInfo.keep_alive_id),
							_msg,
							_cmd,
							_rcode
							);

	if ( IRF_NO_ERROR != res ) {
		return ( FALSE );
	}

	return ( TRUE );
}


BOOL	CSampleCGDlg::SendCameraCommand(
TCAMINFO_PTR			_p_info,
IRF_MESSAGE_TYPE_T		_msg,
uint16_t				_cmd,
uint16_t				_rcode
)
{
	int16_t			res;


	_ASSERT( NULL != _p_info );
	if ( INVALID_HANDLE_VALUE == m_stTCamInfo.h_sdk ) { return ( FALSE ); }

	res = SendCameraMessage(
							_p_info->h_sdk,
							&_p_info->keep_alive_id,
							_msg,
							_cmd,
							_rcode
							);

	if ( IRF_NO_ERROR != res ) {
		return ( FALSE );
	}

	return ( TRUE );
}


BOOL	CSampleCGDlg::SendCameraConfig(
IRF_MESSAGE_TYPE_T		_msg,
uint16_t				_cmd,
uint16_t				_rcode,
uint32_t				_rcode2,
uint32_t				_rcode3,
uint32_t				_rcode4
)
{
	int16_t			res;


	if ( INVALID_HANDLE_VALUE == m_stTCamInfo.h_sdk ) { return ( FALSE ); }

	res = SendMessageToCamera(
							m_stTCamInfo.h_sdk,
							&(m_stTCamInfo.keep_alive_id),
							_msg,
							_cmd,
							_rcode,
							_rcode2,
							_rcode3,
							_rcode4
							);

	if ( IRF_NO_ERROR != res ) {
		return ( FALSE );
	}

	return ( TRUE );
}


BOOL	CSampleCGDlg::SendCameraConfig(
TCAMINFO_PTR			_p_info,
IRF_MESSAGE_TYPE_T		_msg,
uint16_t				_cmd,
uint16_t				_rcode,
uint32_t				_rcode2,
uint32_t				_rcode3,
uint32_t				_rcode4
)
{
	int16_t			res;


	_ASSERT( NULL != _p_info );
	if ( INVALID_HANDLE_VALUE == m_stTCamInfo.h_sdk ) { return ( FALSE ); }

	res = SendMessageToCamera(
							_p_info->h_sdk,
							&_p_info->keep_alive_id,
							_msg,
							_cmd,
							_rcode,
							_rcode2,
							_rcode3,
							_rcode4
							);

	if ( IRF_NO_ERROR != res ) {
		return ( FALSE );
	}

	return ( TRUE );
}


void	CSampleCGDlg::EventMouseMove( POINT _pt )
{
	LRESULT			res;

	BOOL			flag;

	float			temp;
	float			scale_x, scale_y;


	m_ctlScreen.GetScreenScale( scale_x, scale_y );

	if ( 0 < _pt.x ) { _pt.x = (LONG)(_pt.x / scale_x); }
	if ( 0 < _pt.y ) { _pt.y = (LONG)(_pt.y / scale_y); }

	temp = m_stTCamInfo.get_temperature_pt( _pt );
		
	res = ::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_MOUSEON), BM_GETCHECK, 0L, 0L );

	if ( BST_CHECKED == res )	{ flag = TRUE; }
	else						{ flag = FALSE; }

	m_ctlScreen.SetDisplayMouseTemp( flag, temp );
}