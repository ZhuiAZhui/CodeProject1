
// UpdateClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UpdateClient.h"
#include "UpdateClientDlg.h"
#include "afxdialogex.h"
#include "PublicFunction.h"
#include "FilesTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CUpdateClientApp theApp;
extern CGroupTalk *g_pTalk;//���������������ǰ����������ȫ��

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdateClientDlg �Ի���



CUpdateClientDlg::CUpdateClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPDATECLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdateClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EXE, m_ExeList);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_RichEditLog);
}

BEGIN_MESSAGE_MAP(CUpdateClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CUpdateClientDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CUpdateClientDlg::OnBnClickedButtonSave)
	ON_MESSAGE(WM_GROUPTALK, WMGROUPTALK)
	ON_MESSAGE(FILE_SEND_SUCCESS, WMSendFile)
	ON_MESSAGE(FILE_RECEVIE_OVER, WMRecvFile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_TMPDIR, &CUpdateClientDlg::OnBnClickedButtonTmpdir)
	ON_BN_CLICKED(IDC_BUTTON_SYSDIR, &CUpdateClientDlg::OnBnClickedButtonSysdir)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CUpdateClientDlg ��Ϣ�������

BOOL CUpdateClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	m_curPath = PublicFunction::GetCurrentRunPath();

	char szHostName[100] = { 0 };
	gethostname(szHostName, sizeof(szHostName));

	hostent *hn;
	hn = gethostbyname(szHostName);
	m_localIP = inet_ntoa(*(struct in_addr*)hn->h_addr_list[0]);

	SetTimer(WM_TIMER, 1000, NULL);

	LoadConfig();

	CString strSavePath = PublicFunction::M2W((m_curPath + SYSFILEINFO).c_str());
	PublicFunction::InitSystemFileInfo(PublicFunction::M2W(m_sysPath.c_str()) , strSavePath);

	EnableControls(FALSE);
	InitGroupTalk();

	//ShowWindow(SW_MINIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CUpdateClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUpdateClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CUpdateClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUpdateClientDlg::EnableControls(BOOL bState)
{
	GetDlgItem(IDC_SERVER_IP)->EnableWindow(bState);
	GetDlgItem(IDC_EDIT_SERPORT)->EnableWindow(bState);
	GetDlgItem(IDC_EDIT_TMPPATH)->EnableWindow(bState);
	GetDlgItem(IDC_EDIT_SYSPATH)->EnableWindow(bState);
	GetDlgItem(IDC_LIST_EXE)->EnableWindow(bState);
	GetDlgItem(IDC_BUTTON_TMPDIR)->EnableWindow(bState);
	GetDlgItem(IDC_BUTTON_SYSDIR)->EnableWindow(bState);
}

void CUpdateClientDlg::LoadConfig()
{
	CString strSerIP(_T("")), tmpPath(_T("")), sysPath(_T(""));
	unsigned short usPort = 0;
	unsigned short usExeCount = 0;
	CString strExeName = _T("");

	TCHAR szBuffer[MAX_PATH] = { 0 };
	CString strExePath = PublicFunction::M2W((m_curPath+ CONFIG_PATH).c_str()) ;

	GetPrivateProfileStringW(_T("ServerInfo"), _T("IP"), _T(""), szBuffer, MAX_PATH, strExePath);
	strSerIP = szBuffer;
	m_serIP = PublicFunction::W2M(strSerIP);

	usPort = GetPrivateProfileInt(_T("ServerInfo"), _T("PORT"), 0, strExePath);
	m_port = usPort;

	GetPrivateProfileString(_T("LoaclConfig"), _T("TmpPath"), _T(""), szBuffer, MAX_PATH, strExePath);
	tmpPath = szBuffer;
	m_tmpPath = PublicFunction::W2M(tmpPath);

	GetPrivateProfileString(_T("LoaclConfig"), _T("SystemPath"), _T(""), szBuffer, MAX_PATH, strExePath);
	sysPath = szBuffer;
	m_sysPath = PublicFunction::W2M(sysPath);

	usExeCount = GetPrivateProfileInt(_T("LoaclConfig"), _T("count"), 0, strExePath);
	for (int index = 1;index <= usExeCount;index++)
	{
		CString sectionName = _T("");
		sectionName.Format(_T("ListenExeName%d"), index);
		GetPrivateProfileString(sectionName, _T("ExeName"), _T(""), szBuffer, MAX_PATH, strExePath);
		strExeName = szBuffer;

		m_ExeList.InsertItem(index - 1, strExeName);
	}

	SetDlgItemText(IDC_SERVER_IP, strSerIP);
	SetDlgItemInt(IDC_EDIT_SERPORT, usPort);

	SetDlgItemText(IDC_EDIT_TMPPATH, tmpPath);
	SetDlgItemText(IDC_EDIT_SYSPATH, sysPath);
}

