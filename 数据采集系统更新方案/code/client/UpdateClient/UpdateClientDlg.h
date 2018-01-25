
// UpdateClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"


// CUpdateClientDlg �Ի���
class CUpdateClientDlg : public CDialogEx
{
// ����
public:
	CUpdateClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

	// ����update.xml �ļ�
	BOOL SendUpdateFile(string filename, CString &strerror);

	// ���ո����ļ�
	BOOL RecvUpdateFiles(CString &strerror);

	// ���� ����exe����
	void EndProcess();

	// ���� ����exe����
	BOOL BeginProcess(CString &error);

	void InitUpdateXml();

	// ���Ӹ��µ�exe������
	CListCtrl m_ExeList;

	CRichEditCtrl m_RichEditLog;

	string m_serIP;
	u_short m_port;
	string m_tmpPath;
	string m_sysPath;
	string m_localIP;

	string m_curPath;
	string m_sendFileName;
	vector<std::string> m_ExcludeFile;	// �ų��ĸ����ļ�
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
