#include "stdafx.h"
#include "UpdateServer.h"
#include <Shlobj.h>		// BROWSEINFO 结构体需要

#include "PublicFunction.h"

#define WM_ADDLISTITEM WM_USER + 50
#define WM_MODIFYLISTITEM WM_USER + 51
#define WM_DELETELISTITEM WM_USER + 52
#define WM_GROUPTALK	WM_USER + 105    


#define EXECUTE_RESULT_TYPE_SUCCESS		"1"
#define EXECUTE_RESULT_TYPE_FAILED		"0"

/*
* 存放第1列数据
*/
std::vector<CDuiString> g_host;
/*
* 存放第2列数据
*/
std::vector<u_short> g_fileport;

/*
* 存放第3列数据
*/
std::vector<u_short> g_msgport;

// 存放本地目录文件信息
std::vector<FileInfo> g_localFile;

// 存放 执行机目录文件信息
std::vector<FileInfo> g_remoteFile;

// UDP 消息
CGroupTalk* g_pTalk;

CUpdateServer::CUpdateServer()
{
	InitSocket();//加载2.2版本WinSock库
}


CUpdateServer::~CUpdateServer()
{
}

void CUpdateServer::Init()
{
	string strMultiAddr = "";
	string strRemoteIP = "";
	u_short usMultiPort = 0;
	UINT uiExclueCount = 0;
	string strUpdatePath = "";
	char szBuffer[MAX_PATH] = { 0 };

	m_curPath = PublicFunction::GetCurrentRunPath();
	string configPath = m_curPath + CONFIG_NAME;
	string xmlPath = m_curPath + UPDATEXML;

	// 获取组播信息
	GetPrivateProfileStringA(("BoradCastInfo"), ("IP"), (""), szBuffer, MAX_PATH, configPath.c_str());
	strMultiAddr = szBuffer;
	usMultiPort = GetPrivateProfileIntA(("BoradCastInfo"), ("PORT"), GROUP_PORT, configPath.c_str());

	// 获取‘例外文件’列表
	uiExclueCount = GetPrivateProfileIntA(("UpdateFileInfo"), ("ExclusionCount"), 0, configPath.c_str());
	for (UINT index = 1;index <= uiExclueCount;index++)
	{
		char szSection[20] = { 0 };
		sprintf_s(szSection, "ExclusionFile%d", index);
		string strExclueFile = "";
		GetPrivateProfileStringA(szSection, ("FileName"), (""), szBuffer, MAX_PATH, configPath.c_str());
		strExclueFile = szBuffer;
		if (strExclueFile != "")
		{
			m_ExcludeFile.push_back(strExclueFile);
		}
	}
	 
	// 获取更新目录，并初始化其xml文件（文件名，版本，MD5值组成的xml文件）
	GetPrivateProfileStringA(("UpdateFileInfo"), ("path"), (""), szBuffer, MAX_PATH, configPath.c_str());
	strUpdatePath = szBuffer;
	PublicFunction::InitFolderToXml(strUpdatePath, xmlPath, m_ExcludeFile);

	// 获取 执行机IP及文件传输端口，用于文件传输
	int iCount = GetPrivateProfileIntA(("RemoteList"), ("count"), 0, configPath.c_str());
	for (int index = 1;index <= iCount;index++)
	{
		char szSection[10] = { 0 };
		sprintf_s(szSection, "Remote%d", index);
		GetPrivateProfileStringA(szSection, ("IP"), (""), szBuffer, MAX_PATH, configPath.c_str());
		strRemoteIP = szBuffer;
		u_short usFPort = GetPrivateProfileIntA(szSection, ("fileport"), 0, configPath.c_str());

		// 创建对应执行机的文件夹：以IP命名，存在其更新的过程文件
		string strRemoteFolderPath = m_curPath + "\\" +strRemoteIP;
		if (false == PublicFunction::IsDirExisted(strRemoteFolderPath.c_str()))
		{
			if (NULL == CreateDirectoryA(strRemoteFolderPath.c_str(), NULL))
			{
				char szError[MAX_PATH] = { 0 };
				sprintf_s(szError, "创建文件%s失败，错误码：%d\r\n",strRemoteFolderPath.c_str(), GetLastError());
				MessageBoxA(NULL, szError, ("Error"), MB_OK);
			}
		}
		CDuiString strWideIP = PublicFunction::M2W(strRemoteIP.c_str());
		g_host.push_back(strWideIP);
		g_fileport.push_back(usFPort);
		g_msgport.push_back(usMultiPort);
	}

	HWND handle = GetHWND();
	while (::GetParent(handle) != NULL) handle = ::GetParent(handle);

	if (strMultiAddr == "" || usMultiPort == 0)
	{
		g_pTalk = new CGroupTalk(GetHWND(), ::inet_addr(GROUP_ADDRESS), GROUP_PORT, MSG_FROM_SERVER);
	}
	else
	{
		g_pTalk = new CGroupTalk(GetHWND(), ::inet_addr(strMultiAddr.c_str()), usMultiPort, MSG_FROM_SERVER);
	}

	m_pRemoteList = static_cast<CListUI*>(m_pm.FindControl(_T("machineList")));
	m_pHostEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("host")));
	m_pFileportEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("fileport")));
	m_pMsgportEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("msgport")));

	m_pRemoteDirIP = static_cast<CEditUI*>(m_pm.FindControl(_T("remotedir")));
	m_pRemoteDirList = static_cast<CListUI*>(m_pm.FindControl(_T("remotefilelist")));

	m_pLocalDirList = static_cast<CListUI*>(m_pm.FindControl(_T("localfilelist")));
	m_pLocalDir = static_cast<CEditUI*>(m_pm.FindControl(_T("localdir")));

	CVerticalLayoutUI*  pui = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("layer1-top")));
	if (pui) {
		pui->SetVisible(true);

	}

	for (int index = 0;index < g_host.size(); index++)
	{
		int count = m_pRemoteList->GetCount();
		m_pRemoteList->SetTextCallback(this);

		CListTextElementUI* pListElement = new CListTextElementUI;
		pListElement->SetTag(count);
		if (pListElement != NULL)
		{
			::PostMessage(GetHWND(), WM_ADDLISTITEM, RemoteList, (LPARAM)pListElement);
		}
	}
}

