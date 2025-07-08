
// SampleCGDlg.h : header file
//

#pragma once

#include	"ScreenWnd.h"
#include	"CameraSetupDlg.h"
#include "afxwin.h"


#ifdef _USED_BONJURESDK_
#define WM_BONJURE_SERVICE_EVENT			( WM_USER + 0x001 )
#endif

typedef	struct __thermal_camera_information__
{
	//#define	IRBUFSIZE			(1024 * 1024)
	//#define	IRBUFSIZE			(384 * 289)		//	QVGA
	#define	IRBUFSIZE			(640 * 481)		//	VGA or QVGA
	#define	MAX_PALETTE			(13)
	#define	PALETTE_SIZE		(256 * 3)

	void *				p_owner;			//	pointer of owner window

	HANDLE				h_sdk;				//	Thermal SDK Handle
	uint32_t			keep_alive_id;		//	Keep Alive ID

	uint8_t				ip_addr[4];			//	Thermal Camera IP Address
	uint16_t			port;				//	Thermal Camera Port Number

	BOOL				f_run_thread;		//	flag of run thread
	HANDLE				h_thread;			//	thread handle

	BOOL				f_recv_stream;		//	flag of receive stream


	IRF_IR_CAM_DATA_T			ir_data;			//	ir data structure
	IRF_AUTO_RANGE_METHOD_T		agc_ctrl;			//	auto gain control
	
	IRF_IMAGE_INFO_T			ir_size;			//	ir data size

	IRF_PALETTE_TYPE_T			ir_palette;			//	ir palette

	BOOL						f_invert;			//	flag of invert
	BOOL						f_mirror;			//	flag of mirror
	BOOL						f_flip;				//	flag of flip

	BOOL						f_agc_mode;			//	flag of agc mode

	float						agc_level;			//	agc level
	float						agc_span;			//	agc span

	int							temp_mode;			//	temperature mode

	uint8_t *					p_ir_tmp_buf;		//	pointer of ir temporary buffer
	uint8_t *					p_ir_img_buf;		//	pointer of ir image buffer
	float *						p_ir_temp_buf;		//	pointer of ir temperature buffer

	uint8_t *					p_palette_lut[MAX_PALETTE][2];	//	palette look-up table


	__thermal_camera_information__( void ) {
		initial();
	}

	~__thermal_camera_information__( void ) {
		uninitial();
	}

	__inline BOOL	is_run_thread( void ) {
		return ( f_run_thread );
	}
	
	void	reset_member( void ) {
		p_owner = NULL;

		h_sdk			= INVALID_HANDLE_VALUE;
		keep_alive_id	= 0;

		ip_addr[0] = 0;
		ip_addr[1] = 0;
		ip_addr[2] = 0;
		ip_addr[3] = 0;

		port = 0;

		f_run_thread	= FALSE;
		h_thread		= NULL;

		f_recv_stream = FALSE;

		::memset( &agc_ctrl, 0x00, sizeof(agc_ctrl) );

		ir_size.xSize = 0;
		ir_size.ySize = 0;

		ir_palette = GREY_COLOR_MAP;

		f_invert	= FALSE;
		f_mirror	= FALSE;
		f_flip		= FALSE;

		f_agc_mode	= TRUE;
		agc_level	= 0.0f;
		agc_span	= 0.0f;

		temp_mode = 0;
	}

	void	reset_ir_data( void ) {
		::memset( ir_data.ir_image, 0x00, IRBUFSIZE * sizeof(uint16_t) );
		::memset( ir_data.lpNextData, 0x00, IRF_BUFFER_SIZE );

		ir_data.dwSize		= 0;
		ir_data.dwPosition	= 0;

		::memset( p_ir_tmp_buf, 0x00, IRBUFSIZE );
		::memset( p_ir_img_buf, 0x00, IRBUFSIZE * 4 );

		::memset( p_ir_temp_buf, 0x00, IRBUFSIZE * sizeof(float) );

		agc_ctrl.autoScale		= _IRF_AUTO;
		agc_ctrl.inputMethod	= (IRF_AUTO_RANGE_INPUT_METHOD_T)2;
		agc_ctrl.SD_Rate		= 5.0f;
		agc_ctrl.B_Rate			= 0.01f;
		agc_ctrl.outputMethod	= (IRF_AUTO_RANGE_OUTPUT_METHOD_T)0;
		agc_ctrl.intercept		= 0;
		agc_ctrl.gamma			= 1.0f;
		agc_ctrl.plateau		= 100;
		agc_ctrl.epsilon		= 0.5f;
		agc_ctrl.psi			= 0.3f;
		agc_ctrl.prevPalteau	= 0;
	}

	__inline BOOL	is_connect( void ) {
		return ( INVALID_HANDLE_VALUE == h_sdk ? FALSE : TRUE );
	}

	__inline void	set_sel_palette( int _sel_pal ) {
		ir_palette = (IRF_PALETTE_TYPE_T)_sel_pal;
	}

	__inline uint8_t *	get_sel_palette( void ) {
		return p_palette_lut[ir_palette][f_invert?1:0];
	}

	__inline void	set_invert( BOOL _flag ) {
		f_invert = _flag;
	}

	__inline BOOL	get_invert( void ) {
		return ( f_invert );
	}

	__inline void	set_mirror( BOOL _flag ) {
		f_mirror = _flag;
	}

	__inline BOOL	get_mirror( void ) {
		return ( f_mirror );
	}

	__inline void	set_flip( BOOL _flag ) {
		f_flip = _flag;
	}

	__inline BOOL	get_flip( void ) {
		return ( f_flip );
	}

	__inline void	set_agc_mode( BOOL _flag ) {
		f_agc_mode = _flag;
	}

	__inline BOOL	get_agc_mode( void ) {
		return ( f_agc_mode );
	}

	__inline void	set_agc_level( float _level ) {
		agc_level = _level;
	}

	__inline float	get_agc_level( void ) {
		return ( agc_level );
	}

	__inline void	set_agc_span( float _span ) {
		agc_span = _span;
	}

	__inline float	get_agc_span( void ) {
		return ( agc_span );
	}

	__inline void	set_temp_mode( int _mode ) {
		temp_mode = _mode;
	}

	__inline BOOL	get_temp_mode( void ) {
		return ( temp_mode );
	}

	__inline uint16_t *	get_tail_buf( void ) {
		return ( &ir_data.ir_image[ir_size.xSize * ir_size.ySize] );
	}

	__inline float *	get_temperature_buf( void ) {
		return ( p_ir_temp_buf );
	}

	__inline float	get_temperature_pt( POINT _pt ) {
		int				offset;


		if ( NULL == p_ir_temp_buf )	{ return ( 0.0f); }
		if ( _pt.y >= ir_size.ySize )	{ return ( 0.0f); }
		if ( _pt.x >= ir_size.xSize )	{ return ( 0.0f); }

		offset = (_pt.y * ir_size.xSize) + _pt.x;

		return ( p_ir_temp_buf[offset] );
	}

private			:

	void	initial( void ) {
		int					lp;


		p_owner = NULL;

		h_sdk			= INVALID_HANDLE_VALUE;
		keep_alive_id	= 0;

		ip_addr[0] = 0;
		ip_addr[1] = 0;
		ip_addr[2] = 0;
		ip_addr[3] = 0;

		port = 0;

		f_run_thread	= FALSE;
		h_thread		= NULL;

		f_recv_stream = FALSE;

		::memset( &ir_data, 0x00, sizeof(ir_data) );

		ir_data.ir_image			= new uint16_t[IRBUFSIZE];
		ir_data.image_buffer_size	= IRBUFSIZE;
		ir_data.lpNextData			= new uint8_t[IRF_BUFFER_SIZE];

		::memset( &agc_ctrl, 0x00, sizeof(agc_ctrl) );

		ir_size.xSize = 0;
		ir_size.ySize = 0;

		ir_palette = GREY_COLOR_MAP;

		f_invert	= FALSE;
		f_mirror	= FALSE;
		f_flip		= FALSE;

		f_agc_mode	= TRUE;
		agc_level	= 0.0f;
		agc_span	= 0.0f;

		temp_mode = 0;

		p_ir_tmp_buf = new uint8_t[IRBUFSIZE];
		p_ir_img_buf = new uint8_t[IRBUFSIZE * 4];

		p_ir_temp_buf = new float[IRBUFSIZE];

		for ( lp = 0; lp < MAX_PALETTE; lp++ ) {
			p_palette_lut[lp][0] = new BYTE[PALETTE_SIZE];
			p_palette_lut[lp][1] = new BYTE[PALETTE_SIZE];
		}

		reset_ir_data();
	}

	void	uninitial( void ) {
		int					lp;


		if ( NULL != ir_data.ir_image ) {
			delete[] ir_data.ir_image;
			ir_data.ir_image = NULL;
		}

		if ( NULL != ir_data.lpNextData ) {
			delete[] ir_data.lpNextData;
			ir_data.lpNextData = NULL;
		}

		if ( NULL != p_ir_tmp_buf ) {
			delete[] p_ir_tmp_buf;
			p_ir_tmp_buf = NULL;
		}

		if ( NULL != p_ir_img_buf ) {
			delete[] p_ir_img_buf;
			p_ir_img_buf = NULL;
		}

		if ( NULL != p_ir_temp_buf ) {
			delete[] p_ir_temp_buf;
			p_ir_temp_buf = NULL;
		}

		for ( lp = 0; lp < MAX_PALETTE; lp++ ) {
			if ( NULL != p_palette_lut[lp][0] ) {
				delete[] p_palette_lut[lp][0];
				p_palette_lut[lp][0] = NULL;
			}

			if ( NULL != p_palette_lut[lp][1] ) {
				delete[] p_palette_lut[lp][1];
				p_palette_lut[lp][1] = NULL;
			}
		}
	}
}	TCAMINFO, * TCAMINFO_PTR;


