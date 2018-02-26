#include "stdafx.h"
#include "UpdateServer.h"
#include <Shlobj.h>		// BROWSEINFO �ṹ����Ҫ

#include "PublicFunction.h"

#define WM_MODIFYLISTITEM WM_USER + 51
#define WM_DELETELISTITEM WM_USER + 52
#define WM_GROUPTALK	WM_USER + 105    


#define EXECUTE_RESULT_TYPE_SUCCESS		"1"
#define EXECUTE_RESULT_TYPE_FAILED		"0"

#define IP_LENGHT		16
struct RemoteInfo
{
	TCHAR ip[IP_LENGHT];
	u_short fileport;
	u_short msgport;
	UINT state;
};

enum
{
	OFFLINE = 0,
	ONLINE
};
/*
* ���ִ�л�����
*/
std::vector<RemoteInfo> g_host;


// ��ű���Ŀ¼�ļ���Ϣ
std::vector<FileInfo> g_localFile;

// ��� ִ�л�Ŀ¼�ļ���Ϣ
std::vector<FileInfo> g_remoteFile;

// UDP ��Ϣ
CGroupTalk* g_pTalk;


//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CMainPage, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
DUI_END_MESSAGE_MAP()

CMainPage::CMainPage()
{
	m_pPaintManager = NULL;
}

void CMainPage::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	m_pPaintManager = pPaintMgr;
}

void CMainPage::OnClick(TNotifyUI& msg)
{

}

void CMainPage::OnSelectChanged(TNotifyUI &msg)
{

}

void CMainPage::OnItemClick(TNotifyUI &msg)
{

}

//////////////////////////////////////////////////////////////////////////
///
DUI_BEGIN_MESSAGE_MAP(CUpdateServer, WindowImplBase)
DUI_END_MESSAGE_MAP()

CUpdateServer::CUpdateServer()
{
	InitSocket();//����2.2�汾WinSock��
	m_running = false;

	m_pMenu = NULL;
	m_MainPage.SetPaintMagager(&m_pm);
	//AddVirtualWnd(_T("mainpage"), &m_MainPage);
}


CUpdateServer::~CUpdateServer()
{
	m_running = false;

	CMenuWnd::DestroyMenu();
	if (m_pMenu != NULL) {
		delete m_pMenu;
		m_pMenu = NULL;
	}
	//RemoveVirtualWnd(_T("mainpage"));
}

LPCTSTR CUpdateServer::GetWindowClassName() const
{
	return _T("MainWnd");
}

UINT CUpdateServer::GetClassStyle() const
{
	return CS_DBLCLKS;
}

DuiLib::CDuiString CUpdateServer::GetSkinFile()
{
	return _T("XML_MAIN");
}

void CUpdateServer::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
}

CControlUI* CUpdateServer::CreateControl(LPCTSTR pstrClass)
{
	if (lstrcmpi(pstrClass, _T("CircleProgress")) == 0) {
		return new CCircleProgressUI();
	}
	return NULL;
}


