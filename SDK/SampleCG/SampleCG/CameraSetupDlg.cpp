// CameraSetupDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "SampleCG.h"
#include "SampleCGDlg.h"
#include "CameraSetupDlg.h"
#include "afxdialogex.h"


// CCameraSetupDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CCameraSetupDlg, CDialogEx)

CCameraSetupDlg::CCameraSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCameraSetupDlg::IDD, pParent)
{

}

CCameraSetupDlg::~CCameraSetupDlg()
{
}

void CCameraSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLR_ISOBELOW, m_ctlBelowClr);
	DDX_Control(pDX, IDC_CLR_ISOINTERVAL, m_ctlIntervalClr);
	DDX_Control(pDX, IDC_CLR_ISOABOVE, m_ctlAboveClr);
}


BEGIN_MESSAGE_MAP(CCameraSetupDlg, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CCameraSetupDlg 메시지 처리기입니다.


BOOL CCameraSetupDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	DoInitialize();


	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CCameraSetupDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.
}


LRESULT CCameraSetupDlg::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
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

	case	WM_CLOSE					:
		{
			TRACE( _T("WM_CLOSE\r\n") );
		}
		break;

	case	WM_DESTROY					:
		{
			TRACE( _T("WM_DESTROY\r\n") );
		}
		break;
	}

	return CDialogEx::WindowProc( message, wParam, lParam );
}


BOOL CCameraSetupDlg::PreTranslateMessage( MSG* pMsg )
{
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

	return CDialogEx::PreTranslateMessage( pMsg );
}


void	CCameraSetupDlg::SetParentDialog( CSampleCGDlg * _p_parent_dlg )
{
	m_pParentDlg = _p_parent_dlg;
}


void	CCameraSetupDlg::SetCameraConfig( IRF_SAVEDATA_T * _p_save_data )
{
	::memcpy( &m_stCamCfg, _p_save_data, sizeof(IRF_SAVEDATA_T) );
}


void	CCameraSetupDlg::GetCameraConfig( IRF_SAVEDATA_T * _p_save_data )
{
	::memcpy( _p_save_data, &m_stCamCfg, sizeof(IRF_SAVEDATA_T) );
}