#pragma	pack( push, 1 )
#define		MAX_ROI			( 10 )


typedef	struct __roi_config__
{
	BOOL					f_enable;

	uint16_t				x;			//	x position
	uint16_t				y;			//	y position
	uint16_t				w;			//	width
	uint16_t				h;			//	height
}	ROICFG, * ROICFG_PTR;


typedef	struct __roi_information__
{
	uint16_t				max_x;		//	maximum x position
	uint16_t				max_y;		//	maximum y position

	uint16_t				min_x;		//	minimum x position
	uint16_t				min_y;		//	minimum y position

	float					max;		//	maximum temperature
	float					min;		//	maximum temperature
	float					avg;		//	average temperature
}	ROIINFO, * ROIINFO_PTR;


typedef	struct __roi_indicator_position__
{
	uint16_t				max_x;			//	maximum x position
	uint16_t				max_y;			//	maximum y position

	uint16_t				min_x;			//	minimum x position
	uint16_t				min_y;			//	minimum y position
}	ROIINDPOS, * ROIINDPOS_PTR;


typedef	struct __roi_indicator_level__
{
	uint16_t				max_lvl;		//	maximum level
	uint16_t				min_lvl;		//	maximum level
}	ROIINDLVL, * ROIINDLVL_PTR;


typedef	struct __thermal_raw_tail_information_version__
{
	uint32_t				id;						//	ID

	uint32_t				reserved00;
	uint32_t				reserved01;
}	TRAWTAIL, * TRAWTAIL_PTR;


