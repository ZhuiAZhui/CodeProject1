// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
//#include <windows.h>


#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN        // 从 Windows 头中排除极少使用的资料 #endif  
#include <afx.h> 
#include <afxwin.h>         // MFC 核心组件和标准组件 #include <afxext.h>         // MFC 扩展  

#endif

// TODO:  在此处引用程序需要的其他头文件

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