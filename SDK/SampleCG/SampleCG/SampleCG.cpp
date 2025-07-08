
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
	 
    TCHAR		temp_path[MAX_PATH]; 	// 현재 실행된 프로그램의 경로를 저장할 변수이다.
	TCHAR		SubBlock[MAX_PATH]; 
    
	PTCHAR		p_data;

	HRESULT		hr;

	DWORD		h_version_handle;  // 버전 정보를 얻기 위해 사용할 핸들값을 저장하는 변수이다.
	DWORD		version_info_size;

	HANDLE		h_memory; 

    LPVOID		p_info_memory;
    
	UINT		data_size;  // 실제로 읽은 정보의 크기를 저장할 변수이다.
	
	LPCTSTR		lpSubBlock = _T("\\VarFileInfo\\Translation");
	
	struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
	} * lpTranslate;
	

	p_data = NULL;
    data_size = 0;

    // 현재 실행된 프로그램의 경로를 얻는다.
    ::GetModuleFileName(
					::AfxGetInstanceHandle(), 
					(LPTSTR)temp_path,
					sizeof( temp_path )
					);

    // 버전정보는 항목을 사용자가 추가/삭제 할수 있기 때문에 고정된 크기가 아니다.
    // 따라서 현재 프로그램의 버전정보에 대한 크기를 얻어서 그 크기에 맞는 메모리를 할당하고 작업해야한다.
    version_info_size = ::GetFileVersionInfoSize(
											(LPTSTR)temp_path,
											&h_version_handle 
											);

    // 버전정보를 저장하기 위한 시스템 메모리를 생성한다. ( 핸들 형식으로 생성 )
    h_memory = ::GlobalAlloc( GMEM_MOVEABLE, version_info_size ); 

    // 핸들 형식의 메모리를 사용하기 위해서 해당 핸들에 접근할수 있는 주소를 얻는다.
    p_info_memory = ::GlobalLock( h_memory );

    // 현재 프로그램의 버전 정보를 가져온다.
    ::GetFileVersionInfo( 
					(LPTSTR)temp_path,
					h_version_handle, 
					version_info_size,
					p_info_memory 
					);
	
	// 파일 정보를 가져온다		
	::VerQueryValue(	
					p_info_memory, 
					lpSubBlock,
					(LPVOID*)&lpTranslate,
					&data_size
					);


	// 버전정보에 포함된 FileVersion 정보를 얻어서 출력한다.
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