void CUpdateServer::InitWindow()
{
	// �����Խӿ�
	CResourceManager::GetInstance()->SetTextQueryInterface(this);
	//CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
	// Ƥ���ӿ�
	//CSkinManager::GetSkinManager()->AddReceiver(this);

	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	
	Init();
	//m_pSkinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("skinbtn")));
	// ��ʼ��WebBrowser�ؼ�
	/*CWebBrowserUI* pBrowser1 = static_cast<CWebBrowserUI*>(m_pm.FindControl(_T("oneclick_browser1")));
	pBrowser1->SetWebBrowserEventHandler(this);
	CWebBrowserUI* pBrowser2 = static_cast<CWebBrowserUI*>(m_pm.FindControl(_T("oneclick_browser2")));
	pBrowser2->SetWebBrowserEventHandler(this);
	pBrowser1->NavigateUrl(_T("http://blog.csdn.net/duisharp"));
	pBrowser2->NavigateUrl(_T("http://www.winradar.com"));*/

	// ��̬����Combo
	/*CComboUI* pFontSize = static_cast<CComboUI*>(m_pm.FindControl(_T("font_size")));
	if (pFontSize)
	{
		CListLabelElementUI * pElement = new CListLabelElementUI();
		pElement->SetText(_T("���Գ�����"));
		pElement->SetFixedHeight(30);
		pElement->SetFixedWidth(120);
		pFontSize->Add(pElement);
	}
	CComboUI* pCombo = new CComboUI();
	pCombo->SetName(_T("mycombo"));
	pCombo->SetFixedWidth(80);
	pCombo->ApplyAttributeList(m_pm.GetStyle(_T("combo_style")));
	pCombo->SetAttribute(_T("endellipsis"), _T("true"));
	pCombo->SetAttribute(_T("itemendellipsis"), _T("true"));
	CContainerUI* pParent = (CContainerUI*)pFontSize->GetParent();
	pParent->Add(pCombo);
	if (pCombo)
	{
		pCombo->SetFloat(true);
		pCombo->SetFixedXY(CDuiSize(140, 0));
		pCombo->SetItemFont(2);
		CListLabelElementUI * pElement = new CListLabelElementUI();
		pElement->SetText(_T("��̬���ݶ�̬����"));
		pElement->SetFixedHeight(30);
		pElement->SetFixedWidth(120);
		pCombo->Add(pElement);
		pCombo->SelectItem(0);
	}

	// List�ؼ����Ԫ��
	CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("listview")));
	CListContainerElementUI* pListItem = new CListContainerElementUI();
	pListItem->SetChildVAlign(DT_VCENTER);
	pListItem->SetFixedHeight(30);
	pListItem->SetManager(&m_pm, NULL, false);
	pListItem->SetFixedWidth(100);
	pList->Add(pListItem);
	CButtonUI* pBtn1 = new CButtonUI();
	pBtn1->SetManager(&m_pm, NULL, false);
	pBtn1->SetAttribute(_T("style"), _T("btn_style"));
	pBtn1->SetText(_T("���밢��"));
	pBtn1->SetFixedHeight(20);
	pBtn1->SetFixedWidth(30);
	pListItem->Add(pBtn1);
	CButtonUI* pBtn2 = new CButtonUI();
	pBtn2->SetManager(&m_pm, NULL, false);
	pBtn2->SetAttribute(_T("style"), _T("btn_style"));
	pBtn2->SetText(_T("20001"));
	pListItem->Add(pBtn2);

	CDialogBuilder builder1;
	CListContainerElementUI* pListItem1 = (CListContainerElementUI*)builder1.Create(_T("listitem.xml"), NULL, this, &m_pm, NULL);

	pList->Add(pListItem1);
	CControlUI* pLabel = pListItem1->FindSubControl(_T("troy"));
	if (pLabel != NULL) pLabel->SetText(_T("abc_troy"));
	for (int i = 0; i < 20; i++)
	{
		CListTextElementUI* pItem = new CListTextElementUI();
		pItem->SetFixedHeight(30);
		pList->Add(pItem);
		pItem->SetText(0, _T("����"));
		pItem->SetText(1, _T("1000"));
		pItem->SetText(2, _T("100"));
	}

	CTreeViewUI* pTreeView = static_cast<CTreeViewUI*>(m_pm.FindControl(_T("treeview")));
	CTreeNodeUI* pItem = new CTreeNodeUI();
	pItem->SetFixedHeight(30);
	pItem->SetItemText(_T("��̬���"));
	pTreeView->AddAt(pItem, 3);
	COptionUI* pRadio = new COptionUI();
	pRadio->SetText(_T("��ѡ��ť"));
	pItem->Add(pRadio);
	pRadio->SetAttribute(_T("Style"), _T("cb_style"));
	pItem->SetAttribute(_T("itemattr"), _T("valign=&quot;center&quot;"));
	pItem->SetAttribute(_T("Style"), _T("treeview_style"));

	CDialogBuilder builder;
	CControlUI* pParentItem = NULL;
	CTreeNodeUI* pTreeItem = (CTreeNodeUI*)builder.Create(_T("treeitem.xml"), NULL, this, &m_pm, pParentItem);
	if (pParentItem == NULL) pTreeView->Add(pTreeItem);

	// ͼ��ؼ�
	CChartViewUI *pHistpgramView = static_cast<CChartViewUI*>(m_pm.FindControl(_T("ChartView_Histpgram")));
	if (NULL != pHistpgramView)
	{
		pHistpgramView->Add(_T("1��{c #FE5900}13%{/c}"), 13);
		pHistpgramView->Add(_T("2��{c #FE5900}11%{/c}"), 11);
		pHistpgramView->Add(_T("3��{c #FE5900}32%{/c}"), 32);
		pHistpgramView->Add(_T("4��{c #FE5900}17%{/c}"), 17);
		pHistpgramView->Add(_T("5��{c #FE5900}8%{/c}"), 8);
		pHistpgramView->Add(_T("6��{c #FE5900}12%{/c}"), 12);
	}

	CChartViewUI *pPieView = static_cast<CChartViewUI*>(m_pm.FindControl(_T("ChartView_Pie")));
	if (NULL != pPieView)
	{
		pPieView->Add(_T("����{c #FE5900}35%{/c}"), 35);
		pPieView->Add(_T("�Ϻ�{c #FE5900}38%{/c}"), 38);
		pPieView->Add(_T("����{c #FE5900}35%{/c}"), 35);
		pPieView->Add(_T("���{c #FE5900}15%{/c}"), 15);
	}*/

	// ��������
	//CRollTextUI* pRollText = (CRollTextUI*)m_pm.FindControl(_T("rolltext"));
	//pRollText->SetText(_T("����5000����ʹ��\n������ Chrome ����ѵĹ��������\n����ֹ���з��˵Ĺ�漰��������͸��١�"));
	//pRollText->BeginRoll(ROLLTEXT_UP, 200, 20);		//�˶���ʽ���ٶȣ�ʱ��

	//												// ��ɫ��ʹ��
	//CColorPaletteUI* pColorPalette = (CColorPaletteUI*)m_pm.FindControl(_T("Pallet"));
	//pColorPalette->SetSelectColor(0xff0199cb);

	
	// ��չList -- remote list
	//m_pRemoteList = static_cast<CListExUI*>(m_pm.FindControl(_T("machineList")));
	//m_pRemoteList->InitListCtrl();
	/*for (int i = 0; i < 10; i++)
	{
		CListTextExtElementUI* pItem = new CListTextExtElementUI();
		pItem->SetFixedHeight(30);
		pListEx->Add(pItem);
		pItem->SetAttribute(_T("style"), _T("listex_item_style"));
		pItem->SetText(1, _T("����"));
		pItem->SetText(2, _T("1000"));
		pItem->SetText(3, _T("100"));
	}*/
	// ע������ͼ��
	//m_trayIcon.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("Duilib��ʾ��ȫ"));
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

		RemoteInfo remoteInfo;
		CDuiString strWideIP = PublicFunction::M2W(strRemoteIP.c_str());
		_stprintf_s(remoteInfo.ip, _T("%s"), strWideIP.GetData());
		remoteInfo.fileport = usFPort;
		remoteInfo.msgport = usMultiPort;
		remoteInfo.state = OFFLINE;

		g_host.push_back(remoteInfo);
	}

	HWND handle = GetHWND();
	while (::GetParent(handle) != NULL) handle = ::GetParent(handle);

	if (strMultiAddr == "" || usMultiPort == 0)
	{
		g_pTalk = new CGroupTalk(GetHWND(), ::inet_addr(GROUP_ADDRESS), GROUP_PORT, MSG_FROM_SERVER);
		//g_pTalk = new CGroupTalk(GetHWND(), ADDR_ANY, GROUP_PORT, MSG_FROM_SERVER);
	}
	else
	{
		g_pTalk = new CGroupTalk(GetHWND(), ::inet_addr(strMultiAddr.c_str()), usMultiPort, MSG_FROM_SERVER);
		//g_pTalk = new CGroupTalk(GetHWND(), ADDR_ANY, usMultiPort, MSG_FROM_SERVER);
	}

	// ��չList -- remote list
	m_pRemoteList = static_cast<CListExUI*>(m_pm.FindControl(_T("machineList")));
	m_pRemoteList->InitListCtrl();
	//m_pRemoteList = static_cast<CListUI*>(m_pm.FindControl(_T("machineList")));
	m_pHostIP = static_cast<CIPAddressUI*>(m_pm.FindControl(_T("host")));
	m_pHostIP->SetIP(0);

	m_pFileportEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("fileport")));
	m_pMsgportEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("msgport")));

	m_pRemoteDirIP = static_cast<CEditUI*>(m_pm.FindControl(_T("remotedir")));
	m_pRemoteDirList = static_cast<CListUI*>(m_pm.FindControl(_T("remotefilelist")));

	m_pLocalDirList = static_cast<CListUI*>(m_pm.FindControl(_T("localfilelist")));
	m_pLocalDir = static_cast<CEditUI*>(m_pm.FindControl(_T("localdir")));

	m_pLogEdit = static_cast<CRichEditUI*>(m_pm.FindControl(_T("logedit")));

	CVerticalLayoutUI*  pui = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("layer1-top")));
	if (pui) {
		pui->SetVisible(true);
	}

	for (int index = 0;index < g_host.size(); index++)
	{
		CListTextExtElementUI* pItem = new CListTextExtElementUI();
		//pItem->SetManager(&m_pm, NULL, false);
		pItem->SetFixedHeight(30);
		m_pRemoteList->Add(pItem);

		pItem->SetAttribute(_T("style"), _T("listex_item_style"));
		pItem->SetText(1, g_host[index].ip);

		TCHAR szBuf[MAX_PATH] = { 0 };
		_stprintf_s(szBuf, _T("%d"), g_host[index].fileport);

		pItem->SetText(2, szBuf);
		_stprintf_s(szBuf, _T("%d"), g_host[index].msgport);
		pItem->SetText(3, szBuf);

		pItem->SetImageboxState(g_host[index].state);
	
		/*CListContainerElementUI* pListItem = new CListContainerElementUI();
		//pListItem->SetChildVAlign(DT_VCENTER);
		pListItem->SetFixedHeight(30);
		pListItem->SetManager(&m_pm, NULL, false);
		m_pRemoteList->Add(pListItem);

		CCheckBoxUI* pCheckBox = new CCheckBoxUI();
		pCheckBox->SetManager(&m_pm, NULL, false);
		pCheckBox->SetFixedHeight(16);
		pCheckBox->SetFixedWidth(16);
		pCheckBox->SetMaxWidth(16);
		pCheckBox->SetAttribute(_T("style"), _T("checkbox_style"));
		pListItem->Add(pCheckBox);

		CLabelUI *pHostLabel = new CLabelUI();
		pHostLabel->SetManager(&m_pm, NULL, false);
		pHostLabel->SetFixedHeight(30);
		pHostLabel->SetAttribute(_T("align"), _T("center"));
		pHostLabel->SetText(g_host[index]);
		pListItem->Add(pHostLabel);

		CLabelUI *pFilePortLabel = new CLabelUI();
		pFilePortLabel->SetManager(&m_pm, NULL, false);
		pFilePortLabel->SetFixedHeight(30);
		pFilePortLabel->SetAttribute(_T("align"), _T("center"));

		TCHAR szBuf[MAX_PATH] = { 0 };
		_stprintf_s(szBuf, _T("%d"), g_fileport[index]);
		pFilePortLabel->SetText(szBuf);
		pListItem->Add(pFilePortLabel);

		CLabelUI *pMsgPortLabel = new CLabelUI();
		pMsgPortLabel->SetManager(&m_pm, NULL, false);
		pMsgPortLabel->SetFixedHeight(30);
		pMsgPortLabel->SetAttribute(_T("align"), _T("center"));

		_stprintf_s(szBuf, _T("%d"), g_msgport[index]);
		pMsgPortLabel->SetText(szBuf);
		pListItem->Add(pMsgPortLabel);*/

		// add by zhoupeng  ��̬��������״̬ͼ��
		/*CButtonUI *pStateLabel = new CButtonUI();
		pStateLabel->SetManager(&m_pm, NULL, false);
		pStateLabel->SetAttribute(_T("style"), _T("online_style"));
		pStateLabel->SetFixedHeight(20);
		pStateLabel->SetFixedWidth(20);
		pItem->Add(pStateLabel);*/
	}
}