void	CCameraSetupDlg::UpdateCameraConfig( void )
{
	{	//	DISPLAY SETUP
		if ( 0 == m_stCamCfg.f_disp_icon ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_DISPICON), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_DISPICON), BST_CHECKED );
		}

		if ( 0 == m_stCamCfg.show_center ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CENTER), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CENTER), BST_CHECKED );
		}

		if ( 0 == m_stCamCfg.indicator ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_INDICATOR), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_INDICATOR), BST_CHECKED );
		}

		if ( 0 == m_stCamCfg.colorbar ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CLRBAR), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CLRBAR), BST_CHECKED );
		}

		if ( 0 == m_stCamCfg.showinfo ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_TEMPINFO), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_TEMPINFO), BST_CHECKED );
		}

		if ( 0 == m_stCamCfg.show_correction ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CORRINFO), BST_UNCHECKED );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_CORRINFO), BST_CHECKED );
		}

		if ( 2 < m_stCamCfg.show_spot ) { m_stCamCfg.show_spot = 2; }
		ComboBox_SetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_DISPROI), m_stCamCfg.show_spot );

		if ( 4 < m_stCamCfg.alpha ) { m_stCamCfg.alpha = 4; }
		ComboBox_SetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_TRANS), m_stCamCfg.alpha );

		if ( 1 < m_stCamCfg.unit ) { m_stCamCfg.unit = 1; }
		ComboBox_SetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_TEMPTYPE), m_stCamCfg.unit );
	}

	{	//	VIDEO SIZE SETUP
		int				h, v;


		if ( 0 == m_stCamCfg.tv ) {
			::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_TVMODE), _T("NTSC") );
			h = 720;	v = 480;
		}
		else if ( 1 == m_stCamCfg.tv ) {
			::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_TVMODE), _T("PAL") );
			h = 720;	v = 576;
		}
		else {
			::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_TVMODE), _T("UNKNOWN") );
			h = 0;		v = 0;
		}

		if ( 0 != h ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVW), UDM_SETRANGE32, (WPARAM)600, (LPARAM)h );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVH), UDM_SETRANGE32, (WPARAM)400, (LPARAM)v );

			h = h - (m_stCamCfg.scn0_l_margin + m_stCamCfg.scn0_r_margin);
			v = v - (m_stCamCfg.scn0_t_margin + m_stCamCfg.scn0_b_margin);

			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVW, h, FALSE );
			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVH, v, FALSE );

			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVW), UDM_SETPOS32, (WPARAM)0, (LPARAM)h );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVH), UDM_SETPOS32, (WPARAM)0, (LPARAM)v );
		}
		else {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVW), UDM_SETRANGE32, (WPARAM)0, (LPARAM)0 );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVH), UDM_SETRANGE32, (WPARAM)0, (LPARAM)0 );

			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVW, 0, FALSE );
			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVH, 0, FALSE );
		}


		switch ( m_stCamCfg.hdmi_mode ) {
		case	2	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("480p") );
				h = 720;	v = 480;
			}
			break;

		case	3	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("576p") );
				h = 720;	v = 576;
			}
			break;

		case	4	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("720p 50Hz") );
				h = 1280;	v = 720;
			}
			break;

		case	5	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("720p 60Hz") );
				h = 1280;	v = 720;
			}
			break;

		case	6	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("1080i 50Hz") );
				h = 1920;	v = 1080;
			}
			break;

		case	7	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("1080i 60Hz") );
				h = 1920;	v = 1080;
			}
			break;

		case	9	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("1080p 50Hz") );
				h = 1920;	v = 1080;
			}
			break;

		case	10	:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("1080p 60Hz") );
				h = 1920;	v = 1080;
			}
			break;

		default		:
			{
				::Edit_SetText( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIMODE), _T("UNKNOWN") );
				h = 0;		v = 0;
			}
			break;
		}

		switch ( h ) {
		case	720		:
			{
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETRANGE32, (WPARAM)600, (LPARAM)h );
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETRANGE32, (WPARAM)400, (LPARAM)v );
			}
			break;

		case	1280	:
			{
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETRANGE32, (WPARAM)1100, (LPARAM)h );
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETRANGE32, (WPARAM)600, (LPARAM)v );
			}
			break;

		case	1920	:
			{
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETRANGE32, (WPARAM)1700, (LPARAM)h );
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETRANGE32, (WPARAM)900, (LPARAM)v );
			}
			break;

		default			:
			{
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETRANGE32, (WPARAM)0, (LPARAM)0 );
				::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETRANGE32, (WPARAM)0, (LPARAM)0 );
			}
			break;
		}

		if ( 0 != h ) {
			h = h - (m_stCamCfg.scn1_l_margin + m_stCamCfg.scn1_r_margin);
			v = v - (m_stCamCfg.scn1_t_margin + m_stCamCfg.scn1_b_margin);

			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIW, h, FALSE );
			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIH, v, FALSE );

			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETPOS32, (WPARAM)0, (LPARAM)h );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETPOS32, (WPARAM)0, (LPARAM)v );
		}
		else {
			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIW, 0, FALSE );
			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIH, 0, FALSE );
		}
	}

	{	//	VIDEO SETUP
		::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_BRIGHT, (int)m_stCamCfg.brightness, TRUE );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_BRIGHT), UDM_SETPOS32, (WPARAM)0, (LPARAM)(int)m_stCamCfg.brightness );

		::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_CONTRAST, (int)m_stCamCfg.contrast, TRUE );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CONTRAST), UDM_SETPOS32, (WPARAM)0, (LPARAM)(int)m_stCamCfg.contrast );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ZOOM), CB_SETCURSEL, (WPARAM)m_stCamCfg.zoom, (LPARAM)0 );

		if ( 0 == m_stCamCfg.mirror && 0 == m_stCamCfg.flip ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROTATION), CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
		}
		else if ( 1 == m_stCamCfg.mirror && 0 == m_stCamCfg.flip ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROTATION), CB_SETCURSEL, (WPARAM)1, (LPARAM)0 );
		}
		else if ( 0 == m_stCamCfg.mirror && 1 == m_stCamCfg.flip ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROTATION), CB_SETCURSEL, (WPARAM)2, (LPARAM)0 );
		}
		else if ( 1 == m_stCamCfg.mirror && 1 == m_stCamCfg.flip ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ROTATION), CB_SETCURSEL, (WPARAM)3, (LPARAM)0 );
		}

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_PALETTE), CB_SETCURSEL, (WPARAM)m_stCamCfg.color, (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_INVERT), CB_SETCURSEL, (WPARAM)m_stCamCfg.invert, (LPARAM)0 );
	}

	{	//	Image Adjustments Setup
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_AGCMODE), CB_SETCURSEL, (WPARAM)m_stCamCfg.agc, (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.agc_man_max );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.agc_man_min );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NRFLT), CB_SETCURSEL, (WPARAM)m_stCamCfg.noise, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NR1STR), CB_SETCURSEL, (WPARAM)m_stCamCfg.nr1_strength, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NR2STR), CB_SETCURSEL, (WPARAM)m_stCamCfg.nr2_strength, (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_EEFLT), CB_SETCURSEL, (WPARAM)m_stCamCfg.f_edge_enhance, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_EESTR), CB_SETCURSEL, (WPARAM)m_stCamCfg.ee_strength, (LPARAM)0 );

		if ( 0 != m_stCamCfg.agc ) {
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), FALSE );
		}
		else {
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), TRUE );
		}
	}

	{	//	NUC Setup
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NUCMODE), CB_SETCURSEL, (WPARAM)m_stCamCfg.nuc_mode, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NUCTIME), CB_SETCURSEL, (WPARAM)m_stCamCfg.nuc_time, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_NUCTHRE), CB_SETCURSEL, (WPARAM)m_stCamCfg.nuc_thres, (LPARAM)0 );
	}

	{	//	Network Setup
		if ( 0 == m_stCamCfg.dhcp ) {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_USEDHCP), BST_UNCHECKED );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP),		TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET),	TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY),	TRUE );
		}
		else {
			Button_SetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_USEDHCP), BST_CHECKED );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP),		FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET),	FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY),	FALSE );
		}

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.ipaddr );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.netmask );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.gateway );
	}

	{	//	Alarm Output Setup
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMNUM), CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMTYPE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm1_type, (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMMODE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm1_mode, (LPARAM)0 );
		::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ALARMDUR, m_stCamCfg.alarm1_dura, FALSE );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ALARMDUR), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.alarm1_dura );
	}

	{	//	Alarm Config Setup
		TCHAR					msg[64];

		ALRMCFG_PTR				p_alarm_cfg;


		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

		p_alarm_cfg = GetSelectedAlarmMode( 0 );
		_ASSERT( NULL != p_alarm_cfg );

		if ( NULL == p_alarm_cfg ) {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGENABLE), CB_SETCURSEL, 0L, 0L );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGOUTPUT), CB_SETCURSEL, 0L, 0L );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGCONDITION), CB_SETCURSEL, 0L, 0L );

			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ACFGDLY, 0, FALSE );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETPOS32, (WPARAM)0, (LPARAM)0 );
						
			::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMP, _T("0 °C") );
			::_stprintf_s( msg, _T("%.1f °F"), (float)(0.0f / 10.0f) * 1.8f + 32.f );
			::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMPF, msg );
		}
		else {
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGENABLE), CB_SETCURSEL, p_alarm_cfg->f_enable, 0L );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGOUTPUT), CB_SETCURSEL, p_alarm_cfg->output_mask, 0L );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGCONDITION), CB_SETCURSEL, p_alarm_cfg->cond, 0L );

			::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ACFGDLY, p_alarm_cfg->delay, FALSE );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETPOS32, (WPARAM)0, (LPARAM)p_alarm_cfg->delay );

			::_stprintf_s( msg, _T("%.1f °C"), (float)(p_alarm_cfg->temp / 10.0f) );
			::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMP, msg );
			::_stprintf_s( msg, _T("%.1f °F"), (float)(p_alarm_cfg->temp / 10.0f) * 1.8f + 32.f );
			::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMPF, msg );
			::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGTEMP), UDM_SETPOS32, (WPARAM)0, (LPARAM)p_alarm_cfg->temp );
		}
	}

	{	//	Correction Setup
		TCHAR					msg[64];


		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_USEDCORR), CB_SETCURSEL, (WPARAM)m_stCamCfg.correction, (LPARAM)0L );

		::_stprintf_s( msg, _T("%1.02f"), (float)(m_stCamCfg.emissivity / 100.0f) );
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORREM, msg );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORREM), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.emissivity );

		::_stprintf_s( msg, _T("%1.02f"), (float)(m_stCamCfg.transmission / 100.0f) );
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRTR, msg );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRTR), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.transmission );

		::_stprintf_s( msg, _T("%1.02f°C"), (float)(m_stCamCfg.atmosphere / 10.0f) );
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRAT, msg );
		::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.atmosphere / 10.0f) * 1.8f + 32.f );
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRATF, msg );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRAT), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.atmosphere );

		::_stprintf_s( msg, _T("%3.01f"), (float)(m_stCamCfg.zero_offset / 10.0f) );
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ZEROOFFSET, msg );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ZEROOFFSET), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.zero_offset );

		if ( 0 == m_stCamCfg.measure_distance ) {
			::_stprintf_s( msg, _T("OFF") );
		}
		else {
			::_stprintf_s( msg, _T("%d"), m_stCamCfg.measure_distance );
		}
		::SetDlgItemText( GetSafeHwnd(), IDC_EDT_DISTANCE, msg );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_DISTANCE), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.measure_distance );
	}

	{	//	Isotherm Setup
		int						mode;


		switch ( m_stCamCfg.cg_iso[0].mode_mask & 0x07 ) {
		case	0x01	:	mode = 2;	break;
		case	0x02	:	mode = 1;	break;
		case	0x04	:	mode = 3;	break;
		case	0x07	:	mode = 4;	break;
		case	0x00	:
		default			:	mode = 0;	break;
		}

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ISOMODE), CB_SETCURSEL, (WPARAM)mode, (LPARAM)0L );

		UpdateIsotherm();
	}

	{	//	System Setup
		#define		MAX_DISPMODE			( 8 )

		const TCHAR *			HDMI_NONE	= _T("NONE"); 
		const TCHAR *			HDMI_MODE[MAX_DISPMODE]	= { 
													_T(" 480P"),		_T(" 576P"),
													_T(" 720P 50HZ"),	_T(" 720P 60HZ"),
													_T(" 1080I 50HZ"),	_T(" 1080I 60HZ"),
													_T(" 1080P 50HZ"),	_T(" 1080P 60HZ"),
												};

		const DISPLAYMODE		DISP_MODE[MAX_DISPMODE]	= {
														dm_480P,	dm_576P,
														dm_720P50,	dm_720P60,
														dm_1080I50,	dm_1080I60,
														dm_1080P50,	dm_1080P60
												};

		HWND			h_ctrl_wnd;

		int				lp, sub_lp, tot_cnt;
		LRESULT			res;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_HDMIMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );

		for ( lp = 0; lp < MAX_DISPMODE; lp++ ) {
			if ( 0xFF == m_stCamCfg.hdmi_list[lp] ) { continue; }

			for ( sub_lp = 0; sub_lp < MAX_DISPMODE; sub_lp++ ) {
				if ( DISP_MODE[sub_lp] != m_stCamCfg.hdmi_list[lp] ) { continue; }

				res = ::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)HDMI_MODE[sub_lp] );
				if ( CB_ERR == res ) {
				}
				else {
					::SendMessage( h_ctrl_wnd, CB_SETITEMDATA, (WPARAM)res, (LPARAM)DISP_MODE[sub_lp] );
				}
			}
		}

		tot_cnt = (int)::SendMessage( h_ctrl_wnd, CB_GETCOUNT, (WPARAM)0, (LPARAM)0 );
		if ( 0 > tot_cnt ) {
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, (WPARAM)0, (LPARAM)HDMI_NONE );
			::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

			::EnableWindow( h_ctrl_wnd, FALSE );
		}
		else {
			::EnableWindow( h_ctrl_wnd, TRUE );

			for ( lp = 0; lp < tot_cnt; lp++ ) {
				res = ::SendMessage( h_ctrl_wnd, CB_GETITEMDATA, (WPARAM)lp, (LPARAM)0 );
				if ( m_stCamCfg.hdmi_mode != (uint8_t)res ) { continue; }

				::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)lp, (LPARAM)0 );
				break;
			}
		}
		
		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TEMPMODE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)m_stCamCfg.temp_mode, (LPARAM)0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TVMODE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)m_stCamCfg.tv, (LPARAM)0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_PROTMODE );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)m_stCamCfg.srl_protocol, (LPARAM)0L );

		::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_RS485ID, m_stCamCfg.id, FALSE );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_RS485ID), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.id );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_RS485BAUD );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, (WPARAM)m_stCamCfg.baudrate, (LPARAM)0L );
	}
}


