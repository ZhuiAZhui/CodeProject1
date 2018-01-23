#include "stdafx.h"
#include "FilesTransfer.h"
#include "PublicFunction.h"


// �ļ�������
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
	//�ͷ���Դ
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
		error = "�ļ�����Ŀ¼δָ��.";
		return INVALID_HANDLE_VALUE;
	}

	if (false == InitRecv())
	{
		error = "��ʼ��socketʧ�ܣ��������󶨣�������.";
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
	// ���ӳɹ��󷵻�һ��ͨ�ŵ��׽���
	SOCKET acceptSocket = accept(pRecvFile->m_serSocket, (sockaddr*)&(pRecvFile->m_serAddr), &iAddrLen);
	if (acceptSocket == SOCKET_ERROR)
	{
		pRecvFile->m_dwErrorCode = GetLastError();

		// ������Ϣ��������
		sprintf_s(szError, "acceptʧ�ܣ������룺%d", pRecvFile->m_dwErrorCode);
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 1, (LPARAM)szError);
		return -1;
	}

	// ִ�л�����������1-255������IP��ַ������ͬʱ��Ҫ�������������������ļ���10��
	// ÿ���ļ�����һ���߳̽��н��գ�����Ҫ����10-2550���̣߳����ܷ�����ܴ�������
	// ������� -- �����ļ������һ��ѹ���������߳��������ᳬ��256��
	// ���´���ע��
	/*unsigned int recvthreadID = 0;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvFileThreadProc,
	pRecvFile, 0, &recvthreadID);
	pRecvFile->m_recvThreads.Add(handle);*/

	/* ���մ���ģ�� */
	//1. ���� ����ʼ���ա�
	//2. ���� �ļ���Ϣ���ļ�������С
	//3. ��ʼ����
	SockFileStruct fileStruct;
	memset(&fileStruct, 0, sizeof(SockFileStruct));

	send(acceptSocket, "RecvRequest", sizeof("RecvRequest"), 0);//����recv����ʹ���ͺͽ��ձ���һ�£�
	if (recv(acceptSocket, (char*)&fileStruct, sizeof(fileStruct), 0) == SOCKET_ERROR) //�����ļ���Ϣ
	{
		pRecvFile->m_dwErrorCode = GetLastError();
		
		// ������Ϣ��������
		sprintf_s(szError, "recvʧ�ܣ������룺%d", pRecvFile->m_dwErrorCode);
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
			FILE_TRANSFER_FAILED, (LPARAM)szError);
		return -1;
	}

	// �жϽ����ļ��Ƿ�Ϊ��
	if (strlen(fileStruct.FileName) == 0 || fileStruct.Filelength == 0)
	{
		sprintf_s(szError, "�����ļ�Ϊ�գ���ȷ�Ϸ���������Ƿ���ȷ��");
		::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
			FILE_TRANSFER_FAILED, (LPARAM)szError);
		return -1;
	}
	std::string strSavePath = pRecvFile->m_savePath + "\\" + fileStruct.FileName;
	int iRecvLen = 0;
	FILE *pFile = NULL;

	// ��ʼ���ղ�д�ļ�
	try 
	{
	
		errno_t err = fopen_s(&pFile, strSavePath.c_str(), "wb");
		if (err != 0 || pFile == NULL)
		{
			sprintf_s(szError, "���ļ�%sʧ�ܣ�", strSavePath.c_str());
			::SendMessageW(pRecvFile->m_hNotifyWnd, FILE_RECEVIE_OVER, 
				FILE_TRANSFER_FAILED, (LPARAM)szError);
			return -1;
		}
		send(acceptSocket, "RecvRequest", sizeof("RecvRequest"), 0);//����recv����ʹ���ͺͽ��ձ���һ�£�
		while (iRet != SOCKET_ERROR)
		{
			memset(szBuffer, 0, 1024);
			iRet = recv(acceptSocket, szBuffer, 1024, 0);
			// ����ʧ�ܣ�������� �������ӱ����Źر�
			if (iRet == SOCKET_ERROR || iRet == 0)
			{
				pRecvFile->m_dwErrorCode = GetLastError();
				if (pRecvFile->m_dwErrorCode != 0)
				{
					sprintf_s(szError, "���ջ���������ʧ�ܣ������룺%d", pRecvFile->m_dwErrorCode);
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

			// �ļ����ͽ������˳�ѭ��
			if (iRecvLen == fileStruct.Filelength)
			{
				break;
			}
		}

		// ������Դ
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

	// ������Ϣ֪ͨ�����ļ���ɣ������ؽ��յ��ļ���
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
			str = "���յ�";
			str.append(szBuffer);
			str.append("���ļ���ʱ.");

			strError += str;
			bRes = FALSE;
		}
	}
	return bRes;
}

