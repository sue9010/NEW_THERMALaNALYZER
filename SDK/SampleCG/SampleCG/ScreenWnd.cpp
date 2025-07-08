// ScreenWnd.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "SampleCG.h"
#include "ScreenWnd.h"


// CScreenWnd

IMPLEMENT_DYNAMIC(CScreenWnd, CStatic)

CScreenWnd::CScreenWnd()
{
	m_pImageBuf = nullptr;

	m_sImageSize.cx = 0;
	m_sImageSize.cy = 0;

	m_nBitsPixel = 0;

	m_nImageBytes = 0;

	m_fImageScaleX = (float)1.0;
	m_fImageScaleY = (float)1.0;

	m_fScreenScaleX = (float)1.0;
	m_fScreenScaleY = (float)1.0;

	m_fImageFlip	= FALSE;
	m_fImageMirror	= FALSE;

	::SetRectEmpty( &m_rcActiveScreen );

	m_fDisplayMouseTemp = FALSE;
	m_fMouseTemperature = 0.0f;

	m_fDisplayHotPt		= FALSE;
	m_fDisplayColdPt	= FALSE;

	m_fDisplayHotTemp	= FALSE;
	m_fDisplayColdTemp	= FALSE;

	m_fHotTemperature	= 0.0f;
	m_fColdTemperature	= 0.0f;

	m_ptHotPoint.x = -1;
	m_ptHotPoint.y = -1;

	m_ptColdPoint.x = -1;
	m_ptColdPoint.y = -1;

	m_uDispHotColdSize = 7;

	m_ptScreenPoint.x = 0;
	m_ptScreenPoint.y = 0;

	m_szScreenSize.cx = 0;
	m_szScreenSize.cy = 0;

	m_ptImageOffset.x = 0;
	m_ptImageOffset.y = 0;

	::SetRectEmpty( &m_rcClipRegion );

	m_fDispMsg		= FALSE;
	m_eDispMsgPos	= dmp_center;
	::memset( m_szDispMsg, 0x00, sizeof(m_szDispMsg) );

	m_crHotTextColor	= RGB(255, 255, 255);
	m_crHotShadowColor	= RGB(0, 0, 0);

	m_crColdTextColor	= RGB(255, 255, 255);
	m_crColdShadowColor	= RGB(0, 0, 0);

	m_crMsgTextColor	= RGB(255, 255, 255);
	m_crMsgShadowColor	= RGB(0, 0, 0);

	::memset( m_szFontFace, 0x00, sizeof(m_szFontFace) );
	::wcscpy_s( m_szFontFace, sizeof(m_szFontFace) / sizeof(wchar_t),  L"Arial" );
	m_nFontSize = 11;

	m_eDispTempType = ott_celsius;

	m_pfnMouseMove = NULL;
}

CScreenWnd::~CScreenWnd()
{
	Uninitialize();
}