ALRMCFG_PTR		CCameraSetupDlg::GetSelectedAlarmMode( int _mode )
{
	ALRMCFG_PTR			p_temp;


	switch ( _mode ) {
	case	0	:	p_temp = &(m_stCamCfg.temp_alarm.max_temp);		break;
	case	1	:	p_temp = &(m_stCamCfg.temp_alarm.min_temp);		break;
	case	2	:	p_temp = &(m_stCamCfg.temp_alarm.avg_temp);		break;
	case	3	:	p_temp = &(m_stCamCfg.temp_alarm.ctr_temp);		break;
	case	4	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[0]);	break;
	case	5	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[1]);	break;
	case	6	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[2]);	break;
	case	7	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[3]);	break;
	case	8	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[4]);	break;
	case	9	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[5]);	break;
	case	10	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[6]);	break;
	case	11	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[7]);	break;
	case	12	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[8]);	break;
	case	13	:	p_temp = &(m_stCamCfg.temp_alarm.roi_temp[9]);	break;
	default		:	p_temp = NULL;									break;
	}

	return ( p_temp );
}


void	CCameraSetupDlg::UpdateIsotherm( void )
{
	TCHAR					msg[64];

	LRESULT					res;


	res = ::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ISOMODE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0L );
	if ( CB_ERR == res ) {
		return;
	}

	if ( 0x01 & m_stCamCfg.cg_iso[0].tran_mask ) {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0 );
	}
	else {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0 );
	}

	if ( 0x02 & m_stCamCfg.cg_iso[0].tran_mask ) {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0 );
	}
	else {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0 );
	}

	if ( 0x04 & m_stCamCfg.cg_iso[0].tran_mask ) {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0 );
	}
	else {
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0 );
	}

	::_stprintf_s( msg, _T("%.1f"), (float)(m_stCamCfg.cg_iso[0].max_temp / 10.0f) );
	::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMAX, msg );
	::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.cg_iso[0].max_temp / 10.0f) * 1.8f + 32.f );
	::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMAXF, msg );
	::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.cg_iso[0].max_temp );

	::_stprintf_s( msg, _T("%.1f"), (float)(m_stCamCfg.cg_iso[0].min_temp / 10.0f) );
	::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMIN, msg );
	::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.cg_iso[0].min_temp / 10.0f) * 1.8f + 32.f );
	::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMINF, msg );
	::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.cg_iso[0].min_temp );

	m_ctlBelowClr.SetColor( m_stCamCfg.cg_iso[0].below_color );
	m_ctlIntervalClr.SetColor( m_stCamCfg.cg_iso[0].between_color );
	m_ctlAboveClr.SetColor( m_stCamCfg.cg_iso[0].above_color );

	switch ( res ) {
	case	1	:
		{
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), TRUE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAXF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), TRUE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMINF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), FALSE );

			m_ctlBelowClr.EnableWindow( TRUE );
			m_ctlIntervalClr.EnableWindow( FALSE );
			m_ctlAboveClr.EnableWindow( FALSE );
		}
		break;

	case	2	:
		{
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), FALSE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAXF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), FALSE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMINF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), TRUE );

			m_ctlBelowClr.EnableWindow( FALSE );
			m_ctlIntervalClr.EnableWindow( FALSE );
			m_ctlAboveClr.EnableWindow( TRUE );
		}
		break;

	case	3	:
		{
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), FALSE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAXF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), TRUE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMINF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), TRUE );

			m_ctlBelowClr.EnableWindow( FALSE );
			m_ctlIntervalClr.EnableWindow( TRUE );
			m_ctlAboveClr.EnableWindow( FALSE );
		}
		break;

	case	4	:
		{
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), TRUE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAXF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), TRUE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMINF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), TRUE );

			m_ctlBelowClr.EnableWindow( TRUE );
			m_ctlIntervalClr.EnableWindow( TRUE );
			m_ctlAboveClr.EnableWindow( TRUE );
		}
		break;

	case	0	:
	default		:
		{
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMINTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOINTTR), FALSE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_ISOMAXTR), FALSE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAXF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), FALSE );

			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMINF), TRUE );
			::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), FALSE );

			m_ctlBelowClr.EnableWindow( FALSE );
			m_ctlIntervalClr.EnableWindow( FALSE );
			m_ctlAboveClr.EnableWindow( FALSE );
		}
		break;
	}
}