void CUpdateClientDlg::SaveConfig()
{
	CString strSerIP(_T("")), tmpPath(_T("")), sysPath(_T(""));
	unsigned short usPort = 0;
	unsigned short usExeCount = 0;
	CString strExeName = _T("");
	CString strTmp = _T("");
	CString strExePath = PublicFunction::M2W((m_curPath+CONFIG_PATH).c_str());

	GetDlgItemText(IDC_SERVER_IP, strSerIP);
	usPort = GetDlgItemInt(IDC_EDIT_SERPORT);

	m_serIP = PublicFunction::W2M(strSerIP);
	m_port = usPort;

	GetDlgItemText(IDC_EDIT_TMPPATH, tmpPath);
	GetDlgItemText(IDC_EDIT_SYSPATH, sysPath);

	WritePrivateProfileString(_T("ServerInfo"), _T("IP"), strSerIP, strExePath);
	strTmp.Format(_T("%d"),usPort);
	WritePrivateProfileString(_T("ServerInfo"), _T("PORT"), strTmp, strExePath);

	WritePrivateProfileString(_T("LoaclConfig"), _T("TmpPath"), tmpPath, strExePath);
	WritePrivateProfileString(_T("LoaclConfig"), _T("SystemPath"), sysPath, strExePath);
	m_tmpPath = PublicFunction::W2M(tmpPath);
	m_sysPath = PublicFunction::W2M(sysPath);

	usExeCount = m_ExeList.GetItemCount();
	strTmp.Format(_T("%d"), usExeCount);
	WritePrivateProfileString(_T("LoaclConfig"), _T("count"), strTmp, strExePath);
	for (int index = 1;index <= usExeCount;index++)
	{
		strTmp = m_ExeList.GetItemText(index-1, 0);
		CString sectionName = _T("");
		sectionName.Format(_T("ListenExeName%d"), index);
		WritePrivateProfileString(sectionName, _T("ExeName"), strTmp, strExePath);
	}
}

void CUpdateClientDlg::OnBnClickedButtonEdit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//������������
	EnableControls(TRUE);
}


void CUpdateClientDlg::OnBnClickedButtonSave()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// �������������
	EnableControls(FALSE);
	SaveConfig();
}


BOOL CUpdateClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����return �� escape���˳���Ϣ
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
		
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CUpdateClientDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	theApp.ReleaseMutex();
	delete g_pTalk;
	CDialogEx::OnClose();
}

/************************************************************************/
/*CGroupTalk��ĳ�ʼ������Ϣ����                                        */
/************************************************************************/
LRESULT CUpdateClientDlg::WMGROUPTALK(WPARAM wParam, LPARAM lParam)//CGroupTalk���͹�������Ϣ��Ӧ
{
	if (wParam != 0)
	{
		::MessageBox(m_hWnd, (LPCTSTR)lParam, _T("����"), 0);
	}
	else
	{
		HandleGroupMsg(m_hWnd, (GT_HDR*)lParam);
	}
	return 0;
}

