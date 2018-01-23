#pragma once

#include <iostream>

// 文件传输默认端口
#define  FILE_TRANSFER_PORT_DEFAULT			6000

struct SockFileStruct//文件结构体；用于发送
{
	char FileName[MAX_PATH];//用于发送；文件名
	ULONGLONG Filelength;//用于发送；文件大小
	char FilePath[MAX_PATH];//用于打开文件；文件详细路径名；带后缀；c:\b\a.txt
};


/* 在此类调用前，需要初始化winsocket */
class RecvFile
{
public:
	RecvFile();
	RecvFile(HWND hNotifyWnd,std::string sacvPath, u_short port= FILE_TRANSFER_PORT_DEFAULT);
	~RecvFile();
	/* TCP 传输文件 */
public:
	bool InitRecv();			//init the recv
	HANDLE StartRecv(std::string &error);
	static DWORD WINAPI RecvFileThreadProc(LPVOID lpParameter);

	BOOL WaitForMulThread(DWORD dwWaitSecond, string& strError);

public:
	u_short	m_serPort;							//服务端 监听端口
	std::string m_savePath;						//接收文件保存地址

	DWORD m_dwErrorCode;		// 执行错误码
	HWND m_hNotifyWnd;			// 主窗口句柄
protected:
	/* 接收文件
		1、建立服务端监听socket，绑定服务端地址、端口，监听，创建线程接收连接
		2、接收到新连接，新的socket用于接收文件 
		3、再创建线程监听；当前线程 继续接收文件 */
	SOCKET m_serSocket;							//服务端 监听socket	
	sockaddr_in m_serAddr;						//服务端 地址
	CArray<HANDLE, HANDLE&> m_recvThreads;		//动态数组：接收文件线程句柄组
};
	

class FileSend
{
public:
	FileSend();
	FileSend(HWND hNotifyWnd, std::string ip, u_short port = FILE_TRANSFER_PORT_DEFAULT);
	~FileSend();

public:
	HANDLE StartSend(std::string &error);

	static DWORD WINAPI SendFileThreadPro(LPVOID lpParameter);

	bool AddFileToSend(std::string filepath, LPTSTR lpError, UINT uiLen);
	
	//待发送文件列表
	CArray<std::string, std::string&> *m_fileList;
	CArray<SockFileStruct, SockFileStruct&> *m_finfoList;

	DWORD m_dwErrorCode;		// 执行错误码
protected:
	/* 发送文件
	1、根据文件个数，创建线程：最大10个，>10的文件等待当前线程处理完成后创建
	2、每个线程创建socket，连接发送文件目的地址 destIP
	3、发送文件 */
	CArray<HANDLE, HANDLE&> m_sendThreads;		//动态数组：发送文件线程句柄组
	std::string m_destIP;								//发送文件目的地址
	u_short m_destPort;							//目的地址的端口（应与监听的一致）

	HWND m_hNotifyWnd;			// 主窗口句柄
};