BEGIN_MESSAGE_MAP(CScreenWnd, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_GETDLGCODE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()



// CScreenWnd 메시지 처리기입니다.
void	CScreenWnd::Initialize( int _width, int _height, int _bits_pixel )
{
	m_sImageSize.cx = _width;
	m_sImageSize.cy = _height;

	m_nBitsPixel = _bits_pixel;

	m_nImageBytes	= _width * _height * (_bits_pixel / 8);

	if ( m_pImageBuf != NULL ) {
		delete [] m_pImageBuf;
	}
	m_pImageBuf		= new BYTE[m_nImageBytes];

	::memset( m_pImageBuf, 0xFF, m_nImageBytes );
	::memset( &m_biImageInfo, 0x00, sizeof(m_biImageInfo) );

	m_biImageInfo.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    m_biImageInfo.bmiHeader.biPlanes		= 1;
    m_biImageInfo.bmiHeader.biBitCount		= m_nBitsPixel;
    m_biImageInfo.bmiHeader.biCompression	= BI_RGB;
    m_biImageInfo.bmiHeader.biSizeImage		= m_nImageBytes;
    m_biImageInfo.bmiHeader.biXPelsPerMeter = 0;
    m_biImageInfo.bmiHeader.biYPelsPerMeter = 0;
    m_biImageInfo.bmiHeader.biClrUsed		= 0;
    m_biImageInfo.bmiHeader.biClrImportant	= 0;
    m_biImageInfo.bmiHeader.biWidth			= m_sImageSize.cx;
    m_biImageInfo.bmiHeader.biHeight		= m_sImageSize.cy * -1;

	{
		RECT		rect;
		int			cx, cy;


		::GetClientRect( this->GetSafeHwnd(), &rect );

		if (m_szScreenSize.cx >= (m_rcActiveScreen.right - m_rcActiveScreen.left)) {
			rect.left	= m_rcActiveScreen.left - m_ptScreenPoint.x;
			rect.right	= rect.left + (m_rcActiveScreen.right - m_rcActiveScreen.left);
		}

		if (m_szScreenSize.cy >= (m_rcActiveScreen.bottom - m_rcActiveScreen.top)) {
			rect.top	= m_rcActiveScreen.top - m_ptScreenPoint.y;
			rect.bottom	= rect.top + (m_rcActiveScreen.bottom - m_rcActiveScreen.top);
		}

		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;

		TRACE(_T("Call CScreenWnd::Initialize %d %d %d %d\r\n"), m_szScreenSize.cx, m_szScreenSize.cy, m_sImageSize.cx, m_sImageSize.cy);

		if ( 0 == m_sImageSize.cx || 0 == m_sImageSize.cy ) {
			m_fScreenScaleX = (float)1.0;
			m_fScreenScaleY = (float)1.0;
		}
		else {
			m_fScreenScaleX = ((float)m_szScreenSize.cx / (float)m_sImageSize.cx);
			m_fScreenScaleY = ((float)m_szScreenSize.cy / (float)m_sImageSize.cy);
		}
	}
}


void	CScreenWnd::Uninitialize( void )
{
	if ( nullptr != m_pImageBuf ) {
		delete[] m_pImageBuf;
		m_pImageBuf = nullptr;
	}

	m_sImageSize.cx = 0;
	m_sImageSize.cy = 0;

	m_nBitsPixel = 0;

	m_nImageBytes = 0;
}


void	CScreenWnd::SetHotTextColor( COLORREF _text, COLORREF _outer )
{
	m_crHotTextColor	= _text;
	m_crHotShadowColor	= _outer;
}


void	CScreenWnd::GetHotTextColor( COLORREF& _text, COLORREF& _outer )
{
	_text	= m_crHotTextColor;
	_outer	= m_crHotShadowColor;
}


void	CScreenWnd::SetColdTextColor( COLORREF _text, COLORREF _outer )
{
	m_crColdTextColor	= _text;
	m_crColdShadowColor	= _outer;
}


void	CScreenWnd::GetColdTextColor( COLORREF& _text, COLORREF& _outer )
{
	_text	= m_crColdTextColor;
	_outer	= m_crColdShadowColor;
}


void	CScreenWnd::SetMsgTextColor( COLORREF _text, COLORREF _outer )
{
	m_crMsgTextColor	= _text;
	m_crMsgShadowColor	= _outer;
}


void	CScreenWnd::GetMsgTextColor( COLORREF& _text, COLORREF& _outer )
{
	_text	= m_crMsgTextColor;
	_outer	= m_crMsgShadowColor;
}


void	CScreenWnd::SetDisplayMessage( BOOL _flag, DISPMSGPOS _pos )
{
	m_fDispMsg		= _flag;
	m_eDispMsgPos	= _pos;
}


void	CScreenWnd::GetDisplayMessage( BOOL& _flag, DISPMSGPOS& _pos )
{
	_flag	= m_fDispMsg;
	_pos	= m_eDispMsgPos;
}


void	CScreenWnd::SetImageBuf( LPBYTE _p_img_buf )
{
	::memcpy( m_pImageBuf, _p_img_buf, m_nImageBytes );

	this->InvalidateRect( NULL, 0 );
}


void	CScreenWnd::SetImageBuf( LPBYTE _p_img_buf, int	_len, LPCTSTR _msg )
{
	::memcpy( m_pImageBuf, _p_img_buf, m_nImageBytes );

	{
		::memset( m_szDispMsg, 0x00, sizeof(m_szDispMsg) );

		if ( 0 >= _len ) {
		}
		else if ( (MAX_PATH - 1) <= _len ) {
			::lstrcpyn( m_szDispMsg, _msg, (MAX_PATH - 1) );
		}
		else {
			::lstrcpy( m_szDispMsg, _msg );
		}
	}

	this->InvalidateRect( NULL, 0 );
}


void	CScreenWnd::SetImageScale( float _x, float _y )
{
	m_fImageScaleX = _x;
	m_fImageScaleY = _y;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::GetImageScale( float& _x, float& _y )
{
	_x = m_fImageScaleX;
	_y = m_fImageScaleY;
}


void	CScreenWnd::SetImageOffset( int _x, int _y )
{
	m_ptImageOffset.x = _x;
	m_ptImageOffset.y = _y;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::GetImageOffset( int& _x, int& _y )
{
	_x = m_ptImageOffset.x;
	_y = m_ptImageOffset.y;
}


void	CScreenWnd::SetScreenScale( float _x, float _y )
{
	m_fScreenScaleX = _x;
	m_fScreenScaleY = _y;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::GetScreenScale( float& _x, float& _y )
{
	_x = m_fScreenScaleX;
	_y = m_fScreenScaleY;
}


void	CScreenWnd::SetImageFlip( BOOL _flag )
{
	if ( _flag == m_fImageFlip ) { return; }

	m_fImageFlip = _flag;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


BOOL	CScreenWnd::GetImageFlip( void )
{
	return ( m_fImageFlip );
}


void	CScreenWnd::SetImageMirror( BOOL _flag )
{
	if ( _flag == m_fImageMirror ) { return; }

	m_fImageMirror = _flag;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


BOOL	CScreenWnd::GetImageMirror( void )
{
	return ( m_fImageMirror );
}


void	CScreenWnd::RegisterMouseMove( PFN_MOUSEMOVE _pfnc )
{
	m_pfnMouseMove = _pfnc;
}


void	CScreenWnd::SetActiveScreenRect( LPRECT _p_rect )
{
	m_rcActiveScreen = *_p_rect;
}


void	CScreenWnd::GetActiveScreenRect( LPRECT _p_rect )
{
	*_p_rect = m_rcActiveScreen;
}


void	CScreenWnd::SetImageOffsetX( int _x )
{
	m_ptImageOffset.x = _x;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::SetImageOffsetY( int _y )
{
	m_ptImageOffset.y = _y;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::SetDisplayMouseTemp( BOOL _flag, float _temp )
{
	m_fDisplayMouseTemp = _flag;
	m_fMouseTemperature = _temp;

	if ( TRUE == m_fDisplayMouseTemp ) {
		TCHAR			temp_type[3] = { _T('C'), _T('F'), _T('K') };

		CString			msg;
		CSize			size;
		CPoint			text_pt;

		HDC				h_dc = nullptr;
		POINT			pt;


		::GetCursorPos( &pt );
		::ScreenToClient( this->GetSafeHwnd(), &pt );

		if ( 0 <= pt.x && pt.x < m_szScreenSize.cx && 0 <= pt.y && pt.y < m_szScreenSize.cy ) {
			h_dc = ::GetDC( this->GetSafeHwnd() );

			msg.Format( _T("[%.1f°%c]"), m_fMouseTemperature, temp_type[m_eDispTempType] );

			size = MeasureString( h_dc, msg, m_szFontFace, m_nFontSize );

			text_pt.x = pt.x - (size.cx / 2);
			text_pt.y = pt.y + 10;

			// 출력 영역 X 조정
			if ( (text_pt.x + size.cx) > (m_szScreenSize.cx - 2) ) {
				text_pt.x -= ((text_pt.x + size.cx) - (m_szScreenSize.cx - 2));
			}

			if ( text_pt.x < 1 ) {
				text_pt.x = 1;
			}

			// 출력 영역 Y 조정
			if ( (text_pt.y + size.cy) > (m_szScreenSize.cy - 4) ) {
				text_pt.y = pt.y - 10 - size.cy;
			}

			DrawNormalText( h_dc, msg, text_pt.x, text_pt.y, size.cx, size.cy, RGB(255, 255, 255), RGB(0, 0, 0), m_szFontFace, m_nFontSize );

			::ReleaseDC( this->GetSafeHwnd(), h_dc );
			h_dc = nullptr;
		}
	}
}


BOOL	CScreenWnd::GetDisplayMouseTemp( void )
{
	return ( m_fDisplayMouseTemp );
}


void	CScreenWnd::SetDisplayHotCold( BOOL _f_hot, BOOL _f_cold )
{
	m_fDisplayHotPt		= _f_hot;
	m_fDisplayColdPt	= _f_cold;
}


void	CScreenWnd::GetDisplayHotCold( BOOL& _f_hot, BOOL& _f_cold )
{
	_f_hot	= m_fDisplayHotPt;
	_f_cold	= m_fDisplayColdPt;
}


void	CScreenWnd::SetDisplayHotColdTemp( BOOL _f_hot, BOOL _f_cold )
{
	m_fDisplayHotTemp	= _f_hot;
	m_fDisplayColdTemp	= _f_cold;
}


void	CScreenWnd::GetDisplayHotColdTemp( BOOL& _f_hot, BOOL& _f_cold )
{
	_f_hot	= m_fDisplayHotTemp;
	_f_cold	= m_fDisplayColdTemp;
}


void	CScreenWnd::SetHotPoint( POINT _pt, float _temp )
{
	m_ptHotPoint		= _pt;
	m_fHotTemperature	= _temp;
}


void	CScreenWnd::GetHotPoint( POINT& _pt, float& _temp )
{
	_pt		= m_ptHotPoint;
	_temp	= m_fHotTemperature;
}


void	CScreenWnd::SetColdPoint( POINT _pt, float _temp )
{
	m_ptColdPoint		= _pt;
	m_fColdTemperature	= _temp;
}


void	CScreenWnd::GetColdPoint( POINT& _pt, float& _temp )
{
	_pt		= m_ptColdPoint;
	_temp	= m_fColdTemperature;
}


void	CScreenWnd::SetDispHotColdSize( unsigned int _size )
{
	m_uDispHotColdSize = _size;
}


unsigned int	CScreenWnd::GetDispHotColdSize( void )
{
	return ( m_uDispHotColdSize );
}


HBITMAP	CScreenWnd::GetImage( BOOL _f_roi )
{
	HDC				h_dc = nullptr;
	
	HDC				h_mem_dc = nullptr;

	HBITMAP			h_bitmap		= nullptr;
	HBITMAP			h_old_bitmap	= nullptr;


	//	이미지가 없으면...
	if ( 0 == m_nImageBytes ) {
		return ( nullptr );
	}

	h_dc = ::GetDC( this->GetSafeHwnd() );
	
	h_mem_dc = ::CreateCompatibleDC( h_dc );
	h_bitmap = ::CreateCompatibleBitmap( h_dc, m_sImageSize.cx, m_sImageSize.cy );

	h_old_bitmap = (HBITMAP)::SelectObject( h_mem_dc, h_bitmap );
		
	{
		Graphics		gra_src( h_mem_dc );
		Bitmap			bitmap( &m_biImageInfo, m_pImageBuf );

		ImageAttributes image_att;


		if ( TRUE == m_fImageFlip && TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipXY );
		}
		else if ( TRUE == m_fImageFlip ) {
			bitmap.RotateFlip( RotateNoneFlipY );
		}
		else if ( TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipX );
		}

		gra_src.SetSmoothingMode( SmoothingModeHighQuality );
		gra_src.DrawImage(
						&bitmap,
						Rect( 0, 0, m_sImageSize.cx, m_sImageSize.cy ),
						0,
						0,
						bitmap.GetWidth(),
						bitmap.GetHeight(),
						UnitPixel,
						&image_att
						);

		if ( TRUE == _f_roi ) {
			if ( TRUE == m_fDisplayHotPt ) {
				SolidBrush		brush( Color( 255, 0, 0 ) );
				Point			pt[3];


				gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

				pt[0].X = m_ptHotPoint.x;
				pt[0].Y = m_ptHotPoint.y;

				pt[1].X = m_ptHotPoint.x - m_uDispHotColdSize;
				pt[1].Y = m_ptHotPoint.y - m_uDispHotColdSize;

				pt[2].X = m_ptHotPoint.x + m_uDispHotColdSize;
				pt[2].Y = m_ptHotPoint.y - m_uDispHotColdSize;

				gra_src.FillPolygon(
									&brush,
									&pt[0],
									3,
									FillMode::FillModeWinding
									);
			}

			if ( TRUE == m_fDisplayColdPt ) {
				SolidBrush		brush( Color( 0, 0, 255 ) );
				Point			pt[3];


				gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

				pt[0].X = m_ptColdPoint.x;
				pt[0].Y = m_ptColdPoint.y;

				pt[1].X = m_ptColdPoint.x - m_uDispHotColdSize;
				pt[1].Y = m_ptColdPoint.y - m_uDispHotColdSize;

				pt[2].X = m_ptColdPoint.x + m_uDispHotColdSize;
				pt[2].Y = m_ptColdPoint.y - m_uDispHotColdSize;

				gra_src.FillPolygon(
									&brush,
									&pt[0],
									3,
									FillMode::FillModeWinding
									);
			}
		}
	}

	::SelectObject( h_mem_dc, h_old_bitmap );
	::DeleteDC( h_mem_dc );
	h_mem_dc = nullptr;

	::ReleaseDC( this->GetSafeHwnd(), h_dc );
	h_dc = nullptr;
		
	return ( h_bitmap );
}


BOOL	CScreenWnd::GetImage( BOOL _f_roi, LPBYTE _p_img_buf )
{
	HDC				h_dc = nullptr;
	
	HDC				h_mem_dc = nullptr;

	HBITMAP			h_bitmap		= nullptr;
	HBITMAP			h_old_bitmap	= nullptr;


	//	이미지가 없으면...
	if ( 0 == m_nImageBytes || nullptr == _p_img_buf ) {
		return ( FALSE );
	}

	h_dc = ::GetDC( this->GetSafeHwnd() );
	
	h_mem_dc = ::CreateCompatibleDC( h_dc );
	h_bitmap = ::CreateCompatibleBitmap( h_dc, m_sImageSize.cx, m_sImageSize.cy );

	h_old_bitmap = (HBITMAP)::SelectObject( h_mem_dc, h_bitmap );
		
	{
		Graphics		gra_src( h_mem_dc );
		Bitmap			bitmap( &m_biImageInfo, m_pImageBuf );

		ImageAttributes image_att;


		if ( TRUE == m_fImageFlip && TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipXY );
		}
		else if ( TRUE == m_fImageFlip ) {
			bitmap.RotateFlip( RotateNoneFlipY );
		}
		else if ( TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipX );
		}

		gra_src.SetSmoothingMode( SmoothingModeHighQuality );
		gra_src.DrawImage(
						&bitmap,
						Rect( 0, 0, m_sImageSize.cx, m_sImageSize.cy ),
						0,
						0,
						bitmap.GetWidth(),
						bitmap.GetHeight(),
						UnitPixel,
						&image_att
						);

		if ( TRUE == _f_roi ) {
			if ( TRUE == m_fDisplayHotPt ) {
				SolidBrush		brush( Color( 255, 0, 0 ) );
				Point			pt[3];


				gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

				pt[0].X = m_ptHotPoint.x;
				pt[0].Y = m_ptHotPoint.y;

				pt[1].X = m_ptHotPoint.x - m_uDispHotColdSize;
				pt[1].Y = m_ptHotPoint.y - m_uDispHotColdSize;

				pt[2].X = m_ptHotPoint.x + m_uDispHotColdSize;
				pt[2].Y = m_ptHotPoint.y - m_uDispHotColdSize;

				gra_src.FillPolygon(
									&brush,
									&pt[0],
									3,
									FillMode::FillModeWinding
									);
			}

			if ( TRUE == m_fDisplayColdPt ) {
				SolidBrush		brush( Color( 0, 0, 255 ) );
				Point			pt[3];


				gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

				pt[0].X = m_ptColdPoint.x;
				pt[0].Y = m_ptColdPoint.y;

				pt[1].X = m_ptColdPoint.x - m_uDispHotColdSize;
				pt[1].Y = m_ptColdPoint.y - m_uDispHotColdSize;

				pt[2].X = m_ptColdPoint.x + m_uDispHotColdSize;
				pt[2].Y = m_ptColdPoint.y - m_uDispHotColdSize;

				gra_src.FillPolygon(
									&brush,
									&pt[0],
									3,
									FillMode::FillModeWinding
									);
			}
		}
	}

	{
		BITMAPINFO			bi = { 0 };


        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        ::GetDIBits( h_mem_dc, h_bitmap, 0, 0, 0, &bi, DIB_RGB_COLORS );
		::GetDIBits( h_mem_dc, h_bitmap, 0, bi.bmiHeader.biHeight, _p_img_buf, &bi, DIB_RGB_COLORS);  
	}

	::SelectObject( h_mem_dc, h_old_bitmap );

	::DeleteObject( h_bitmap );
	h_bitmap = nullptr;

	::DeleteDC( h_mem_dc );
	h_mem_dc = nullptr;

	::ReleaseDC( this->GetSafeHwnd(), h_dc );
	h_dc = nullptr;
		
	return ( TRUE );
}


void	CScreenWnd::IncreseOffsetX( int _offset )
{
	m_ptImageOffset.x += _offset;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::DecreseOffsetX( int _offset )
{
	m_ptImageOffset.x -= _offset;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::IncreseOffsetY( int _offset )
{
	m_ptImageOffset.y += _offset;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::DecreseOffsetY( int _offset )
{
	m_ptImageOffset.y -= _offset;

	::InvalidateRect( this->GetSafeHwnd(), NULL, FALSE );
}


void	CScreenWnd::SetClipRegion( RECT _rect )
{
	m_rcClipRegion = _rect;
}


void	CScreenWnd::GetClipRegion( RECT& _rect )
{
	_rect = m_rcClipRegion;
}


void	CScreenWnd::SetDrawFontInfo( wchar_t* _name, int _size )
{
	::memset( m_szFontFace, 0x00, sizeof(m_szFontFace) );
	::wcscpy_s( m_szFontFace, _name );
	m_nFontSize = _size;
}


void	CScreenWnd::GetDrawFontInfo( wchar_t* _name, int _len, int& _size )
{
	::wcscpy_s( _name, _len,  m_szFontFace );
	_size = m_nFontSize;
}


LRESULT CScreenWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CStatic::WindowProc(message, wParam, lParam);
}


void CScreenWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CStatic::OnLButtonDown(nFlags, point);
}


void CScreenWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CStatic::OnLButtonUp(nFlags, point);
}


void CScreenWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CStatic::OnRButtonDown(nFlags, point);
}


void CScreenWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CStatic::OnRButtonUp(nFlags, point);
}


void CScreenWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if ( 0 == ((MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_SHIFT | MK_CONTROL) & nFlags) ) {
		if ( nullptr != m_pfnMouseMove ) {
			m_pfnMouseMove( point );
		}
	}

	CStatic::OnMouseMove(nFlags, point);
}


void CScreenWnd::OnPaint()
{
	CPaintDC		dc(this); // device context for painting
	
	HDC				h_src_dc		= nullptr;
	HDC				h_dst_dc		= nullptr;

	HRGN			h_rgn			= nullptr;

	HBITMAP			h_bm_src		= nullptr;
	HBITMAP			h_old_bm_src	= nullptr;

	HBITMAP			h_bm_dst		= nullptr;
	HBITMAP			h_old_bm_dst	= nullptr;

	HBRUSH			h_brush			= nullptr;

	RECT			rect;

	int				bits_pixel;

	int				cx, cy;


	::GetClientRect( this->GetSafeHwnd(), &rect );
	bits_pixel = ::GetDeviceCaps( dc.GetSafeHdc(), BITSPIXEL );

	//	이미지가 없으면...
	if ( 0 == m_nImageBytes ) {
		h_brush = ::CreateSolidBrush( RGB( 0, 164, 242 ) );
		::FillRect( dc.GetSafeHdc(), &rect, h_brush );
		::DeleteObject( h_brush );

		return;
	}
	
	//h_rgn = ::CreateRectRgn( 0, 0, (rect.right - rect.left), (rect.bottom - rect.top) );
	{	// 다시 그려질 영역 설정
		RECT		clip;


		clip = m_rcClipRegion;
		if ( rect.left <= 0 )				{ clip.left += 2; }
		if ( rect.right >= clip.right )		{ clip.right -= 2; }
		if ( 0 >= rect.top )				{ clip.top += 2; }
		if ( rect.bottom >= clip.bottom )	{ clip.bottom -= 2; }

		h_rgn = ::CreateRectRgn(clip.left, clip.top, clip.right, clip.bottom);
		::SelectClipRgn(dc.GetSafeHdc(), h_rgn);
	}

	h_src_dc = ::CreateCompatibleDC( dc.GetSafeHdc() );
	h_bm_src = ::CreateCompatibleBitmap( dc.GetSafeHdc(), m_sImageSize.cx, m_sImageSize.cy );

	h_old_bm_src = (HBITMAP)::SelectObject( h_src_dc, h_bm_src );

	h_dst_dc = ::CreateCompatibleDC( dc.GetSafeHdc() );
	h_bm_dst = ::CreateCompatibleBitmap( dc.GetSafeHdc(), (rect.right - rect.left), (rect.bottom - rect.top) );

	h_old_bm_dst = (HBITMAP)::SelectObject( h_dst_dc, h_bm_dst );

	{
		h_brush = ::CreateSolidBrush( RGB( 0, 69, 102 ) );
		::FillRect( h_src_dc, &rect, h_brush );
		::DeleteObject( h_brush );
	}

	{
		TCHAR			temp_type[3] = { _T('C'), _T('F'), _T('K') };

		Graphics		gra_src( h_src_dc );
		Bitmap			bitmap( &m_biImageInfo, m_pImageBuf );

		ImageAttributes image_att;


		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;

		if ( TRUE == m_fImageFlip && TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipXY );
		}
		else if ( TRUE == m_fImageFlip ) {
			bitmap.RotateFlip( RotateNoneFlipY );
		}
		else if ( TRUE == m_fImageMirror ) {
			bitmap.RotateFlip( RotateNoneFlipX );
		}

		gra_src.SetSmoothingMode( SmoothingModeHighQuality );
		gra_src.DrawImage(
						&bitmap,
						Rect( rect.left, rect.top, m_sImageSize.cx, m_sImageSize.cy ),
						0,
						0,
						bitmap.GetWidth(),
						bitmap.GetHeight(),
						UnitPixel,
						&image_att
						);
				
		if ( TRUE == m_fDisplayHotPt ) {
			Pen				pen( Color( 255, 255, 255 ), 2 );
			SolidBrush		brush( Color( 255, 0, 0 ) );
			GraphicsPath	path;
			Point			pt[3];


			gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

			pt[0].X = m_ptHotPoint.x;
			pt[0].Y = m_ptHotPoint.y;

			pt[1].X = m_ptHotPoint.x + m_uDispHotColdSize;
			pt[1].Y = m_ptHotPoint.y + m_uDispHotColdSize;

			pt[2].X = m_ptHotPoint.x - m_uDispHotColdSize;
			pt[2].Y = m_ptHotPoint.y + m_uDispHotColdSize;
						
			path.AddPolygon( &pt[0], 3 );

			gra_src.DrawPath( &pen, &path );    
			gra_src.FillPath( &brush, &path );
			
			if ( TRUE == m_fDisplayHotTemp ) {
				CString			msg;
				CSize			size;
				CPoint			text_pt;


				msg.Format( _T("Hot [%.1f°%c]"), m_fHotTemperature, temp_type[m_eDispTempType] );

				size = MeasureString( h_src_dc, msg, m_szFontFace, m_nFontSize );

				text_pt.x = m_ptHotPoint.x + 1;
				text_pt.y = (m_ptHotPoint.y - 1) - size.cy;

				// 출력 영역 X 조정
				if ( (text_pt.x + size.cx) > (m_szScreenSize.cx - 1) ) {
					text_pt.x -= ((text_pt.x + size.cx) - (m_szScreenSize.cx - 1));
				}

				// 출력 영역 Y 조정
				if ( text_pt.y < 1 ) {
					text_pt.y = m_uDispHotColdSize + 1;
				}

				DrawNormalText( h_src_dc, msg, text_pt.x, text_pt.y, size.cx, size.cy, m_crHotTextColor, m_crHotShadowColor, m_szFontFace, m_nFontSize );
			}
		}

		if ( TRUE == m_fDisplayColdPt ) {
			Pen				pen( Color( 255, 255, 255 ), 2 );
			SolidBrush		brush( Color( 0, 0, 255 ) );
			GraphicsPath	path;
			Point			pt[3];


			gra_src.SetSmoothingMode( SmoothingModeAntiAlias );

			pt[0].X = m_ptColdPoint.x;
			pt[0].Y = m_ptColdPoint.y;

			pt[1].X = m_ptColdPoint.x - m_uDispHotColdSize;
			pt[1].Y = m_ptColdPoint.y - m_uDispHotColdSize;

			pt[2].X = m_ptColdPoint.x + m_uDispHotColdSize;
			pt[2].Y = m_ptColdPoint.y - m_uDispHotColdSize;

			path.AddPolygon( &pt[0], 3 );

			gra_src.DrawPath( &pen, &path );    
			gra_src.FillPath( &brush, &path );
			
			if ( TRUE == m_fDisplayColdTemp ) {
				CString			msg;
				CSize			size;
				CPoint			text_pt;


				msg.Format( _T("Cold [%.1f°%c]"), m_fColdTemperature, temp_type[m_eDispTempType] );

				size = MeasureString( h_src_dc, msg, m_szFontFace, m_nFontSize );

				text_pt.x = m_ptColdPoint.x + 1;
				text_pt.y = m_ptColdPoint.y + 1;

				// 출력 영역 X 조정
				if ( (text_pt.x + size.cx) > (m_szScreenSize.cx - 1) ) {
					text_pt.x -= ((text_pt.x + size.cx) - (m_szScreenSize.cx - 1));
				}

				// 출력 영역 Y 조정
				if ( (text_pt.y + size.cy) > (m_szScreenSize.cy - 1) ) {
					text_pt.y = m_ptColdPoint.y - m_uDispHotColdSize - size.cy;
				}

				DrawNormalText( h_src_dc, msg, text_pt.x, text_pt.y, size.cx, size.cy, m_crColdTextColor, m_crColdShadowColor, m_szFontFace, m_nFontSize );
			}
		}

		if ( TRUE == m_fDispMsg ) {
			CSize			size;
			CPoint			text_pt;


			size = MeasureString( h_src_dc, m_szDispMsg, m_szFontFace, m_nFontSize );

			switch ( m_eDispMsgPos ) {
			case	dmp_left_top		:
				{
					text_pt.x = 10;
					text_pt.y = 10;
				}
				break;

			case	dmp_left_bottom		:
				{
					text_pt.x = 10;
					text_pt.y = m_sImageSize.cy - (size.cy + 10);
				}
				break;
			case	dmp_right_top		:
				{
					text_pt.x = m_sImageSize.cx - (size.cx + 10);
					text_pt.y = 10;
				}
				break;

			case	dmp_right_bottom	:
				{
					text_pt.x = m_sImageSize.cx - (size.cx + 10);
					text_pt.y = m_sImageSize.cy - (size.cy + 10);
				}
				break;

			case	dmp_center			:
			default						:
				{
					text_pt.x = (m_sImageSize.cx / 2) - (size.cx / 2);
					text_pt.y = (m_sImageSize.cy / 2) - (size.cy / 2);
				}
				break;
			}

			DrawNormalText( h_src_dc, m_szDispMsg, text_pt.x, text_pt.y, size.cx, size.cy, m_crMsgTextColor, m_crMsgShadowColor, m_szFontFace, m_nFontSize );
		}
	}

	{
		Graphics		gra_dst( h_dst_dc );
		Bitmap			bitmap( h_bm_src, NULL );

		ImageAttributes image_att;


		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;

		//cx = (int)(m_fScreenScaleX * cx);
		//cy = (int)(m_fScreenScaleY * cy);

		cx = (int)(m_fImageScaleX * cx);
		cy = (int)(m_fImageScaleY * cy);

		gra_dst.SetInterpolationMode( InterpolationModeHighQualityBilinear );
		
		gra_dst.SetSmoothingMode( SmoothingModeHighQuality );
		gra_dst.DrawImage(
						&bitmap,
						Rect( rect.left, rect.top, cx, cy ),
						m_ptImageOffset.x,
						m_ptImageOffset.y,
						bitmap.GetWidth(),
						bitmap.GetHeight(),
						UnitPixel,
						&image_att
						);
	}

	if ( this->GetSafeHwnd() == ::GetFocus() ) {
		HPEN			h_pen;
		HPEN			h_old_pen;

		h_pen = ::CreatePen( PS_DOT, 1, RGB( 0, 255, 255 ) );

		h_old_pen = (HPEN)::SelectObject( h_dst_dc, h_pen );

		::MoveToEx( h_dst_dc, rect.left + 1, rect.top + 1, NULL );

		::LineTo( h_dst_dc, rect.right - 1,	rect.top + 1 );
		::LineTo( h_dst_dc, rect.right - 1,	rect.bottom - 1 );
		::LineTo( h_dst_dc, rect.left + 1,	rect.bottom - 1 );
		::LineTo( h_dst_dc, rect.left + 1,	rect.top + 1 );

		::SelectObject( h_dst_dc, h_old_pen );
		::DeleteObject( h_pen );
	}

	::BitBlt(
			dc.GetSafeHdc(),
			rect.left, rect.top, rect.right, rect.bottom,
			h_dst_dc,
			0, 0,
			SRCCOPY
			);

	if ( TRUE == m_fDisplayMouseTemp ) {
		TCHAR			temp_type[3] = { _T('C'), _T('F'), _T('K') };

		CString			msg;
		CSize			size;
		CPoint			text_pt;

		POINT			pt;


		::GetCursorPos( &pt );
		::ScreenToClient( this->GetSafeHwnd(), &pt );
	
		if ( 0 <= pt.x && pt.x < m_szScreenSize.cx && 0 <= pt.y && pt.y < m_szScreenSize.cy ) {
			msg.Format( _T("[%.1f°%c]"), m_fMouseTemperature, temp_type[m_eDispTempType] );

			size = MeasureString( dc.GetSafeHdc(), msg, m_szFontFace, m_nFontSize );

			text_pt.x = pt.x - (size.cx / 2);
			text_pt.y = pt.y + 10;

			// 출력 영역 X 조정
			if ( (text_pt.x + size.cx) > (m_szScreenSize.cx - 2) ) {
				text_pt.x -= ((text_pt.x + size.cx) - (m_szScreenSize.cx - 2));
			}

			if ( text_pt.x < 1 ) {
				text_pt.x = 1;
			}

			// 출력 영역 Y 조정
			if ( (text_pt.y + size.cy) > (m_szScreenSize.cy - 4) ) {
				text_pt.y = pt.y - 10 - size.cy;
			}

			DrawNormalText( dc.GetSafeHdc(), msg, text_pt.x, text_pt.y, size.cx, size.cy, RGB(255, 255, 255), RGB(0, 0, 0), m_szFontFace, m_nFontSize );
		}
	}

	::SelectObject( h_dst_dc, h_old_bm_dst );
	::DeleteDC( h_dst_dc );
	h_dst_dc = nullptr;

	if ( nullptr != h_bm_dst ) {
		::DeleteObject( h_bm_dst );
		h_bm_dst = nullptr;
	}

	::SelectObject( h_src_dc, h_old_bm_src );
	::DeleteDC( h_src_dc );
	h_src_dc = nullptr;

	if ( nullptr != h_bm_src ) {
		::DeleteObject( h_bm_src );
		h_bm_src = nullptr;
	}

	if ( nullptr != h_rgn ) {
		::DeleteObject( h_rgn );
		h_rgn = nullptr;
	}

	::SelectClipRgn( dc.GetSafeHdc(), NULL );
}