typedef	struct __thermal_raw_tail_information_version_2__
{
	uint32_t				id;						//	ID

	uint32_t				reserved00;
	uint32_t				reserved01;

	uint16_t				reserved02;
	uint16_t				reserved03;
	uint16_t				reserved04;
	uint16_t				reserved05;

	ROIINDPOS				roi_pos[MAX_ROI];		//	ROI Indicator Position
	ROIINDPOS				frame_pos;				//	Frame Indicator Position
	ROIINDLVL				roi_lvl[MAX_ROI];		//	ROI Indicator Level
	ROIINDLVL				frame_lvl;				//	Frame Indicator Level
	uint32_t				roi_sum[MAX_ROI];		//	ROI Total Level Sum
	uint32_t				frame_sum;				//	Frame Total Level Sum
	uint32_t				roi_tot_px[MAX_ROI];	//	ROI Total Pixel
	uint32_t				frame_tot_px;			//	Frame Total Pixel

	uint16_t				ctr_avg;				//	center average level

	uint32_t				reserved06;
	uint16_t				reserved07;

	uint32_t				frame_cnt;				//	frame count
}	TRAWTAILV2, * TRAWTAILV2_PTR;


typedef	struct __thermal_raw_tail_information_version_3__
{
	uint32_t				id;						//	ID

	uint32_t				reserved00;
	uint32_t				reserved01;
	uint32_t				reserved02;
	uint32_t				reserved03;
	uint32_t				reserved04;

	uint16_t				radiometric;			//	0 : Disable, 1 : Enable
	uint16_t				radiometric_mode;
	uint16_t				temp_mode;
	uint16_t				distance_sel;
	uint16_t				user_corr;

	uint32_t				reserved05;

	uint16_t				reserved06;
	uint16_t				reserved07;
	uint16_t				reserved08;
	uint16_t				reserved09;
	
	uint16_t				reserved[9];

	ROIINDPOS				roi_pos[MAX_ROI];		//	ROI Indicator Position
	ROIINDPOS				frame_pos;				//	Frame Indicator Position
	ROIINDLVL				roi_lvl[MAX_ROI];		//	ROI Indicator Level
	ROIINDLVL				frame_lvl;				//	Frame Indicator Level
	uint32_t				roi_sum[MAX_ROI];		//	ROI Total Level Sum
	uint32_t				frame_sum;				//	Frame Total Level Sum
	uint32_t				roi_tot_px[MAX_ROI];	//	ROI Total Pixel
	uint32_t				frame_tot_px;			//	Frame Total Pixel

	uint16_t				ctr_avg;				//	center average level

	uint32_t				reserved10;
	uint16_t				reserved11;

	uint32_t				frame_cnt;				//	frame count
}	TRAWTAILV3, * TRAWTAILV3_PTR;
#pragma	pack( pop )


