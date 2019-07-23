#pragma once

class CLog
{
public:
	CLog(void);
	~CLog(void);
	int Output(char *pLog, int nLen);
	int PrintLog(char *sLog, int nLen);
	int StartHandle();
	int StopHandle();
	CQueue *GetQueue();
	int IsExit();
private:
	int m_nExit;
	HANDLE m_hHandle;
	CQueue *m_pLogQueue;
};