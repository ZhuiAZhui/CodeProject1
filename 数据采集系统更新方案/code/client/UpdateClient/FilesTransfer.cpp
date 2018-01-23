#include "stdafx.h"
#include "FilesTransfer.h"
#include "PublicFunction.h"


// 文件接收类
RecvFile::RecvFile()
{
	m_savePath == "";
	m_serPort = 0;
	m_dwErrorCode = 0;
}

RecvFile::RecvFile(HWND hNotifyWnd, std::string savePath, u_short port)
{
	m_hNotifyWnd = hNotifyWnd;
	m_savePath = savePath;
	m_serPort = port;
	m_dwErrorCode = 0;
}


RecvFile::~RecvFile()
{
	//释放资源
	for (int i = 0;i<m_recvThreads.GetSize();i++)
	{
		CloseHandle(m_recvThreads.GetAt(i));
	}
	m_recvThreads.RemoveAll();
	closesocket(m_serSocket);
}

bool RecvFile::InitRecv()
{
	//create server socket
	m_serSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	m_serAddr.sin_family = AF_INET;
	m_serAddr.sin_port = htons(m_serPort);
	m_serAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// bind the server address and port
	if (bind(m_serSocket, (sockaddr*)&m_serAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		closesocket(m_serSocket);
		m_dwErrorCode = GetLastError();
		return false;
	}

	// listen the socket
	if (listen(m_serSocket, 10) == SOCKET_ERROR)
	{
		closesocket(m_serSocket);
		m_dwErrorCode = GetLastError();
		return false;
	}
	return true;
}

HANDLE RecvFile::StartRecv(std::string &error)
{
	if (m_savePath == "")
	{
		error = "文件接收目录未指定.";
		return INVALID_HANDLE_VALUE;
	}

	if (false == InitRecv())
	{
		error = "初始化socket失败（创建，绑定，监听）.";
		return INVALID_HANDLE_VALUE;
	}

	unsigned int recvthreadID = 0;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvFileThreadProc,
		this, 0, &recvthreadID);
	m_recvThreads.Add(handle);
	return handle;
}


DWORD RecvFile::RecvFileThreadProc(LPVOID lpParameter)
{
	RecvFile* pRecvFile = static_cast<RecvFile*>(lpParameter);
	if (pRecvFile == NULL)
	{
		return -1;
	}

	int iRet = 0;
	char szError[MAX_PATH] = { 0 };
	char szBuffer[1024] = { 0 };
	int iAddrLen = sizeof(sockaddr_in);
	// 连接成功后返回一个通信的套接字
	SOCKET acceptSocket = accept(pRecvFile->m_serSocket, (sockaddr*)&(pRecvFile->m_serAddr), &iAddrLen);
	if (acceptSocket == SOCKET_ERROR)
	{
		pRecvFile->m_dwErrorCode = GetLastError();

		// 发送消息至主窗口
		sprintf_s(szError, "accept失败，错误码：%d", pRecvFile->m_dwErrorCode);
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 1, (LPARAM)szError);
		return -1;
	}

	// 执行机数量可能有1-255（根据IP地址），如同时需要更新升级，假设升级文件有10个
	// 每个文件开启一个线程进行接收：则需要创建10-2550个线程；性能方面可能存在问题
	// 替代方案 -- 升级文件打包成一个压缩包，则线程总量不会超过256；
	// 以下代码注释
	/*unsigned int recvthreadID = 0;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvFileThreadProc,
	pRecvFile, 0, &recvthreadID);
	pRecvFile->m_recvThreads.Add(handle);*/

	/* 接收处理模块 */
	//1. 发送 “开始接收”
	//2. 接收 文件信息：文件名，大小
	//3. 开始接收
	SockFileStruct fileStruct;
	memset(&fileStruct, 0, sizeof(SockFileStruct));

	send(acceptSocket, "RecvRequest", sizeof("RecvRequest"), 0);//发送recv请求；使发送和接收保持一致；
	if (recv(acceptSocket, (char*)&fileStruct, sizeof(fileStruct), 0) == SOCKET_ERROR) //接收文件信息
	{
		pRecvFile->m_dwErrorCode = GetLastError();
		
		// 发送消息至主窗口
		sprintf_s(szError, "recv失败，错误码：%d", pRecvFile->m_dwErrorCode);
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
			FILE_TRANSFER_FAILED, (LPARAM)szError);
		return -1;
	}

	// 判断接收文件是否为空
	if (strlen(fileStruct.FileName) == 0 || fileStruct.Filelength == 0)
	{
		sprintf_s(szError, "接收文件为空，请确认服务端配置是否正确！");
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
			FILE_TRANSFER_FAILED, (LPARAM)szError);
		return -1;
	}
	std::string strSavePath = pRecvFile->m_savePath + "\\" + fileStruct.FileName;
	int iRecvLen = 0;
	FILE *pFile = NULL;

	// 开始接收并写文件
	try 
	{
	
		errno_t err = fopen_s(&pFile, strSavePath.c_str(), "wb");
		if (err != 0 || pFile == NULL)
		{
			sprintf_s(szError, "打开文件%s失败！", strSavePath.c_str());
			::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
				FILE_TRANSFER_FAILED, (LPARAM)szError);
			return -1;
		}
		send(acceptSocket, "RecvRequest", sizeof("RecvRequest"), 0);//发送recv请求；使发送和接收保持一致；
		while (iRet != SOCKET_ERROR)
		{
			memset(szBuffer, 0, 1024);
			iRet = recv(acceptSocket, szBuffer, 1024, 0);
			// 接收失败，网络错误 或者连接被优雅关闭
			if (iRet == SOCKET_ERROR || iRet == 0)
			{
				pRecvFile->m_dwErrorCode = GetLastError();
				if (pRecvFile->m_dwErrorCode != 0)
				{
					sprintf_s(szError, "接收缓冲区数据失败，错误码：%d", pRecvFile->m_dwErrorCode);
					::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER,
						FILE_TRANSFER_FAILED, (LPARAM)szError);
				}
				break;
			}
			if (pFile != NULL)
			{
				fwrite(szBuffer,sizeof(char), iRet, pFile);
			}
			iRecvLen += iRet;

			// 文件发送结束，退出循环
			if (iRecvLen == fileStruct.Filelength)
			{
				break;
			}
		}

		// 清理资源
		if (pFile != NULL)
		{
			fclose(pFile);
			pFile = NULL;
		}
		
	}
	catch (exception &e)
	{
		pRecvFile->m_dwErrorCode = GetLastError();
		e.what();
	}
	closesocket(acceptSocket);

	// 发送消息通知接收文件完成，并返回接收的文件名
	::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, FILE_TRANSFER_SUCESS, 
		(LPARAM)strSavePath.c_str());
	return 0;
}

