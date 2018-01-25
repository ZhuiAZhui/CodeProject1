#include "stdafx.h"
#include "UpdateServer.h"
#include <Shlobj.h>		// BROWSEINFO �ṹ����Ҫ

#include "PublicFunction.h"

#define WM_ADDLISTITEM WM_USER + 50
#define WM_MODIFYLISTITEM WM_USER + 51
#define WM_DELETELISTITEM WM_USER + 52
#define WM_GROUPTALK	WM_USER + 105    


#define EXECUTE_RESULT_TYPE_SUCCESS		"1"
#define EXECUTE_RESULT_TYPE_FAILED		"0"

/*
* ��ŵ�1������
*/
std::vector<CDuiString> g_host;
/*
* ��ŵ�2������
*/
std::vector<u_short> g_fileport;

/*
* ��ŵ�3������
*/
std::vector<u_short> g_msgport;

// ��ű���Ŀ¼�ļ���Ϣ
std::vector<FileInfo> g_localFile;

// ��� ִ�л�Ŀ¼�ļ���Ϣ
std::vector<FileInfo> g_remoteFile;

// UDP ��Ϣ
CGroupTalk* g_pTalk;

CUpdateServer::CUpdateServer()
{
	InitSocket();//����2.2�汾WinSock��
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

	// ��ȡ�鲥��Ϣ
	GetPrivateProfileStringA(("BoradCastInfo"), ("IP"), (""), szBuffer, MAX_PATH, configPath.c_str());
	strMultiAddr = szBuffer;
	usMultiPort = GetPrivateProfileIntA(("BoradCastInfo"), ("PORT"), GROUP_PORT, configPath.c_str());

	// ��ȡ�������ļ����б�
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
	 
	// ��ȡ����Ŀ¼������ʼ����xml�ļ����ļ������汾��MD5ֵ��ɵ�xml�ļ���
	GetPrivateProfileStringA(("UpdateFileInfo"), ("path"), (""), szBuffer, MAX_PATH, configPath.c_str());
	strUpdatePath = szBuffer;
	PublicFunction::InitFolderToXml(strUpdatePath, xmlPath, m_ExcludeFile);

	// ��ȡ ִ�л�IP���ļ�����˿ڣ������ļ�����
	int iCount = GetPrivateProfileIntA(("RemoteList"), ("count"), 0, configPath.c_str());
	for (int index = 1;index <= iCount;index++)
	{
		char szSection[10] = { 0 };
		sprintf_s(szSection, "Remote%d", index);
		GetPrivateProfileStringA(szSection, ("IP"), (""), szBuffer, MAX_PATH, configPath.c_str());
		strRemoteIP = szBuffer;
		u_short usFPort = GetPrivateProfileIntA(szSection, ("fileport"), 0, configPath.c_str());

		// ������Ӧִ�л����ļ��У���IP��������������µĹ����ļ�
		string strRemoteFolderPath = m_curPath + "\\" +strRemoteIP;
		if (false == PublicFunction::IsDirExisted(strRemoteFolderPath.c_str()))
		{
			if (NULL == CreateDirectoryA(strRemoteFolderPath.c_str(), NULL))
			{
				char szError[MAX_PATH] = { 0 };
				sprintf_s(szError, "�����ļ�%sʧ�ܣ������룺%d\r\n",strRemoteFolderPath.c_str(), GetLastError());
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
			// �ͷſؼ��������뿪��Ϣ
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
			// ��� �Զ����°�ť
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
		// ������վ����ļ�����Ϣ��ʾ����
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
				MessageBox(NULL, _T("������Ϸ�·��"), _T("error"), MB_OK);
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

		// ���ִ�л������ݣ���ʾ��edit����
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{
			int iSel = m_pRemoteList->GetCurSel();	// ���д��룬��bug������¼���Ҫ������Σ����������Ӧ��ȷ����
			if (iSel < 0) return;
			//int iIndex = msg.pSender->GetTag();    // ���д��룬��ɾ���г�������bug��
													 // ɾ�����к��ٽ��е�����У�tag=1����Ϊ0
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

		// ˫��ִ�л������ݣ���ʾ��ӦIP�� �����ļ�����Ϣ
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{
			// ��ȡ�����ִ�л����У���������Ŀ¼�ṹ�ļ�����ʾ��list��
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
			MessageBox(NULL, _T("�����ظ����ִ�л�IP"), _T("Error"), MB_OK);
			return;
		}
	}

	// ������Ӧִ�л����ļ��У���IP��������������µĹ����ļ�
	CDuiString strRemoteFolderPath = _T("");
	//strRemoteFolderPath.Format(_T("%s\\%s"),PublicFunction::M2W(m_curPath.c_str()), strhost);
	strRemoteFolderPath = CDuiString(PublicFunction::M2W(m_curPath.c_str()))+ _T("\\")+ strhost;
	string strPath = PublicFunction::W2M(strRemoteFolderPath.GetData());
	if (false == PublicFunction::IsDirExisted(strPath))
	{
		if ( NULL == CreateDirectory(strRemoteFolderPath, NULL))
		{
			CDuiString error = _T("");
			error.Format(_T("�����ļ�%sʧ�ܣ������룺%d\r\n"), strRemoteFolderPath, GetLastError());
			MessageBox(NULL, error, _T("Error"), MB_OK);
			return;
		}
	}


	// д�������ļ�
	string configPath = m_curPath + CONFIG_NAME;
	int iCount = GetPrivateProfileIntA(("RemoteList"), ("count"), 0, configPath.c_str());
	char szSection[10] = { 0 };
	char szCount[4] = { 0 };	// IP������256
	sprintf_s(szSection, "Remote%d", iCount+1);
	sprintf_s(szCount, "%d", iCount + 1);
	WritePrivateProfileStringA("RemoteList", "count", szCount, configPath.c_str());
	WritePrivateProfileStringA(szSection, "IP", PublicFunction::W2M(strhost.GetData()).c_str(), configPath.c_str());
	WritePrivateProfileStringA(szSection, "fileport", PublicFunction::W2M(strfileport.GetData()).c_str(), configPath.c_str());
	
	// д��ȫ�ֱ���
	g_host.push_back(strhost);
	g_fileport.push_back(_ttoi(strfileport));
	g_msgport.push_back(_ttoi(strmsgport));

	// ��յ�ǰ����
	m_pHostEdit->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	// ����list�ص�����������list
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
		MessageBox(NULL, _T("�����޸���δ���ִ�л�IP"), _T("Error"), MB_OK);
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
		MessageBox(NULL, _T("����ɾ����δ���ִ�л�IP"), _T("Error"), MB_OK);
		return;
	}
}

void CUpdateServer::OnUpdateSystem()
{
	/* */
	// 1����ȡUpdate.xml�����Ƚ��Ƿ���Ҫ����
	//		a ��Ҫ����
	//			1)���ͽ���������Ϣ
	//			2)��ʼ�����ļ�
	//			3)��������������Ϣ
	//		b ����Ҫ���£���ʾ�û�������������
	CDuiString strHostIp = m_pRemoteDirIP->GetText();
	if (strHostIp.IsEmpty())
	{
		MessageBox(NULL, _T("����ѡ��ִ�л�"), _T("Notice"), MB_OK);
		return;
	}

	// ��ȡ ��ͻ���ѭ������Ϣ
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

	// �����ļ� 30s��ʱ
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30 * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("�����ļ�Update.xml��ʱ�������룺%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("�����ļ�Update.xml��ʱ."));
		}
		delete fileRecv;
		::MessageBox(NULL, error, _T("Error"), MB_OK);
		return false;
	}

	// �����ļ������Ƕ�� 
	/*if (WAIT_TIMEOUT == fileRecv->WaitForMulThread(10 * 1000, strerror))
	{
		delete fileRecv;
		::MessageBox(NULL, _T("�����ļ���ʱ."), _T("Error"), MB_OK);
		return -1;
	}*/
	// �ͷ���Դ
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

	// �����ļ� 30s��ʱ
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30 * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("�����ļ���ʱ�������룺%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("�����ļ���ʱ."));
		}
		delete fileRecv;
		::MessageBox(pUpdateObj->m_hWnd, error, _T("Error"), MB_OK);
		return -1;
	}

	// �ͷ���Դ
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
		error.Format(_T("%s�������룺%d"), CDuiString(PublicFunction::M2W(strerror.c_str())), 
					fileRecv->m_dwErrorCode);
		delete fileRecv;
		return false;
	}

	// �����ļ������Ƕ�� 
	/*if (WAIT_TIMEOUT == fileRecv->WaitForMulThread(dwWaitSencond * 1000, strerror))
	{
		delete fileRecv;
		error = _T("�����ļ���ʱ.");
		return false;
	}*/

	// �����ļ�
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, dwWaitSencond * 1000))
	{
		if (fileRecv->m_dwErrorCode != 0)
		{
			error.Format(_T("�����ļ���ʱ�������룺%d"), fileRecv->m_dwErrorCode);
		}
		else
		{
			error.Format(_T("�����ļ���ʱ."));
		}
		delete fileRecv;
		return false;
	}

	// �ͷ���Դ
	delete fileRecv;
	return true;
}

