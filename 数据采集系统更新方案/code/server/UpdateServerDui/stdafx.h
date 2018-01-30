// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
//#include <windows.h>


#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN        // �� Windows ͷ���ų�����ʹ�õ����� #endif  
#include <afx.h> 
#include <afxwin.h>         // MFC ��������ͱ�׼��� #include <afxext.h>         // MFC ��չ  

#endif

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�

#include "..\DuiLib\UIlib.h"

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include "InitSocket.h"
#include "GroupTalk.h"
#include "File64.h"
#include "FilesTransfer.h"

#define GROUP_ADDRESS		"234.5.6.7"
#define GROUP_PORT			4567

#define CONFIG_NAME			"\\config.ini"
#define SYSFILEINFO			"\\SystemPathInfo.txt"
#define UPDATELOG			"\\Update.log"
#define UPDATEXML			"\\Update.xml"

#pragma comment(lib,"version.lib")

using namespace DuiLib;

#ifdef _DEBUG
#	ifdef _UNICODE
#		pragma comment(lib, "..\\bin\\DuiLib_d.lib")
#	else
#		pragma comment(lib, "..\\bin\\DuiLibA_d.lib")
#	endif
#else
#	ifdef _UNICODE
#		pragma comment(lib, "..\\bin\\DuiLib.lib")
#	else
#		pragma comment(lib, "..\\bin\\DuiLibA.lib")
#	endif
#endif

struct FILEINFO
{
	char szFileName[100];
	char szFileType[20];
	char szFileTime[30];
	ULONGLONG ulLength;
};