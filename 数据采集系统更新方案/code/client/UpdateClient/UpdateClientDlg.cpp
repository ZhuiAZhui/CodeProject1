
// UpdateClientDlg.cpp : 实现文件
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
extern CGroupTalk *g_pTalk;//声明有这个变量，前提是设置了全局

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CUpdateClientDlg 对话框



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


// CUpdateClientDlg 消息处理程序

BOOL CUpdateClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

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

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUpdateClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
	// TODO: 在此添加控件通知处理程序代码
	//激活参数输入框
	EnableControls(TRUE);
}


void CUpdateClientDlg::OnBnClickedButtonSave()
{
	// TODO: 在此添加控件通知处理程序代码
	// 锁定参数输入框
	EnableControls(FALSE);
	SaveConfig();
}


BOOL CUpdateClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽return 和 escape键退出消息
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
		
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CUpdateClientDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	theApp.ReleaseMutex();
	delete g_pTalk;
	CDialogEx::OnClose();
}

/************************************************************************/
/*CGroupTalk类的初始化及消息处理                                        */
/************************************************************************/
LRESULT CUpdateClientDlg::WMGROUPTALK(WPARAM wParam, LPARAM lParam)//CGroupTalk发送过来的消息响应
{
	if (wParam != 0)
	{
		::MessageBox(m_hWnd, (LPCTSTR)lParam, _T("出错！"), 0);
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
		UpdateLog(_T("文件接收完成！\r\n"));

		// 文件接收完成，紧接着解压至 原数采目录
		// 解压接收的更新包 至原数采系统目录
		CString sysPath = _T("");
		GetDlgItemText(IDC_EDIT_SYSPATH, sysPath);

		string winrarPath = m_curPath + WINRARNAME;
		string packagePath = (char*)lParam;// "C:\\Users\\fopen_sciss\\Downloads\\MFC-FileBrowser-master.zip";
		string destPath = PublicFunction::W2M(sysPath);//"G:\\测试目录\\MFC-FileBrowser-master\\";

													   // 简便化处理，默认是压缩包更新，压缩包名字由接收完成消息传递过来
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
	UpdateLog(_T("文件发送完成！\r\n"));
	return LRESULT(0);
}

void CUpdateClientDlg::HandleGroupMsg(HWND hDlg, GT_HDR *pHeader)//函数实现体
{
	if (NULL == pHeader)
	{
		return;
	}

	SOCKADDR_IN addr_ip;
	addr_ip.sin_addr.S_un.S_addr = pHeader->dwAddr;
	string strIP = inet_ntoa(addr_ip.sin_addr);
	string strUser = pHeader->szUser;

	// 客户端往组播地址发送消息
	DWORD dwAddress = 0;//inet_addr(strIP.c_str());

	CString strError = _T("");
	CString strLog = _T("");
	CString strLogHead = _T("");
	
	strLogHead.Format(_T(" From:%s, computer name:%s,operation:"), 
					PublicFunction::M2W(strIP.c_str()), PublicFunction::M2W(strUser.c_str()));

	switch (pHeader->gt_type)
	{
		case MT_JION:		// 新用户加入
		{
			// 显示给用户
			// 当前时间 + 消息来源 + 消息类型 + 消息
			strLog = strLogHead + _T("join.\r\n");
			break;
		}
		case MT_LEAVE:		// 用户离开
		{
			// 显示给用户
			strLog = strLogHead + _T("leave.\r\n");
			break;
		}
		case MT_MESG:		// 用户发送过来消息
		{
			strLog = strLogHead + _T("收到消息-") + PublicFunction::M2W(pHeader->data()) + _T("\r\n");
			UpdateLog(strLog);
			UpdateLog(_T("准备发送目录结构文件SystemPathInfo.txt...\r\n"));
			strLog = _T("");

			// 服务器请求获取 数采目录结构信息
			// 发送 SystemPathInfo.txt文件（ 设定数采目录时写入该文件）
			if (strcmp(pHeader->data(), "GetRemoteDir") == 0)
			{
				m_sendFileName = SYSFILEINFO;
				unsigned int recvthreadID = 0;
				HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
					this, 0, &recvthreadID);

				/*if (FALSE == SendUpdateFile(SYSFILEINFO, strError))
				{
					strLog += _T("发送更新标识文件Update.xml失败：") + strError + _T("\r\n");
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
				//		strLog.Format(_T("发送文件超时！错误码：%d\r\n"), filesend->m_dwErrorCode);
				//		//UpdateLog(_T("发送文件超时！"));
				//	}
				//	else
				//	{
				//		strLog = _T("发送文件超时！");
				//	}
				//	
				//}
				//else
				//{
				//	if (filesend->m_dwErrorCode != 0)
				//	{
				//		strLog.Format(_T("发送文件失败！错误码：%d\r\n"), filesend->m_dwErrorCode);
				//	}
				//}
			}
			break;
		}
		case MT_BEGIN_UPDATE_MESG:		// 客户端收到开始升级消息： 发送更新检查 消息和Update.xml
		{
			strLog = strLogHead + _T("收到消息-MT_BEGIN_UPDATE_MESG 更新准备...\r\n");
			UpdateLog(strLog);

			g_pTalk->SendText("", 0, MT_CHECK_MESG, dwAddress);
			UpdateLog(_T("'更新检查'消息MT_CHECK_MESG已发送.\r\n"));
			UpdateLog(_T("准备发送更新标识文件Update.xml...\r\n"));
			strLog = _T("");

			m_sendFileName = UPDATEXML;
			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
				this, 0, &recvthreadID);
			/*if (FALSE == SendUpdateFile(UPDATEXML, strError))
			{
				strLog += _T("发送更新标识文件Update.xml失败：") + strError + _T("\r\n");
			}*/
			break;
		}
		case MT_CHECKRES_MESG:		// 客户端收到 更新检查结果消息：1-需要升级；0-无需升级
		{
			if (strcmp(pHeader->data(), "1") == 0)
			{
				strLog = strLogHead + _T("收到消息-MT_CHECKRES_MESG， 本执行机需要更新升级！\r\n");
			}
			else if (strcmp(pHeader->data(), "0") == 0)
			{
				strLog = strLogHead + _T("收到消息-MT_CHECKRES_MESG， 本执行机不需要更新升级！\r\n");
			}
			else
			{
				strLog = strLogHead + _T("收到消息-MT_CHECKRES_MESG， 但标识错误，请联系维护人员！\r\n");
			}
			break;
		}
		case MT_ENDPRO_MESG:		// 客户端接收到 结束进程exe消息：1、结束进程；2、返回结束进程结果
		{
			strLog = strLogHead + _T("收到消息-MT_ENDPRO_MESG, 强制结束数采进程准备...\r\n");
			UpdateLog(strLog);
			EndProcess();
			UpdateLog(_T("结束数采进程成功！\r\n"));

			// 默认结束进程正常 0--结束exe失败； 1--结束进程exe成功
			g_pTalk->SendText("1", 2, MT_ENDPRORES_MESG, dwAddress);
			UpdateLog(_T("'结束数采进程'结果消息MT_ENDPRORES_MESG已发送.\r\n"));
			break;
		}
		case MT_UPDATE_MESG:		// 客户端接收到 更新文件消息：开始接收文件至临时目录，并返回接收结果
		{
			strLog = strLogHead + _T("收到消息-MT_UPDATE_MESG, 接收更新包文件准备...\r\n");
			UpdateLog(strLog);
			strLog = _T("");

			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvFileThreadProc,
				this, 0, &recvthreadID);

			/*if (FALSE == RecvUpdateFiles(strError))
			{
				strLog = _T("接收更新文件失败:") + strError + _T("\r\n");
				g_pTalk->SendText("0", 2, MT_UPDATERES_MESG, dwAddress);
			}
			else
			{
				g_pTalk->SendText("1", 2, MT_UPDATERES_MESG, dwAddress);
			}*/
			break;
		}
		case MT_BEGINPRO_MESG:		// 客户端接收到 开启进程exe消息：启动数采进程
		{
			strLog = strLogHead + _T("收到消息-MT_BEGINPRO_MESG, 重新启动数采进程准备...\r\n");
			UpdateLog(strLog);
			if (FALSE == BeginProcess(strError))
			{
				g_pTalk->SendText("0", 2, MT_BEGINPRORES_MESG, dwAddress);
				strLog.Format(_T("启动数采进程失败，错误信息：%s.已发送结果消息MT_BEGINPRORES_MESG 至服务端.\r\n"), strError);
			}
			else
			{
				g_pTalk->SendText("1", 2, MT_BEGINPRORES_MESG, dwAddress);
				strLog.Format(_T("启动数采进程成功！已发送结果消息MT_BEGINPRORES_MESG 至服务端.更新成功！\r\n"));
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

	// 保存到日志文件
	string strLogPath = m_curPath + UPDATELOG;
	if (false == PublicFunction::SaveLogToFile(strLogPath, PublicFunction::W2M(strCurTime + strLog)))
	{
		m_RichEditLog.SetSel(-1, -1);
		m_RichEditLog.ReplaceSel(strCurTime + _T("保存到日志文件失败.\r\n"));
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

	// 发送 Update.xml 文件超时时间1分钟
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 60 * 1000))
	{
		strerror.Format(_T("发送%s文件超时！"), PublicFunction::M2W(filename.c_str()));
		return FALSE;
	}


	// 释放资源
	delete fileSend;
	CString strResult = _T("");
	if (error == "")
	{
		strerror.Format(_T("发送%s文件成功！"), PublicFunction::M2W(filename.c_str()));
	}
	else
	{
		strerror.Format(_T("发送%s文件失败，原因：%s"), PublicFunction::M2W(filename.c_str()),
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
		// 接收文件可能是压缩文件 to do  接收文件超时时间应该由配置文件配置
		if (FALSE == fileRecv->WaitForMulThread(60 * 1000, error))
		{
			delete fileRecv;
			strerror = PublicFunction::M2W(error.c_str());//_T("接收文件超时！");
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
			strError.Format(_T("启动进程：strCommand失败，错误码%d.\r\n"),GetLastError());
			error += strError;
		}
	}
	return bRet;
}


void CUpdateClientDlg::InitGroupTalk()//初始化CGroupTalk
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
		strLog.Format(_T("发送文件%s失败：%s\r\n"), 
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
		strLog = _T("接收更新文件失败:") + strError + _T("\r\n");
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
	// TODO: 在此添加控件通知处理程序代码
	BROWSEINFO bi;            //BROWSEINFO结构体
	TCHAR Buffer[512] = _T("");
	TCHAR FullPath[512] = _T("");
	bi.hwndOwner = m_hWnd;   //m_hWnd你的程序主窗口
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer; //返回选择的目录名的缓冲区
	bi.lpszTitle = _T("选择临时存在目录"); //弹出的窗口的文字提示
	bi.ulFlags = BIF_RETURNONLYFSDIRS; //只返回目录。其他标志看MSDN
	bi.lpfn = NULL;                         //回调函数，有时很有用
	bi.lParam = 0;
	bi.iImage = 0;
	ITEMIDLIST* pidl = ::SHBrowseForFolder(&bi); //显示弹出窗口，ITEMIDLIST很重要
	if (::SHGetPathFromIDList(pidl, FullPath))     //在ITEMIDLIST中得到目录名的整个路径
	{
		SetDlgItemText(IDC_EDIT_TMPPATH, FullPath);
		//成功
	}
	else
	{
		MessageBox(_T("请选择临时存在目录！"));
		//失败
	}
}


void CUpdateClientDlg::OnBnClickedButtonSysdir()
{
	// TODO: 在此添加控件通知处理程序代码
	BROWSEINFO bi;            //BROWSEINFO结构体
	TCHAR Buffer[512] = _T("");
	TCHAR FullPath[512] = _T("");
	bi.hwndOwner = m_hWnd;   //m_hWnd你的程序主窗口
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer; //返回选择的目录名的缓冲区
	bi.lpszTitle = _T("选择数采系统本地目录"); //弹出的窗口的文字提示
	bi.ulFlags = BIF_RETURNONLYFSDIRS; //只返回目录。其他标志看MSDN
	bi.lpfn = NULL;                         //回调函数，有时很有用
	bi.lParam = 0;
	bi.iImage = 0;
	ITEMIDLIST* pidl = ::SHBrowseForFolder(&bi); //显示弹出窗口，ITEMIDLIST很重要
	if (::SHGetPathFromIDList(pidl, FullPath))     //在ITEMIDLIST中得到目录名的整个路径
	{
		SetDlgItemText(IDC_EDIT_TMPPATH, FullPath);
		//成功
	}
	else
	{
		MessageBox(_T("请选择数采系统本地目录！"));
		//失败
	}
}


void CUpdateClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
	//strTime.Format(_T("当前时间：%02d-%02d-%02d %02d:%02d:%02d\n"), ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
		//ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);

	strTime.Format(_T("当前时间：%s %s\n"), szDate, szTime);	
	strLocalInfo.Format(_T("本机地址：%s"), PublicFunction::M2W(m_localIP.c_str()));

	SetDlgItemText(IDC_STATIC_LOCALADDR, strTime + strLocalInfo);
	CDialogEx::OnTimer(nIDEvent);
}
