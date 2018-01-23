/////////////////////////////////////////
// GroupTalk.h //此类用API写，不使用MFC方法
#define _WIN32_WINNT 0x0510


#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib")

#include <assert.h>

#define WM_GROUPTALK	WM_USER + 105    

/*

  uMsg:		WM_GROUPTALK
  wParam:	错误代码，0表示没有错误
  lParam:	消息的GT_HDR头指针

*/


#define BUFFER_SIZE 4096


const enum
{
	MT_JION = 1,		// 一个用户加入
	MT_LEAVE,			// 一个用户离开
	MT_MESG,			// 一个用户发送消息
						// 内部使用，告诉新加入的用户自己的用户信息
	MT_MINE,

	MT_BEGIN_UPDATE,		// 服务端发送 开始升级 消息
	MT_CHECK_MESG,			// 客户端发送 更新检查 消息：服务端接收后接收文件，并检查文件
	MT_CHECKRES_MESG,		/* 服务端 发送检查结果 消息：客户端接收后，
							1-需要升级，准备升级；0-无需升级，退出升级*/
	MT_ENDPRO_MESG,			// 服务端发送 结束进程exe 消息：客户端接收后，结束进程 并发送操作结果消息
	MT_ENDPRORES_MESG,		/* 客户端发送 结束进程 结果消息，由server接收：
								0--结束exe失败； 1--结束进程exe成功*/

	MT_UPDATE_MESG,			// 服务端发送 开始更新文件 消息，并发送文件等待客户端接收
	MT_UPDATERES_MESG,		/* 客户端发送 接收结果消息，由server接收：
							0--接收文件失败； 1--接收文件成功*/

	MT_BEGINPRO_MESG,		// 服务端发送 启动进程exe 消息
	MT_BEGINPRORES_MESG,	/* 客户端发送 启动进程exe结果 消息，由server接收：
							0--启动进程exe失败； 1--启动进程exe成功*/
};

//UDP发送的各种消息
const unsigned int uiMSGCheckState = 0;			//检查升级状态(客户端返回文件列表，服务端进行比对)
const unsigned int uiMSGEndPorcess = 1;			//结束exe进程
const unsigned int uiMSGBeginProcess = 2;		//启动exe进程
const unsigned int uiMSGUpdateFiles = 3;		//升级文件


const unsigned int IP_LENGTH = 16;				//IP 长度
const unsigned int FILEMAX = 200;				//最大文件个数
const unsigned int PATH_MAX = 256;				//文件路径长度
const unsigned int TIME_LENGTH = 32;			//时间最大长度
const unsigned int VERSION_LENGTH = 32;			//文件版本信息最大长度

// 增加消息类型
const enum
{
	MSG_FROM_CLIENT = 0,		//消息来源于客户端
	MSG_FROM_SERVER				//消息来源于服务端
};

typedef struct gt_hdr
{
	u_char gt_type;			// 消息类型
	u_char gt_fromtype;		// 消息来源类型 0 -- 客户端， 1 -- 服务端
	DWORD dwAddr;			// 发送此消息的用户的IP地址
	char szUser[15];		// 发送此消息的用户的用户名

	int nDataLength;		// 后面数据的长度
	char *data() { return (char*)(this + 1); }
} GT_HDR;


typedef  void(*GroupCallBack)(UINT uMsg, WPARAM wParam, LPARAM lParam);

class CGroupTalk
{
public:
	// 构造函数，创建工作线程，加入会议组
	CGroupTalk(HWND hNotifyWnd,  DWORD dwMultiAddr, u_short usMultiPort, u_char type = MSG_FROM_CLIENT, 
		DWORD dwLocalAddr = INADDR_ANY, int nTTL = 64);
	// 析构函数，清理资源，离开会议组
	~CGroupTalk();

	// 向其它成员发送消息。dwRemoteAddr为目标成员的地址，如果为0则向所有成员发送
	BOOL SendText(char *szText, int nLen, u_char type = MT_MESG, DWORD dwRemoteAddr = 0);

	void SendMessageTest()
	{	
		assert(::IsWindow(m_hNotifyWnd));
		::SendMessage(m_hNotifyWnd, WM_GROUPTALK, 0, 0);
	}

protected:
		// 帮助函数
	// 加入一个多播组
	BOOL JoinGroup();
	// 离开一个多播组
	BOOL LeaveGroup();
	// 向指定地址发送UDP封包
	BOOL Send(char *szText, int nLen, DWORD dwRemoteAddr);

protected:
		// 具体实现
	// 处理到来的封包
	void DispatchMsg(GT_HDR *pHeader, int nLen);
	// 工作线程
	friend DWORD WINAPI _GroupTalkEntry(LPVOID lpParam);

	HWND m_hNotifyWnd;		// 主窗口句柄
	DWORD m_dwMultiAddr;	// 此组使用的多播地址
	u_short m_usMultiPort;	// 此组使用的多播端口
	DWORD m_dwLocalAddr;	// 用户要使用的本地接口
	u_char m_usMsgFrom;		// 消息来源： 两客户端消息不需要进行处理
	int m_nTTL;				// 多播封包的TTL值
	HANDLE m_hThread;		// 工作线程句柄
	HANDLE m_hEvent;		// 事件句柄，用来使用重叠I/O接收数据

	SOCKET m_sRead;			// 接收数据的套节字，它必须加入多播组
	SOCKET m_sSend;			// 发送数据的套节字，不必加入多播组

	BOOL m_bQuit;			// 用来通知工作线程退出
	
	char m_szUser[256];		// 用户名

	GroupCallBack m_lpFunction;
};




/*

  可以使用下面的结构
#define MAX_EXPIRED 1000*60*5

struct USER_INFO
{
	DWORD dwAddr;
	char szUser[15];
	DWORD dwLastActiveTime;
};

*/