#pragma once
//#include "E:\����\Project\�����Ը���ϵͳ\UpdateServerDui\DuiLib\Core\UIBase.h"
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

public:// UI��ʼ��
	DuiLib::CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void InitWindow();
	void OnFinalMessage(HWND hWnd);

public:// �ӿڻص�
	CControlUI* CreateControl(LPCTSTR pstrClass);
	//LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);

public:// UI֪ͨ��Ϣ
	void Notify(TNotifyUI& msg);
	void OnLClick(CControlUI *pControl);

	DUI_DECLARE_MESSAGE_MAP()


public:// ϵͳ��Ϣ
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:// WebBrowser
	//virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
	//virtual HRESULT STDMETHODCALLTYPE GetHostInfo(CWebBrowserUI* pWeb, DOCHOSTUIINFO __RPC_FAR *pInfo);
	//virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(CWebBrowserUI* pWeb, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);

	
private:// UI����
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI* m_pSkinBtn;
	CMenuWnd* m_pMenu;
	CStdStringPtrMap m_MenuInfos;
	CTrayIcon m_trayIcon;


	CListExUI* m_pRemoteList;			//remote list�ؼ�
	CIPAddressUI* m_pHostIP;			//host IP�ؼ�
	CEditUI* m_pFileportEdit;		//�ļ�����˿ڿؼ�
	CEditUI* m_pMsgportEdit;		//��Ϣ����˿ڿؼ�

	CEditUI* m_pRemoteDirIP;		// Զ�� ���ӵ�IP
	CListUI* m_pRemoteDirList;		//Զ�� Ŀ¼��Ϣ

	CEditUI* m_pLocalDir;			// ����վ�����Ŀ¼
	CListUI* m_pLocalDirList;		// ����վ��Ŀ¼list

	CRichEditUI* m_pLogEdit;		// ��־��ʾ��
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

	// ѡ��ĳ���ļ���
	CDuiString SelectDir();
	// ��ʼ�� ����վ��Ŀ¼�ṹ
	void InitLocalDirToList(CDuiString dir);
	// ��ʼ�� Զ��վ��Ŀ¼�ṹ
	void InitRemoteDirToList(int index);

	// �ಥ��Ϣ����
	LRESULT OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void HandleGroupMsg(HWND hDlg, GT_HDR *pHeader);//����ʵ����;

													// �ļ�������Ϣ����
	LRESULT OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// ��¼��־
	void UpdateLog(const string ip, const CDuiString log);

	void SetRemoteState(CDuiString strIP, UINT state);
	// ����ִ�л�״̬
	void UpdateRemoteState();

	void BatStartUpdate();

public:
	string m_curPath = "";			// ��ǰexe����·��
	vector<std::string> m_ExcludeFile;	// �ų��ĸ����ļ�
public:
	bool m_running;					// ���б�ʶ

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
//		// ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
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
//	// ѡ��ĳ���ļ���
//	CDuiString SelectDir();
//	// ��ʼ�� ����վ��Ŀ¼�ṹ
//	void InitLocalDirToList(CDuiString dir);
//	// ��ʼ�� Զ��վ��Ŀ¼�ṹ
//	void InitRemoteDirToList(int index);
//
//	/*
//	* �ؼ��Ļص�������IListCallbackUI �е�һ���麯������Ⱦʱ������,��[1]�������˻ص�����
//	*/
//	LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);
//
//	// ����ִ�л�list��¼
//	LRESULT OnAddListItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//
//	// �ಥ��Ϣ����
//	LRESULT OnWMGROUPTALK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	
//	void HandleGroupMsg(HWND hDlg, GT_HDR *pHeader);//����ʵ����;
//
//	// �ļ�������Ϣ����
//	LRESULT OnFileTranferOver(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//
//	// ��¼��־
//	void UpdateLog(const string ip, const CDuiString log);
//
//	void BatStartUpdate();
//public:
//	string m_curPath = "";			// ��ǰexe����·��
//	vector<std::string> m_ExcludeFile;	// �ų��ĸ����ļ�
//public:
//	CPaintManagerUI m_pm;
//
//	bool m_running;					// ���б�ʶ
//
//	CListUI* m_pRemoteList;			//remote list�ؼ�
//	CEditUI* m_pHostEdit;			//host IP�ؼ�
//	CEditUI* m_pFileportEdit;		//�ļ�����˿ڿؼ�
//	CEditUI* m_pMsgportEdit;		//��Ϣ����˿ڿؼ�
//
//	CEditUI* m_pRemoteDirIP;		// Զ�� ���ӵ�IP
//	CListUI* m_pRemoteDirList;		//Զ�� Ŀ¼��Ϣ
//
//	CEditUI* m_pLocalDir;			// ����վ�����Ŀ¼
//	CListUI* m_pLocalDirList;		// ����վ��Ŀ¼list
//};
//
