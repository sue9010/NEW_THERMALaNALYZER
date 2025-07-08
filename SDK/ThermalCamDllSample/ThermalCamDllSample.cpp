
// ThermalCamDllSample.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
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


// CThermalCamDllSampleApp ����

CThermalCamDllSampleApp::CThermalCamDllSampleApp()
{
	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// ������ CThermalCamDllSampleApp ��ü�Դϴ�.

CThermalCamDllSampleApp theApp;


// CThermalCamDllSampleApp �ʱ�ȭ

BOOL CThermalCamDllSampleApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ��ȭ ���ڿ� �� Ʈ�� �� �Ǵ�
	// �� ��� �� ��Ʈ���� ���ԵǾ� �ִ� ��� �� �����ڸ� ����ϴ�.
	CShellManager *pShellManager = new CShellManager;

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));
	
	GetFileVersion();

	CThermalCamDllSampleDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ��ȭ ���ڰ� �������Ƿ� ���� ���α׷��� �޽��� ������ �������� �ʰ�  ���� ���α׷��� ���� �� �ֵ��� FALSE��
	// ��ȯ�մϴ�.
	return FALSE;
}


void CThermalCamDllSampleApp::GetFileVersion()
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