void CUpdateServer::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("windowinit"))
		OnPrepare(msg);
	else if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("closebtn"))
		{
			// 释放控件，发送离开消息
			delete g_pTalk;
			PostQuitMessage(0);
			return;
		}
		else if (msg.pSender->GetName() == _T("minbtn"))
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if (msg.pSender->GetName() == _T("maxbtn"))
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return;
		}
		else if (msg.pSender->GetName() == _T("restorebtn"))
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return;
		}
		else if (msg.pSender->GetName() == _T("addbtn"))
		{
			OnAddRemote();
		}
		else if (msg.pSender->GetName() == _T("modifybtn"))
		{
			OnModifyRemote();
		}
		else if (msg.pSender->GetName() == _T("deletebtn"))
		{
			OnDeleteRemote();
			//msg.pSender->SetTag(NULL);
		}
		else if (msg.pSender->GetName() == _T("localdirtn"))
		{
			CDuiString strLocalDir = SelectDir();
			m_pLocalDir->SetText(strLocalDir);
			if (strLocalDir.GetLength() != 0)
			{
				InitLocalDirToList(strLocalDir);
			}
		}
		else if (msg.pSender->GetName() == _T("updatebtn"))
		{
			// 点击 自动更新按钮
			OnUpdateSystem();
		}
		else if (msg.pSender->GetName() == _T("comparebtn"))
		{
			g_pTalk->SendMessageTest();
		}
	}
	else if (msg.sType == _T("return"))
	{
		if (msg.pSender->GetName() == _T("remotedir"))
		{
			// to do
			MessageBox(NULL, _T("click enter"), _T("remotedir"), 0);
			return;
		}
		// 将本地站点的文件夹信息显示出来
		else if (msg.pSender->GetName() == _T("localdir"))
		{
			CDuiString strLocalDir = m_pLocalDir->GetText();
			string strPath = PublicFunction::W2M(strLocalDir.GetData());
			if (strLocalDir.GetLength() != 0 && true == PublicFunction::IsDirExisted(strPath) )
			{
				InitLocalDirToList(strLocalDir);
			}
			else
			{
				MessageBox(NULL, _T("请输入合法路径"), _T("error"), MB_OK);
			}
			
		}
	}
	/*else if (msg.sType == _T("setfocus"))
	{
	}*/
	else if (msg.sType == _T("itemclick"))
	{
		CListTextElementUI* pListElement = (CListTextElementUI*)msg.pSender;
		CDuiString strTmp = _T("");

		// 点击执行机表数据，显示到edit框中
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{
			int iSel = m_pRemoteList->GetCurSel();	// 此行代码，有bug。点击事件需要点击两次，界面才有响应正确的行
			if (iSel < 0) return;
			//int iIndex = msg.pSender->GetTag();    // 此行代码，在删除行场景下有bug：
													 // 删除首行后再进行点击首行，tag=1，不为0
			m_pHostEdit->SetText(g_host[iSel]);
			strTmp.Format(_T("%d"), g_fileport[iSel]);
			m_pFileportEdit->SetText(strTmp);
			strTmp.Format(_T("%d"), g_msgport[iSel]);
			m_pMsgportEdit->SetText(strTmp);
		}
	}
	else if (msg.sType == _T("itemactivate"))
	{
		CListTextElementUI* pListElement = (CListTextElementUI*)msg.pSender;

		// 双击执行机表数据，显示对应IP的 关联文件夹信息
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{
			// 获取点击的执行机序列，并下载其目录结构文件，显示到list中
			int iSel = m_pRemoteList->GetCurSel();
			InitRemoteDirToList(iSel);
		}
	}
	/*else if (msg.sType == _T("menu"))
	{*/
		/*if (msg.pSender->GetName() != _T("domainlist")) return;
		CMenuWnd* pMenu = new CMenuWnd();
		if (pMenu == NULL) { return; }
		POINT pt = { msg.ptMouse.x, msg.ptMouse.y };
		::ClientToScreen(*this, &pt);
		pMenu->Init(msg.pSender, pt);*/
	/*}
	else if (msg.sType == _T("menu_Delete")) {*/
		/*CListUI* pList = static_cast<CListUI*>(msg.pSender);
		int nSel = pList->GetCurSel();
		if (nSel < 0) return;
		pList->RemoveAt(nSel);
		domain.erase(domain.begin() + nSel);
		desc.erase(desc.begin() + nSel);*/
	//}
}

LRESULT CUpdateServer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);
	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(_T("UI.xml"), (UINT)0, NULL, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);

	Init();
	return 0;
}

LRESULT CUpdateServer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostQuitMessage(0L);

	bHandled = FALSE;
	return 0;
}