BOOL RecvFile::WaitForMulThread(DWORD dwWaitSecond, string& strError)
{
	BOOL bRes = TRUE;
	char szBuffer[MAX_PATH] = { 0 };
	strError = "";
	string str = "";

	// 
	for (int i = 0;i<m_recvThreads.GetSize();i++)
	{
		if (WAIT_TIMEOUT == WaitForSingleObject(m_recvThreads[i], dwWaitSecond))
		{
			sprintf_s(szBuffer, "%d", i+1);
			str = "接收第";
			str.append(szBuffer);
			str.append("个文件超时.");

			strError += str;
			bRes = FALSE;
		}
	}
	return bRes;
}

// 文件发送类
FileSend::FileSend()
{
	m_fileList = new CArray<std::string, std::string&>;
	m_finfoList = new CArray<SockFileStruct, SockFileStruct&>;
	m_dwErrorCode = 0;
}

FileSend::FileSend(HWND hNotifyWnd, std::string ip, u_short port)
{
	m_hNotifyWnd = hNotifyWnd;
	m_destIP = ip;
	m_destPort = port;
	m_fileList = new CArray<std::string, std::string&>;
	m_finfoList = new CArray<SockFileStruct, SockFileStruct&>;
	m_dwErrorCode = 0;
}

FileSend::~FileSend()
{
	//WaitForMultipleObjects(m_sendThreads.GetSize(), m_sendThreads, TRUE, 60 * 1000);
	//释放资源
	for (int i = 0;i<m_sendThreads.GetSize();i++)
	{
		CloseHandle(m_sendThreads.GetAt(i));
	}
	//m_sendThreads.RemoveAll();
	delete m_fileList;
	delete m_finfoList;
}


HANDLE FileSend::StartSend(std::string & error)
{
	/* 每一个文件对应一个socket，同一时间最大10个
	分别创建线程发送文件 */
	unsigned int sendthreadID = 0;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadPro,
		this, 0, &sendthreadID);
	m_sendThreads.Add(handle);
	return handle;
}