BOOL CScreenWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if ( 0 == m_nImageBytes ) {
		RECT		rect;

		CBrush		brush;
		

		this->GetClientRect( &rect );

		brush.CreateSolidBrush( RGB( 0, 164, 242) );
		pDC->FillRect( &rect, &brush );
	}
	else {
		return ( TRUE );
	}

	return CStatic::OnEraseBkgnd(pDC);
}


BOOL CScreenWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	//::SetCursor( ::AfxGetApp()->LoadStandardCursor( (IDC_CROSS ) ) );
    //return TRUE;
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}


void CScreenWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CStatic::OnLButtonDblClk(nFlags, point);
}


void CScreenWnd::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CStatic::OnRButtonDblClk(nFlags, point);
}


void CScreenWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	//TRACE( _T("OnKeyDown %d %d %08X\r\n"), nChar, nRepCnt, nFlags );
	CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CScreenWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	//TRACE( _T("OnKeyUp %d %d %08X\r\n"), nChar, nRepCnt, nFlags );
	CStatic::OnKeyUp(nChar, nRepCnt, nFlags);
}


UINT CScreenWnd::OnGetDlgCode()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return ( DLGC_WANTARROWS );
	//return CStatic::OnGetDlgCode();
}


void CScreenWnd::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	RECT		rect;


	TRACE(_T("Call CScreenWnd::OnSize\r\n"));

	::GetClientRect( this->GetSafeHwnd(), &rect );

	if (m_szScreenSize.cx >= (m_rcActiveScreen.right - m_rcActiveScreen.left)) {
		rect.left	= m_rcActiveScreen.left - m_ptScreenPoint.x;
		rect.right	= rect.left + (m_rcActiveScreen.right - m_rcActiveScreen.left);
	}

	if (m_szScreenSize.cy >= (m_rcActiveScreen.bottom - m_rcActiveScreen.top)) {
		rect.top	= m_rcActiveScreen.top - m_ptScreenPoint.y;
		rect.bottom	= rect.top + (m_rcActiveScreen.bottom - m_rcActiveScreen.top);
	}

	m_rcClipRegion = rect;

	if ( 0 == m_sImageSize.cx || 0 == m_sImageSize.cy ) {
		m_fScreenScaleX = (float)1.0;
		m_fScreenScaleY = (float)1.0;
	}
	else {
		m_fScreenScaleX = ((float)cx / (float)m_sImageSize.cx);
		m_fScreenScaleY = ((float)cy / (float)m_sImageSize.cy);
	}
}


void CScreenWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CStatic::OnWindowPosChanged(lpwndpos);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	RECT			rect;


	::GetClientRect(this->GetSafeHwnd(), &rect);

	m_ptScreenPoint.x = lpwndpos->x + ((lpwndpos->cx - (rect.right - rect.left)) / 2);
	m_ptScreenPoint.y = lpwndpos->y + ((lpwndpos->cy - (rect.bottom - rect.top)) / 2);

	//	실제 클라이언트 영역으로 크기를 구한다. ( 프레임 크기 제외 )
	m_szScreenSize.cx = rect.right - rect.left;
	m_szScreenSize.cy = rect.bottom - rect.top;

	if ( m_szScreenSize.cx >= (m_rcActiveScreen.right - m_rcActiveScreen.left)) {
		rect.left	= m_rcActiveScreen.left - m_ptScreenPoint.x;
		rect.right	= rect.left + (m_rcActiveScreen.right - m_rcActiveScreen.left);
	}

	if ( m_szScreenSize.cy >= (m_rcActiveScreen.bottom - m_rcActiveScreen.top)) {
		rect.top	= m_rcActiveScreen.top - m_ptScreenPoint.y;
		rect.bottom	= rect.top + (m_rcActiveScreen.bottom - m_rcActiveScreen.top);
	}

	m_rcClipRegion = rect;

	if ( 0 == m_sImageSize.cx || 0 == m_sImageSize.cy ) {
		m_fScreenScaleX = (float)1.0;
		m_fScreenScaleY = (float)1.0;
	}
	else {
		m_fScreenScaleX = ((float)m_szScreenSize.cx / (float)m_sImageSize.cx);
		m_fScreenScaleY = ((float)m_szScreenSize.cy / (float)m_sImageSize.cy);
	}
}