// CSampleCGDlg dialog
class CSampleCGDlg : public CDialogEx
{
// Construction
public			:
	CSampleCGDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SAMPLECG_DIALOG };

protected		:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


private			:
#ifdef _USED_BONJURESDK_
	DNSServiceRef				m_pServiceRef;
	DNSServiceRef				m_pResolveRef;
#endif

	TCAMINFO					m_stTCamInfo;			//	Thermal Camera Information

	ROICFG						m_stRoiCfg[MAX_ROI];	//	ROI Config

	ROIINFO						m_stRoiCam[MAX_ROI];	//	ROI Camera Information
	ROIINFO						m_stRoiSDK[MAX_ROI];	//	ROI SDK Information


	DWORD						m_dwStrmBaseTick;		//	Stream Base Tick
	DWORD						m_dwRecvStrmCnt;		//	Receive Stream Count

	BOOL						m_fFocusNear;			//	Flag Of Focus Near
	BOOL						m_fFocusFar;			//	Flag Of Focus Far
	BOOL						m_fZoomTele;			//	Flag Of Zoom Tele
	BOOL						m_fZoomWide;			//	Flag Of Zoom Wide

	CCameraSetupDlg *			m_pCamSetupDlg;			//	Camera Setup Dialog ( Modaless Dialog )


	char						m_ip_addr[32];
	char						m_port_num[16];