LRESULT CUpdateServer::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::IsIconic(*this)) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CUpdateServer::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CUpdateServer::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CUpdateServer::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	if (!::IsZoomed(*this)) {
		RECT rcSizeBox = m_pm.GetSizeBox();
		if (pt.y < rcClient.top + rcSizeBox.top) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
			return HTTOP;
		}
		else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}
		if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
		if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
	}

	RECT rcCaption = m_pm.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
		CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
		if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 &&
			_tcscmp(pControl->GetClass(), DUI_CTR_OPTION) != 0 &&
			_tcscmp(pControl->GetClass(), DUI_CTR_TEXT) != 0 &&
			_tcscmp(pControl->GetClass(), DUI_CTR_EDIT) != 0)
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CUpdateServer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		RECT rc = { rcWnd.left, rcWnd.top + szRoundCorner.cx, rcWnd.right, rcWnd.bottom };
		HRGN hRgn1 = ::CreateRectRgnIndirect(&rc);
		HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom - szRoundCorner.cx, szRoundCorner.cx, szRoundCorner.cy);
		::CombineRgn(hRgn1, hRgn1, hRgn2, RGN_OR);
		::SetWindowRgn(*this, hRgn1, TRUE);
		::DeleteObject(hRgn1);
		::DeleteObject(hRgn2);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CUpdateServer::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg) {
	case WM_ADDLISTITEM:   lRes = OnAddListItem(uMsg, wParam, lParam, bHandled); break;
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_GROUPTALK:	   lRes = OnWMGROUPTALK(uMsg, wParam, lParam, bHandled); break;
	case FILE_TRANSFER_SUCCESS:		lRes = OnFileTranferOver(uMsg, wParam, lParam, bHandled); break;
	//case WM_RBUTTONDOWN:   break;
	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CUpdateServer::OnAddRemote()
{
	if (!m_pRemoteList || !m_pHostEdit || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostEdit->GetText();
	CDuiString strfileport = m_pFileportEdit->GetText();
	CDuiString strmsgport = m_pMsgportEdit->GetText();

	if (strhost.GetLength() == 0 || strfileport.GetLength() == 0 ||
		strmsgport.GetLength() == 0)
	{
		return;
	}

	for (size_t index = 0; index < g_host.size();index++)
	{
		if (strhost == g_host[index])
		{
			MessageBox(NULL, _T("不能重复添加执行机IP"), _T("Error"), MB_OK);
			return;
		}
	}

	// 创建对应执行机的文件夹：以IP命名，存在其更新的过程文件
	CDuiString strRemoteFolderPath = _T("");
	//strRemoteFolderPath.Format(_T("%s\\%s"),PublicFunction::M2W(m_curPath.c_str()), strhost);
	strRemoteFolderPath = CDuiString(PublicFunction::M2W(m_curPath.c_str()))+ _T("\\")+ strhost;
	string strPath = PublicFunction::W2M(strRemoteFolderPath.GetData());
	if (false == PublicFunction::IsDirExisted(strPath))
	{
		if ( NULL == CreateDirectory(strRemoteFolderPath, NULL))
		{
			CDuiString error = _T("");
			error.Format(_T("创建文件%s失败，错误码：%d\r\n"), strRemoteFolderPath, GetLastError());
			MessageBox(NULL, error, _T("Error"), MB_OK);
			return;
		}
	}


	// 写入配置文件
	string configPath = m_curPath + CONFIG_NAME;
	int iCount = GetPrivateProfileIntA(("RemoteList"), ("count"), 0, configPath.c_str());
	char szSection[10] = { 0 };
	char szCount[4] = { 0 };	// IP最大个数256
	sprintf_s(szSection, "Remote%d", iCount+1);
	sprintf_s(szCount, "%d", iCount + 1);
	WritePrivateProfileStringA("RemoteList", "count", szCount, configPath.c_str());
	WritePrivateProfileStringA(szSection, "IP", PublicFunction::W2M(strhost.GetData()).c_str(), configPath.c_str());
	WritePrivateProfileStringA(szSection, "fileport", PublicFunction::W2M(strfileport.GetData()).c_str(), configPath.c_str());
	
	// 写入全局变量
	g_host.push_back(strhost);
	g_fileport.push_back(_ttoi(strfileport));
	g_msgport.push_back(_ttoi(strmsgport));

	// 清空当前输入
	m_pHostEdit->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	// 设置list回调函数，绘制list
	int count = m_pRemoteList->GetCount();
	m_pRemoteList->SetTextCallback(this);

	CListTextElementUI* pListElement = new CListTextElementUI;
	pListElement->SetTag(count);
	if (pListElement != NULL)
	{
		::PostMessage(GetHWND(), WM_ADDLISTITEM, RemoteList, (LPARAM)pListElement);
	}
}

void CUpdateServer::OnModifyRemote()
{
	if (!m_pRemoteList || !m_pHostEdit || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostEdit->GetText();
	CDuiString strfileport = m_pFileportEdit->GetText();
	CDuiString strmsgport = m_pMsgportEdit->GetText();

	if (strhost.GetLength() == 0 || strfileport.GetLength() == 0 ||
		strmsgport.GetLength() == 0)
	{
		return;
	}

	bool bIsExisted = false;
	int index = 0;
	for (size_t index = 0; index < g_host.size();index++)
	{
		if (strhost == g_host[index])
		{
			g_fileport[index] = _ttoi(strfileport);
			g_msgport[index] = _ttoi(strmsgport);
			bIsExisted = true;
			m_pRemoteList->RemoveAt(index);
			break;
		}
	}
	if (bIsExisted != true)
	{
		MessageBox(NULL, _T("不能修改尚未添加执行机IP"), _T("Error"), MB_OK);
		return;
	}	
	m_pHostEdit->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	m_pRemoteList->SetTextCallback(this);

	CListTextElementUI* pListElement = new CListTextElementUI;
	pListElement->SetTag(index);
	if (pListElement != NULL)
	{
		::PostMessage(GetHWND(), WM_ADDLISTITEM, RemoteList, (LPARAM)pListElement);
	}
}

void CUpdateServer::OnDeleteRemote()
{
	if (!m_pRemoteList || !m_pHostEdit || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostEdit->GetText();
	CDuiString strfileport = m_pFileportEdit->GetText();
	CDuiString strmsgport = m_pMsgportEdit->GetText();

	if (strhost.GetLength() == 0 || strfileport.GetLength() == 0 ||
		strmsgport.GetLength() == 0)
	{
		return;
	}

	bool bIsExisted = false;
	int index = 0;
	//for( std::vector<CDuiString>::iterator iter = g_)
	for (size_t index = 0; index < g_host.size();index++)
	{
		if (strhost == g_host[index])
		{
			bIsExisted = true;

			//m_pRemoteList->SetDelayedDestroy(false);
			m_pRemoteList->RemoveAt(index, true);
			//m_pRemoteList->SetTag(index);
			//m_pRemoteList->NeedUpdate();
			g_host.erase(g_host.begin() + index);
			g_fileport.erase(g_fileport.begin() + index);
			g_msgport.erase(g_msgport.begin() + index);
			break;
		}
	}
	if (bIsExisted != true)
	{
		MessageBox(NULL, _T("不能删除尚未添加执行机IP"), _T("Error"), MB_OK);
		return;
	}
}

void CUpdateServer::OnUpdateSystem()
{
	/* */
	// 1、获取Update.xml，并比较是否需要更新
	//		a 需要更新
	//			1)发送结束进程消息
	//			2)开始发送文件
	//			3)发送启动进程消息
	//		b 不需要更新，提示用户，返回主界面
	CDuiString strHostIp = m_pRemoteDirIP->GetText();
	if (strHostIp.IsEmpty())
	{
		MessageBox(NULL, _T("请先选择执行机"), _T("Notice"), MB_OK);
		return;
	}

	// 采取 与客户端循环发消息
	DWORD dwAddress = inet_addr( PublicFunction::W2M(strHostIp.GetData()).c_str());
	g_pTalk->SendText("", 0, MT_BEGIN_UPDATE, dwAddress);
	return;
}

DWORD CUpdateServer::RecvUpdateFileThreadProc(LPVOID lpParameter)
{
	GT_HDR* pHeader = static_cast<GT_HDR*>(lpParameter);
	if (pHeader == NULL)
	{
		return -1;
	}
	SOCKADDR_IN addr_ip;
	addr_ip.sin_addr.S_un.S_addr = pHeader->dwAddr;
	string strIP = inet_ntoa(addr_ip.sin_addr);

	string strerror = "";
	CDuiString error = _T("");
	string strPath = PublicFunction::GetCurrentRunPath() + "\\" + strIP;
	RecvFile *fileRecv = new RecvFile(NULL, strPath);
	HANDLE handle = fileRecv->StartRecv(strerror, 10);

	if (handle == INVALID_HANDLE_VALUE)
	{
		delete fileRecv;
		return -1;
	}

	// 接收文件 30s超时
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30 * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("接收文件Update.xml超时，错误码：%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("接收文件Update.xml超时."));
		}
		delete fileRecv;
		::MessageBox(NULL, error, _T("Error"), MB_OK);
		return false;
	}

	// 接收文件可能是多个 
	/*if (WAIT_TIMEOUT == fileRecv->WaitForMulThread(10 * 1000, strerror))
	{
		delete fileRecv;
		::MessageBox(NULL, _T("接收文件超时."), _T("Error"), MB_OK);
		return -1;
	}*/
	// 释放资源
	delete fileRecv;
	return 0;
}

DWORD CUpdateServer::RecvRemoteDirFileThreadProc(LPVOID lpParameter)
{
	CUpdateServer* pUpdateObj = static_cast<CUpdateServer*>(lpParameter);
	if (pUpdateObj == NULL)
	{
		return -1;
	}
	
	CDuiString strIP = pUpdateObj->m_pRemoteDirIP->GetText();

	string strerror = "";
	CDuiString error = _T("");
	string strPath = PublicFunction::GetCurrentRunPath() + "\\" + PublicFunction::W2M(strIP.GetData());
	RecvFile *fileRecv = new RecvFile(pUpdateObj->m_hWnd, strPath);
	HANDLE handle = fileRecv->StartRecv(strerror, 10);

	if (handle == INVALID_HANDLE_VALUE)
	{
		delete fileRecv;
		return -1;
	}

	// 接收文件 30s超时
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30 * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("接收文件超时，错误码：%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("接收文件超时."));
		}
		delete fileRecv;
		::MessageBox(pUpdateObj->m_hWnd, error, _T("Error"), MB_OK);
		return -1;
	}

	// 释放资源
	delete fileRecv;
	return 0;
}

bool CUpdateServer::RecvFileFromClient(string strRemoteIP, DWORD dwWaitSencond, CDuiString &error)
{
	string strerror = "";
	string strPath = PublicFunction::GetCurrentRunPath() + "\\" + strRemoteIP;
	RecvFile *fileRecv = new RecvFile(m_hWnd, strPath);
	HANDLE handle = fileRecv->StartRecv(strerror, dwWaitSencond);

	if (handle == INVALID_HANDLE_VALUE)
	{
		error.Format(_T("%s，错误码：%d"), CDuiString(PublicFunction::M2W(strerror.c_str())), 
					fileRecv->m_dwErrorCode);
		delete fileRecv;
		return false;
	}

	// 接收文件可能是多个 
	/*if (WAIT_TIMEOUT == fileRecv->WaitForMulThread(dwWaitSencond * 1000, strerror))
	{
		delete fileRecv;
		error = _T("接收文件超时.");
		return false;
	}*/

	// 接收文件
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, dwWaitSencond * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("接收文件超时，错误码：%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("接收文件超时."));
		}
		delete fileRecv;
		return false;
	}

	// 释放资源
	delete fileRecv;
	return true;
}

/* 检查升级文件,是否升级
	由于服务端&客户端生成的Update.xml 算法一致
	此处简单处理，比较客户端和服务端的Update.xml的MD5值是否一致*/
// 
bool CUpdateServer::CheckUpdateFile(std::string strIP)
{
	string strClientXmlPath = m_curPath + "\\" + strIP + UPDATEXML;
	string strServerXmlPath = m_curPath + UPDATEXML;

	string strClientMD5 = "";
	string strServerMD5 = "";
	strClientMD5 = PublicFunction::GetFileMD5(strClientXmlPath);
	strServerMD5 = PublicFunction::GetFileMD5(strServerXmlPath);

	if (strClientMD5 == strServerMD5 && strServerMD5 != "")
	{
		return true;
	}

	return false;
}

DWORD CUpdateServer::SendFileThreadProc(LPVOID lpParameter)
{
	CUpdateServer* pUpdateObj = static_cast<CUpdateServer*>(lpParameter);
	if (pUpdateObj == NULL)
	{
		return -1;
	}

	CDuiString strerror = _T("");
	string error = "";

	CDuiString strLocalPath = pUpdateObj->m_pLocalDir->GetText();
	string strIP = PublicFunction::W2M(pUpdateObj->m_pRemoteDirIP->GetText().GetData());

	string strLogPath = PublicFunction::GetCurrentRunPath() + "\\" + strIP;

	FileSend *fileSend = new FileSend(pUpdateObj->m_hWnd, strIP, FILE_TRANSFER_PORT_DEFAULT);
	
	// 循环
	for (size_t index = 0;index < g_localFile.size(); index++)
	{
		CDuiString strName = g_localFile[index].strFileName;
		CDuiString strFilePath = strLocalPath + _T("\\")+strName;

		TCHAR szError[MAX_PATH] = { 0 };
		if (false == fileSend->AddFileToSend(PublicFunction::W2M(strFilePath.GetData()), szError, MAX_PATH))
		{
			strerror = szError;
			return -1;
		}
	}
		
	HANDLE handle = fileSend->StartSend(error);
	if (handle == INVALID_HANDLE_VALUE)
	{
		delete fileSend;
		return -1;
	}

	// 发送 文件超时时间3分钟
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 3*60 * 1000))
	{
		strerror = _T("发送文件超时！");
		return -1;
	}

	// 释放资源
	delete fileSend;
	CString strResult = _T("");
	if (error == "")
	{
		strerror = _T("发送文件成功！");
	}
	else
	{
		strerror.Format(_T("发送文件失败，原因：%s"), CDuiString(PublicFunction::M2W(error.c_str())));
		return -1;
	}
	return 0;
}