LRESULT CUpdateClientDlg::WMRecvFile(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case FILE_TRANSFER_SUCESS:
	{
		UpdateLog(_T("�ļ�������ɣ�\r\n"));

		// �ļ�������ɣ������Ž�ѹ�� ԭ����Ŀ¼
		// ��ѹ���յĸ��°� ��ԭ����ϵͳĿ¼
		CString sysPath = _T("");
		GetDlgItemText(IDC_EDIT_SYSPATH, sysPath);

		string winrarPath = m_curPath + WINRARNAME;
		string packagePath = (char*)lParam;// "C:\\Users\\fopen_sciss\\Downloads\\MFC-FileBrowser-master.zip";
		string destPath = PublicFunction::W2M(sysPath);//"G:\\����Ŀ¼\\MFC-FileBrowser-master\\";

													   // ��㻯����Ĭ����ѹ�������£�ѹ���������ɽ��������Ϣ���ݹ���
		int iRet = PublicFunction::DecompressPackage(winrarPath, packagePath, destPath);
		break;
	}
	case FILE_TRANSFER_FAILED:
	{
		CString strBuffer = PublicFunction::M2W((char*)lParam);
		UpdateLog(strBuffer);
		break;
	}
	}
	return LRESULT(0);
}

LRESULT CUpdateClientDlg::WMSendFile(WPARAM wParam, LPARAM lParam)
{
	UpdateLog(_T("�ļ�������ɣ�\r\n"));
	return LRESULT(0);
}

