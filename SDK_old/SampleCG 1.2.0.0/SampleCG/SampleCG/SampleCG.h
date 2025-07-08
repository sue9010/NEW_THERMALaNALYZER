
// SampleCG.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSampleCGApp:
// See SampleCG.cpp for the implementation of this class
//

class CSampleCGApp : public CWinApp
{
public:
	CSampleCGApp();
	
	
public:
	CString		m_szFileVersion;

	void GetFileVersion();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSampleCGApp theApp;