CDuiString CUpdateServer::SelectDir()
{
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
		return CDuiString(FullPath);
		//成功
	}
	return CDuiString(_T(""));
}

void CUpdateServer::InitLocalDirToList(CDuiString dir)
{
	// 分析路径下的文件显示在list表中
	CDuiString strFileType = _T("文件");
	CDuiString strFileName = _T("");
	CDuiString strLastDate = _T("");
	ULONGLONG ulLength = 0;

	WIN32_FIND_DATA data;
	TCHAR szFind[MAX_PATH] = { 0 };

	_tcscpy_s(szFind, MAX_PATH, dir);
	_tcscat_s(szFind, _T("\\*.*"));

	//m_pLocalDirList->SetDelayedDestroy(false);
	m_pLocalDirList->RemoveAll();
	//m_pLocalDirList->NeedUpdate();
	g_localFile.clear();			// 清理上次内容

	HANDLE handle = ::FindFirstFile(szFind, &data);

	do
	{
		string strVersion = "";
		string strMD5 = "";

		FileInfo fileinfo;
		if (data.cFileName[0] != _T('.'))
		{
			
			fileinfo.strFileName = data.cFileName;
			//_tprintf(_T("%s\\%s\n"), )
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 文件夹
				fileinfo.strFileType = _T("文件夹");
			}
			else
			{
				fileinfo.strFileType = _T("文件");
				ulLength = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;
				fileinfo.ulLength = ulLength; 

				// 文件过大，易造成计算MD5耗时过多 100M以上不计算
				if (ulLength < 1024 * 1024 * 100)
				{
					CDuiString strPath = dir + _T("\\") + data.cFileName;
					PublicFunction::GetFileVersion(PublicFunction::W2M(strPath.GetData()), strVersion);
					strMD5 = PublicFunction::GetFileMD5(PublicFunction::W2M(strPath.GetData()));
				}
			}
			FILETIME ft = data.ftLastWriteTime;
			SYSTEMTIME st;
			_FILETIME localTime;

			// 先将文件时间转换为本地文件时间，再转换为系统时间（防止出现时区差异）
			FileTimeToLocalFileTime(&ft, &localTime);
			FileTimeToSystemTime(&localTime, &st);
			fileinfo.strLastDate.Format(_T("%04d-%02d-%02d %02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

			g_localFile.push_back(fileinfo);
			int count = m_pLocalDirList->GetCount();
			m_pLocalDirList->SetTextCallback(this);

			CListTextElementUI* pListElement = new CListTextElementUI;
			pListElement->SetTag(count);
			if (pListElement != NULL)
			{
				::PostMessageW(GetHWND(), WM_ADDLISTITEM, LocalDirList, (LPARAM)pListElement);
			}
		} 
		
	} while (::FindNextFile(handle, &data) != 0 && ::GetLastError() != ERROR_NO_MORE_FILES);

	FindClose(handle);
}

void CUpdateServer::InitRemoteDirToList(int index)
{
	CDuiString host = g_host[index];
	u_short usFilePort = g_fileport[index];
	u_short usMsgPort = g_msgport[index];
	string strRemoteIP = PublicFunction::W2M(host.GetData());

	m_pRemoteDirIP->SetText(host);

	char szText[] = "GetRemoteDir";
	DWORD dwRemoteAddr = inet_addr(strRemoteIP.c_str());
	g_pTalk->SendText(szText, sizeof(szText), MT_MESG,dwRemoteAddr);

	// 接收数采目录结构文件，超时时间设为10s

	unsigned int recvthreadID = 0;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvRemoteDirFileThreadProc,
		this, 0, &recvthreadID);

	/*CDuiString error = _T("");
	if (false == RecvFileFromClient(strRemoteIP, 10, error))
	{
		MessageBox(NULL, error, _T("Error"), MB_OK);
	}*/
}

