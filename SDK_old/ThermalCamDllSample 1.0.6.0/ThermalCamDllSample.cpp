
// ThermalCamDllSample.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "ThermalCamDllSample.h"
#include "ThermalCamDllSampleDlg.h"

#include <strsafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment (lib, "version.lib")

// CThermalCamDllSampleApp

BEGIN_MESSAGE_MAP(CThermalCamDllSampleApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CThermalCamDllSampleApp 생성

CThermalCamDllSampleApp::CThermalCamDllSampleApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CThermalCamDllSampleApp 개체입니다.

CThermalCamDllSampleApp theApp;


// CThermalCamDllSampleApp 초기화

BOOL CThermalCamDllSampleApp::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));
	
	GetFileVersion();

	CThermalCamDllSampleDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}


void CThermalCamDllSampleApp::GetFileVersion()
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


