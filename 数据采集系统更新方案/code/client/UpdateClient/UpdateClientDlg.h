
// UpdateClientDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"


// CUpdateClientDlg 对话框
class CUpdateClientDlg : public CDialogEx
{
// 构造
public:
	CUpdateClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT WMGROUPTALK(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT WMRecvFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT WMSendFile(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	void EnableControls(BOOL bState);
	void LoadConfig();
	void SaveConfig();

	void HandleGroupMsg(HWND hDlg, GT_HDR *pHeader);

	void UpdateLog(CString strLog);

	// 发送update.xml 文件
	BOOL SendUpdateFile(string filename, CString &strerror);

	// 接收更新文件
	BOOL RecvUpdateFiles(CString &strerror);

	// 结束 数采exe进程
	void EndProcess();

	// 开启 数采exe进程
	BOOL BeginProcess(CString &error);

	void InitUpdateXml();

	// 监视更新的exe进程名
	CListCtrl m_ExeList;

	CRichEditCtrl m_RichEditLog;

	string m_serIP;
	u_short m_port;
	string m_tmpPath;
	string m_sysPath;
	string m_localIP;

	string m_curPath;
	string m_sendFileName;
	vector<std::string> m_ExcludeFile;	// 排除的更新文件
private:
	void InitGroupTalk();

	static DWORD WINAPI SendFileThreadProc(LPVOID lpParameter);
	static DWORD WINAPI RecvFileThreadProc(LPVOID lpParameter);
public:
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonSave();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonTmpdir();
	afx_msg void OnBnClickedButtonSysdir();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