DWORD FileSend::SendFileThreadPro(LPVOID lpParameter)
{
	FileSend* pSendFile = static_cast<FileSend*>(lpParameter);

	std::string strFilePath = "";
	//发送文件结构体
	SockFileStruct fileStruct;
	memset(&fileStruct, 0, sizeof(SockFileStruct));
	//while (!(pSendFile->m_fileList)->IsEmpty() && !(pSendFile->m_finfoList)->IsEmpty())
	//{
	//	strFilePath = (pSendFile->m_fileList)->GetAt(0);
	//	fileStuct = (pSendFile->m_finfoList)->GetAt(0);
	//	//移除 已经创建发送线程的文件，防止重复创建
	//	(pSendFile->m_fileList)->RemoveAt(0);
	//	(pSendFile->m_finfoList)->RemoveAt(0);

	// 执行机数量可能有1-255（根据IP地址），如同时需要更新升级，假设升级文件有10个
	// 每个文件开启一个线程进行接收：则需要创建10-2550个线程；性能方面可能存在问题
	// 替代方案 -- 升级文件打包成一个压缩包，则线程总量不会超过256；
	// 以下代码注释
	// 当前有文件需要发送，需要重新创建线程进行发送
	//if ((pSendFile->m_fileList)->GetSize() != 0)
	//{
	//	unsigned int sendthreadID = 0;
	//			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadPro,
	//				pSendFile, 0, &sendthreadID);
	//	pSendFile->m_sendThreads.Add(handle);
	//
	//	Sleep(500);		//延时0.5s，防止循环过快，数组未改变
	//}

	//}

	// 创建连接socket
	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pSendFile->m_destPort);
	addr.sin_addr.S_un.S_addr = inet_addr(pSendFile->m_destIP.c_str());

	if (connect(connectSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)//主动连接服务器
	{
		closesocket(connectSocket);
		pSendFile->m_dwErrorCode = GetLastError();
		return -1;
	}

	//while (!(pSendFile->m_fileList)->IsEmpty() && !(pSendFile->m_finfoList)->IsEmpty())
	//{
		strFilePath = (pSendFile->m_fileList)->GetAt(0);
		fileStruct = (pSendFile->m_finfoList)->GetAt(0);
		//移除 已经创建发送线程的文件，防止重复创建
		(pSendFile->m_fileList)->RemoveAt(0);
		(pSendFile->m_finfoList)->RemoveAt(0);

		char SendRequest[20];//发送请求
		recv(connectSocket, SendRequest, 20, 0);//接受发送请求，准备发送

		// 填充文件信息至结构体中
		if (send(connectSocket, (char*)&fileStruct, sizeof(SockFileStruct), 0)
			== SOCKET_ERROR)
		{
			closesocket(connectSocket);
			pSendFile->m_dwErrorCode = GetLastError();
			return -1;
		}

		recv(connectSocket, SendRequest, 20, 0);//接受发送请求，准备发送

		// 发送文件
		FILE *pFile = NULL;
		errno_t err = fopen_s(&pFile, strFilePath.c_str(), "rb");
		if (err != 0 || pFile == NULL)
		{
			return -1;
		}

		/*CFile file;
		CFileException e;
		if (!file.Open(PublicFunction::M2W(strFilePath.c_str()), CFile::modeRead |
			CFile::typeBinary, &e))
		{
			e.ReportError();
			return -1;
		}*/
		int iSendLen = 0;
		int iReadLen = 0;
		int iTotalSendLen = 0;
		char szBuffer[1024] = { 0 };
		void *pRead = malloc(1024);
		while (iSendLen != SOCKET_ERROR)
		{
			//memset(szBuffer, 0, 1024);
			iReadLen = fread_s(pRead, 1024, 1, 1024, pFile);//file.Read(pRead, sizeof(szBuffer));
			iSendLen = send(connectSocket, (char*)pRead, iReadLen, 0);

			iTotalSendLen += iSendLen;
			if (iTotalSendLen == fileStruct.Filelength)		//文件发送完成
			{
				break;
			}

			if (iSendLen == SOCKET_ERROR)
			{
				break;
			}
		}

		free(pRead);
		fclose(pFile);
		//file.Close();
	//}

	closesocket(connectSocket);
	// 发送消息通知发送文件完成
	::SendMessage(pSendFile->m_hNotifyWnd, FILE_SEND_SUCCESS, 0, 0);
	return 0;
}


bool FileSend::AddFileToSend(std::string filepath, LPTSTR lpError, UINT uiLen)
{
	// 填充文件信息至结构体中
	SockFileStruct sockFile;
	std::string strName = PathFindFileNameA(filepath.c_str());
	strcpy_s(sockFile.FileName, strName.c_str());
	strcpy_s(sockFile.FilePath, filepath.c_str());
	// 获取文件大小
	CFile file;
	CFileException e;

	if (!file.Open(PublicFunction::M2W(filepath.c_str()), CFile::modeRead |
		CFile::typeBinary, &e))
	{
		e.GetErrorMessage(lpError, uiLen);
		return false;
	}
	sockFile.Filelength = file.GetLength();
	file.Close();

	m_fileList->Add(filepath);
	m_finfoList->Add(sockFile);
	return true;
}
