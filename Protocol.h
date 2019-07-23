#pragma once

#define CMD_TRANSLATE 0x0000
#define USR_REGISTER 0x0001
#define USR_UNREGISTER 0x0002
#define USR_LOGON 0x0003
#define USR_LOGOFF 0x0004
#define USR_DEV_REGISTER 0x0005
#define USR_DEV_UNREGISTER 0x0006
#define DEV_CONNECT 0x0007
#define DEV_DISCONNECT 0x0008
#define USR_CON_DEV 0x0009
#define DEV_CON_USR 0x000A
#define USR_DEV_LIST 0x000B

class CProtocol
{
public:
	CProtocol(void);
	~CProtocol(void);

	int Init();
	int StartHandle();
	int StopHandle();
	int IsExit();
	int SetLog(CLog *pLog);
	CLog *GetLog();
	int PrintLog(char *pStr);
	CQueue *GetRecvQueue();
	CQueue *GetSendQueue();
	//char *Data2String(char *pData, int nLen);
	int CmdTranslate(CPeer *pPeer);
	int UsrRegister(CPeer *pPeer);
	int UsrUnregister(CPeer *pPeer);
	int UsrLogon(CPeer *pPeer);
	int UsrLogoff(CPeer *pPeer);
	int UsrDevRegister(CPeer *pPeer);
	int UsrDevUnregister(CPeer *pPeer);
	int DevConnect(CPeer *pPeer);
	int DevDisconnect(CPeer *pPeer);
	int UsrP2pDev(CPeer *pPeer);
	int Echo2Client(CPeer *pPeer);
	int UsrDevList(CPeer *pPeer);
	int NetControl(CNode *pNode);
	int DataGenerate(unsigned short usAddr, char ucCode, unsigned short usOffset, char *pParam, unsigned char ucParlen, char *pTx, int nSize);
private:
	int m_nExit;
	CLog *m_pLog;
	CQueue *m_pRecvQueue;
	CQueue *m_pSendQueue;
	CUdpServer *m_pUdpServer;
	HANDLE m_hHandle;
	CDevMgr *m_pDevMgr;
	CUserMgr *m_pUserMgr;
};

