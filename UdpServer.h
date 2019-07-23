#pragma once


class CUdpServer
{
public:
	CUdpServer(void);
	~CUdpServer(void);

	int SetRecvQueue(CQueue *pQueue);
	int SetSendQueue(CQueue *pQueue);
	CQueue *GetRecvQueue();
	CQueue *GetSendQueue();
	int StartRecv();
	int StopRecv();
	int StartSend();
	int StopSend();
	int InitSocket();
	int SetLog(CLog *pLog);
	CLog *GetLog();
	int PrintLog(char *pStr);
	int IsRecvExit();
	int IsSendExit();
	SOCKET GetSocket();
private:
	int m_nRecvExit;
	int m_nSendExit;
	SOCKET m_sUdp;
	HANDLE m_hRecv;
	HANDLE m_hSend;
	CQueue *m_pRecvQueue;
	CQueue *m_pSendQueue;
	CLog *m_pLog;

};