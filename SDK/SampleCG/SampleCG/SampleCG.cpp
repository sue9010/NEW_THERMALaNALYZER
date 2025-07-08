
// SampleCG.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SampleCG.h"
#include "SampleCGDlg.h"

#include <strsafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment (lib, "version.lib")


// CSampleCGApp

BEGIN_MESSAGE_MAP(CSampleCGApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSampleCGApp construction

CSampleCGApp::CSampleCGApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSampleCGApp object

CSampleCGApp theApp;

#if defined(_M_X64)
ULONG_PTR		gdiplusToken;
#else
ULONG			gdiplusToken;
#endif


// CSampleCGApp initialization

BOOL CSampleCGApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	GdiplusStartupInput			GdiplusStartupInput;


	if ( ::GdiplusStartup(&gdiplusToken, &GdiplusStartupInput, NULL) != Ok ) {
		AfxMessageBox(_T("Error : Failed to initialize GDI+ library!"));
		return FALSE;
	}

	GetFileVersion();

	CSampleCGDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	::GdiplusShutdown( gdiplusToken );

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


void CSampleCGApp::GetFileVersion()
{
	 
    TCHAR		temp_path[MAX_PATH]; 	// ���� ����� ���α׷��� ��θ� ������ �����̴�.
	TCHAR		SubBlock[MAX_PATH]; 
    
	PTCHAR		p_data;

	HRESULT		hr;

	DWORD		h_version_handle;  // ���� ������ ��� ���� ����� �ڵ鰪�� �����ϴ� �����̴�.
	DWORD		version_info_size;

	HANDLE		h_memory; 

    LPVOID		p_info_memory;
    
	UINT		data_size;  // ������ ���� ������ ũ�⸦ ������ �����̴�.
	
	LPCTSTR		lpSubBlock = _T("\\VarFileInfo\\Translation");
	
	struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
	} * lpTranslate;
	

	p_data = NULL;
    data_size = 0;

    // ���� ����� ���α׷��� ��θ� ��´�.
    ::GetModuleFileName(
					::AfxGetInstanceHandle(), 
					(LPTSTR)temp_path,
					sizeof( temp_path )
					);

    // ���������� �׸��� ����ڰ� �߰�/���� �Ҽ� �ֱ� ������ ������ ũ�Ⱑ �ƴϴ�.
    // ���� ���� ���α׷��� ���������� ���� ũ�⸦ �� �� ũ�⿡ �´� �޸𸮸� �Ҵ��ϰ� �۾��ؾ��Ѵ�.
    version_info_size = ::GetFileVersionInfoSize(
											(LPTSTR)temp_path,
											&h_version_handle 
											);

    // ���������� �����ϱ� ���� �ý��� �޸𸮸� �����Ѵ�. ( �ڵ� �������� ���� )
    h_memory = ::GlobalAlloc( GMEM_MOVEABLE, version_info_size ); 

    // �ڵ� ������ �޸𸮸� ����ϱ� ���ؼ� �ش� �ڵ鿡 �����Ҽ� �ִ� �ּҸ� ��´�.
    p_info_memory = ::GlobalLock( h_memory );

    // ���� ���α׷��� ���� ������ �����´�.
    ::GetFileVersionInfo( 
					(LPTSTR)temp_path,
					h_version_handle, 
					version_info_size,
					p_info_memory 
					);
	
	// ���� ������ �����´�		
	::VerQueryValue(	
					p_info_memory, 
					lpSubBlock,
					(LPVOID*)&lpTranslate,
					&data_size
					);


	// ���������� ���Ե� FileVersion ������ �� ����Ѵ�.
	hr = ::StringCchPrintf(
							SubBlock, 
							200,
							_T("\\StringFileInfo\\%04x%04x\\FileVersion"),
							lpTranslate[0].wLanguage,
							lpTranslate[0].wCodePage
							);

	::VerQueryValue(
					p_info_memory, 
					SubBlock, 
					(LPVOID*)&p_data, 
					&data_size
					);

	m_szFileVersion.Format( _T("%s"), p_data );
}