/*
* 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
*/
LPCTSTR CUpdateServer::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
	TCHAR szBuf[MAX_PATH] = { 0 };
	CDuiString strName = pControl->GetParent()->GetParent()->GetName();

	// 本地站点的目录结构 && 防止vector容器越界
	if (strName == _T("localfilelist") && iIndex < g_localFile.size())
	{		
		switch (iSubItem)
		{
		case 0:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_localFile[iIndex].strFileName);
#else
			_stprintf(szBuf, g_localFile[iIndex].strFileName.c_str());
#endif
		}
		break;
		case 1:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_localFile[iIndex].strLastDate);
#else
			_stprintf(szBuf, g_localFile[iIndex].strLastDate.c_str());
#endif
		}
		break;
		case 2:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_localFile[iIndex].strFileType);
#else
			_stprintf(szBuf, g_localFile[iIndex].strFileType.c_str());
#endif
		}
		break;
		case 3:
		{
			if (g_localFile[iIndex].strFileType == _T("文件夹"))
			{
				return _T("");
			}
			ULONGLONG ulLength = g_localFile[iIndex].ulLength;
			double dLength = ulLength;
#ifdef _UNICODE
			if (ulLength < 1024)	// 小于1KB
			{
				swprintf_s(szBuf, _T("%lld字节"), ulLength);
			}
			else if (ulLength > 1024*1024 && ulLength < (1024*1024*1000))		// 1MB -- 1000MB
			{
				swprintf_s(szBuf, _T("%.2fMB"), dLength/(1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB之间
			{
				swprintf_s(szBuf, _T("%.2fKB"), dLength /1024);
			}
			
#else
			if (ulLength < 1024)	// 小于1KB
			{
				sprintf_s(szBuf, ("%lld字节"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB之间
			{
				sprintf_s(szBuf, ("%.2fKB"), dLength / 1024);
			}
#endif
		}
		break;
		}
	}
	else if (strName == _T("remotefilelist") && iIndex < g_remoteFile.size())
	{
		switch (iSubItem)
		{
		case 0:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_remoteFile[iIndex].strFileName);
#else
			_stprintf(szBuf, g_remoteFile[iIndex].strFileName.c_str());
#endif
		}
		break;
		case 1:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_remoteFile[iIndex].strLastDate);
#else
			_stprintf(szBuf, g_remoteFile[iIndex].strLastDate.c_str());
#endif
		}
		break;
		case 2:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_remoteFile[iIndex].strFileType);
#else
			_stprintf(szBuf, g_remoteFile[iIndex].strFileType.c_str());
#endif
		}
		break;
		case 3:
		{
			if (g_remoteFile[iIndex].strFileType == _T("文件夹"))
			{
				return _T("");
			}
			ULONGLONG ulLength = g_remoteFile[iIndex].ulLength;
			double dLength = ulLength;
#ifdef _UNICODE
			if (ulLength < 1024)	// 小于1KB
			{
				swprintf_s(szBuf, _T("%lld字节"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				swprintf_s(szBuf, _T("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB之间
			{
				swprintf_s(szBuf, _T("%.2fKB"), dLength / 1024);
			}

#else
			if (ulLength < 1024)	// 小于1KB
			{
				sprintf_s(szBuf, ("%lld字节"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB之间
			{
				sprintf_s(szBuf, ("%.2fKB"), dLength / 1024);
			}
#endif
		}
		break;
		}
	}
	else if (strName == _T("machineList") && iIndex < g_host.size() )
	{
		switch (iSubItem)
		{
		case 0:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, g_host[iIndex]);
			/*int iLen = g_host[iIndex].length();
			LPWSTR lpText = new WCHAR[iLen + 1];
			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, g_host[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
			_stprintf(szBuf, lpText);
			delete[] lpText;*/
#else
			_stprintf(szBuf, g_host[iIndex].c_str());
#endif
		}
		break;
		case 1:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, _T("%d"), g_fileport[iIndex]);
			/*int iLen = g_fileport[iIndex].length();
			LPWSTR lpText = new WCHAR[iLen + 1];
			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, g_fileport[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
			_stprintf(szBuf, lpText);
			delete[] lpText;*/
#else
			_stprintf(szBuf, g_fileport[iIndex].c_str());
#endif
		}
		break;
		case 2:
		{
#ifdef _UNICODE		
			_stprintf_s(szBuf, _T("%d"), g_msgport[iIndex]);
			/*int iLen = g_msgport[iIndex].length();
			LPWSTR lpText = new WCHAR[iLen + 1];
			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, g_msgport[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
			_stprintf(szBuf, lpText);
			delete[] lpText;*/
#else
			_stprintf(szBuf, g_msgport[iIndex].c_str());
#endif
		}
		break;
		}
	}

	pControl->SetUserData(szBuf);
	return pControl->GetUserData();
}

/* 以下是消息处理接口 */

// 增加执行机list记录
LRESULT CUpdateServer::OnAddListItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CListTextElementUI* pListElement = (CListTextElementUI*)lParam;
	switch (wParam)
	{
	case RemoteList:
		if (m_pRemoteList) m_pRemoteList->Add(pListElement);
		break;
	case RemoteDirList:
		if (m_pRemoteDirList) m_pRemoteDirList->Add(pListElement);
		break;
	case LocalDirList:
		if (m_pLocalDirList) m_pLocalDirList->Add(pListElement);
		break;
	}
	return 0;
}

