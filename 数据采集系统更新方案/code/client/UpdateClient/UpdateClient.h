
// UpdateClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUpdateClientApp: 
// �йش����ʵ�֣������ UpdateClient.cpp
//

class CUpdateClientApp : public CWinApp
{
public:
	CUpdateClientApp();

// ��д
public:
	virtual BOOL InitInstance();

	BOOL SetMutex();
	void ReleaseMutex();

	HANDLE m_hMutex;

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUpdateClientApp theApp;