void CUpdateClientDlg::HandleGroupMsg(HWND hDlg, GT_HDR *pHeader)//����ʵ����
{
	if (NULL == pHeader)
	{
		return;
	}

	SOCKADDR_IN addr_ip;
	addr_ip.sin_addr.S_un.S_addr = pHeader->dwAddr;
	string strIP = inet_ntoa(addr_ip.sin_addr);
	string strUser = pHeader->szUser;

	// �ͻ������鲥��ַ������Ϣ
	DWORD dwAddress = 0;//inet_addr(strIP.c_str());

	CString strError = _T("");
	CString strLog = _T("");
	CString strLogHead = _T("");
	
	strLogHead.Format(_T(" From:%s, computer name:%s,operation:"), 
					PublicFunction::M2W(strIP.c_str()), PublicFunction::M2W(strUser.c_str()));

	switch (pHeader->gt_type)
	{
		case MT_JION:		// ���û�����
		{
			// ��ʾ���û�
			// ��ǰʱ�� + ��Ϣ��Դ + ��Ϣ���� + ��Ϣ
			strLog = strLogHead + _T("join.\r\n");
			break;
		}
		case MT_LEAVE:		// �û��뿪
		{
			// ��ʾ���û�
			strLog = strLogHead + _T("leave.\r\n");
			break;
		}
		case MT_MESG:		// �û����͹�����Ϣ
		{
			strLog = strLogHead + _T("�յ���Ϣ-") + PublicFunction::M2W(pHeader->data()) + _T("\r\n");
			UpdateLog(strLog);
			UpdateLog(_T("׼������Ŀ¼�ṹ�ļ�SystemPathInfo.txt...\r\n"));
			strLog = _T("");

			// �����������ȡ ����Ŀ¼�ṹ��Ϣ
			// ���� SystemPathInfo.txt�ļ��� �趨����Ŀ¼ʱд����ļ���
			if (strcmp(pHeader->data(), "GetRemoteDir") == 0)
			{
				m_sendFileName = SYSFILEINFO;
				unsigned int recvthreadID = 0;
				HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
					this, 0, &recvthreadID);

				/*if (FALSE == SendUpdateFile(SYSFILEINFO, strError))
				{
					strLog += _T("���͸��±�ʶ�ļ�Update.xmlʧ�ܣ�") + strError + _T("\r\n");
				}*/
				/*FileSend *filesend = new FileSend(m_hWnd, m_serIP);
				string error = "";
				TCHAR szError[MAX_PATH] = { 0 };
				if (false == filesend->AddFileToSend(m_curPath + SYSFILEINFO, szError, MAX_PATH))
				{
					strLog = szError;
					strLog += _T("\r\n");
					break;
				}
				HANDLE handle = filesend->StartSend(error);*/

				//if (WAIT_TIMEOUT == WaitForSingleObject(handle, 60 * 1000))
				//{
				//	if (filesend->m_dwErrorCode != 0)
				//	{
				//		strLog.Format(_T("�����ļ���ʱ�������룺%d\r\n"), filesend->m_dwErrorCode);
				//		//UpdateLog(_T("�����ļ���ʱ��"));
				//	}
				//	else
				//	{
				//		strLog = _T("�����ļ���ʱ��");
				//	}
				//	
				//}
				//else
				//{
				//	if (filesend->m_dwErrorCode != 0)
				//	{
				//		strLog.Format(_T("�����ļ�ʧ�ܣ������룺%d\r\n"), filesend->m_dwErrorCode);
				//	}
				//}
			}
			break;
		}
		case MT_BEGIN_UPDATE_MESG:		// �ͻ����յ���ʼ������Ϣ�� ���͸��¼�� ��Ϣ��Update.xml
		{
			strLog = strLogHead + _T("�յ���Ϣ-MT_BEGIN_UPDATE_MESG ����׼��...\r\n");
			UpdateLog(strLog);

			g_pTalk->SendText("", 0, MT_CHECK_MESG, dwAddress);
			UpdateLog(_T("'���¼��'��ϢMT_CHECK_MESG�ѷ���.\r\n"));
			UpdateLog(_T("׼�����͸��±�ʶ�ļ�Update.xml...\r\n"));
			strLog = _T("");

			m_sendFileName = UPDATEXML;
			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
				this, 0, &recvthreadID);
			/*if (FALSE == SendUpdateFile(UPDATEXML, strError))
			{
				strLog += _T("���͸��±�ʶ�ļ�Update.xmlʧ�ܣ�") + strError + _T("\r\n");
			}*/
			break;
		}
		case MT_CHECKRES_MESG:		// �ͻ����յ� ���¼������Ϣ��1-��Ҫ������0-��������
		{
			if (strcmp(pHeader->data(), "1") == 0)
			{
				strLog = strLogHead + _T("�յ���Ϣ-MT_CHECKRES_MESG�� ��ִ�л���Ҫ����������\r\n");
			}
			else if (strcmp(pHeader->data(), "0") == 0)
			{
				strLog = strLogHead + _T("�յ���Ϣ-MT_CHECKRES_MESG�� ��ִ�л�����Ҫ����������\r\n");
			}
			else
			{
				strLog = strLogHead + _T("�յ���Ϣ-MT_CHECKRES_MESG�� ����ʶ��������ϵά����Ա��\r\n");
			}
			break;
		}
		case MT_ENDPRO_MESG:		// �ͻ��˽��յ� ��������exe��Ϣ��1���������̣�2�����ؽ������̽��
		{
			strLog = strLogHead + _T("�յ���Ϣ-MT_ENDPRO_MESG, ǿ�ƽ������ɽ���׼��...\r\n");
			UpdateLog(strLog);
			EndProcess();
			UpdateLog(_T("�������ɽ��̳ɹ���\r\n"));

			// Ĭ�Ͻ����������� 0--����exeʧ�ܣ� 1--��������exe�ɹ�
			g_pTalk->SendText("1", 2, MT_ENDPRORES_MESG, dwAddress);
			UpdateLog(_T("'�������ɽ���'�����ϢMT_ENDPRORES_MESG�ѷ���.\r\n"));
			break;
		}
		case MT_UPDATE_MESG:		// �ͻ��˽��յ� �����ļ���Ϣ����ʼ�����ļ�����ʱĿ¼�������ؽ��ս��
		{
			strLog = strLogHead + _T("�յ���Ϣ-MT_UPDATE_MESG, ���ո��°��ļ�׼��...\r\n");
			UpdateLog(strLog);
			strLog = _T("");

			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvFileThreadProc,
				this, 0, &recvthreadID);

			/*if (FALSE == RecvUpdateFiles(strError))
			{
				strLog = _T("���ո����ļ�ʧ��:") + strError + _T("\r\n");
				g_pTalk->SendText("0", 2, MT_UPDATERES_MESG, dwAddress);
			}
			else
			{
				g_pTalk->SendText("1", 2, MT_UPDATERES_MESG, dwAddress);
			}*/
			break;
		}
		case MT_BEGINPRO_MESG:		// �ͻ��˽��յ� ��������exe��Ϣ���������ɽ���
		{
			strLog = strLogHead + _T("�յ���Ϣ-MT_BEGINPRO_MESG, �����������ɽ���׼��...\r\n");
			UpdateLog(strLog);
			if (FALSE == BeginProcess(strError))
			{
				g_pTalk->SendText("0", 2, MT_BEGINPRORES_MESG, dwAddress);
				strLog.Format(_T("�������ɽ���ʧ�ܣ�������Ϣ��%s.�ѷ��ͽ����ϢMT_BEGINPRORES_MESG �������.\r\n"), strError);
			}
			else
			{
				g_pTalk->SendText("1", 2, MT_BEGINPRORES_MESG, dwAddress);
				strLog.Format(_T("�������ɽ��̳ɹ����ѷ��ͽ����ϢMT_BEGINPRORES_MESG �������.���³ɹ���\r\n"));
			}		
			break;
		}	
	}
	UpdateLog(strLog);
}

