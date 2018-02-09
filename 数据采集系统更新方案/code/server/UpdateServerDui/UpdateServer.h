#pragma once
//#include "E:\任务\Project\数采自更新系统\UpdateServerDui\DuiLib\Core\UIBase.h"
#include "Resource.h"
#include "stdafx.h"

#include "ControlEx.h"

//#include "..\DuiLib\Core\UIManager.h"

enum AddList
{
	RemoteList = 0,
	RemoteDirList,
	LocalDirList
};

struct FileInfo
{
	CDuiString strFileName = _T("");
	CDuiString strFileType = _T("");
	CDuiString strLastDate = _T("");
	ULONGLONG ulLength = 0;
};

//////////////////////////////////////////////////////////////////////////
///

class CMainPage : public CNotifyPump
{
public:
	CMainPage();

public:
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);

	DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);

private:
	CPaintManagerUI* m_pPaintManager;
};

//////////////////////////////////////////////////////////////////////////
///

class CUpdateServer : public WindowImplBase//, public CWebBrowserEventHandler
{
public:
	CUpdateServer();
	~CUpdateServer();

public:// UI初始化
	DuiLib::CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void InitWindow();
	void OnFinalMessage(HWND hWnd);

public:// 接口回调
	CControlUI* CreateControl(LPCTSTR pstrClass);
	//LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);

public:// UI通知消息
	void Notify(TNotifyUI& msg);
	void OnLClick(CControlUI *pControl);

	DUI_DECLARE_MESSAGE_MAP()


public:// 系统消息
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:// WebBrowser
	//virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
	//virtual HRESULT STDMETHODCALLTYPE GetHostInfo(CWebBrowserUI* pWeb, DOCHOSTUIINFO __RPC_FAR *pInfo);
	//virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(CWebBrowserUI* pWeb, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);

	
private:// UI变量
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI* m_pSkinBtn;
	CMenuWnd* m_pMenu;
	CStdStringPtrMap m_MenuInfos;
	CTrayIcon m_trayIcon;


	CListExUI* m_pRemoteList;			//remote list控件
	CIPAddressUI* m_pHostIP;			//host IP控件
	CEditUI* m_pFileportEdit;		//文件传输端口控件
	CEditUI* m_pMsgportEdit;		//消息传输端口控件

	CEditUI* m_pRemoteDirIP;		// 远端 连接的IP
	CListUI* m_pRemoteDirList;		//远端 目录信息

	CEditUI* m_pLocalDir;			// 本地站点关联目录
	CListUI* m_pLocalDirList;		// 本地站点目录list

	CRichEditUI* m_pLogEdit;		// 日志显示框
public:
	CMainPage m_MainPage;
	//CPaintManagerUI m_pm;

public:
	void Init();
	void OnAddRemote();
	void OnModifyRemote();
	void OnDeleteRemote();

	void OnClickItem(int iSel);

	void UpdateConfigIni();

	void OnUpdateSystem();

	static DWORD WINAPI RecvUpdateFileThreadProc(LPVOID lpParameter);
	static DWORD WINAPI RecvRemoteDirFileThreadProc(LPVOID lpParameter);
	bool RecvFileFromClient(string strRemoteIP, DWORD dwWaitSencond, CDuiString &error);
	bool CheckUpdateFile(std::string strIP);

	static DWORD WINAPI SendFileThreadProc(LPVOID lpParameter);

	// 选择某个文件夹
	CDuiString SelectDir();
	// 初始化 本地站点目录结构
	void InitLocalDirToList(CDuiString dir);
	// 初始化 远程站点目录结构
	void InitRemoteDirToList(int index);

	// 多播消息处理
	LRESULT OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void HandleGroupMsg(HWND hDlg, GT_HDR *pHeader);//函数实现体;

													// 文件传输消息处理
	LRESULT OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// 记录日志
	void UpdateLog(const string ip, const CDuiString log);

	void SetRemoteState(CDuiString strIP, UINT state);
	// 更新执行机状态
	void UpdateRemoteState();

	void BatStartUpdate();

public:
	string m_curPath = "";			// 当前exe运行路径
	vector<std::string> m_ExcludeFile;	// 排除的更新文件
public:
	bool m_running;					// 运行标识

};


//class CUpdateServer :public CWindowWnd, public INotifyUI, public IListCallbackUI
//{
//public:
//	CUpdateServer();
//	~CUpdateServer();
//
//	LPCTSTR GetWindowClassName() const { return _T("UIFrame"); };
//	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
//	void OnFinalMessage(HWND /*hWnd*/) { delete this; };
//
//	void Init();
//	
//	void OnPrepare(TNotifyUI& msg)
//	{
//
//	}
//
//	void Notify(TNotifyUI& msg);
//
//	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
//
//	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	{
//		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
//		if (wParam == SC_CLOSE) {
//			::PostQuitMessage(0L);
//			bHandled = TRUE;
//			return 0;
//		}
//		BOOL bZoomed = ::IsZoomed(*this);
//		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
//		if (::IsZoomed(*this) != bZoomed) {
//			if (!bZoomed) {
//				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
//				if (pControl) pControl->SetVisible(false);
//				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
//				if (pControl) pControl->SetVisible(true);
//			}
//			else {
//				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
//				if (pControl) pControl->SetVisible(true);
//				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
//				if (pControl) pControl->SetVisible(false);
//			}
//		}
//		return lRes;
//	}
//
//	void OnAddRemote();
//	void OnModifyRemote();
//	void OnDeleteRemote();
//
//	void OnUpdateSystem();
//
//	static DWORD WINAPI RecvUpdateFileThreadProc(LPVOID lpParameter);
//	static DWORD WINAPI RecvRemoteDirFileThreadProc(LPVOID lpParameter);
//	bool RecvFileFromClient(string strRemoteIP, DWORD dwWaitSencond, CDuiString &error);
//	bool CheckUpdateFile(std::string strIP);
//
//	static DWORD WINAPI SendFileThreadProc(LPVOID lpParameter);
//
//	// 选择某个文件夹
//	CDuiString SelectDir();
//	// 初始化 本地站点目录结构
//	void InitLocalDirToList(CDuiString dir);
//	// 初始化 远程站点目录结构
//	void InitRemoteDirToList(int index);
//
//	/*
//	* 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
//	*/
//	LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);
//
//	// 增加执行机list记录
//	LRESULT OnAddListItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//
//	// 多播消息处理
//	LRESULT OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	
//	void HandleGroupMsg(HWND hDlg, GT_HDR *pHeader);//函数实现体;
//
//	// 文件传输消息处理
//	LRESULT OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//
//	// 记录日志
//	void UpdateLog(const string ip, const CDuiString log);
//
//	void BatStartUpdate();
//public:
//	string m_curPath = "";			// 当前exe运行路径
//	vector<std::string> m_ExcludeFile;	// 排除的更新文件
//public:
//	CPaintManagerUI m_pm;
//
//	bool m_running;					// 运行标识
//
//	CListUI* m_pRemoteList;			//remote list控件
//	CEditUI* m_pHostEdit;			//host IP控件
//	CEditUI* m_pFileportEdit;		//文件传输端口控件
//	CEditUI* m_pMsgportEdit;		//消息传输端口控件
//
//	CEditUI* m_pRemoteDirIP;		// 远端 连接的IP
//	CListUI* m_pRemoteDirList;		//远端 目录信息
//
//	CEditUI* m_pLocalDir;			// 本地站点关联目录
//	CListUI* m_pLocalDirList;		// 本地站点目录list
//};
//
