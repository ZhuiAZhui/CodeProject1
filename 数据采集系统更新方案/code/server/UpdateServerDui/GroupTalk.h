/////////////////////////////////////////
// GroupTalk.h //������APIд����ʹ��MFC����
#define _WIN32_WINNT 0x0510


#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib")

#include <assert.h>

#define WM_GROUPTALK	WM_USER + 105    

/*

  uMsg:		WM_GROUPTALK
  wParam:	������룬0��ʾû�д���
  lParam:	��Ϣ��GT_HDRͷָ��

*/


#define BUFFER_SIZE 4096


const enum
{
	MT_JION = 1,		// һ���û�����
	MT_LEAVE,			// һ���û��뿪
	MT_MESG,			// һ���û�������Ϣ
						// �ڲ�ʹ�ã������¼�����û��Լ����û���Ϣ
	MT_MINE,

	MT_BEGIN_UPDATE,		// ����˷��� ��ʼ���� ��Ϣ
	MT_CHECK_MESG,			// �ͻ��˷��� ���¼�� ��Ϣ������˽��պ�����ļ���������ļ�
	MT_CHECKRES_MESG,		/* ����� ���ͼ���� ��Ϣ���ͻ��˽��պ�
							1-��Ҫ������׼��������0-�����������˳�����*/
	MT_ENDPRO_MESG,			// ����˷��� ��������exe ��Ϣ���ͻ��˽��պ󣬽������� �����Ͳ��������Ϣ
	MT_ENDPRORES_MESG,		/* �ͻ��˷��� �������� �����Ϣ����server���գ�
								0--����exeʧ�ܣ� 1--��������exe�ɹ�*/

	MT_UPDATE_MESG,			// ����˷��� ��ʼ�����ļ� ��Ϣ���������ļ��ȴ��ͻ��˽���
	MT_UPDATERES_MESG,		/* �ͻ��˷��� ���ս����Ϣ����server���գ�
							0--�����ļ�ʧ�ܣ� 1--�����ļ��ɹ�*/

	MT_BEGINPRO_MESG,		// ����˷��� ��������exe ��Ϣ
	MT_BEGINPRORES_MESG,	/* �ͻ��˷��� ��������exe��� ��Ϣ����server���գ�
							0--��������exeʧ�ܣ� 1--��������exe�ɹ�*/
};

//UDP���͵ĸ�����Ϣ
const unsigned int uiMSGCheckState = 0;			//�������״̬(�ͻ��˷����ļ��б�����˽��бȶ�)
const unsigned int uiMSGEndPorcess = 1;			//����exe����
const unsigned int uiMSGBeginProcess = 2;		//����exe����
const unsigned int uiMSGUpdateFiles = 3;		//�����ļ�


const unsigned int IP_LENGTH = 16;				//IP ����
const unsigned int FILEMAX = 200;				//����ļ�����
const unsigned int PATH_MAX = 256;				//�ļ�·������
const unsigned int TIME_LENGTH = 32;			//ʱ����󳤶�
const unsigned int VERSION_LENGTH = 32;			//�ļ��汾��Ϣ��󳤶�

// ������Ϣ����
const enum
{
	MSG_FROM_CLIENT = 0,		//��Ϣ��Դ�ڿͻ���
	MSG_FROM_SERVER				//��Ϣ��Դ�ڷ����
};

typedef struct gt_hdr
{
	u_char gt_type;			// ��Ϣ����
	u_char gt_fromtype;		// ��Ϣ��Դ���� 0 -- �ͻ��ˣ� 1 -- �����
	DWORD dwAddr;			// ���ʹ���Ϣ���û���IP��ַ
	char szUser[15];		// ���ʹ���Ϣ���û����û���

	int nDataLength;		// �������ݵĳ���
	char *data() { return (char*)(this + 1); }
} GT_HDR;


typedef  void(*GroupCallBack)(UINT uMsg, WPARAM wParam, LPARAM lParam);

class CGroupTalk
{
public:
	// ���캯�������������̣߳����������
	CGroupTalk(HWND hNotifyWnd,  DWORD dwMultiAddr, u_short usMultiPort, u_char type = MSG_FROM_CLIENT, 
		DWORD dwLocalAddr = INADDR_ANY, int nTTL = 64);
	// ����������������Դ���뿪������
	~CGroupTalk();

	// ��������Ա������Ϣ��dwRemoteAddrΪĿ���Ա�ĵ�ַ�����Ϊ0�������г�Ա����
	BOOL SendText(char *szText, int nLen, u_char type = MT_MESG, DWORD dwRemoteAddr = 0);

	void SendMessageTest()
	{	
		assert(::IsWindow(m_hNotifyWnd));
		::SendMessage(m_hNotifyWnd, WM_GROUPTALK, 0, 0);
	}

protected:
		// ��������
	// ����һ���ಥ��
	BOOL JoinGroup();
	// �뿪һ���ಥ��
	BOOL LeaveGroup();
	// ��ָ����ַ����UDP���
	BOOL Send(char *szText, int nLen, DWORD dwRemoteAddr);

protected:
		// ����ʵ��
	// �������ķ��
	void DispatchMsg(GT_HDR *pHeader, int nLen);
	// �����߳�
	friend DWORD WINAPI _GroupTalkEntry(LPVOID lpParam);

	HWND m_hNotifyWnd;		// �����ھ��
	DWORD m_dwMultiAddr;	// ����ʹ�õĶಥ��ַ
	u_short m_usMultiPort;	// ����ʹ�õĶಥ�˿�
	DWORD m_dwLocalAddr;	// �û�Ҫʹ�õı��ؽӿ�
	u_char m_usMsgFrom;		// ��Ϣ��Դ�� ���ͻ�����Ϣ����Ҫ���д���
	int m_nTTL;				// �ಥ�����TTLֵ
	HANDLE m_hThread;		// �����߳̾��
	HANDLE m_hEvent;		// �¼����������ʹ���ص�I/O��������

	SOCKET m_sRead;			// �������ݵ��׽��֣����������ಥ��
	SOCKET m_sSend;			// �������ݵ��׽��֣����ؼ���ಥ��

	BOOL m_bQuit;			// ����֪ͨ�����߳��˳�
	
	char m_szUser[256];		// �û���

	GroupCallBack m_lpFunction;
};




/*

  ����ʹ������Ľṹ
#define MAX_EXPIRED 1000*60*5

struct USER_INFO
{
	DWORD dwAddr;
	char szUser[15];
	DWORD dwLastActiveTime;
};

*/