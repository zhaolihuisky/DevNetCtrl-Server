#pragma once

class CUserMgr
{
public:
	CUserMgr(void);
	~CUserMgr(void);
	int AddUser(CUser *pUser);
	int DelUser(char *pName);
	int ModUser(CUser *pUser);
	CUser *GetUser(char *pName);
	int SetLog(CLog *pLog);
	int IsNameValid(char *pName);
	int IsExit();
	int StartFresh();
	int StopFresh();
	int PrintLog(char *pStr);
	CQueue *GetUserQueue();

	int LoadUsers();
	int LoadUsers(char *sDir);
private:
	CQueue *m_pUserQueue;
	CLog *m_pLog;
	int m_nExit;
	HANDLE m_hHandle;
};