void	CCameraSetupDlg::SendIsothermMode( void )
{
	uint8_t			mode_mask;

	DWORD			code2;
	DWORD			code3;
	DWORD			code4;

	LRESULT			res;


	res = ::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ISOMODE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0L );
	if ( CB_ERR == res ) {
		return;
	}

	switch ( res ) {
	case	1	:	mode_mask = 0x02;	break;
	case	2	:	mode_mask = 0x01;	break;
	case	3	:	mode_mask = 0x04;	break;
	case	4	:	mode_mask = 0x07;	break;
	case	0	:
	default		:	mode_mask = 0x00;	break;
	}

	m_stCamCfg.cg_iso[0].mode_mask = mode_mask;

	code2 = (m_stCamCfg.cg_iso[0].mode_mask << 0) | (m_stCamCfg.cg_iso[0].tran_mask << 8);
	code3 = m_stCamCfg.cg_iso[0].max_temp;
	code4 = m_stCamCfg.cg_iso[0].min_temp;
	

	_ASSERT( NULL != m_pParentDlg );
	m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CGISO_CFG, 0, code2, code3, code4 );
}


void	CCameraSetupDlg::SendIsothermColor( void )
{
	_ASSERT( NULL != m_pParentDlg );
	m_pParentDlg->SendCameraConfig(
								_IRF_SET_CAM_DATA,
								CMD_CGISO_CLR,
								0,
								m_stCamCfg.cg_iso[0].above_color,
								m_stCamCfg.cg_iso[0].below_color,
								m_stCamCfg.cg_iso[0].between_color
								);
}


void	CCameraSetupDlg::DoInitialize( void )
{
	::memset( &m_stCamCfg, 0x00, sizeof(m_stCamCfg) );

	m_pParentDlg = NULL;

	{	//	DISPLAY SETUP
		{
			const	TCHAR *			DISPROI_MODE[3] = {
										_T(" Off"),
										_T(" Empty"),
										_T(" Filled")
									};

			HWND			h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_DISPROI );


			::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPROI_MODE[0] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPROI_MODE[1] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPROI_MODE[2] );
			::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
		}

		{
			const	TCHAR *			DISPTRANS_MODE[5] = {
										_T(" Off"),
										_T(" 20%"),
										_T(" 40%"),
										_T(" 60%"),
										_T(" 80%")
									};

			HWND			h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TRANS );


			::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTRANS_MODE[0] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTRANS_MODE[1] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTRANS_MODE[2] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTRANS_MODE[3] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTRANS_MODE[4] );
			::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
		}

		{
			const	TCHAR *			DISPTEMP_TYPE[3] = {
										_T(" Celsius"),
										_T(" Fahrenheit")
									};

			HWND			h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TEMPTYPE );


			::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTEMP_TYPE[0] );
			::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)DISPTEMP_TYPE[1] );
			::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
		}
	}

	{	//	VIDEO SIZE SETUP
		UDACCEL				accel;


		accel.nSec = 0;
		accel.nInc = 4;

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVW), UDM_SETACCEL, (WPARAM)1, (LPARAM)&accel );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVH), UDM_SETACCEL, (WPARAM)1, (LPARAM)&accel );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETACCEL, (WPARAM)1, (LPARAM)&accel );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETACCEL, (WPARAM)1, (LPARAM)&accel );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVW), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_TVW), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_TVH), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_TVH), (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIW), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIW), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_HDMIH), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_HDMIH), (LPARAM)0 );
	}

	{	//	VIDEO SETUP
		const TCHAR *			VDOZOOM_MODE[3]	= { _T(" Off"), _T(" x2"), _T(" x4") };
		const TCHAR *			VDOROTA_MODE[4]	= { _T(" Off"), _T(" Mirror"), _T(" Flip"), _T(" M + F") };
		const TCHAR *			VDOPALE_MODE[7]	= { _T(" Grey"), _T(" Iron"), _T(" Rainbow"), _T(" Grey Red"), _T(" Glowbow"), _T(" Yellow"), _T(" MidGrey") };
		const TCHAR *			VDOINVT_MODE[2]	= { _T(" Off"), _T(" On") };

		HWND			h_ctrl_wnd;


		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_BRIGHT), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_BRIGHT), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CONTRAST), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_CONTRAST), (LPARAM)0 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_BRIGHT), UDM_SETRANGE32, (WPARAM)-40, (LPARAM)40 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CONTRAST), UDM_SETRANGE32, (WPARAM)-10, (LPARAM)10 );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ZOOM );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOZOOM_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOZOOM_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOZOOM_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ROTATION );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOROTA_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOROTA_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOROTA_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOROTA_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_PALETTE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOPALE_MODE[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_INVERT );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOINVT_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)VDOINVT_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{	//	Image Adjustments Setup
		const TCHAR *			AGC_MODE[2]		= { _T(" Manual"), _T(" Auto") };
		const TCHAR *			NR_FILTER[4]	= { _T(" Off"), _T(" NR1"), _T(" NR2"), _T(" NR1+NR2") };
		const TCHAR *			EE_FILTER[2]	= { _T(" Off"), _T(" On") };
		const TCHAR *			STR_LEVEL[7]	= { _T(" Level0"), _T(" Level1"), _T(" Level2"), _T(" Level3"), _T(" Level4"), _T(" Level5"), _T(" Level6") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_AGCMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)AGC_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)AGC_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), UDM_SETRANGE32, (WPARAM)0, (LPARAM)16383 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), UDM_SETRANGE32, (WPARAM)0, (LPARAM)16383 );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NRFLT );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NR_FILTER[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NR_FILTER[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NR_FILTER[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NR_FILTER[3] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NR1STR );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NR2STR );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_EEFLT );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)EE_FILTER[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)EE_FILTER[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_EESTR );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)STR_LEVEL[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{	//	NUC Setup
		const TCHAR *			NUC_MODE[4]	= { _T(" Off"), _T(" Time"), _T(" Auto"), _T(" Time + Auto") };
		const TCHAR *			NUC_TIME[5]	= { _T("  1 min"), _T("  5 min"), _T(" 10 min"), _T(" 30 min"), _T(" 60 min") };
		const TCHAR *			NUC_THRE[5]	= { _T("  Lowest"), _T("  Low"), _T(" Middle"), _T(" High"), _T(" Highest") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NUCMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NUCTIME );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_TIME[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_TIME[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_TIME[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_TIME[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_TIME[4] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_NUCTHRE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_THRE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_THRE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_THRE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_THRE[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)NUC_THRE[4] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}

	{	//	Network Setup
	}

	{	//	Alarm Output Setup
		const TCHAR *			ALARM_TYPE[2] = { _T("NO"), _T("NC") };
		const TCHAR *			ALARM_MODE[7] = { _T("OFF"), _T("READY FOR USE"), _T("ALIVE PWM"), _T("TEMPERATURE"), _T("REMOTE CONTROL"), _T("TEST ON"), _T("TEST OFF") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ALARMNUM );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)_T("01") );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)_T("02") );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ALARMTYPE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_TYPE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_TYPE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ALARMMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_MODE[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ALARMDUR), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ALARMDUR), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ALARMDUR), UDM_SETRANGE32, (WPARAM)0, (LPARAM)99 );
	}

	{	//	Alarm Config Setup
		const TCHAR *			ALARM_SRC[14]	= { _T(" Maximum"), _T(" Minimum"), _T(" Average"), _T(" Center"),
													_T(" ROI 01"), _T(" ROI 02"), _T(" ROI 03"), _T(" ROI 04"), _T(" ROI 05"),
													_T(" ROI 06"), _T(" ROI 07"), _T(" ROI 08"), _T(" ROI 09"), _T(" ROI 10")
													};
		const TCHAR *			ALARM_EN[2]		= { _T(" Off"), _T(" On") };
		const TCHAR *			ALARM_OUT[4]	= { _T(" Off"), _T(" Alarm01"), _T(" Alarm02"), _T(" A01 + A02") };
		const TCHAR *			ALARM_COND[2]	= { _T(" Above"), _T(" Below") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ACFGMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[6] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[7] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[8] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[9] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[10] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[11] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[12] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_SRC[13] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ACFGENABLE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_EN[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_EN[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ACFGOUTPUT );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_OUT[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_OUT[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_OUT[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_OUT[3] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ACFGCONDITION );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_COND[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ALARM_COND[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ACFGDLY), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETRANGE32, (WPARAM)0, (LPARAM)99 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGTEMP), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ACFGTEMP), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGTEMP), UDM_SETRANGE32, (WPARAM)-200, (LPARAM)6500 );
	}

	{	//	Correction Setup
		const TCHAR *				CORR_USED[2] = { _T(" OFF"), _T(" ON") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_USEDCORR );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)CORR_USED[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)CORR_USED[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORREM), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_CORREM), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORREM), UDM_SETRANGE32, (WPARAM)1, (LPARAM)100 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRTR), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_CORRTR), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRTR), UDM_SETRANGE32, (WPARAM)1, (LPARAM)100 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRAT), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_CORRAT), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_CORRAT), UDM_SETRANGE32, (WPARAM)-500, (LPARAM)1000 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ZEROOFFSET), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ZEROOFFSET), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ZEROOFFSET), UDM_SETRANGE32, (WPARAM)-200, (LPARAM)200 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_DISTANCE), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_DISTANCE), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_DISTANCE), UDM_SETRANGE32, (WPARAM)0, (LPARAM)100 );
	}

	{	//	Isotherm Setup
		const TCHAR *				ISO_MODE[5] = { _T(" Off"), _T(" Max. Below"), _T(" Min. Above"), _T(" Interval"), _T(" All") };

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_ISOMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ISO_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ISO_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ISO_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ISO_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)ISO_MODE[4] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMAX), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMAX), UDM_SETRANGE32, (WPARAM)-200, (LPARAM)6500 );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_ISOMIN), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ISOMIN), UDM_SETRANGE32, (WPARAM)-200, (LPARAM)6500 );

		m_ctlBelowClr.SetColor( RGB(0, 0, 0) );
		m_ctlIntervalClr.SetColor( RGB(0, 0, 0) );
		m_ctlAboveClr.SetColor( RGB(0, 0, 0) );
	}

	{	//	System Setup
		const TCHAR *			TEMP_MODE[2] =	{ _T(" Normal"), _T(" High") };
		const TCHAR *			CVBS_MODE[2] =	{ _T(" NTSC"), _T(" PAL") };
		const TCHAR *			PROT_MODE[2] =	{ _T(" Pelco-D"), _T(" COX") };
		const TCHAR *			BAUD_MODE[7] =	{
													_T(" 2400"), _T(" 4800"), _T(" 9600"),
													_T(" 19200"), _T(" 38400"), _T(" 57600"),
													_T(" 115200")
												};

		HWND			h_ctrl_wnd;


		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TEMPMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)TEMP_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)TEMP_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_TVMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)CVBS_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)CVBS_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_HDMIMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_PROTMODE );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)PROT_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)PROT_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );

		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_RS485ID), UDM_SETBUDDY, (WPARAM)::GetDlgItem(GetSafeHwnd(), IDC_EDT_RS485ID), (LPARAM)0 );
		::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_RS485ID), UDM_SETRANGE32, (WPARAM)0, (LPARAM)255 );

		h_ctrl_wnd = ::GetDlgItem( GetSafeHwnd(), IDC_CMB_RS485BAUD );
		::SendMessage( h_ctrl_wnd, CB_RESETCONTENT, 0L, 0L );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[0] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[1] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[2] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[3] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[4] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[5] );
		::SendMessage( h_ctrl_wnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)BAUD_MODE[6] );
		::SendMessage( h_ctrl_wnd, CB_SETCURSEL, 0, 0L );
	}
}