void CUpdateClientDlg::UpdateLog(CString strLog)
{
	SYSTEMTIME *tm = NULL;
	TCHAR szTime[MAX_PATH] = { 0 };
	TCHAR szDate[MAX_PATH] = { 0 };
	CString strCurTime = _T("");

	if (strLog.IsEmpty())
	{
		return;
	}

	GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, tm,
		_T("hh:mm:ss"), szTime, MAX_PATH);
	GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_AUTOLAYOUT, tm, _T("yyyy-MM-dd"),
		szDate, MAX_PATH, NULL);
	strCurTime.Format(_T("%s %s "), szDate, szTime);

	m_RichEditLog.SetSel(-1, -1);
	m_RichEditLog.ReplaceSel(strCurTime + strLog);
	UpdateData(TRUE);

	// ���浽��־�ļ�
	string strLogPath = m_curPath + UPDATELOG;
	if (false == PublicFunction::SaveLogToFile(strLogPath, PublicFunction::W2M(strCurTime + strLog)))
	{
		m_RichEditLog.SetSel(-1, -1);
		m_RichEditLog.ReplaceSel(strCurTime + _T("���浽��־�ļ�ʧ��.\r\n"));
		UpdateData(TRUE);
	}
}

BOOL CUpdateClientDlg::SendUpdateFile(string filename, CString &strerror)
{
	FileSend *fileSend = new FileSend(m_hWnd, m_serIP, m_port);
	string error = "";
	//to do, add update.xml
	TCHAR szError[MAX_PATH] = { 0 };
	if (false == fileSend->AddFileToSend(m_curPath + filename, szError, MAX_PATH))
	{
		strerror = szError;
		return FALSE;
	}
	HANDLE handle = fileSend->StartSend(error);
	if (handle == INVALID_HANDLE_VALUE)
	{
		delete fileSend;
		return FALSE;
	}

	// ���� Update.xml �ļ���ʱʱ��1����
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 60 * 1000))
	{
		strerror.Format(_T("����%s�ļ���ʱ��"), PublicFunction::M2W(filename.c_str()));
		return FALSE;
	}


	// �ͷ���Դ
	delete fileSend;
	CString strResult = _T("");
	if (error == "")
	{
		strerror.Format(_T("����%s�ļ��ɹ���"), PublicFunction::M2W(filename.c_str()));
	}
	else
	{
		strerror.Format(_T("����%s�ļ�ʧ�ܣ�ԭ��%s"), PublicFunction::M2W(filename.c_str()),
			PublicFunction::M2W(error.c_str()));
		return FALSE;
	}

	return TRUE;
}

