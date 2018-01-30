// UpdateServerDui.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "UpdateServerDui.h"
#include "UpdateServer.h"

#include <exdisp.h>
#include <comdef.h>
#include <ShellAPI.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	// OLE
	HRESULT hRes = ::OleInitialize(NULL);
	// ��ʼ��UI������
	CPaintManagerUI::SetResourceType(UILIB_FILE);
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
	CResourceManager::GetInstance()->LoadResource(_T("res.xml"), NULL);

	// ע��ؼ�
	//REGIST_DUICONTROL(CCircleProgressUI);
	///REGIST_DUICONTROL(CMyComboUI);
	//REGIST_DUICONTROL(CChartViewUI);
	REGIST_DUICONTROL(CWndUI);

	CUpdateServer* pFrame = new CUpdateServer();
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, _T("UpdateServer"), UI_WNDSTYLE_DIALOG, WS_EX_STATICEDGE | 
		WS_EX_APPWINDOW, 0, 0, 800, 600);
	pFrame->CenterWindow();
	//::ShowWindow(*pFrame, SW_SHOW);

	// ��Ϣѭ��
	CPaintManagerUI::MessageLoop();
	// ���ٴ���
	delete pFrame;
	pFrame = NULL;
	// ������Դ
	CPaintManagerUI::Term();
	// OLE
	OleUninitialize();
	// COM
	::CoUninitialize();

	return 0;
}