public			:
#ifdef _USED_BONJURESDK_
	void	AddResolveCamera( char * _model, char * _ip, char * _mac );

	void	RefreshCameraList( void );
	
	void	CreateServiceDNS( void );
	void	CloseServiceDNS( void );

	void	BrowseResolve( uint32_t _if_idx, const char * _reply_name, const char * _reply_type, const char * _reply_domain );

	static	void	DNSSD_API	ResolveReply(
										DNSServiceRef			_p_sd_ref, 
										DNSServiceFlags			_flags, 
										uint32_t				_if_idx,
										DNSServiceErrorType		_err_code,
										const char *			_full_name, 
										const char *			_host_target,
										uint16_t				_port,
										uint16_t				_txt_len,
										const unsigned char *	_txt_record, 
										void *					_context
										);

	static	void	DNSSD_API	BrowseReply(
										DNSServiceRef			_p_sd_ref, 
										const DNSServiceFlags	_flags, 
										uint32_t				_if_idx,
										DNSServiceErrorType		_err_code,
										const char *			_reply_name, 
										const char *			_reply_type, 
										const char *			_reply_domain, 
										void *					_context
										);
#endif

	BOOL	IsConnectCamera( void );

	BOOL	ConnectCamera( TCAMINFO_PTR _p_info );
	BOOL	DisconnectCamera( TCAMINFO_PTR _p_info );

	BOOL	CreateRecvThread( TCAMINFO_PTR _p_info );
	BOOL	DestroyRecvThread( TCAMINFO_PTR _p_info );

	BOOL	WaitRecvData( TCAMINFO_PTR _p_info );

	void	DoRecvACK( TCAMINFO_PTR _p_info );
	void	DoRecvNAK( TCAMINFO_PTR _p_info );
	void	DoRecvCamData( TCAMINFO_PTR _p_info );
	void	DoRecvStreamData( TCAMINFO_PTR _p_info );

	static unsigned int __stdcall RECEIVE_PROC( void* _param );

	BOOL	SendCameraCommand(
							IRF_MESSAGE_TYPE_T	_msg,
							uint16_t			_cmd	= 0,
							uint16_t			_rcode	= 0
							);

	BOOL	SendCameraCommand(
							TCAMINFO_PTR		_p_info,
							IRF_MESSAGE_TYPE_T	_msg,
							uint16_t			_cmd	= 0,
							uint16_t			_rcode	= 0
							);

	BOOL	SendCameraConfig(
							IRF_MESSAGE_TYPE_T	_msg,
							uint16_t			_cmd	= 0,
							uint16_t			_rcode	= 0,
							uint32_t			_rcode2	= 0,
							uint32_t			_rcode3	= 0,
							uint32_t			_rcode4	= 0
							);

	BOOL	SendCameraConfig(
							TCAMINFO_PTR		_p_info,
							IRF_MESSAGE_TYPE_T	_msg,
							uint16_t			_cmd	= 0,
							uint16_t			_rcode	= 0,
							uint32_t			_rcode2	= 0,
							uint32_t			_rcode3	= 0,
							uint32_t			_rcode4	= 0
							);

	void	EventMouseMove( POINT _pt );


protected		:
	void	DoInitialize( void );
	void	DoUninitialize( void );

	void	DoInitControl( BOOL _f_connect );

	BOOL	DoControlCommand( HWND _h_wnd, WORD _ctrl_id, WORD _notify_code, LRESULT* _p_result );
	BOOL	DoControlNotify( NMHDR* _p_nmhdr, LRESULT* _p_result );


// Implementation
protected		:
	HICON				m_hIcon;

	CIPAddressCtrl		m_ctlIPAddress;
	CListCtrl			m_ctlListCtrl;
	CScreenWnd			m_ctlScreen;
	
	CButton				m_autoReconnect;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
public:
	virtual BOOL PreTranslateMessage( MSG* pMsg );
};
