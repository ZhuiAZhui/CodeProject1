#include "stdafx.h"
#include "UpdateServer.h"
#include <Shlobj.h>		// BROWSEINFO 结构体需要

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
* 存放执行机数据
*/
std::vector<RemoteInfo> g_host;


// 存放本地目录文件信息
std::vector<FileInfo> g_localFile;

// 存放 执行机目录文件信息
std::vector<FileInfo> g_remoteFile;

// UDP 消息
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
	InitSocket();//加载2.2版本WinSock库
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
	// 多语言接口
	CResourceManager::GetInstance()->SetTextQueryInterface(this);
	//CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
	// 皮肤接口
	//CSkinManager::GetSkinManager()->AddReceiver(this);

	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	
	Init();
	//m_pSkinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("skinbtn")));
	// 初始化WebBrowser控件
	/*CWebBrowserUI* pBrowser1 = static_cast<CWebBrowserUI*>(m_pm.FindControl(_T("oneclick_browser1")));
	pBrowser1->SetWebBrowserEventHandler(this);
	CWebBrowserUI* pBrowser2 = static_cast<CWebBrowserUI*>(m_pm.FindControl(_T("oneclick_browser2")));
	pBrowser2->SetWebBrowserEventHandler(this);
	pBrowser1->NavigateUrl(_T("http://blog.csdn.net/duisharp"));
	pBrowser2->NavigateUrl(_T("http://www.winradar.com"));*/

	// 动态创建Combo
	/*CComboUI* pFontSize = static_cast<CComboUI*>(m_pm.FindControl(_T("font_size")));
	if (pFontSize)
	{
		CListLabelElementUI * pElement = new CListLabelElementUI();
		pElement->SetText(_T("测试长文字"));
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
		pElement->SetText(_T("动态数据动态数据"));
		pElement->SetFixedHeight(30);
		pElement->SetFixedWidth(120);
		pCombo->Add(pElement);
		pCombo->SelectItem(0);
	}

	// List控件添加元素
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
	pBtn1->SetText(_T("代码阿呆"));
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
		pItem->SetText(0, _T("张三"));
		pItem->SetText(1, _T("1000"));
		pItem->SetText(2, _T("100"));
	}

	CTreeViewUI* pTreeView = static_cast<CTreeViewUI*>(m_pm.FindControl(_T("treeview")));
	CTreeNodeUI* pItem = new CTreeNodeUI();
	pItem->SetFixedHeight(30);
	pItem->SetItemText(_T("动态添加"));
	pTreeView->AddAt(pItem, 3);
	COptionUI* pRadio = new COptionUI();
	pRadio->SetText(_T("单选按钮"));
	pItem->Add(pRadio);
	pRadio->SetAttribute(_T("Style"), _T("cb_style"));
	pItem->SetAttribute(_T("itemattr"), _T("valign=&quot;center&quot;"));
	pItem->SetAttribute(_T("Style"), _T("treeview_style"));

	CDialogBuilder builder;
	CControlUI* pParentItem = NULL;
	CTreeNodeUI* pTreeItem = (CTreeNodeUI*)builder.Create(_T("treeitem.xml"), NULL, this, &m_pm, pParentItem);
	if (pParentItem == NULL) pTreeView->Add(pTreeItem);

	// 图表控件
	CChartViewUI *pHistpgramView = static_cast<CChartViewUI*>(m_pm.FindControl(_T("ChartView_Histpgram")));
	if (NULL != pHistpgramView)
	{
		pHistpgramView->Add(_T("1月{c #FE5900}13%{/c}"), 13);
		pHistpgramView->Add(_T("2月{c #FE5900}11%{/c}"), 11);
		pHistpgramView->Add(_T("3月{c #FE5900}32%{/c}"), 32);
		pHistpgramView->Add(_T("4月{c #FE5900}17%{/c}"), 17);
		pHistpgramView->Add(_T("5月{c #FE5900}8%{/c}"), 8);
		pHistpgramView->Add(_T("6月{c #FE5900}12%{/c}"), 12);
	}

	CChartViewUI *pPieView = static_cast<CChartViewUI*>(m_pm.FindControl(_T("ChartView_Pie")));
	if (NULL != pPieView)
	{
		pPieView->Add(_T("北京{c #FE5900}35%{/c}"), 35);
		pPieView->Add(_T("上海{c #FE5900}38%{/c}"), 38);
		pPieView->Add(_T("广州{c #FE5900}35%{/c}"), 35);
		pPieView->Add(_T("香港{c #FE5900}15%{/c}"), 15);
	}*/

	// 滚动文字
	//CRollTextUI* pRollText = (CRollTextUI*)m_pm.FindControl(_T("rolltext"));
	//pRollText->SetText(_T("超过5000万人使用\n适用于 Chrome 的免费的广告拦截器\n可阻止所有烦人的广告及恶意软件和跟踪。"));
	//pRollText->BeginRoll(ROLLTEXT_UP, 200, 20);		//运动方式，速度，时间

	//												// 调色板使用
	//CColorPaletteUI* pColorPalette = (CColorPaletteUI*)m_pm.FindControl(_T("Pallet"));
	//pColorPalette->SetSelectColor(0xff0199cb);

	
	// 拓展List -- remote list
	//m_pRemoteList = static_cast<CListExUI*>(m_pm.FindControl(_T("machineList")));
	//m_pRemoteList->InitListCtrl();
	/*for (int i = 0; i < 10; i++)
	{
		CListTextExtElementUI* pItem = new CListTextExtElementUI();
		pItem->SetFixedHeight(30);
		pListEx->Add(pItem);
		pItem->SetAttribute(_T("style"), _T("listex_item_style"));
		pItem->SetText(1, _T("张三"));
		pItem->SetText(2, _T("1000"));
		pItem->SetText(3, _T("100"));
	}*/
	// 注册托盘图标
	//m_trayIcon.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("Duilib演示大全"));
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

	// 拓展List -- remote list
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

		// add by zhoupeng  动态增加连接状态图标
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
				MessageBox(m_hWnd, _T("有更新任务正在运行，不能退出."), _T("提示"), MB_OK);
				return;
			}
			if(IDYES == MessageBox(m_hWnd, _T("确定退出数据采集系统更新程序？"), _T("提示"), MB_YESNO))
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
			// 点击 自动更新按钮
			OnUpdateSystem();
		}
		else if (name.CompareNoCase(_T("batupdatebtn")) == 0 )
		{
			// 开启批量更新任务
			BatStartUpdate();
			//g_pTalk->SendMessageTest();
		}
		else if (name.CompareNoCase(_T("refreshbtn")) == 0 )
		{
			// 先广播hello消息，然后刷新界面
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
		// 将本地站点的文件夹信息显示出来
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

		// 点击执行机表数据，显示到edit框中
		if (pListElement && pListElement->GetOwner() == m_pRemoteList)
		{			
			int iSel = m_pRemoteList->GetCurSel();	// 此行代码，有bug。点击事件需要点击两次，界面才有响应正确的行
			if (iSel < 0) return;
			OnClickItem(iSel);			
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
	// 释放控件，发送离开消息
	delete g_pTalk;

	::PostQuitMessage(0L);
	bHandled = FALSE;
	return 0;
}


LRESULT CUpdateServer::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 关闭窗口，退出程序
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
			MessageBox(NULL, _T("不能重复添加执行机IP"), _T("Error"), MB_OK);
			return;
		}
	}

	// 写入全局变量
	RemoteInfo remoteInfo;
	_stprintf_s(remoteInfo.ip, _T("%s"), strhost.GetData());
	remoteInfo.fileport = _ttoi(strfileport);
	remoteInfo.msgport = _ttoi(strmsgport);
	remoteInfo.state = OFFLINE;

	g_host.push_back(remoteInfo);

	// 写入配置文件
	UpdateConfigIni();

	// 清空当前输入
	m_pHostIP->SetText(_T(""));
	m_pFileportEdit->SetText(_T(""));
	m_pMsgportEdit->SetText(_T(""));

	// 显示到界面
	CListTextExtElementUI* pItem = new CListTextExtElementUI();
	pItem->SetFixedHeight(30);
	m_pRemoteList->Add(pItem);

	pItem->SetAttribute(_T("style"), _T("listex_item_style"));
	pItem->SetText(1, strhost);
	pItem->SetText(2, strfileport);
	pItem->SetText(3, strmsgport);

	// 创建对应执行机的文件夹：以IP命名，存在其更新的过程文件
	CDuiString strRemoteFolderPath = _T("");
	//strRemoteFolderPath.Format(_T("%s\\%s"),PublicFunction::M2W(m_curPath.c_str()), strhost);
	strRemoteFolderPath = CDuiString(PublicFunction::M2W(m_curPath.c_str())) + _T("\\") + strhost;
	string strPath = PublicFunction::W2M(strRemoteFolderPath.GetData());
	if (false == PublicFunction::IsDirExisted(strPath))
	{
		if (NULL == CreateDirectory(strRemoteFolderPath, NULL))
		{
			CDuiString error = _T("");
			error.Format(_T("创建文件%s失败，错误码：%d\r\n"), strRemoteFolderPath, GetLastError());
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
			
			// 修改Item值
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
		MessageBox(NULL, _T("不能修改尚未添加执行机IP"), _T("Error"), MB_OK);
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
		MessageBox(NULL, _T("不能删除尚未添加执行机IP"), _T("Error"), MB_OK);
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

	// 显示当期选中执行机信息
	m_pHostIP->SetIP(PublicFunction::ReserveIP(strRemoteIP));
	strTmp.Format(_T("%d"), g_host[iSel].fileport);
	m_pFileportEdit->SetText(strTmp);
	strTmp.Format(_T("%d"), g_host[iSel].msgport);
	m_pMsgportEdit->SetText(strTmp);

	// 显示当前选中执行机更新日志
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
	// 写入配置文件
	string configPath = m_curPath + CONFIG_NAME;

	TCHAR szPath[MAX_PATH] = { 0 };
	TCHAR szCount[4] = { 0 };	// IP最大个数256
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

	unsigned short usPort = FILE_TRANSFER_PORT_DEFAULT;
	// 根据IP 获取文件传输的端口号
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

	unsigned short usPort = FILE_TRANSFER_PORT_DEFAULT;
	// 根据IP 获取文件传输的端口号
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

			CListTextElementUI* pItem = new CListTextElementUI();
			pItem->SetFixedHeight(30);
			m_pLocalDirList->Add(pItem);

			pItem->SetText(0, fileinfo.strFileName);
			pItem->SetText(1, fileinfo.strLastDate);
			pItem->SetText(2, fileinfo.strFileType);


			TCHAR szBuf[MAX_PATH] = { 0 };
			if (strFileType == _T("文件"))
			{

				double dLength = ulLength;
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
//LPCTSTR CUpdateServer::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
//{
//	TCHAR szBuf[MAX_PATH] = { 0 };
//	CDuiString strName = pControl->GetParent()->GetParent()->GetName();
//
//	// 本地站点的目录结构 && 防止vector容器越界
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
//			if (g_localFile[iIndex].strFileType == _T("文件夹"))
//			{
//				return _T("");
//			}
//			ULONGLONG ulLength = g_localFile[iIndex].ulLength;
//			double dLength = ulLength;
//#ifdef _UNICODE
//			if (ulLength < 1024)	// 小于1KB
//			{
//				swprintf_s(szBuf, _T("%lld字节"), ulLength);
//			}
//			else if (ulLength > 1024*1024 && ulLength < (1024*1024*1000))		// 1MB -- 1000MB
//			{
//				swprintf_s(szBuf, _T("%.2fMB"), dLength/(1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB之间
//			{
//				swprintf_s(szBuf, _T("%.2fKB"), dLength /1024);
//			}
//			
//#else
//			if (ulLength < 1024)	// 小于1KB
//			{
//				sprintf_s(szBuf, ("%lld字节"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB之间
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
//			if (g_remoteFile[iIndex].strFileType == _T("文件夹"))
//			{
//				return _T("");
//			}
//			ULONGLONG ulLength = g_remoteFile[iIndex].ulLength;
//			double dLength = ulLength;
//#ifdef _UNICODE
//			if (ulLength < 1024)	// 小于1KB
//			{
//				swprintf_s(szBuf, _T("%lld字节"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				swprintf_s(szBuf, _T("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				swprintf_s(szBuf, _T("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB之间
//			{
//				swprintf_s(szBuf, _T("%.2fKB"), dLength / 1024);
//			}
//
//#else
//			if (ulLength < 1024)	// 小于1KB
//			{
//				sprintf_s(szBuf, ("%lld字节"), ulLength);
//			}
//			else if (ulLength > 1024 * 1024 && ulLength < (1024 * 1024 * 1000))		// 1MB -- 1000MB
//			{
//				sprintf_s(szBuf, ("%.2fMB"), dLength / (1024 * 1024));
//			}
//			else if (ulLength >= (1024 * 1024 * 1000))		// >1GB
//			{
//				sprintf_s(szBuf, ("%.2fGB"), dLength / (1024 * 1024 * 1024));
//			}
//			else                               // 1KB -- 1MB之间
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

/* 以下是消息处理接口 */
LRESULT CUpdateServer::OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != 0)
	{
		::MessageBoxA(m_hWnd, (LPCSTR)lParam, ("出错"), 0);
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
			SetRemoteState(strIPW, ONLINE);
			UpdateRemoteState();
			break;
		}
		case MT_LEAVE:		// 用户离开
		{
			// 显示给用户
			strLog = strLogHead + _T("leave.\r\n");
			SetRemoteState(strIPW, OFFLINE);
			UpdateRemoteState();
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

			CListTextElementUI* pItem = new CListTextElementUI();
			pItem->SetFixedHeight(30);
			m_pRemoteDirList->Add(pItem);

			pItem->SetText(0, fileinfo.strFileName);
			pItem->SetText(1, fileinfo.strLastDate);
			pItem->SetText(2, fileinfo.strFileType);

			TCHAR szBuf[MAX_PATH] = { 0 };
			if (fileinfo.strFileType == _T("文件"))
			{
				ULONGLONG ulLength = fileinfo.ulLength;
				double dLength = ulLength;
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
			}
			pItem->SetText(3, szBuf);
		}

		// 释放资源
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

	// 1、保存至文件
	std::string strPath = m_curPath + "\\" + ip + UPDATELOG;
	std::string strLog = PublicFunction::W2M(strCurTime.GetData())
				+ PublicFunction::W2M(log.GetData());
	bool bRet = PublicFunction::SaveLogToFile(strPath, strLog);
	// 2、显示到界面（to do）
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

			// 采取 与客户端循环发消息
			DWORD dwAddress = inet_addr(PublicFunction::W2M(strIP.GetData()).c_str());
			g_pTalk->SendText("", 0, MT_BEGIN_UPDATE, dwAddress);
		}
	}
}