void CUpdateServer::Notify(TNotifyUI& msg)
{
	CDuiString name = msg.pSender->GetName();
	if (msg.sType == _T("windowinit"))
	{
	}
	else if (msg.sType == _T("click"))
	{
		if (name.CompareNoCase(_T("closebtn")) == 0)
		{
			if (m_running == true)
			{
				MessageBox(m_hWnd, _T("�и��������������У������˳�."), _T("��ʾ"), MB_OK);
				return;
			}
			if(IDYES == MessageBox(m_hWnd, _T("ȷ���˳����ݲɼ�ϵͳ���³���"), _T("��ʾ"), MB_YESNO))
			{
				::DestroyWindow(m_hWnd);
			}			
			return;
		}
		else if (name.CompareNoCase(_T("minbtn")) == 0 )
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if (name.CompareNoCase(_T("maxbtn")) == 0 )
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return;
		}
		else if (name.CompareNoCase(_T("restorebtn")) == 0 )
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return;
		}
		else if (name.CompareNoCase(_T("addbtn")) == 0 )
		{
			OnAddRemote();
		}
		else if (name.CompareNoCase(_T("modifybtn")) == 0 )
		{
			OnModifyRemote();
		}
		else if (name.CompareNoCase(_T("deletebtn")) == 0 )
		{
			OnDeleteRemote();
			//msg.pSender->SetTag(NULL);
		}
		else if (name.CompareNoCase(_T("localdirtn")) == 0 )
		{
			CDuiString strLocalDir = SelectDir();
			m_pLocalDir->SetText(strLocalDir);
			if (strLocalDir.GetLength() != 0)
			{
				InitLocalDirToList(strLocalDir);
			}
		}
		else if (name.CompareNoCase(_T("updatebtn")) == 0 )
		{
			// ��� �Զ����°�ť
			OnUpdateSystem();
		}
		else if (name.CompareNoCase(_T("batupdatebtn")) == 0 )
		{
			// ����������������
			BatStartUpdate();
			//g_pTalk->SendMessageTest();
		}
		else if (name.CompareNoCase(_T("refreshbtn")) == 0 )
		{
			// �ȹ㲥hello��Ϣ��Ȼ��ˢ�½���
			g_pTalk->SendText("", 0, MT_JION);
			//UpdateRemoteState();
		}
	}
	else if (msg.sType == _T("return"))
	{
		if (name.CompareNoCase(_T("remotedir")) == 0 )
		{
			// to do
			MessageBox(NULL, _T("click enter"), _T("remotedir"), 0);
			return;
		}
		// ������վ����ļ�����Ϣ��ʾ����
		else if (name.CompareNoCase(_T("localdir")) == 0)
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

		// ���ִ�л������ݣ���ʾ��edit����
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{			
			int iSel = m_pRemoteList->GetCurSel();	// ���д��룬��bug������¼���Ҫ������Σ����������Ӧ��ȷ����
			if (iSel < 0) return;
			OnClickItem(iSel);			
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
	else if (msg.sType == _T("selectchanged"))
	{
		CTabLayoutUI* pTabSwitch = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tab_switch")));
		if (name.CompareNoCase(_T("basic_tab")) == 0) pTabSwitch->SelectItem(0);
		if (name.CompareNoCase(_T("rich_tab")) == 0) pTabSwitch->SelectItem(1);
		if (name.CompareNoCase(_T("ex_tab")) == 0) pTabSwitch->SelectItem(2);
		if (name.CompareNoCase(_T("ani_tab")) == 0) pTabSwitch->SelectItem(3);
		if (name.CompareNoCase(_T("split_tab")) == 0) pTabSwitch->SelectItem(4);
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


LRESULT CUpdateServer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// �ͷſؼ��������뿪��Ϣ
	delete g_pTalk;

	::PostQuitMessage(0L);
	bHandled = FALSE;
	return 0;
}


LRESULT CUpdateServer::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// �رմ��ڣ��˳�����
	if (uMsg == WM_DESTROY)
	{
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	else if (uMsg == WM_TIMER)
	{
		bHandled = FALSE;
	}
	else if (uMsg == WM_SHOWWINDOW)
	{
		bHandled = FALSE;
		m_pMinBtn->NeedParentUpdate();
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
	else if (uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN) {
		int a = 0;
	}
	else if (uMsg == WM_MENUCLICK)
	{		
		bHandled = TRUE;
		return 0;
	}
	else if (uMsg == UIMSG_TRAYICON)
	{
		
	}
	else if (uMsg == WM_GROUPTALK)
	{
		OnWMGROUPTALK(uMsg, wParam, lParam, bHandled);
	}
	else if (uMsg == FILE_TRANSFER_SUCCESS)
	{
		OnFileTranferOver(uMsg, wParam, lParam, bHandled);
	}
	bHandled = FALSE;
	return 0;
}


//LRESULT CUpdateServer::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	LRESULT lRes = 0;
//	BOOL bHandled = TRUE;
//	switch (uMsg) {
//	case WM_ADDLISTITEM:   lRes = OnAddListItem(uMsg, wParam, lParam, bHandled); break;
//	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
//	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
//	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
//	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
//	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
//	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
//	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
//	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
//	case WM_GROUPTALK:	   lRes = OnWMGROUPTALK(uMsg, wParam, lParam, bHandled); break;
//	case FILE_TRANSFER_SUCCESS:		lRes = OnFileTranferOver(uMsg, wParam, lParam, bHandled); break;
//	//case WM_RBUTTONDOWN:   break;
//	default:
//		bHandled = FALSE;
//	}
//	if (bHandled) return lRes;
//	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
//	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
//}

LRESULT CUpdateServer::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////

void CUpdateServer::OnAddRemote()
{
	if (!m_pRemoteList || !m_pHostIP || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostIP->GetText();
	CDuiString strfileport = m_pFileportEdit->GetText();
	CDuiString strmsgport = m_pMsgportEdit->GetText();

	if (strhost.GetLength() == 0 || strfileport.GetLength() == 0 ||
		strmsgport.GetLength() == 0)
	{
		return;
	}

	for (size_t index = 0; index < g_host.size();index++)
	{
		if (strhost == g_host[index].ip)
		{
			MessageBox(NULL, _T("�����ظ����ִ�л�IP"), _T("Error"), MB_OK);
			return;
		}
	}

	// д��ȫ�ֱ���
	RemoteInfo remoteInfo;
	_stprintf_s(remoteInfo.ip, _T("%s"), strhost.GetData());
	remoteInfo.fileport = _ttoi(strfileport);
	remoteInfo.msgport = _ttoi(strmsgport);
	remoteInfo.state = OFFLINE;

	g_host.push_back(remoteInfo);

	// д�������ļ�
	UpdateConfigIni();

	// ��յ�ǰ����
	m_pHostIP->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	// ��ʾ������
	CListTextExtElementUI* pItem = new CListTextExtElementUI();
	pItem->SetFixedHeight(30);
	m_pRemoteList->Add(pItem);

	pItem->SetAttribute(_T("style"), _T("listex_item_style"));
	pItem->SetText(1, strhost);
	pItem->SetText(2, strfileport);
	pItem->SetText(3, strmsgport);

	// ������Ӧִ�л����ļ��У���IP��������������µĹ����ļ�
	CDuiString strRemoteFolderPath = _T("");
	//strRemoteFolderPath.Format(_T("%s\\%s"),PublicFunction::M2W(m_curPath.c_str()), strhost);
	strRemoteFolderPath = CDuiString(PublicFunction::M2W(m_curPath.c_str())) + _T("\\") + strhost;
	string strPath = PublicFunction::W2M(strRemoteFolderPath.GetData());
	if (false == PublicFunction::IsDirExisted(strPath))
	{
		if (NULL == CreateDirectory(strRemoteFolderPath, NULL))
		{
			CDuiString error = _T("");
			error.Format(_T("�����ļ�%sʧ�ܣ������룺%d\r\n"), strRemoteFolderPath, GetLastError());
			MessageBox(NULL, error, _T("Error"), MB_OK);
			return;
		}
	}
}

void CUpdateServer::OnModifyRemote()
{
	if (!m_pRemoteList || !m_pHostIP || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostIP->GetText();
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
		if (strhost == g_host[index].ip)
		{
			g_host[index].fileport = _ttoi(strfileport);
			g_host[index].msgport = _ttoi(strmsgport);
			bIsExisted = true;
			
			// �޸�Itemֵ
			//m_pRemoteList->RemoveAt(index);
			CControlUI *p = m_pRemoteList->GetItemAt(index);
			CListTextExtElementUI *pItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
			if (pItem != NULL)
			{
				pItem->SetText(2, strfileport);
				pItem->SetText(3, strmsgport);
			}
			break;
		}
	}
	if (bIsExisted != true)
	{
		MessageBox(NULL, _T("�����޸���δ���ִ�л�IP"), _T("Error"), MB_OK);
		return;
	}	
	
	UpdateConfigIni();

	m_pHostIP->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	//CListTextExtElementUI* pItem = new CListTextExtElementUI();
	//pItem->SetFixedHeight(30);
	//m_pRemoteList->Add(pItem);
	//pItem->SetAttribute(_T("style"), _T("listex_item_style"));

	//pItem->SetText(1, strhost);
	//pItem->SetText(2, strfileport);
	//pItem->SetText(3, strmsgport);
}

void CUpdateServer::OnDeleteRemote()
{
	if (!m_pRemoteList || !m_pHostIP || !m_pFileportEdit || !m_pMsgportEdit)
	{
		return;
	}

	CDuiString strhost = m_pHostIP->GetText();
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
		if (strhost == g_host[index].ip)
		{
			bIsExisted = true;
			m_pRemoteList->RemoveAt(index);
			g_host.erase(g_host.begin() + index);
			break;
		}
	}
	if (bIsExisted != true)
	{
		MessageBox(NULL, _T("����ɾ����δ���ִ�л�IP"), _T("Error"), MB_OK);
		return;
	}
	UpdateConfigIni();

	m_pHostIP->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));
}

void CUpdateServer::OnClickItem(int iSel)
{
	if (iSel < 0 || g_host.size() < iSel || m_pMsgportEdit == NULL)
	{
		return;
	}
	CDuiString strTmp = _T("");
	string strRemoteIP = PublicFunction::W2M(g_host[iSel].ip);

	// ��ʾ����ѡ��ִ�л���Ϣ
	m_pHostIP->SetIP(PublicFunction::ReserveIP(strRemoteIP));
	strTmp.Format(_T("%d"), g_host[iSel].fileport);
	m_pFileportEdit->SetText(strTmp);
	strTmp.Format(_T("%d"), g_host[iSel].msgport);
	m_pMsgportEdit->SetText(strTmp);

	// ��ʾ��ǰѡ��ִ�л�������־
	if (!m_pLogEdit)
	{
		return;
	}
	m_pLogEdit->SetText(_T(""));
	

	
	std::string strPath = m_curPath + "\\" + strRemoteIP + UPDATELOG;

	string strLog = PublicFunction::ReadUpdateLog(strPath);


	/*char szBuffer[1024] = { 0 };

	FILE *pFile = NULL;
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	if (err != 0 || pFile == NULL)
	{
		return;
	}

	while (!feof(pFile))
	{
		err = fread_s(szBuffer, 1024 * sizeof(char), sizeof(char), 1024, pFile);
		if (err == 0)
		{
			fclose(pFile);
			pFile = NULL;
			break;
		}
		strLog += szBuffer;
	}
	
	fclose(pFile);*/

	m_pLogEdit->SetText(PublicFunction::M2W(strLog.c_str()));
	SIZE siz;
	siz.cx = 0;
	siz.cy = 0;
	siz = m_pLogEdit->GetScrollRange();
	m_pLogEdit->SetScrollPos(siz);
}

void CUpdateServer::UpdateConfigIni()
{
	// д�������ļ�
	string configPath = m_curPath + CONFIG_NAME;

	TCHAR szPath[MAX_PATH] = { 0 };
	TCHAR szCount[4] = { 0 };	// IP������256
	TCHAR szSection[10] = { 0 };
	TCHAR szFileport[10] = { 0 };
	int count = g_host.size();

	_stprintf_s(szPath, _T("%s"), PublicFunction::M2W(configPath.c_str()).GetData());
	_stprintf_s(szCount, _T("%d"), count );
	WritePrivateProfileString(_T("RemoteList"), _T("count"), szCount, szPath);
	for (int index = 0;index < count;index++ )
	{
		_stprintf_s(szSection, _T("Remote%d"), index + 1);
		_stprintf_s(szFileport, _T("%d"), g_host[index].fileport);
		WritePrivateProfileString(szSection, _T("IP"), g_host[index].ip, szPath);
		WritePrivateProfileString(szSection, _T("fileport"), szFileport, szPath);
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

	unsigned short usPort = FILE_TRANSFER_PORT_DEFAULT;
	// ����IP ��ȡ�ļ�����Ķ˿ں�
	for (int index = 0;index < g_host.size();index++)
	{
		string tmpip = PublicFunction::W2M(g_host[index].ip);
		if (strcmp(tmpip.c_str(), strIP.c_str()) == 0)
		{
			usPort = g_host[index].fileport;
			break;
		}
	}

	string strerror = "";
	CDuiString error = _T("");
	string strPath = PublicFunction::GetCurrentRunPath() + "\\" + strIP;
	RecvFile *fileRecv = new RecvFile(NULL, strPath, usPort);
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
	CDuiString strPort = pUpdateObj->m_pFileportEdit->GetText();
	unsigned short usPort = atoi(PublicFunction::W2M(strPort.GetData()).c_str());

	string strerror = "";
	CDuiString error = _T("");
	string strPath = PublicFunction::GetCurrentRunPath() + "\\" + PublicFunction::W2M(strIP.GetData());
	RecvFile *fileRecv = new RecvFile(pUpdateObj->m_hWnd, strPath, usPort);
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

	unsigned short usPort = FILE_TRANSFER_PORT_DEFAULT;
	// ����IP ��ȡ�ļ�����Ķ˿ں�
	for (int index = 0;index < g_host.size();index++)
	{
		string tmpip = PublicFunction::W2M(g_host[index].ip);
		if (strcmp(tmpip.c_str(), strIP.c_str()) == 0)
		{
			usPort = g_host[index].fileport;
			break;
		}
	}

	FileSend *fileSend = new FileSend(pUpdateObj->m_hWnd, strIP, usPort);
	
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

			CListTextElementUI* pItem = new CListTextElementUI();
			pItem->SetFixedHeight(30);
			m_pLocalDirList->Add(pItem);

			pItem->SetText(0, fileinfo.strFileName);
			pItem->SetText(1, fileinfo.strLastDate);
			pItem->SetText(2, fileinfo.strFileType);


			TCHAR szBuf[MAX_PATH] = { 0 };
			if (strFileType == _T("�ļ�"))
			{

				double dLength = ulLength;
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
			}
			pItem->SetText(3, szBuf);
		} 
		
	} while (::FindNextFile(handle, &data) != 0 && ::GetLastError() != ERROR_NO_MORE_FILES);

	FindClose(handle);
}

