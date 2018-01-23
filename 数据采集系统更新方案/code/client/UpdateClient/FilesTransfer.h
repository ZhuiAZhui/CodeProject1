#pragma once

#include <iostream>

// �ļ�����Ĭ�϶˿�
#define  FILE_TRANSFER_PORT_DEFAULT			6000

struct SockFileStruct//�ļ��ṹ�壻���ڷ���
{
	char FileName[MAX_PATH];//���ڷ��ͣ��ļ���
	ULONGLONG Filelength;//���ڷ��ͣ��ļ���С
	char FilePath[MAX_PATH];//���ڴ��ļ����ļ���ϸ·����������׺��c:\b\a.txt
};


/* �ڴ������ǰ����Ҫ��ʼ��winsocket */
class RecvFile
{
public:
	RecvFile();
	RecvFile(HWND hNotifyWnd,std::string sacvPath, u_short port= FILE_TRANSFER_PORT_DEFAULT);
	~RecvFile();
	/* TCP �����ļ� */
public:
	bool InitRecv();			//init the recv
	HANDLE StartRecv(std::string &error);
	static DWORD WINAPI RecvFileThreadProc(LPVOID lpParameter);

	BOOL WaitForMulThread(DWORD dwWaitSecond, string& strError);

public:
	u_short	m_serPort;							//����� �����˿�
	std::string m_savePath;						//�����ļ������ַ

	DWORD m_dwErrorCode;		// ִ�д�����
	HWND m_hNotifyWnd;			// �����ھ��
protected:
	/* �����ļ�
		1����������˼���socket���󶨷���˵�ַ���˿ڣ������������߳̽�������
		2�����յ������ӣ��µ�socket���ڽ����ļ� 
		3���ٴ����̼߳�������ǰ�߳� ���������ļ� */
	SOCKET m_serSocket;							//����� ����socket	
	sockaddr_in m_serAddr;						//����� ��ַ
	CArray<HANDLE, HANDLE&> m_recvThreads;		//��̬���飺�����ļ��߳̾����
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
	
	//�������ļ��б�
	CArray<std::string, std::string&> *m_fileList;
	CArray<SockFileStruct, SockFileStruct&> *m_finfoList;

	DWORD m_dwErrorCode;		// ִ�д�����
protected:
	/* �����ļ�
	1�������ļ������������̣߳����10����>10���ļ��ȴ���ǰ�̴߳�����ɺ󴴽�
	2��ÿ���̴߳���socket�����ӷ����ļ�Ŀ�ĵ�ַ destIP
	3�������ļ� */
	CArray<HANDLE, HANDLE&> m_sendThreads;		//��̬���飺�����ļ��߳̾����
	std::string m_destIP;								//�����ļ�Ŀ�ĵ�ַ
	u_short m_destPort;							//Ŀ�ĵ�ַ�Ķ˿ڣ�Ӧ�������һ�£�

	HWND m_hNotifyWnd;			// �����ھ��
};
