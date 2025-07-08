#pragma once


typedef	void (*PFN_MOUSEMOVE)( POINT );

// CScreenWnd

class CScreenWnd : public CStatic
{
	DECLARE_DYNAMIC(CScreenWnd)

public:
	CScreenWnd();
	virtual ~CScreenWnd();

	typedef	enum	__display_message_position__
	{
		dmp_left_top = 0,
		dmp_left_bottom,
		dmp_right_top,
		dmp_right_bottom,
		dmp_center
	} DISPMSGPOS;


	/**
		@brief		Object ¿Âµµ Ç¥½Ã Á¾·ù
	*/
	typedef	enum	__object_temperature_type__
	{
		ott_celsius = 0, ///< ¼·¾¾
		ott_fahrenheit, ///< È­¾¾
		ott_kelvin ///< ÄÌºó
	} OBJTEMPTYPE;

protected:
	BITMAPINFO			m_biImageInfo;
	LPBYTE				m_pImageBuf;

	SIZE				m_sImageSize;

	int					m_nBitsPixel;
	int					m_nImageBytes;

	float				m_fImageScaleX;
	float				m_fImageScaleY;

	float				m_fScreenScaleX;
	float				m_fScreenScaleY;

	POINT				m_ptImageOffset;

	BOOL				m_fImageFlip;
	BOOL				m_fImageMirror;

	PFN_MOUSEMOVE		m_pfnMouseMove;

	RECT				m_rcActiveScreen;
	POINT				m_ptScreenPoint;
	SIZE				m_szScreenSize;

	BOOL				m_fDisplayMouseTemp;
	float				m_fMouseTemperature;

	BOOL				m_fDisplayHotPt;
	BOOL				m_fDisplayColdPt;

	BOOL				m_fDisplayHotTemp;
	BOOL				m_fDisplayColdTemp;

	POINT				m_ptHotPoint;
	POINT				m_ptColdPoint;

	float				m_fHotTemperature;
	float				m_fColdTemperature;

	unsigned int		m_uDispHotColdSize;

	RECT				m_rcClipRegion;

	BOOL				m_fDispMsg;
	DISPMSGPOS			m_eDispMsgPos;
	TCHAR				m_szDispMsg[MAX_PATH];

	COLORREF			m_crHotTextColor;
	COLORREF			m_crHotShadowColor;

	COLORREF			m_crColdTextColor;
	COLORREF			m_crColdShadowColor;

	COLORREF			m_crMsgTextColor;
	COLORREF			m_crMsgShadowColor;

	OBJTEMPTYPE			m_eDispTempType;

	wchar_t				m_szFontFace[MAX_PATH];
	int					m_nFontSize;

	
public:
	void	Initialize( int _width, int _height, int _bits_pixel );
	void	Uninitialize( void );

	void	SetHotTextColor( COLORREF _text, COLORREF _outer );
	void	GetHotTextColor( COLORREF& _text, COLORREF& _outer );

	void	SetColdTextColor( COLORREF _text, COLORREF _outer );
	void	GetColdTextColor( COLORREF& _text, COLORREF& _outer );

	void	SetMsgTextColor( COLORREF _text, COLORREF _outer );
	void	GetMsgTextColor( COLORREF& _text, COLORREF& _outer );

	void	SetDisplayMessage( BOOL _flag, DISPMSGPOS _pos );
	void	GetDisplayMessage( BOOL& _flag, DISPMSGPOS& _pos );

	void	SetImageBuf( LPBYTE _p_img_buf );
	void	SetImageBuf( LPBYTE _p_img_buf, int	_len, LPCTSTR _msg );
		
	void	SetImageScale( float _x, float _y );
	void	GetImageScale( float& _x, float& _y );

	void	SetImageOffset( int _x, int _y );
	void	GetImageOffset( int& _x, int& _y );

	void	SetScreenScale( float _x, float _y );
	void	GetScreenScale( float& _x, float& _y );
		
	void	SetImageFlip( BOOL _flag );
	BOOL	GetImageFlip( void );

	void	SetImageMirror( BOOL _flag );
	BOOL	GetImageMirror( void );

	void	RegisterMouseMove( PFN_MOUSEMOVE _pfnc );
		
	void	SetActiveScreenRect( LPRECT _p_rect );
	void	GetActiveScreenRect( LPRECT _p_rect );

	void	SetImageOffsetX( int _x );
	void	SetImageOffsetY( int _y );

	void	SetDisplayMouseTemp( BOOL _flag, float _temp );
	BOOL	GetDisplayMouseTemp( void );

	void	SetDisplayHotCold( BOOL _f_hot, BOOL _f_cold );
	void	GetDisplayHotCold( BOOL& _f_hot, BOOL& _f_cold );

	void	SetDisplayHotColdTemp( BOOL _f_hot, BOOL _f_cold );
	void	GetDisplayHotColdTemp( BOOL& _f_hot, BOOL& _f_cold );

	void	SetHotPoint( POINT _pt, float _temp );
	void	GetHotPoint( POINT& _pt, float& _temp );

	void	SetColdPoint( POINT _pt, float _temp );
	void	GetColdPoint( POINT& _pt, float& _temp );

	void	SetDispHotColdSize( unsigned int _size );
	unsigned int	GetDispHotColdSize( void );

	HBITMAP	GetImage( BOOL _f_roi );
	BOOL	GetImage( BOOL _f_roi, LPBYTE _p_img_buf );


	void	IncreseOffsetX( int _offset = 1 );
	void	DecreseOffsetX( int _offset = 1 );

	void	IncreseOffsetY( int _offset = 1 );
	void	DecreseOffsetY( int _offset = 1 );

	void	SetClipRegion( RECT _rect );
	void	GetClipRegion( RECT& _rect );

	void	SetDrawFontInfo( wchar_t* _name, int _size );
	void	GetDrawFontInfo( wchar_t* _name, int _len, int& _size );

	SIZE	MeasureString( HDC _h_dc, LPCTSTR _str, wchar_t * _font_name, int _font_size );
	void	DrawNormalText(
						HDC					_h_dc,
						LPCTSTR				_text,
						long				_x,
						long				_y,
						long				_cx,
						long				_cy,
						COLORREF			_text_clr,
						COLORREF			_line_clr,
						wchar_t *			_font_name,
						int					_font_size
						);

protected:
	

protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
};