void CUpdateServer::InitRemoteDirToList(int index)
{
	CDuiString host = g_host[index].ip;
	u_short usFilePort = g_host[index].fileport;
	u_short usMsgPort = g_host[index].msgport;
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
//LPCTSTR CUpdateServer::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
//{
//	TCHAR szBuf[MAX_PATH] = { 0 };
//	CDuiString strName = pControl->GetParent()->GetParent()->GetName();
//
//	// ����վ���Ŀ¼�ṹ && ��ֹvector����Խ��
//	if (strName == _T("localfilelist") && iIndex < g_localFile.size())
//	{		
//		switch (iSubItem)
//		{
//		case 0:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_localFile[iIndex].strFileName);
//#else
//			_stprintf(szBuf, g_localFile[iIndex].strFileName.c_str());
//#endif
//		}
//		break;
//		case 1:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_localFile[iIndex].strLastDate);
//#else
//			_stprintf(szBuf, g_localFile[iIndex].strLastDate.c_str());
//#endif
//		}
//		break;
//		case 2:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_localFile[iIndex].strFileType);
//#else
//			_stprintf(szBuf, g_localFile[iIndex].strFileType.c_str());
//#endif
//		}
//		break;
//		case 3:
//		{
//			if (g_localFile[iIndex].strFileType == _T("�ļ���"))
//			{
//				return _T("");
//			}
//			ULONGLONG ulLength = g_localFile[iIndex].ulLength;
//			double dLength = ulLength;
//#ifdef _UNICODE
//			if (ulLength < 1024)	// С��1KB
//			{
//				swprintf_s(szBuf, _T("%lld�ֽ�"), ulLength);
//			}
//			else if (ulLength > 1024*1024 && ulLength < (1024*1024*1000))		// 1MB -- 1000MB
//			{
//				swprintf_s(szBuf, _T("%.2fMB"), dLength/(1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB֮��
//			{
//				swprintf_s(szBuf, _T("%.2fKB"), dLength /1024);
//			}
//			
//#else
//			if (ulLength < 1024)	// С��1KB
//			{
//				sprintf_s(szBuf, ("%lld�ֽ�"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB֮��
//			{
//				sprintf_s(szBuf, ("%.2fKB"), dLength / 1024);
//			}
//#endif
//		}
//		break;
//		}
//	}
//	else if (strName == _T("remotefilelist") && iIndex < g_remoteFile.size())
//	{
//		switch (iSubItem)
//		{
//		case 0:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_remoteFile[iIndex].strFileName);
//#else
//			_stprintf(szBuf, g_remoteFile[iIndex].strFileName.c_str());
//#endif
//		}
//		break;
//		case 1:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_remoteFile[iIndex].strLastDate);
//#else
//			_stprintf(szBuf, g_remoteFile[iIndex].strLastDate.c_str());
//#endif
//		}
//		break;
//		case 2:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_remoteFile[iIndex].strFileType);
//#else
//			_stprintf(szBuf, g_remoteFile[iIndex].strFileType.c_str());
//#endif
//		}
//		break;
//		case 3:
//		{
//			if (g_remoteFile[iIndex].strFileType == _T("�ļ���"))
//			{
//				return _T("");
//			}
//			ULONGLONG ulLength = g_remoteFile[iIndex].ulLength;
//			double dLength = ulLength;
//#ifdef _UNICODE
//			if (ulLength < 1024)	// С��1KB
//			{
//				swprintf_s(szBuf, _T("%lld�ֽ�"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				swprintf_s(szBuf, _T("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB֮��
//			{
//				swprintf_s(szBuf, _T("%.2fKB"), dLength / 1024);
//			}
//
//#else
//			if (ulLength < 1024)	// С��1KB
//			{
//				sprintf_s(szBuf, ("%lld�ֽ�"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB֮��
//			{
//				sprintf_s(szBuf, ("%.2fKB"), dLength / 1024);
//			}
//#endif
//		}
//		break;
//		}
//	}
//	else if (strName == _T("machineList") && iIndex < g_host.size() )
//	{
//		switch (iSubItem)
//		{
//		case 0:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, g_host[iIndex]);
//			/*int iLen = g_host[iIndex].length();
//			LPWSTR lpText = new WCHAR[iLen + 1];
//			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//			::MultiByteToWideChar(CP_ACP, 0, g_host[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
//			_stprintf(szBuf, lpText);
//			delete[] lpText;*/
//#else
//			_stprintf(szBuf, g_host[iIndex].c_str());
//#endif
//		}
//		break;
//		case 1:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, _T("%d"), g_fileport[iIndex]);
//			/*int iLen = g_fileport[iIndex].length();
//			LPWSTR lpText = new WCHAR[iLen + 1];
//			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//			::MultiByteToWideChar(CP_ACP, 0, g_fileport[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
//			_stprintf(szBuf, lpText);
//			delete[] lpText;*/
//#else
//			_stprintf(szBuf, g_fileport[iIndex].c_str());
//#endif
//		}
//		break;
//		case 2:
//		{
//#ifdef _UNICODE		
//			_stprintf_s(szBuf, _T("%d"), g_msgport[iIndex]);
//			/*int iLen = g_msgport[iIndex].length();
//			LPWSTR lpText = new WCHAR[iLen + 1];
//			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//			::MultiByteToWideChar(CP_ACP, 0, g_msgport[iIndex].c_str(), -1, (LPWSTR)lpText, iLen);
//			_stprintf(szBuf, lpText);
//			delete[] lpText;*/
//#else
//			_stprintf(szBuf, g_msgport[iIndex].c_str());
//#endif
//		}
//		break;
//		}
//	}
//
//	pControl->SetUserData(szBuf);
//	return pControl->GetUserData();
//}

/* ��������Ϣ����ӿ� */
LRESULT CUpdateServer::OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != 0)
	{
		::MessageBoxA(m_hWnd, (LPCSTR)lParam, ("����"), 0);
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
			SetRemoteState(strIPW, ONLINE);
			UpdateRemoteState();
			break;
		}
		case MT_LEAVE:		// �û��뿪
		{
			// ��ʾ���û�
			strLog = strLogHead + _T("leave.\r\n");
			SetRemoteState(strIPW, OFFLINE);
			UpdateRemoteState();
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

			CListTextElementUI* pItem = new CListTextElementUI();
			pItem->SetFixedHeight(30);
			m_pRemoteDirList->Add(pItem);

			pItem->SetText(0, fileinfo.strFileName);
			pItem->SetText(1, fileinfo.strLastDate);
			pItem->SetText(2, fileinfo.strFileType);

			TCHAR szBuf[MAX_PATH] = { 0 };
			if (fileinfo.strFileType == _T("�ļ�"))
			{
				ULONGLONG ulLength = fileinfo.ulLength;
				double dLength = ulLength;
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
			}
			pItem->SetText(3, szBuf);
		}

		// �ͷ���Դ
		fclose(pFile);
		free(pFileInfo);		
	}
	return LRESULT(0);
}