LRESULT CUpdateServer::OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

void CUpdateServer::HandleGroupMsg(HWND hDlg, GT_HDR *pHeader)//函数实现体
{
	if (NULL == pHeader) { return; }

	SOCKADDR_IN addr_ip;
	addr_ip.sin_addr.S_un.S_addr = pHeader->dwAddr;
	CDuiString strLog = _T("");
	CDuiString strLogHead = _T("");
	string strIP = inet_ntoa(addr_ip.sin_addr);
	string strUser = pHeader->szUser;
	DWORD dwAddress = inet_addr(strIP.c_str());

	CDuiString strError = _T("");

	CDuiString strIPW = CDuiString(PublicFunction::M2W(strIP.c_str()));
	CDuiString strUserW = CDuiString(PublicFunction::M2W(strUser.c_str()));
	strLogHead.Format(_T(" From:%s, computer name:%s,operation:"),strIPW.GetData(), strUserW.GetData());
	switch (pHeader->gt_type)
	{
		case MT_JION:		// 新用户加入
		{
			// 显示给用户
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
			strLog = strLogHead + _T("收到消息-") + CDuiString(PublicFunction::M2W(pHeader->data())) + _T("\r\n");
			break;
		}
		case MT_CHECK_MESG:			// 客户端发送过来的 升级检查 消息
		{
			// 准备接收 Update.xml，并回复检查结果：1-需要升级，准备升级；0-无需升级，退出升级
			strLog.Append(strLogHead);
			strLog.Append(_T("收到消息-MT_CHECK_MESG, 更新准备(接收并检查Update.xml文件)...\r\n"));
			//strLog = strLogHead + _T("收到消息-MT_CHECK_MESG, 更新准备(接收并检查Update.xml文件)...\r\n");
			UpdateLog(strIP, strLog);

			// 删除原先Update.xml
			string strXmlPath = m_curPath + "\\" + strIP + UPDATEXML;
			DeleteFileA(strXmlPath.c_str());

			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvUpdateFileThreadProc,
				pHeader, 0, &recvthreadID);

			if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30*1000))
			{
				MessageBox(NULL, strError, _T("接收文件超时."), MB_OK);
			}

			// 检查 升级文件Update.xml：false -- 升级，true -- 不升级
			if (true == CheckUpdateFile(strIP))
			{
				UpdateLog(strIP, _T("升级文件检查结果：该执行机不需要升级.\r\n"));
				g_pTalk->SendText(EXECUTE_RESULT_TYPE_FAILED, 2, MT_CHECKRES_MESG, dwAddress);
				UpdateLog(strIP, _T("'检查结果'消息MT_CHECKRES_MESG 已发送.\r\n"));
			}
			else
			{
				UpdateLog(strIP, _T("升级文件检查结果：该执行机需要升级.\r\n"));
				g_pTalk->SendText(EXECUTE_RESULT_TYPE_SUCCESS, 2, MT_CHECKRES_MESG, dwAddress);
				UpdateLog(strIP, _T("'检查结果'消息MT_CHECKRES_MESG 已发送.\r\n"));
				// 休眠1s，发出下一个消息：要求客户端结束进程
				Sleep(1000);
				UpdateLog(strIP, _T("执行机'结束数采进程'准备...\r\n"));
				g_pTalk->SendText("", 0, MT_ENDPRO_MESG, dwAddress);
				UpdateLog(strIP, _T("'结束进程'消息MT_ENDPRO_MESG 已发送.\r\n"));
			}
		
			strLog = _T("");
			break;
		}
		case MT_ENDPRORES_MESG:		// 客户端结束进程结果消息
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// 结束进程exe 成功，开始发送文件
			{
				strLog = strLogHead + _T("收到'结束进程'结果消息-MT_ENDPRORES_MESG,准备发送更新包...\r\n");
				UpdateLog(strIP, strLog);
				g_pTalk->SendText("", 0, MT_UPDATE_MESG, dwAddress);
				UpdateLog(strIP, _T("'准备更新文件'消息MT_UPDATE_MESG 已发送,发送文件...\r\n"));
				strLog = _T("");

				// 休眠1s，给客户端时间响应启动接收
				Sleep(1000);

				// 开始发送文件（文件为当前站点内的所有文件；后续考虑增加勾选功能 to do）
				unsigned int recvthreadID = 0;
				HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
					this, 0, &recvthreadID);
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("收到'结束进程'结果消息-MT_ENDPRORES_MESG， 执行机结束进程异常！\r\n");
			}
			else
			{
				strLog = strLogHead + _T("收到'结束进程'结果消息-MT_ENDPRORES_MESG， 但标识错误，请联系维护人员！\r\n");
			}
			//UpdateLog(strIP, strLog);
			break;
		}
		case MT_UPDATERES_MESG:		// 客户端接收 更新文件结果消息
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// 接收 更新文件成功：发送启动exe消息
			{
				strLog = strLogHead + _T("收到'接收更新包'结果消息-MT_UPDATERES_MESG,准备发送更新包...\r\n");
				UpdateLog(strIP, strLog);
				g_pTalk->SendText("", 0, MT_BEGINPRO_MESG, dwAddress);
				UpdateLog(strIP, _T("'启动数采进程'消息MT_BEGINPRO_MESG 已发送.\r\n"));
				strLog = _T("");
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("收到'接收更新包'结果消息-MT_UPDATERES_MESG， 执行机接收文件失败！\r\n");
			}
			else
			{
				strLog = strLogHead + _T("收到'接收更新包'结果消息-MT_UPDATERES_MESG， 但标识错误，请联系维护人员！\r\n");
			}
			break;
		}
		case MT_BEGINPRORES_MESG:	// 客户端启动数采进程exe 结果消息
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// 启动进程exe成功：至此更新结束
			{
				// to do 
				strLog = strLogHead + _T("收到'启动数采进程'结果消息-MT_BEGINPRORES_MESG,更新成功！\r\n");
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("收到'启动数采进程'结果消息-MT_BEGINPRORES_MESG， 执行机启动数采进程失败！\r\n");
			}
			else
			{
				strLog = strLogHead + _T("收到'启动数采进程'结果消息-MT_BEGINPRORES_MESG， 但标识错误，请联系维护人员！\r\n");
			}
			break;
		}
	}
	UpdateLog(strIP, strLog);
}


