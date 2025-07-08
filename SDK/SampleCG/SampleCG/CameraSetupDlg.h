#pragma once

//#include	"SampleCGDlg.h"


// CCameraSetupDlg 대화 상자입니다.
class CSampleCGDlg;
class CCameraSetupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraSetupDlg)

public			:
	CCameraSetupDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CCameraSetupDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_CAMSETUP };


protected		:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.


private			:

	typedef	enum	__display_mode__
	{
		dm_INVALID  = -1,               /**< Display Mode Invalid */
		dm_480I     = 0,				/**< Display Mode 480I */
		dm_576I     = 1,				/**< Display Mode 576I */
		dm_480P     = 2,				/**< Display Mode 480P */
		dm_576P     = 3,				/**< Display Mode 576P */
		dm_720P50   = 4,				/**< Display Mode 720P 50HZ */
		dm_720P60   = 5,				/**< Display Mode 720P 60HZ */
		dm_1080I50  = 6,				/**< Display Mode 1080I 50HZ */
		dm_1080I60  = 7,				/**< Display Mode 1080I 60HZ */
		dm_1080P24  = 8,				/**< Display Mode 1080P 24HZ */
		dm_1080P50  = 9,				/**< Display Mode 1080P 50HZ */
		dm_1080P60  = 10,				/**< Display Mode 1080P 60HZ */
		dm_PAL      = 11,				/**< Display Mode PAL */
		dm_PAL_S    = 12,				/**< Display Mode PAL S-Video */
		dm_NONE     = 13,               /**< Display Mode NONE */
		dm_NTSC     = 14,				/**< Display Mode NTSC */
		dm_NTSC_S   = 15,				/**< Display Mode NTSC S-Video */
		dm_MAX_NUM
	}	DISPLAYMODE;

	CSampleCGDlg *				m_pParentDlg;

	IRF_SAVEDATA_T				m_stCamCfg;


public			:
	void	SetParentDialog( CSampleCGDlg * _p_parent_dlg );

	void	SetCameraConfig( IRF_SAVEDATA_T * _p_save_data );
	void	GetCameraConfig( IRF_SAVEDATA_T * _p_save_data );

	void	UpdateCameraConfig( void );

	ALRMCFG_PTR		GetSelectedAlarmMode( int _mode );

	void	UpdateIsotherm( void );
	void	SendIsothermMode( void );
	void	SendIsothermColor( void );


protected		:
	void	DoInitialize( void );
	void	DoUninitialize( void );

	void	DoInitControl( BOOL _f_connect );

	BOOL	DoControlCommand( HWND _h_wnd, WORD _ctrl_id, WORD _notify_code, LRESULT* _p_result );
	BOOL	DoControlNotify( NMHDR* _p_nmhdr, LRESULT* _p_result );


protected		:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );


public			:
	CMFCColorButton			m_ctlBelowClr;
	CMFCColorButton			m_ctlIntervalClr;
	CMFCColorButton			m_ctlAboveClr;

	virtual BOOL PreTranslateMessage( MSG* pMsg );
};