void	CCameraSetupDlg::DoUninitialize( void )
{
}


void	CCameraSetupDlg::DoInitControl( BOOL _f_connect )
{
}


BOOL	CCameraSetupDlg::DoControlCommand( HWND _h_wnd, WORD _ctrl_id, WORD _notify_code, LRESULT* _p_result )
{
	*_p_result = 0;

	switch ( _ctrl_id ) {
	case	IDC_CHK_DISPICON		:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.f_disp_icon = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.f_disp_icon = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_SHOW_ICON, m_stCamCfg.f_disp_icon );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_CENTER			:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.show_center = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.show_center = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CENTER_CROSS, m_stCamCfg.show_center );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_INDICATOR		:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.indicator = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.indicator = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_TEMP_INDICATOR, m_stCamCfg.indicator );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_CLRBAR			:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.colorbar = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.colorbar = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_COLORBAR, m_stCamCfg.colorbar );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_TEMPINFO		:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.showinfo = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.showinfo = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_TEMP_VIEW, m_stCamCfg.showinfo );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_CORRINFO		:
		{
			if ( BN_CLICKED == _notify_code ) {
				int			res;


				res = Button_GetCheck( _h_wnd );
				if ( BST_UNCHECKED == res ) {
					m_stCamCfg.show_correction = 0;
				}
				else if ( BST_CHECKED == res ) {
					m_stCamCfg.show_correction = 1;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_SHOW_CORR, m_stCamCfg.show_correction );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_DISPROI			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.show_spot = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CG_ROI_INFO, m_stCamCfg.show_spot );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_TRANS			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.alpha = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_TRANSPARENCY, m_stCamCfg.alpha );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_TEMPTYPE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.unit = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_TEMP_TYPE, m_stCamCfg.unit );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_SETTV			:
		{
			if ( BN_CLICKED == _notify_code ) {
				uint32_t			org_w, org_h;
				uint32_t			chg_w, chg_h;
				uint32_t			code2, code3;


				if ( 0 == m_stCamCfg.tv ) {
					org_w = 720;
					org_h = 480;
				}
				else if ( 1 == m_stCamCfg.tv ) {
					org_w = 720;
					org_h = 576;
				}
				else {
					org_w = 0;
					org_h = 0;

					return ( TRUE );
				}

				chg_w = ::GetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVW, NULL, FALSE );
				chg_h = ::GetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVH, NULL, FALSE );

				m_stCamCfg.scn0_l_margin = (org_w - chg_w) / 2;
				m_stCamCfg.scn0_r_margin = (org_w - chg_w) / 2;
				m_stCamCfg.scn0_t_margin = (org_h - chg_h) / 2;
				m_stCamCfg.scn0_b_margin = (org_h - chg_h) / 2;

				code2 = (m_stCamCfg.scn0_l_margin << 16) | (m_stCamCfg.scn0_r_margin & 0xFFFF);
				code3 = (m_stCamCfg.scn0_t_margin << 16) | (m_stCamCfg.scn0_b_margin & 0xFFFF);
				
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CVBS_VDO_SIZE, 0, code2, code3, 0 );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_SETHDMI			:
		{
			if ( BN_CLICKED == _notify_code ) {
				uint32_t			org_w, org_h;
				uint32_t			chg_w, chg_h;
				uint32_t			code2, code3;


				if ( 0 == m_stCamCfg.tv ) {
					org_w = 720;
					org_h = 480;
				}
				else if ( 1 == m_stCamCfg.tv ) {
					org_w = 720;
					org_h = 576;
				}
				else {
					org_w = 0;
					org_h = 0;

					return ( TRUE );
				}
				switch ( m_stCamCfg.hdmi_mode ) {
				case	2	:
					{
						org_w = 720;
						org_h = 480;
					}
					break;

				case	3	:
					{
						org_w = 720;
						org_h = 576;
					}
					break;

				case	4	:
				case	5	:
					{
						org_w = 1280;
						org_h = 720;
					}
					break;

				case	6	:
				case	7	:
				case	9	:
				case	10	:
					{
						org_w = 1920;
						org_h = 1080;
					}
					break;

				default		:
					{
					}
					return ( TRUE );
				}

				chg_w = ::GetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIW, NULL, FALSE );
				chg_h = ::GetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIH, NULL, FALSE );

				m_stCamCfg.scn1_l_margin = (org_w - chg_w) / 2;
				m_stCamCfg.scn1_r_margin = (org_w - chg_w) / 2;
				m_stCamCfg.scn1_t_margin = (org_h - chg_h) / 2;
				m_stCamCfg.scn1_b_margin = (org_h - chg_h) / 2;

				code2 = (m_stCamCfg.scn1_l_margin << 16) | (m_stCamCfg.scn1_r_margin & 0xFFFF);
				code3 = (m_stCamCfg.scn1_t_margin << 16) | (m_stCamCfg.scn1_b_margin & 0xFFFF);
				
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_HDMI_VDO_SIZE, 0, code2, code3, 0 );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ZOOM			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.zoom = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_ZOOM, m_stCamCfg.zoom );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ROTATION		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_VDO_ROT, (uint16_t)res );

					if ( 0 == res ) {
						m_stCamCfg.mirror	= 0;
						m_stCamCfg.flip		= 0;
					}
					else if ( 1 == res ) {
						m_stCamCfg.mirror	= 1;
						m_stCamCfg.flip		= 0;
					}
					else if ( 2 == res ) {
						m_stCamCfg.mirror	= 0;
						m_stCamCfg.flip		= 1;
					}
					else if ( 3 == res ) {
						m_stCamCfg.mirror	= 1;
						m_stCamCfg.flip		= 1;
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_PALETTE			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.color = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_PALETTE, m_stCamCfg.color );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_INVERT			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.invert = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_INVERT, m_stCamCfg.invert );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_AGCMODE			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.agc = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_AGC, m_stCamCfg.agc );

					if ( 0 != m_stCamCfg.agc ) {
						::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), FALSE );
						::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), FALSE );
					}
					else {
						::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MAX), TRUE );
						::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_MIN), TRUE );
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NRFLT			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.noise = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NOISE_FILTER, m_stCamCfg.noise );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NR1STR			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.nr1_strength = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NR1_LEVEL, m_stCamCfg.nr1_strength );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NR2STR			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.nr2_strength = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NR2_LEVEL, m_stCamCfg.nr2_strength );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_EEFLT			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.f_edge_enhance = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_EDGE_FILTER, m_stCamCfg.f_edge_enhance );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_EESTR			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.ee_strength = (uint8_t)res;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_EE_LEVEL, m_stCamCfg.ee_strength );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_NUCONETIME		:
		{
			if ( BN_CLICKED == _notify_code ) {
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NUC_ONETIME );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NUCMODE			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;
				uint32_t			code2;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.nuc_mode = (uint8_t)res;

					code2 = (m_stCamCfg.nuc_time << 16) | m_stCamCfg.nuc_thres;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NUC_CFG, m_stCamCfg.nuc_mode, code2 );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NUCTIME			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;
				uint32_t			code2;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.nuc_time = (uint8_t)res;

					code2 = (m_stCamCfg.nuc_time << 16) | m_stCamCfg.nuc_thres;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NUC_CFG, m_stCamCfg.nuc_mode, code2 );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_NUCTHRE			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;
				uint32_t			code2;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					m_stCamCfg.nuc_thres = (uint16_t)res;

					code2 = (m_stCamCfg.nuc_time << 16) | m_stCamCfg.nuc_thres;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_NUC_CFG, m_stCamCfg.nuc_mode, code2 );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_USEDHCP			:
		{
			if ( BN_CLICKED == _notify_code ) {
				m_stCamCfg.dhcp = Button_GetCheck( ::GetDlgItem(GetSafeHwnd(), IDC_CHK_USEDHCP) );

				if ( 0 == m_stCamCfg.dhcp ) {
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP),		TRUE );
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET),	TRUE );
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY),	TRUE );

					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP), IPM_SETADDRESS, (WPARAM)0, (LPARAM)0xC0A80064 );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET), IPM_SETADDRESS, (WPARAM)0, (LPARAM)0xFFFFFF00 );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY), IPM_SETADDRESS, (WPARAM)0, (LPARAM)0xC0A80001 );
				}
				else {
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP),		FALSE );
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET),	FALSE );
					::EnableWindow( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY),	FALSE );

					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.ipaddr );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.netmask );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY), IPM_SETADDRESS, (WPARAM)0, (LPARAM)m_stCamCfg.gateway );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_NETSETUP		:
		{
			if ( BN_CLICKED == _notify_code ) {
				if ( 0 == m_stCamCfg.dhcp ) {
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_IP), IPM_GETADDRESS, (WPARAM)0, (LPARAM)&m_stCamCfg.ipaddr );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_SUBNET), IPM_GETADDRESS, (WPARAM)0, (LPARAM)&m_stCamCfg.netmask );
					::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_IPCTL_GATEWAY), IPM_GETADDRESS, (WPARAM)0, (LPARAM)&m_stCamCfg.gateway );
				}
				else {
					m_stCamCfg.ipaddr	= 0;
					m_stCamCfg.netmask	= 0;
					m_stCamCfg.gateway	= 0;
				}

				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig(
										_IRF_SET_CAM_DATA,
										CMD_ETHERNET,
										m_stCamCfg.dhcp,
										m_stCamCfg.ipaddr,
										m_stCamCfg.netmask,
										m_stCamCfg.gateway
										);

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ALARMNUM		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					if ( 0 == res ) {
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMTYPE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm1_type, (LPARAM)0 );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMMODE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm1_mode, (LPARAM)0 );
						::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ALARMDUR, m_stCamCfg.alarm1_dura, FALSE );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ALARMDUR), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.alarm1_dura );
					}
					else if ( 1 == res ) {
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMTYPE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm2_type, (LPARAM)0 );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMMODE), CB_SETCURSEL, (WPARAM)m_stCamCfg.alarm2_mode, (LPARAM)0 );
						::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ALARMDUR, m_stCamCfg.alarm2_dura, FALSE );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ALARMDUR), UDM_SETPOS32, (WPARAM)0, (LPARAM)m_stCamCfg.alarm2_dura );
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ALARMTYPE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;
				LRESULT				num;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					num = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMNUM) );

					if ( 0 == num ) {
						m_stCamCfg.alarm1_type = (uint8_t)res;
					}
					else if ( 1 == num ) {
						m_stCamCfg.alarm2_type = (uint8_t)res;
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ALARMMODE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				LRESULT				res;
				LRESULT				num;


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					num = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMNUM) );

					if ( 0 == num ) {
						m_stCamCfg.alarm1_mode = (uint8_t)res;
					}
					else if ( 1 == num ) {
						m_stCamCfg.alarm2_mode = (uint8_t)res;
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_ALARMSETUP		:
		{
			if ( BN_CLICKED == _notify_code ) {
				LRESULT				num;
				DWORD				code2;


				num = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMNUM) );

				if ( 0 == num ) {
					code2 = (m_stCamCfg.alarm1_type << 24) | (m_stCamCfg.alarm1_mode << 16) | (m_stCamCfg.alarm1_dura << 8);

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_ALARM_CONFIG, 0, code2 );
				}
				else if ( 1 == num ) {
					code2 = (m_stCamCfg.alarm2_type << 24) | (m_stCamCfg.alarm2_mode << 16) | (m_stCamCfg.alarm2_dura << 8);

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_ALARM_CONFIG, 1, code2 );
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ACFGMODE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;

				TCHAR				msg[64];


				res = ComboBox_GetCurSel( _h_wnd );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL == p_alarm_cfg ) {
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGENABLE), CB_SETCURSEL, 0L, 0L );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGOUTPUT), CB_SETCURSEL, 0L, 0L );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGCONDITION), CB_SETCURSEL, 0L, 0L );

						::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ACFGDLY, 0, FALSE );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETPOS32, (WPARAM)0, (LPARAM)0 );
						
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMP, _T("0 °C") );
						::_stprintf_s( msg, _T("%.1f °F"), (float)(0.0f / 10.0f) * 1.8f + 32.f );
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMPF, msg );
					}
					else {
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGENABLE), CB_SETCURSEL, p_alarm_cfg->f_enable, 0L );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGOUTPUT), CB_SETCURSEL, p_alarm_cfg->output_mask, 0L );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGCONDITION), CB_SETCURSEL, p_alarm_cfg->cond, 0L );

						::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ACFGDLY, p_alarm_cfg->delay, FALSE );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGDLY), UDM_SETPOS32, (WPARAM)0, (LPARAM)p_alarm_cfg->delay );

						::_stprintf_s( msg, _T("%.1f °C"), (float)(p_alarm_cfg->temp / 10.0f) );
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMP, msg );
						::_stprintf_s( msg, _T("%.1f °F"), (float)(p_alarm_cfg->temp / 10.0f) * 1.8f + 32.f );
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMPF, msg );
						::SendMessage( ::GetDlgItem(GetSafeHwnd(), IDC_SPN_ACFGTEMP), UDM_SETPOS32, (WPARAM)0, (LPARAM)p_alarm_cfg->temp );
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ACFGENABLE		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						res = ComboBox_GetCurSel( _h_wnd );
						if ( CB_ERR == res ) {
						}
						else {
							p_alarm_cfg->f_enable = (uint8_t)res;
						}
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ACFGOUTPUT		:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						res = ComboBox_GetCurSel( _h_wnd );
						if ( CB_ERR == res ) {
						}
						else {
							p_alarm_cfg->output_mask = (uint8_t)res;
						}
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ACFGCONDITION	:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						res = ComboBox_GetCurSel( _h_wnd );
						if ( CB_ERR == res ) {
						}
						else {
							p_alarm_cfg->cond = (uint8_t)res;
						}
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_ACFGSETUP		:
		{
			if ( BN_CLICKED == _notify_code ) {
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					WORD				sel_id;
					DWORD				code2, code3;


					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						switch ( res ) {
						case	0	:	sel_id = 0;		break;
						case	1	:	sel_id = 1;		break;
						case	2	:	sel_id = 2;		break;
						case	3	:	sel_id = 3;		break;
						case	4	:	sel_id = 11;	break;
						case	5	:	sel_id = 12;	break;
						case	6	:	sel_id = 13;	break;
						case	7	:	sel_id = 14;	break;
						case	8	:	sel_id = 15;	break;
						case	9	:	sel_id = 16;	break;
						case	10	:	sel_id = 17;	break;
						case	11	:	sel_id = 18;	break;
						case	12	:	sel_id = 19;	break;
						case	13	:	sel_id = 20;	break;
						default		:	return ( 0 );
						}
														
						code2 = (p_alarm_cfg->f_enable << 24) | (p_alarm_cfg->output_mask << 16) | (p_alarm_cfg->cond << 8);
						code3 = (p_alarm_cfg->delay << 16) | p_alarm_cfg->temp;

						_ASSERT( NULL != m_pParentDlg );
						m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_ALARM_TEMPCFG, sel_id, code2, code3 );
					}
				}

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_CORRSETUP		:
		{
			if ( BN_CLICKED == _notify_code ) {
				DWORD				code2, code3;


				code2 = (m_stCamCfg.emissivity << 24) | (m_stCamCfg.transmission << 16) | m_stCamCfg.atmosphere;
				code3 = (m_stCamCfg.measure_distance << 16) | m_stCamCfg.zero_offset;
				
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_TEMP_CORRECT, m_stCamCfg.correction, code2, code3 );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CMB_ISOMODE			:
		{
			if ( CBN_SELCHANGE == _notify_code ) {
				UpdateIsotherm();
				SendIsothermMode();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_ISOMAXTR		:
		{
			if ( BN_CLICKED == _notify_code ) {
				LRESULT				res;


				res = ::SendMessage( _h_wnd, BM_GETCHECK, (WPARAM)0L, (LPARAM)0L );
				if ( BST_CHECKED == res )	{ m_stCamCfg.cg_iso[0].tran_mask |= 0x02; }
				else						{ m_stCamCfg.cg_iso[0].tran_mask &= ~0x02; }

				SendIsothermMode();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_ISOMINTR		:
		{
			if ( BN_CLICKED == _notify_code ) {
				LRESULT				res;


				res = ::SendMessage( _h_wnd, BM_GETCHECK, (WPARAM)0L, (LPARAM)0L );
				if ( BST_CHECKED == res )	{ m_stCamCfg.cg_iso[0].tran_mask |= 0x01; }
				else						{ m_stCamCfg.cg_iso[0].tran_mask &= ~0x01; }

				SendIsothermMode();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CHK_ISOINTTR		:
		{
			if ( BN_CLICKED == _notify_code ) {
				LRESULT				res;


				res = ::SendMessage( _h_wnd, BM_GETCHECK, (WPARAM)0L, (LPARAM)0L );
				if ( BST_CHECKED == res )	{ m_stCamCfg.cg_iso[0].tran_mask |= 0x04; }
				else						{ m_stCamCfg.cg_iso[0].tran_mask &= ~0x04; }

				SendIsothermMode();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CLR_ISOBELOW		:
		{
			if ( BN_CLICKED == _notify_code ) {
				m_stCamCfg.cg_iso[0].below_color = m_ctlBelowClr.GetColor();

				SendIsothermColor();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CLR_ISOINTERVAL		:
		{
			if ( BN_CLICKED == _notify_code ) {
				m_stCamCfg.cg_iso[0].between_color = m_ctlIntervalClr.GetColor();

				SendIsothermColor();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_CLR_ISOABOVE		:
		{
			if ( BN_CLICKED == _notify_code ) {
				m_stCamCfg.cg_iso[0].above_color = m_ctlAboveClr.GetColor();

				SendIsothermColor();

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;

	case	IDC_BTN_CLOSE			:
		{
			if ( BN_CLICKED == _notify_code ) {
				::ShowWindow( GetSafeHwnd(), SW_HIDE );

				*_p_result = 0;

				return ( TRUE );
			}
		}
		break;
	}

	return ( FALSE );
}


BOOL	CCameraSetupDlg::DoControlNotify( NMHDR* _p_nmhdr, LRESULT* _p_result )
{
	*_p_result = 0;

	if ( UDN_DELTAPOS == _p_nmhdr->code ) {
		LPNMUPDOWN		p_updown = (LPNMUPDOWN)_p_nmhdr;

		int				min, max;


		::SendMessage( _p_nmhdr->hwndFrom, UDM_GETRANGE32, (WPARAM)&min, (LPARAM)&max );

		p_updown->iPos += p_updown->iDelta;

		if ( max < p_updown->iPos )	{ p_updown->iPos = max; }
		if ( min > p_updown->iPos )	{ p_updown->iPos = min; }

		switch ( _p_nmhdr->idFrom ) {
		case	IDC_SPN_TVW			:
			{
				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVW, p_updown->iPos, FALSE );
				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_TVH			:
			{
				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_TVH, p_updown->iPos, FALSE );
				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_HDMIW		:
			{
				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIW, p_updown->iPos, FALSE );
				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_HDMIH		:
			{
				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_HDMIH, p_updown->iPos, FALSE );
				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_BRIGHT		:
			{
				int				bright = (int)m_stCamCfg.brightness;


				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_BRIGHT, p_updown->iPos, TRUE );
				if ( bright != p_updown->iPos ) {
					m_stCamCfg.brightness = (uint8_t)p_updown->iPos;
					bright = p_updown->iPos;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CG_BRIGHT, bright );
				}

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_CONTRAST	:
			{
				int				contrast = (int)m_stCamCfg.contrast;


				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_CONTRAST, p_updown->iPos, TRUE );
				if ( contrast != p_updown->iPos ) {
					m_stCamCfg.contrast = (uint8_t)p_updown->iPos;
					contrast = p_updown->iPos;

					_ASSERT( NULL != m_pParentDlg );
					m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_CG_CONTRAST, contrast );
				}

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_MAX			:
			{
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_AGC_MANMAX, p_updown->iDelta );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_MIN			:
			{
				_ASSERT( NULL != m_pParentDlg );
				m_pParentDlg->SendCameraConfig( _IRF_SET_CAM_DATA, CMD_AGC_MANMIN, p_updown->iDelta );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_ALARMDUR	:
			{
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ALARMNUM) );
				if ( 0 == res )			{ m_stCamCfg.alarm1_dura = p_updown->iPos; }
				else if ( 1 == res )	{ m_stCamCfg.alarm2_dura = p_updown->iPos; }

				::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ALARMDUR, p_updown->iPos, FALSE );
			}
			return ( TRUE );

		case	IDC_SPN_ACFGDLY		:
			{
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						p_alarm_cfg->delay = p_updown->iPos;
						::SetDlgItemInt( GetSafeHwnd(), IDC_EDT_ACFGDLY, p_updown->iPos, FALSE );
					}
				}
			}
			return ( TRUE );

		case	IDC_SPN_ACFGTEMP	:
			{
				ALRMCFG_PTR			p_alarm_cfg;
				LRESULT				res;

				TCHAR				msg[64];


				res = ComboBox_GetCurSel( ::GetDlgItem(GetSafeHwnd(), IDC_CMB_ACFGMODE) );
				if ( CB_ERR == res ) {
				}
				else {
					p_alarm_cfg = GetSelectedAlarmMode( (int)res );
					_ASSERT( NULL != p_alarm_cfg );

					if ( NULL != p_alarm_cfg ) {
						p_alarm_cfg->temp = p_updown->iPos;

						::_stprintf_s( msg, _T("%.1f °C"), (float)(p_alarm_cfg->temp / 10.0f) );
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMP, msg );
						::_stprintf_s( msg, _T("%.1f °F"), (float)(p_alarm_cfg->temp / 10.0f) * 1.8f + 32.f );
						::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ACFGTEMPF, msg );
					}
				}
			}
			return ( TRUE );

		case	IDC_SPN_CORREM		:
			{
				TCHAR			msg[24];


				m_stCamCfg.emissivity = (uint8_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%1.02f"), (float)(m_stCamCfg.emissivity / 100.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORREM, msg );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_CORRTR		:
			{
				TCHAR			msg[24];


				m_stCamCfg.transmission = (uint8_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%1.02f"), (float)(m_stCamCfg.transmission / 100.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRTR, msg );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_CORRAT		:
			{
				TCHAR			msg[24];


				m_stCamCfg.atmosphere = (int16_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%3.01f °C"), (float)(m_stCamCfg.atmosphere / 10.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRAT, msg );
				::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.atmosphere / 10.0f) * 1.8f + 32.f );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_CORRATF, msg );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_ZEROOFFSET	:
			{
				TCHAR			msg[24];


				m_stCamCfg.zero_offset = (int16_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%3.01f"), (float)(m_stCamCfg.zero_offset / 10.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ZEROOFFSET, msg );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_DISTANCE	:
			{
				TCHAR			msg[24];


				m_stCamCfg.measure_distance = (uint16_t)p_updown->iPos;

				if ( 0 == m_stCamCfg.measure_distance ) {
					::_stprintf_s( msg, _T("OFF") );
				}
				else {
					::_stprintf_s( msg, _T("%d"), m_stCamCfg.measure_distance );
				}
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_DISTANCE, msg );

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_ISOMAX		:
			{
				TCHAR			msg[24];


				m_stCamCfg.cg_iso[0].max_temp = (int16_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%.1f °C"), (float)(m_stCamCfg.cg_iso[0].max_temp / 10.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMAX, msg );
				::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.cg_iso[0].max_temp / 10.0f) * 1.8f + 32.f );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMAXF, msg );

				SendIsothermMode();

				*_p_result = 0;
			}
			return ( TRUE );

		case	IDC_SPN_ISOMIN		:
			{
				TCHAR			msg[24];


				m_stCamCfg.cg_iso[0].min_temp = (int16_t)p_updown->iPos;

				::_stprintf_s( msg, _T("%.1f °C"), (float)(m_stCamCfg.cg_iso[0].min_temp / 10.0f) );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMIN, msg );
				::_stprintf_s( msg, _T("%.1f °F"), (float)(m_stCamCfg.cg_iso[0].min_temp / 10.0f) * 1.8f + 32.f );
				::SetDlgItemText( GetSafeHwnd(), IDC_EDT_ISOMINF, msg );

				SendIsothermMode();

				*_p_result = 0;
			}
			return ( TRUE );
		}
	}

	return ( FALSE );
}