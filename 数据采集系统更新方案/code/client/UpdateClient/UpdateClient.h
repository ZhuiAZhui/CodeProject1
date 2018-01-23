
// UpdateClient.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CUpdateClientApp: 
// 有关此类的实现，请参阅 UpdateClient.cpp
//

class CUpdateClientApp : public CWinApp
{
public:
	CUpdateClientApp();

// 重写
public:
	virtual BOOL InitInstance();

	BOOL SetMutex();
	void ReleaseMutex();

	HANDLE m_hMutex;

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CUpdateClientApp theApp;