BOOL CUpdateClientDlg::RecvUpdateFiles(CString &strerror)
{
	string error = "";
	RecvFile *fileRecv = new RecvFile(m_hWnd, m_tmpPath, m_port);
	HANDLE handle = fileRecv->StartRecv(error);

	if (error == "")
	{
		// �����ļ�������ѹ���ļ� to do  �����ļ���ʱʱ��Ӧ���������ļ�����
		if (FALSE == fileRecv->WaitForMulThread(60 * 1000, error))
		{
			delete fileRecv;
			strerror = PublicFunction::M2W(error.c_str());//_T("�����ļ���ʱ��");
			return FALSE;
		}
	}
	delete fileRecv;
	return TRUE;
}

void CUpdateClientDlg::EndProcess()
{
	u_short usExeCount = 0;
	CString strExeName = _T("");
	CString strCommand = _T("");

	usExeCount = m_ExeList.GetItemCount();
	for (int index = 1;index <= usExeCount;index++)
	{
		strExeName = m_ExeList.GetItemText(index - 1, 0);
		strCommand.Format(_T("TASKKILL /IM %s /T /F"), strExeName);
		PublicFunction::ExecuteSysCmd(PublicFunction::W2M(strCommand));
	}
}

BOOL CUpdateClientDlg::BeginProcess(CString &error)
{
	u_short usExeCount = 0;
	CString strExeName = _T("");
	string strCommand = "";
	CString strError = _T("");
	BOOL bRet = TRUE;

	usExeCount = m_ExeList.GetItemCount();
	for (int index = 1;index <= usExeCount;index++)
	{
		strExeName = m_ExeList.GetItemText(index - 1, 0);
		strCommand = m_sysPath + "\\"+ PublicFunction::W2M(strExeName);
		if (FALSE == PublicFunction::BeginProcess(m_sysPath + "\\", strCommand))
		{
			bRet = FALSE;
			strError.Format(_T("�������̣�strCommandʧ�ܣ�������%d.\r\n"),GetLastError());
			error += strError;
		}
	}
	return bRet;
}


void CUpdateClientDlg::InitGroupTalk()//��ʼ��CGroupTalk
{
	string strMultiAddr = "";
	u_short usMultiPort = 0;
	char szBuffer[MAX_PATH] = { 0 };
	string strExePath = m_curPath + CONFIG_PATH;

	GetPrivateProfileStringA("BoradCastInfo", "IP", "", szBuffer, MAX_PATH, strExePath.c_str());
	strMultiAddr = szBuffer;

	usMultiPort = GetPrivateProfileIntA("BoradCastInfo", "PORT", 0, strExePath.c_str());
	if (strMultiAddr == "" || usMultiPort == 0)
	{
		g_pTalk = new CGroupTalk(m_hWnd, ::inet_addr(GROUP_ADDRESS), GROUP_PORT);
	}
	else
	{
		g_pTalk = new CGroupTalk(m_hWnd, ::inet_addr(szBuffer), usMultiPort);
	}
}

DWORD CUpdateClientDlg::SendFileThreadProc(LPVOID lpParameter)
{
	CUpdateClientDlg *pObj = static_cast<CUpdateClientDlg*>(lpParameter);
	if (pObj == NULL)
	{
		return 0;
	}

	CString strError = _T("");
	CString strLog = _T("");
	if (FALSE == pObj->SendUpdateFile(pObj->m_sendFileName, strError))
	{
		strLog.Format(_T("�����ļ�%sʧ�ܣ�%s\r\n"), 
			PublicFunction::M2W(pObj->m_sendFileName.c_str()), strError );
	}
	return 0;
}

DWORD CUpdateClientDlg::RecvFileThreadProc(LPVOID lpParameter)
{
	CUpdateClientDlg *pObj = static_cast<CUpdateClientDlg*>(lpParameter);
	if (pObj == NULL)
	{
		return 0;
	}

	CString strError = _T("");
	CString strLog = _T("");
	if (FALSE == pObj->RecvUpdateFiles(strError))
	{
		strLog = _T("���ո����ļ�ʧ��:") + strError + _T("\r\n");
		g_pTalk->SendText("0", 2, MT_UPDATERES_MESG, 0);
	}
	else
	{
		g_pTalk->SendText("1", 2, MT_UPDATERES_MESG, 0);
	}
	return 0;
}