/* ��������ļ�,�Ƿ�����
	���ڷ����&�ͻ������ɵ�Update.xml �㷨һ��
	�˴��򵥴����ȽϿͻ��˺ͷ���˵�Update.xml��MD5ֵ�Ƿ�һ��*/
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
	
	// ѭ��
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

	// ���� �ļ���ʱʱ��3����
	if (WAIT_TIMEOUT == WaitForSingleObject(handle, 3*60 * 1000))
	{
		strerror = _T("�����ļ���ʱ��");
		return -1;
	}

	// �ͷ���Դ
	delete fileSend;
	CString strResult = _T("");
	if (error == "")
	{
		strerror = _T("�����ļ��ɹ���");
	}
	else
	{
		strerror.Format(_T("�����ļ�ʧ�ܣ�ԭ��%s"), CDuiString(PublicFunction::M2W(error.c_str())));
		return -1;
	}
	return 0;
}


CDuiString CUpdateServer::SelectDir()
{
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
		return CDuiString(FullPath);
		//�ɹ�
	}
	return CDuiString(_T(""));
}

void CUpdateServer::InitLocalDirToList(CDuiString dir)
{
	// ����·���µ��ļ���ʾ��list����
	CDuiString strFileType = _T("�ļ�");
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
	g_localFile.clear();			// �����ϴ�����

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
				// �ļ���
				fileinfo.strFileType = _T("�ļ���");
			}
			else
			{
				fileinfo.strFileType = _T("�ļ�");
				ulLength = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;
				fileinfo.ulLength = ulLength; 

				// �ļ���������ɼ���MD5��ʱ���� 100M���ϲ�����
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

			// �Ƚ��ļ�ʱ��ת��Ϊ�����ļ�ʱ�䣬��ת��Ϊϵͳʱ�䣨��ֹ����ʱ�����죩
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

	// ��������Ŀ¼�ṹ�ļ�����ʱʱ����Ϊ10s

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
* �ؼ��Ļص�������IListCallbackUI �е�һ���麯������Ⱦʱ������,��[1]�������˻ص�����
*/
LPCTSTR CUpdateServer::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
	TCHAR szBuf[MAX_PATH] = { 0 };
	CDuiString strName = pControl->GetParent()->GetParent()->GetName();

	// ����վ���Ŀ¼�ṹ && ��ֹvector����Խ��
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
			if (g_localFile[iIndex].strFileType == _T("�ļ���"))
			{
				return _T("");
			}
			ULONGLONG ulLength = g_localFile[iIndex].ulLength;
			double dLength = ulLength;
#ifdef _UNICODE
			if (ulLength < 1024)	// С��1KB
			{
				swprintf_s(szBuf, _T("%lld�ֽ�"), ulLength);
			}
			else if (ulLength > 1024*1024 && ulLength < (1024*1024*1000))		// 1MB -- 1000MB
			{
				swprintf_s(szBuf, _T("%.2fMB"), dLength/(1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB֮��
			{
				swprintf_s(szBuf, _T("%.2fKB"), dLength /1024);
			}
			
#else
			if (ulLength < 1024)	// С��1KB
			{
				sprintf_s(szBuf, ("%lld�ֽ�"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB֮��
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
			if (g_remoteFile[iIndex].strFileType == _T("�ļ���"))
			{
				return _T("");
			}
			ULONGLONG ulLength = g_remoteFile[iIndex].ulLength;
			double dLength = ulLength;
#ifdef _UNICODE
			if (ulLength < 1024)	// С��1KB
			{
				swprintf_s(szBuf, _T("%lld�ֽ�"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				swprintf_s(szBuf, _T("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB֮��
			{
				swprintf_s(szBuf, _T("%.2fKB"), dLength / 1024);
			}

#else
			if (ulLength < 1024)	// С��1KB
			{
				sprintf_s(szBuf, ("%lld�ֽ�"), ulLength);
			}
			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
			{
				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
			}
			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
			{
				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
			}
			else                               // 1KB -- 1MB֮��
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

/* ��������Ϣ����ӿ� */

// ����ִ�л�list��¼
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
		::MessageBox(m_hWnd, (LPCTSTR)lParam, _T("����"), 0);
	}
	else
	{
		HandleGroupMsg(m_hWnd, (GT_HDR*)lParam);
	}
	return 0;
}

void CUpdateServer::HandleGroupMsg(HWND hDlg, GT_HDR *pHeader)//����ʵ����
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
		case MT_JION:		// ���û�����
		{
			// ��ʾ���û�
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
			strLog = strLogHead + _T("�յ���Ϣ-") + CDuiString(PublicFunction::M2W(pHeader->data())) + _T("\r\n");
			break;
		}
		case MT_CHECK_MESG:			// �ͻ��˷��͹����� ������� ��Ϣ
		{
			// ׼������ Update.xml�����ظ��������1-��Ҫ������׼��������0-�����������˳�����
			strLog.Append(strLogHead);
			strLog.Append(_T("�յ���Ϣ-MT_CHECK_MESG, ����׼��(���ղ����Update.xml�ļ�)...\r\n"));
			//strLog = strLogHead + _T("�յ���Ϣ-MT_CHECK_MESG, ����׼��(���ղ����Update.xml�ļ�)...\r\n");
			UpdateLog(strIP, strLog);

			// ɾ��ԭ��Update.xml
			string strXmlPath = m_curPath + "\\" + strIP + UPDATEXML;
			DeleteFileA(strXmlPath.c_str());

			unsigned int recvthreadID = 0;
			HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&RecvUpdateFileThreadProc,
				pHeader, 0, &recvthreadID);

			if (WAIT_TIMEOUT == WaitForSingleObject(handle, 30*1000))
			{
				MessageBox(NULL, strError, _T("�����ļ���ʱ."), MB_OK);
			}

			// ��� �����ļ�Update.xml��false -- ������true -- ������
			if (true == CheckUpdateFile(strIP))
			{
				UpdateLog(strIP, _T("�����ļ����������ִ�л�����Ҫ����.\r\n"));
				g_pTalk->SendText(EXECUTE_RESULT_TYPE_FAILED, 2, MT_CHECKRES_MESG, dwAddress);
				UpdateLog(strIP, _T("'�����'��ϢMT_CHECKRES_MESG �ѷ���.\r\n"));
			}
			else
			{
				UpdateLog(strIP, _T("�����ļ����������ִ�л���Ҫ����.\r\n"));
				g_pTalk->SendText(EXECUTE_RESULT_TYPE_SUCCESS, 2, MT_CHECKRES_MESG, dwAddress);
				UpdateLog(strIP, _T("'�����'��ϢMT_CHECKRES_MESG �ѷ���.\r\n"));
				// ����1s��������һ����Ϣ��Ҫ��ͻ��˽�������
				Sleep(1000);
				UpdateLog(strIP, _T("ִ�л�'�������ɽ���'׼��...\r\n"));
				g_pTalk->SendText("", 0, MT_ENDPRO_MESG, dwAddress);
				UpdateLog(strIP, _T("'��������'��ϢMT_ENDPRO_MESG �ѷ���.\r\n"));
			}
		
			strLog = _T("");
			break;
		}
		case MT_ENDPRORES_MESG:		// �ͻ��˽������̽����Ϣ
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// ��������exe �ɹ�����ʼ�����ļ�
			{
				strLog = strLogHead + _T("�յ�'��������'�����Ϣ-MT_ENDPRORES_MESG,׼�����͸��°�...\r\n");
				UpdateLog(strIP, strLog);
				g_pTalk->SendText("", 0, MT_UPDATE_MESG, dwAddress);
				UpdateLog(strIP, _T("'׼�������ļ�'��ϢMT_UPDATE_MESG �ѷ���,�����ļ�...\r\n"));
				strLog = _T("");

				// ����1s�����ͻ���ʱ����Ӧ��������
				Sleep(1000);

				// ��ʼ�����ļ����ļ�Ϊ��ǰվ���ڵ������ļ��������������ӹ�ѡ���� to do��
				unsigned int recvthreadID = 0;
				HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&SendFileThreadProc,
					this, 0, &recvthreadID);
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("�յ�'��������'�����Ϣ-MT_ENDPRORES_MESG�� ִ�л����������쳣��\r\n");
			}
			else
			{
				strLog = strLogHead + _T("�յ�'��������'�����Ϣ-MT_ENDPRORES_MESG�� ����ʶ��������ϵά����Ա��\r\n");
			}
			//UpdateLog(strIP, strLog);
			break;
		}
		case MT_UPDATERES_MESG:		// �ͻ��˽��� �����ļ������Ϣ
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// ���� �����ļ��ɹ�����������exe��Ϣ
			{
				strLog = strLogHead + _T("�յ�'���ո��°�'�����Ϣ-MT_UPDATERES_MESG,׼�����͸��°�...\r\n");
				UpdateLog(strIP, strLog);
				g_pTalk->SendText("", 0, MT_BEGINPRO_MESG, dwAddress);
				UpdateLog(strIP, _T("'�������ɽ���'��ϢMT_BEGINPRO_MESG �ѷ���.\r\n"));
				strLog = _T("");
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("�յ�'���ո��°�'�����Ϣ-MT_UPDATERES_MESG�� ִ�л������ļ�ʧ�ܣ�\r\n");
			}
			else
			{
				strLog = strLogHead + _T("�յ�'���ո��°�'�����Ϣ-MT_UPDATERES_MESG�� ����ʶ��������ϵά����Ա��\r\n");
			}
			break;
		}
		case MT_BEGINPRORES_MESG:	// �ͻ����������ɽ���exe �����Ϣ
		{
			if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_SUCCESS) == 0)		// ��������exe�ɹ������˸��½���
			{
				// to do 
				strLog = strLogHead + _T("�յ�'�������ɽ���'�����Ϣ-MT_BEGINPRORES_MESG,���³ɹ���\r\n");
			}
			else if (strcmp(pHeader->data(), EXECUTE_RESULT_TYPE_FAILED) == 0)
			{
				strLog = strLogHead + _T("�յ�'�������ɽ���'�����Ϣ-MT_BEGINPRORES_MESG�� ִ�л��������ɽ���ʧ�ܣ�\r\n");
			}
			else
			{
				strLog = strLogHead + _T("�յ�'�������ɽ���'�����Ϣ-MT_BEGINPRORES_MESG�� ����ʶ��������ϵά����Ա��\r\n");
			}
			break;
		}
	}
	UpdateLog(strIP, strLog);
}


/* ����˽����ļ��ɹ�������Ϣ��
	wParamΪ0 -- ��ʾ���յ���ִ�л�Ŀ¼�ṹ�ļ�*/
LRESULT CUpdateServer::OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	// ��ȡִ�л�exeĿ¼��Ϣ�ļ� ��Ϣ����
	if (wParam == 0)
	{
		CDuiString strIP = m_pRemoteDirIP->GetText();
		CDuiString strOutput = _T("");
		string strPath = PublicFunction::GetCurrentRunPath() + "\\" + 
			PublicFunction::W2M(strIP.GetData()) + SYSFILEINFO;

		// ���ļ�
		FILE *pFile = NULL;
		errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
		if (err != 0 || pFile == NULL)
		{
			strOutput.Format(_T("���ļ�%sʧ��"),CDuiString(PublicFunction::M2W(strPath.c_str())));
			MessageBox(NULL, strOutput, _T("Error"), MB_OK);
			return 0;
		}

		// �ļ�ָ�붨λ���ļ�ͷ
		fseek(pFile, 0, SEEK_SET);

		// ����ռ�
		FILEINFO *pFileInfo = NULL;
		pFileInfo = (FILEINFO*)malloc(sizeof(FILEINFO));
		if (pFileInfo == NULL)
		{
			fclose(pFile);
			MessageBox(NULL, _T("malloc����ռ�ʧ��"), _T("Error"), MB_OK);
			return 0;
		}

		// �����������
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
			// ���µ�list
			int count = m_pRemoteDirList->GetCount();
			m_pRemoteDirList->SetTextCallback(this);

			CListTextElementUI* pListElement = new CListTextElementUI;
			pListElement->SetTag(count);
			if (pListElement != NULL)
			{
				::PostMessageW(GetHWND(), WM_ADDLISTITEM, RemoteDirList, (LPARAM)pListElement);
			}
		}

		// �ͷ���Դ
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

	// 1���������ļ�
	std::string strPath = m_curPath + "\\" + ip + UPDATELOG;
	std::string strLog = PublicFunction::W2M(strCurTime.GetData())
				+ PublicFunction::W2M(log.GetData());
	bool bRet = PublicFunction::SaveLogToFile(strPath, strLog);
	// 2����ʾ�����棨to do��
}