void CUpdateServer::UpdateLog(const string ip, const CDuiString log)
{
	SYSTEMTIME tm ;
	CDuiString strCurTime = _T("");

	if (log.GetLength() == 0)
	{
		return;
	}

	GetLocalTime(&tm);
	strCurTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d "), tm.wYear, tm.wMonth, tm.wDay, 
		tm.wHour, tm.wMinute, tm.wSecond);

	// 1���������ļ�
	std::string strPath = m_curPath + "\\" + ip + UPDATELOG;
	std::string strLog = PublicFunction::W2M(strCurTime.GetData())
				+ PublicFunction::W2M(log.GetData());
	bool bRet = PublicFunction::SaveLogToFile(strPath, strLog);
	// 2����ʾ�����棨to do��
}

void CUpdateServer::SetRemoteState(CDuiString strIP, UINT state)
{
	for (int index = 0;index < g_host.size(); index++)
	{
		if (_tcscmp(strIP.GetData(), g_host[index].ip) == 0)
		{
			g_host[index].state = state;
			break;
		}
	}
}

void CUpdateServer::UpdateRemoteState()
{
	for (int index = 0; index < m_pRemoteList->GetCount(); index++)
	{
		CControlUI *p = m_pRemoteList->GetItemAt(index);
		CListTextExtElementUI *pListItem = static_cast<CListTextExtElementUI *>(p->GetInterface(_T("ListTextExElement")));
		if (pListItem != NULL)
		{
			pListItem->SetImageboxState(g_host[index].state);
		}
	}
	
}

void CUpdateServer::BatStartUpdate()
{
	CDuiString strIP = _T("");
	for (int index = 0;index < m_pRemoteList->GetCount(); index++)
	{
		CControlUI *p = m_pRemoteList->GetItemAt(index);
		CListTextExtElementUI *pListItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
		if (pListItem != NULL && pListItem->GetCheck())
		{
			strIP = pListItem->GetText(1);//g_host[index].ip;

			// ��ȡ ��ͻ���ѭ������Ϣ
			DWORD dwAddress = inet_addr(PublicFunction::W2M(strIP.GetData()).c_str());
			g_pTalk->SendText("", 0, MT_BEGIN_UPDATE, dwAddress);
		}
	}
}
