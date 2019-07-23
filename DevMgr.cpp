#include "StdAfx.h"
#include "DevMgr.h"

int FreshDevStatus(void* param)
{
	CDevMgr *pDevMgr = (CDevMgr *)param;
	CQueue *pDevQueue = pDevMgr->GetDevQueue();
	CNode *pNode = NULL;
	CDevice *pDev = NULL;
	int nCount = 0;

	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CDevMgr FreshDevStatus()\n");
	pDevMgr->PrintLog(szLog);

	while(pDevMgr->IsExit() == 0)
	{
		nCount++;
		if(nCount < STATUS_FRESH)
		{
			Sleep(1000);
			continue;
		}
		nCount = 0;
		pNode = pDevQueue->GetFirst();
		while(pNode)
		{
			pDev = (CDevice *)pNode->m_pObject;
			int nStatus = pDev->GetStatus();
			time_t tNow = time(NULL);
			time_t nDiff = tNow - pDev->GetLastTime();
			if(nDiff > STATUS_FRESH)
			{
				if(nStatus == STATUS_ONLINE)
				{
					pDev->SetStatus(STATUS_OFFLINE);
				}
			}
			else
			{
				if(nStatus == STATUS_OFFLINE)
				{
					pDev->SetStatus(STATUS_ONLINE);
				}
			}
			//sprintf_s(szLog, BUF_SIZE, "CDevMgr FreshDevStatus() mac=%s\n", pDev->GetMac());
			//pDevMgr->PrintLog(szLog);
			pNode = pDevQueue->GetNext();
		}
	}
	return 0;
}

CDevMgr::CDevMgr(void)
{
	m_pDevQueue = new CQueue();
	m_pLog = NULL;
	m_nExit = 0;
	m_hHandle = NULL;
}

CDevMgr::~CDevMgr(void)
{
	delete m_pDevQueue;
}

int CDevMgr::SetLog(CLog *pLog)
{
	m_pLog = pLog;
	return 0;
}

int CDevMgr::AddDev(CDevice *pDev)
{
	char szDir[PATH_SIZE] = {0};
	time_t tFirst = pDev->GetFirstTime();
	struct tm tmFirst;
	localtime_s(&tmFirst, &tFirst);
	sprintf_s(szDir, PATH_SIZE, "%s\\%04d\\%02d", DEVICES_PATH, tmFirst.tm_year+1900, tmFirst.tm_mon+1);
	pDev->SetDir(szDir, strlen(szDir));
	pDev->Save();

	CNode *p = new CNode();
	p->m_pObject = (CBase *)pDev;
	m_pDevQueue->Push(p);
	return 0;
}

int CDevMgr::DelDev(char *pMac)
{
	CNode *p = m_pDevQueue->GetFirst();
	while(p != NULL)
	{
		CDevice *d = (CDevice *)(p->m_pObject);
		if(strcmp(pMac, d->GetMac()) == 0)
		{
			d->Delete();
			m_pDevQueue->Remove(p);
			break;
		}
		p = m_pDevQueue->GetNext();
	}
	return 0;
}

int CDevMgr::ModDev(CDevice *pDev)
{
	CNode *p = m_pDevQueue->GetFirst();
	while(p != NULL)
	{
		CDevice *d = (CDevice *)(p->m_pObject);
		if(strcmp(pDev->GetMac(), d->GetMac()) == 0)
		{
			delete d;
			p->m_pObject = (CBase *)pDev;
			break;
		}
		p = m_pDevQueue->GetNext();
	}
	return 0;
}

int CDevMgr::DelDevByOwner(char *pName)
{
	CNode *p = m_pDevQueue->GetFirst();
	while(p != NULL)
	{
		CNode *pDel = NULL;
		CDevice *d = (CDevice *)(p->m_pObject);
		if(strcmp(pName, d->GetOwner()) == 0)
		{
			//d->SetOwner(NULL, 0);
			pDel = p;
		}
		p = m_pDevQueue->GetNext();
		if(pDel != NULL)
		{
			d->Delete();
			m_pDevQueue->Remove(pDel);
			pDel = NULL;
		}
	}
	return 0;
}

CQueue *CDevMgr::GetDevQueue()
{
	return m_pDevQueue;
}

CDevice *CDevMgr::GetDev(char *pMac)
{
	CDevice *d = NULL;
	CNode *p = m_pDevQueue->GetFirst();
	while(p != NULL)
	{
		d = (CDevice *)(p->m_pObject);
		if(strcmp(pMac, d->GetMac()) == 0)
		{
			break;
		}
		p = m_pDevQueue->GetNext();
		d = NULL;
	}
	return d;
}

int CDevMgr::GetDevsByOwner(char *pName, CDevice *pDevs[])
{
	int nCnt = 0;
	CDevice *d = NULL;
	CNode *p = m_pDevQueue->GetFirst();
	while(p != NULL)
	{
		d = (CDevice *)(p->m_pObject);
		if(strcmp(pName, d->GetOwner()) == 0)
		{
			pDevs[nCnt++] = d;
			if(USR_MAX_DEV <= nCnt)
			{
				break;
			}
		}
		p = m_pDevQueue->GetNext();
	}
	return nCnt;
}

int CDevMgr::IsExit()
{
	return m_nExit;
}

int CDevMgr::PrintLog(char *pStr)
{
	if(m_pLog != NULL)
	{
		m_pLog->Output(pStr, strlen(pStr));
	}
	return 0;
}

int CDevMgr::StartFresh()
{
	LoadDevices();
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CDevMgr::StartFresh()\n");
	PrintLog(szLog);
	m_hHandle = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))FreshDevStatus, this, 0, 0);
	return 0;
}

int CDevMgr::StopFresh()
{
	char szLog[BUF_SIZE] = {0};

	m_nExit = 1;
	WaitForSingleObject(m_hHandle, INFINITE);
    CloseHandle(m_hHandle);

	sprintf_s(szLog, BUF_SIZE, "CDevMgr::StopFresh()\n");
	PrintLog(szLog);
	return 0;
}

int CDevMgr::LoadDevices()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CDevMgr::LoadDevices()\n");
	PrintLog(szLog);
	LoadDevices(DEVICES_PATH);
	return 0;
}

int CDevMgr::LoadDevices(char *sDir)
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
			sprintf_s(szLog, BUF_SIZE, "CDevMgr::LoadDevices() szPath=%s\n", szPath);
			PrintLog(szLog);
			if(fd.attrib == _A_SUBDIR)
			{
				LoadDevices(szPath);
			}
			else if(fd.attrib == _A_ARCH)
			{
				char *pExt = strstr(fd.name, ".ini");
				if(pExt)
				{
					char szMac[MAC_SIZE] = {0};
					memset(szMac, 0, MAC_SIZE);
					memcpy(szMac, fd.name, pExt-fd.name);
					CDevice *pNewDev = new CDevice();
					pNewDev->SetMac(szMac, strlen(szMac));
					pNewDev->SetDir(sDir, strlen(sDir));
					nErr = pNewDev->Load();
					sprintf_s(szLog, BUF_SIZE, "CDevMgr::LoadDevices() szDir=%s szMac=%s nErr=%d\n", sDir, szMac, nErr);
					PrintLog(szLog);
					if(nErr != -1)
					{
						AddDev(pNewDev);
					}
					else
					{
						delete pNewDev;
					}
				}
			}
		}
		_findclose(handle);
	}
	return 0;
}