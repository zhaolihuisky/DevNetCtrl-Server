#include "StdAfx.h"
#include "UserMgr.h"

int FreshUserStatus(void* param)
{
	CUserMgr *pUserMgr = (CUserMgr *)param;
	CQueue *pUserQueue = pUserMgr->GetUserQueue();
	CNode *pNode = NULL;
	CUser *pUsr = NULL;
	int nCount = 0;

	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUserMgr FreshUserStatus()\n");
	pUserMgr->PrintLog(szLog);

	while(pUserMgr->IsExit() == 0)
	{
		nCount++;
		if(nCount < STATUS_FRESH)
		{
			Sleep(1000);
			continue;
		}
		nCount = 0;
		pNode = pUserQueue->GetFirst();
		while(pNode)
		{
			pUsr = (CUser *)pNode->m_pObject;
			int nStatus = pUsr->GetStatus();
			time_t tNow = time(NULL);
			time_t nDiff = tNow - pUsr->GetLastTime();
			if(nDiff > STATUS_FRESH)
			{
				if(nStatus == STATUS_ONLINE)
				{
					pUsr->SetStatus(STATUS_OFFLINE);
				}
			}
			else
			{
				if(nStatus == STATUS_OFFLINE)
				{
					pUsr->SetStatus(STATUS_ONLINE);
				}
			}
			//sprintf_s(szLog, BUF_SIZE, "CUserMgr FreshUserStatus() name=%s pwd=%s\n", pUsr->GetName(), pUsr->GetPwd());
			//pUserMgr->PrintLog(szLog);
			pNode = pUserQueue->GetNext();
		}
		
	}
	return 0;
}

CUserMgr::CUserMgr(void)
{
	m_pUserQueue = new CQueue();
	m_pLog = NULL;
	m_nExit = 0;
	m_hHandle = NULL;
}

CUserMgr::~CUserMgr(void)
{
	delete m_pUserQueue;
}

int CUserMgr::AddUser(CUser *pUser)
{
	char szDir[PATH_SIZE] = {0};
	time_t tFirst = pUser->GetFirstTime();
	struct tm tmFirst;
	localtime_s(&tmFirst, &tFirst);
	sprintf_s(szDir, PATH_SIZE, "%s\\%04d\\%02d", USERS_PATH, tmFirst.tm_year+1900, tmFirst.tm_mon+1);
	pUser->SetDir(szDir, strlen(szDir));
	pUser->Save();

	CNode *p = new CNode();
	p->m_pObject = (CBase *)pUser;
	m_pUserQueue->Push(p);
	return 0;
}

int CUserMgr::DelUser(char *pName)
{
	CNode *p = m_pUserQueue->GetFirst();
	while(p != NULL)
	{
		CUser *u = (CUser *)(p->m_pObject);
		if(strcmp(pName, u->GetName()) == 0)
		{
			u->Delete();
			m_pUserQueue->Remove(p);
			break;
		}
		p = m_pUserQueue->GetNext();
	}
	return 0;
}

int CUserMgr::ModUser(CUser *pUser)
{
	CNode *p = m_pUserQueue->GetFirst();
	while(p != NULL)
	{
		CUser *u = (CUser *)(p->m_pObject);
		if(strcmp(pUser->GetName(), u->GetName()) == 0)
		{
			delete u;
			p->m_pObject = (CBase *)pUser;
			break;
		}
		p = m_pUserQueue->GetNext();
	}
	return 0;
}

CUser *CUserMgr::GetUser(char *pName)
{
	CUser *u = NULL;
	CNode *p = m_pUserQueue->GetFirst();
	while(p != NULL)
	{
		u = (CUser *)(p->m_pObject);
		if(strcmp(pName, u->GetName()) == 0)
		{
			break;
		}
		p = m_pUserQueue->GetNext();
		u = NULL;
	}
	return u;
}

int CUserMgr::SetLog(CLog *pLog)
{
	m_pLog = pLog;
	return 0;
}

int CUserMgr::IsNameValid(char *pName)
{
	int nErr = 0;
	while(*pName != '\0')
	{
		if(('0'<= *pName && *pName <= '9') || ('A' <= *pName && *pName <= 'Z') || ('a' <= *pName && *pName <= 'z'))
		{
			nErr = 0;
		}
		else
		{
			nErr = 1;
			break;
		}
		pName++;
	}
	return nErr;
}

CQueue *CUserMgr::GetUserQueue()
{
	return m_pUserQueue;
}

int CUserMgr::IsExit()
{
	return m_nExit;
}

int CUserMgr::PrintLog(char *pStr)
{
	if(m_pLog != NULL)
	{
		m_pLog->Output(pStr, strlen(pStr));
	}
	return 0;
}

int CUserMgr::StartFresh()
{
	LoadUsers();
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUserMgr::StartFresh()\n");
	PrintLog(szLog);
	m_hHandle = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))FreshUserStatus, this, 0, 0);
	return 0;
}

int CUserMgr::StopFresh()
{
	char szLog[BUF_SIZE] = {0};

	m_nExit = 1;
	WaitForSingleObject(m_hHandle, INFINITE);
    CloseHandle(m_hHandle);

	sprintf_s(szLog, BUF_SIZE, "CUserMgr::StopFresh()\n");
	PrintLog(szLog);
	return 0;
}

int CUserMgr::LoadUsers()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUserMgr::LoadUsers()\n");
	PrintLog(szLog);
	LoadUsers(USERS_PATH);
	return 0;
}

int CUserMgr::LoadUsers(char *sDir)
{
	long handle = 0;
	char szDir[PATH_SIZE] = {0};
	char szLog[BUF_SIZE] = {0};
	char szPath[PATH_SIZE] = {0};
	int nErr = 0;
	sprintf_s(szDir, PATH_SIZE, "%s\\*", sDir);
	struct _finddata_t fd;
	handle = _findfirst(szDir, &fd);
	if(handle != -1)
	{
		while(!_findnext(handle, &fd))
		{
			if(strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
			{
				continue;
			}
			sprintf_s(szPath, PATH_SIZE, "%s\\%s", sDir, fd.name);
			sprintf_s(szLog, BUF_SIZE, "CUserMgr::LoadUsers() szPath=%s fd.attrib=%d\n", szPath, fd.attrib);
			PrintLog(szLog);
			if(fd.attrib == _A_SUBDIR)
			{
				LoadUsers(szPath);
			}
			else if(fd.attrib == _A_ARCH)
			{
				char *pExt = strstr(fd.name, ".ini");
				if(pExt)
				{
					char szName[NAME_SIZE] = {0};
					memset(szName, 0, NAME_SIZE);
					memcpy(szName, fd.name, pExt-fd.name);
					CUser *pNewUsr = new CUser();
					pNewUsr->SetName(szName, strlen(szName));
					pNewUsr->SetDir(sDir, strlen(sDir));
					nErr = pNewUsr->Load();
					sprintf_s(szLog, BUF_SIZE, "CUserMgr::LoadUsers() szDir=%s szName=%s nErr=%d\n", szDir, szName, nErr);
					PrintLog(szLog);
					if(nErr != -1)
					{
						AddUser(pNewUsr);
					}
					else
					{
						delete pNewUsr;
					}
				}
			}
		}
		_findclose(handle);
	}
	return 0;
}