
// ThermalCamDllSample.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.
#include <stdint.h>

/* User Message */
#define MY_MESSAGE1	(WM_USER + 1)	// OnErrorNotificationUserHandler
#define MY_MESSAGE2 (WM_USER + 2)	// OnDisplayImageHandler
#define MY_MESSAGE3 (WM_USER + 3)	// OnGetSaveDataHandler

#define MIN_SPAN	1		// 1.0
#define MAX_SPAN	100		// 100.0


// CThermalCamDllSampleApp:
// �� Ŭ������ ������ ���ؼ��� ThermalCamDllSample.cpp�� �����Ͻʽÿ�.
//

class CThermalCamDllSampleApp : public CWinApp
{
public:
	CThermalCamDllSampleApp();
	
public:
	CString		m_szFileVersion;

	void GetFileVersion();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CThermalCamDllSampleApp theApp;