/* 服务端接收文件成功处理消息：
	wParam为0 -- 表示接收的是执行机目录结构文件*/
LRESULT CUpdateServer::OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	// 获取执行机exe目录信息文件 消息处理
	if (wParam == 0)
	{
		CDuiString strIP = m_pRemoteDirIP->GetText();
		CDuiString strOutput = _T("");
		string strPath = PublicFunction::GetCurrentRunPath() + "\\" + 
			PublicFunction::W2M(strIP.GetData()) + SYSFILEINFO;

		// 打开文件
		FILE *pFile = NULL;
		errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
		if (err != 0 || pFile == NULL)
		{
			strOutput.Format(_T("打开文件%s失败"),CDuiString(PublicFunction::M2W(strPath.c_str())));
			MessageBox(NULL, strOutput, _T("Error"), MB_OK);
			return 0;
		}

		// 文件指针定位到文件头
		fseek(pFile, 0, SEEK_SET);

		// 分配空间
		FILEINFO *pFileInfo = NULL;
		pFileInfo = (FILEINFO*)malloc(sizeof(FILEINFO));
		if (pFileInfo == NULL)
		{
			fclose(pFile);
			MessageBox(NULL, _T("malloc分配空间失败"), _T("Error"), MB_OK);
			return 0;
		}

		// 清理既有数据
		m_pRemoteDirList->RemoveAll();
		g_remoteFile.clear();

		while (TRUE)
		{
			FileInfo fileinfo;
			size_t readsize = fread_s(pFileInfo, sizeof(FILEINFO), sizeof(FILEINFO), 1, pFile);
			if (readsize != 1)
			{
				break;
			}

			fileinfo.strFileName = CDuiString(PublicFunction::M2W(pFileInfo->szFileName));
			fileinfo.strFileType = CDuiString(PublicFunction::M2W(pFileInfo->szFileType));
			fileinfo.strLastDate = CDuiString(PublicFunction::M2W(pFileInfo->szFileTime));
			fileinfo.ulLength = pFileInfo->ulLength;

			g_remoteFile.push_back(fileinfo);
			// 更新到list
			int count = m_pRemoteDirList->GetCount();
			m_pRemoteDirList->SetTextCallback(this);

			CListTextElementUI* pListElement = new CListTextElementUI;
			pListElement->SetTag(count);
			if (pListElement != NULL)
			{
				::PostMessageW(GetHWND(), WM_ADDLISTITEM, RemoteDirList, (LPARAM)pListElement);
			}
		}

		// 释放资源
		fclose(pFile);
		free(pFileInfo);		
	}
	return LRESULT(0);
}

void CUpdateServer::UpdateLog(const string ip, const CDuiString log)
{
	SYSTEMTIME *tm = NULL;
	TCHAR szTime[MAX_PATH] = { 0 };
	TCHAR szDate[MAX_PATH] = { 0 };
	CDuiString strCurTime = _T("");

	if (log.GetLength() == 0)
	{
		return;
	}

	GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, tm,
		_T("hh:mm:ss"), szTime, MAX_PATH);
	GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_AUTOLAYOUT, tm, L"yyyy-MM-dd",
		szDate, MAX_PATH, NULL);
	strCurTime.Format(_T("%s %s "), szDate, szTime);

	// 1、保存至文件
	std::string strPath = m_curPath + "\\" + ip + UPDATELOG;
	std::string strLog = PublicFunction::W2M(strCurTime.GetData())
				+ PublicFunction::W2M(log.GetData());
	bool bRet = PublicFunction::SaveLogToFile(strPath, strLog);
	// 2、显示到界面（to do）
}
