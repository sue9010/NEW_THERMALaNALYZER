
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit


// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++	SDK Working Area
//
//			Project Property Page -> Configuration Properties -> VC++ Directories
//			Include Directories : add Bonjure SDK & Thermal Camera SDK include path
//			Library Directories : add Bonjure SDK & Thermal Camera SDK library path

#define		_USED_BONJURESDK_

#include <Windowsx.h>

#include <gdiPlus.h>
#pragma comment(lib, "gdiplus")
using namespace Gdiplus;

#include	<winsock2.h>
#include	<Ws2tcpip.h>
#pragma	comment( lib, "Ws2_32.lib" )


#ifdef _USED_BONJURESDK_
#include	<dns_sd.h>						//	Bonjure SDK
#pragma	comment( lib, "dnssd.lib" )
#endif // _USED_BONJURESDK_

#include	<ThermalCamSDK.h>				//	Thermal Camera SDK
#if defined(_M_X64)
	#ifdef _DEBUG
		#pragma	comment( lib, "ThermalCamSDK_x64_D.lib" )
	#else
		#pragma	comment( lib, "ThermalCamSDK_x64.lib" )
	#endif
#elif defined(_M_IX86)
	#ifdef _DEBUG
		#pragma	comment( lib, "ThermalCamSDK_D.lib" )
	#else
		#pragma	comment( lib, "ThermalCamSDK.lib" )
	#endif
#else
#endif



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define		_WINSOCK_DEPRECATED_NO_WARNINGS			1





#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


