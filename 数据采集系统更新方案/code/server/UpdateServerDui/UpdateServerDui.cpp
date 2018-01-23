// UpdateServerDui.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "UpdateServerDui.h"
#include "UpdateServer.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	CUpdateServer* pFrame = new CUpdateServer();
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, _T("UpdateServer"), UI_WNDSTYLE_DIALOG, WS_EX_STATICEDGE | 
		WS_EX_APPWINDOW, 0, 0, 800, 600);
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);

	CPaintManagerUI::MessageLoop();
	::CoUninitialize();
	return 0;
}