SIZE	CScreenWnd::MeasureString(
HDC					_h_dc, ///< HDC
LPCTSTR				_str, ///< 출력 문자열
wchar_t *			_font_name, ///< Font Face name
int					_font_size ///< Font Size
)
{
	Graphics			graphics( _h_dc );

	FontFamily			ffamily( _font_name );
	Gdiplus::Font		font( &ffamily, (REAL)_font_size, FontStyleRegular, UnitPixel );
	StringFormat		sfmt;

	PointF			origin(0, 0);
	RectF			rect; 

	SIZE			size;

	wchar_t			txt[256];


	::memset( &txt, 0x00, sizeof(txt) );
#ifdef	_MBCS	//	UNICODE 가 아닌 경우
	::MultiByteToWideChar(
						CP_ACP,
						0,
						_str,
						-1,
						txt,
						::strlen( _str )
						);
#else
	::wcscpy_s( txt, _str );
#endif

	graphics.MeasureString( txt, (int)::wcslen( txt ), &font, origin, &rect );

	size.cx = (LONG)rect.Width;
	size.cy = (LONG)rect.Height;

	return ( size );
}


void	CScreenWnd::DrawNormalText(
HDC					_h_dc, ///< HDC
LPCTSTR				_text, ///< 출력 문자열
long				_x, ///< 출력 X 좌표
long				_y, ///< 출력 Y 좌표
long				_cx, ///< 출력 WIDTH
long				_cy, ///< 출력 HEIGHT
COLORREF			_text_clr, ///< 텍스트 칼라
COLORREF			_line_clr, ///< Outline 칼라
wchar_t *			_font_name, ///< Font Face name
int					_font_size ///< Font Size
)
{
	Graphics		graphics( _h_dc );

	FontFamily		fontFamily( _font_name );
	StringFormat	str_fmt;

	GraphicsPath	path;

	wchar_t			txt[256];


	::memset( &txt, 0x00, sizeof(txt) );
#ifdef	_MBCS	//	UNICODE 가 아닌 경우
	::MultiByteToWideChar(
						CP_ACP,
						0,
						_text,
						-1,
						txt,
						::strlen( _text )
						);
#else
	::wcscpy_s( txt, _text );
#endif

    graphics.SetSmoothingMode( SmoothingModeAntiAlias );
    graphics.SetInterpolationMode( InterpolationModeHighQualityBicubic );
	    
    path.AddString(
				txt,
				(int)::wcslen( txt ),
				&fontFamily,
				FontStyleRegular,
				(REAL)_font_size,
				Gdiplus::Point( _x, _y ),
				&str_fmt
				);

    Pen				pen( Color( GetRValue( _line_clr ), GetGValue( _line_clr ), GetBValue( _line_clr ) ), 2 );
	SolidBrush		brush( Color( GetRValue( _text_clr ), GetGValue( _text_clr ), GetBValue( _text_clr ) ) );


    graphics.DrawPath( &pen, &path );    
    graphics.FillPath( &brush, &path );
}