void CUpdateClientDlg::OnBnClickedButtonTmpdir()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BROWSEINFO bi;            //BROWSEINFO�ṹ��
	TCHAR Buffer[512] = _T("");
	TCHAR FullPath[512] = _T("");
	bi.hwndOwner = m_hWnd;   //m_hWnd��ĳ���������
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer; //����ѡ���Ŀ¼���Ļ�����
	bi.lpszTitle = _T("ѡ����ʱ����Ŀ¼"); //�����Ĵ��ڵ�������ʾ
	bi.ulFlags = BIF_RETURNONLYFSDIRS; //ֻ����Ŀ¼��������־��MSDN
	bi.lpfn = NULL;                         //�ص���������ʱ������
	bi.lParam = 0;
	bi.iImage = 0;
	ITEMIDLIST* pidl = ::SHBrowseForFolder(&bi); //��ʾ�������ڣ�ITEMIDLIST����Ҫ
	if (::SHGetPathFromIDList(pidl, FullPath))     //��ITEMIDLIST�еõ�Ŀ¼��������·��
	{
		SetDlgItemText(IDC_EDIT_TMPPATH, FullPath);
		//�ɹ�
	}
	else
	{
		MessageBox(_T("��ѡ����ʱ����Ŀ¼��"));
		//ʧ��
	}
}


void CUpdateClientDlg::OnBnClickedButtonSysdir()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BROWSEINFO bi;            //BROWSEINFO�ṹ��
	TCHAR Buffer[512] = _T("");
	TCHAR FullPath[512] = _T("");
	bi.hwndOwner = m_hWnd;   //m_hWnd��ĳ���������
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer; //����ѡ���Ŀ¼���Ļ�����
	bi.lpszTitle = _T("ѡ������ϵͳ����Ŀ¼"); //�����Ĵ��ڵ�������ʾ
	bi.ulFlags = BIF_RETURNONLYFSDIRS; //ֻ����Ŀ¼��������־��MSDN
	bi.lpfn = NULL;                         //�ص���������ʱ������
	bi.lParam = 0;
	bi.iImage = 0;
	ITEMIDLIST* pidl = ::SHBrowseForFolder(&bi); //��ʾ�������ڣ�ITEMIDLIST����Ҫ
	if (::SHGetPathFromIDList(pidl, FullPath))     //��ITEMIDLIST�еõ�Ŀ¼��������·��
	{
		SetDlgItemText(IDC_EDIT_TMPPATH, FullPath);
		//�ɹ�
	}
	else
	{
		MessageBox(_T("��ѡ������ϵͳ����Ŀ¼��"));
		//ʧ��
	}
}


void CUpdateClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CString strTime = _T("");
	CString strLocalInfo = _T("");

	SYSTEMTIME *tm = NULL;
	TCHAR szTime[MAX_PATH] = { 0 };
	TCHAR szDate[MAX_PATH] = { 0 };

	GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, tm, 
		_T("hh:mm:ss"), szTime, MAX_PATH);
	GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_AUTOLAYOUT, tm, _T("yyyy-MM-dd"),
		szDate, MAX_PATH, NULL);

	//time_t lt;
	//time(&lt);
	//struct tm *ptminfo = localtime(&lt);	
	//strTime.Format(_T("��ǰʱ�䣺%02d-%02d-%02d %02d:%02d:%02d\n"), ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
		//ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);

	strTime.Format(_T("��ǰʱ�䣺%s %s\n"), szDate, szTime);	
	strLocalInfo.Format(_T("������ַ��%s"), PublicFunction::M2W(m_localIP.c_str()));

	SetDlgItemText(IDC_STATIC_LOCALADDR, strTime + strLocalInfo);
	CDialogEx::OnTimer(nIDEvent);
}