// �ļ�������
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
	//�ͷ���Դ
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
	/* ÿһ���ļ���Ӧһ��socket��ͬһʱ�����10��
	�ֱ𴴽��̷߳����ļ� */
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
	//�����ļ��ṹ��
	SockFileStruct fileStruct;
	memset(&fileStruct, 0, sizeof(SockFileStruct));
	//while (!(pSendFile->m_fileList)->IsEmpty() && !(pSendFile->m_finfoList)->IsEmpty())
	//{
	//	strFilePath = (pSendFile->m_fileList)->GetAt(0);
	//	fileStuct = (pSendFile->m_finfoList)->GetAt(0);
	//	//�Ƴ� �Ѿ����������̵߳��ļ�����ֹ�ظ�����
	//	(pSendFile->m_fileList)->RemoveAt(0);
	//	(pSendFile->m_finfoList)->RemoveAt(0);

	// ִ�л�����������1-255������IP��ַ������ͬʱ��Ҫ�������������������ļ���10��
	// ÿ���ļ�����һ���߳̽��н��գ�����Ҫ����10-2550���̣߳����ܷ�����ܴ�������
	// ������� -- �����ļ������һ��ѹ���������߳��������ᳬ��256��
	// ���´���ע��
	// ��ǰ���ļ���Ҫ���ͣ���Ҫ���´����߳̽��з���
	//if ((pSendFile->m_fileList)->GetSize() != 0)
	//{
	//	unsigned int sendthreadID = 0;
	//			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadPro,
	//				pSendFile, 0, &sendthreadID);
	//	pSendFile->m_sendThreads.Add(handle);
	//
	//	Sleep(500);		//��ʱ0.5s����ֹѭ�����죬����δ�ı�
	//}

	//}

	// ��������socket
	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pSendFile->m_destPort);
	addr.sin_addr.S_un.S_addr = inet_addr(pSendFile->m_destIP.c_str());

	if (connect(connectSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)//�������ӷ�����
	{
		closesocket(connectSocket);
		pSendFile->m_dwErrorCode = GetLastError();
		return -1;
	}

	//while (!(pSendFile->m_fileList)->IsEmpty() && !(pSendFile->m_finfoList)->IsEmpty())
	//{
		strFilePath = (pSendFile->m_fileList)->GetAt(0);
		fileStruct = (pSendFile->m_finfoList)->GetAt(0);
		//�Ƴ� �Ѿ����������̵߳��ļ�����ֹ�ظ�����
		(pSendFile->m_fileList)->RemoveAt(0);
		(pSendFile->m_finfoList)->RemoveAt(0);

		char SendRequest[20];//��������
		recv(connectSocket, SendRequest, 20, 0);//���ܷ�������׼������

		// ����ļ���Ϣ���ṹ����
		if (send(connectSocket, (char*)&fileStruct, sizeof(SockFileStruct), 0)
			== SOCKET_ERROR)
		{
			closesocket(connectSocket);
			pSendFile->m_dwErrorCode = GetLastError();
			return -1;
		}

		recv(connectSocket, SendRequest, 20, 0);//���ܷ�������׼������

		// �����ļ�
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
			if (iTotalSendLen == fileStruct.Filelength)		//�ļ��������
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
	// ������Ϣ֪ͨ�����ļ����
	::SendMessage(pSendFile->m_hNotifyWnd, FILE_SEND_SUCCESS, 0, 0);
	return 0;
}


bool FileSend::AddFileToSend(std::string filepath, LPTSTR lpError, UINT uiLen)
{
	// ����ļ���Ϣ���ṹ����
	SockFileStruct sockFile;
	std::string strName = PathFindFileNameA(filepath.c_str());
	strcpy_s(sockFile.FileName, strName.c_str());
	strcpy_s(sockFile.FilePath, filepath.c_str());
	// ��ȡ�ļ���С
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
