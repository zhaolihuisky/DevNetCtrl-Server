#pragma once
class CDevMgr
{
public:
	CDevMgr(void);
	~CDevMgr(void);
	int SetLog(CLog *pLog);
	int AddDev(CDevice *pDev);
	int DelDev(char *pMac);
	int ModDev(CDevice *pDev);
	int DelDevByOwner(char *pName);
	CQueue *GetDevQueue();
	CDevice *GetDev(char *pMac);
	int IsExit();
	int StartFresh();
	int StopFresh();
	int PrintLog(char *pStr);
	int LoadDevices();
	int LoadDevices(char *sDir);
	int GetDevsByOwner(char *pName, CDevice *pDevs[]);
private:
	CQueue *m_pDevQueue;
	CLog *m_pLog;
	int m_nExit;
	HANDLE m_hHandle;
};

