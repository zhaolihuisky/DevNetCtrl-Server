#include "StdAfx.h"
#include "Log.h"

int HandleLog(void* param)
{
	CLog *pLog = (CLog *)param;
	CQueue *pQueue = pLog->GetQueue();
	CNode *pNode = NULL;
	CBase *pRec = NULL;
	int nCnt = 0;
	char *sLog = NULL;
	int nLen = 0;

	while(pLog->IsExit() == 0)
	{
		nCnt = pQueue->GetCount();
		if(nCnt > 0)
		{
			pNode = pQueue->Pop();
			pRec = pNode->m_pObject;
			sLog = pRec->GetData();
			nLen = pRec->GetSize();
			if(nLen > 0)
			{
				pLog->PrintLog(sLog, nLen);
			}
			delete pNode;
		}
		else
		{
			Sleep(100);
		}
	}
	return 0;
}

int CLog::PrintLog(char *sLog, int nLen)
{
	int nCnt = 0;
	time_t t;
	t = time(NULL);
	struct tm p;
	localtime_s(&p, &t);
	char szBuf[BUF_SIZE] = {0};
	nCnt = sprintf_s(szBuf, BUF_SIZE, "%04d-%02d-%02d %02d:%02d:%02d ", p.tm_year+1900, p.tm_mon+1,p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
	nCnt += sprintf_s(szBuf+nCnt, BUF_SIZE-nCnt, "%s", sLog);
	//int nLen = WideCharToMultiByte(CP_ACP, NULL, LPCWSTR(strTime), -1, LPSTR(szLogAnsi), BUF_SIZE, NULL, FALSE);
	char *sFile = ".\\Log.txt";
	FILE *fLog = NULL;
	int nRet = fopen_s(&fLog, sFile, "a+");
	if(fLog)
	{
		long lMaxSize = LOG_SIZE_4MB;
		fseek(fLog, 0, SEEK_END);
		long lSize = ftell(fLog);
		if(lSize >= lMaxSize)
		{
			int i = 0;
			fclose(fLog);
			fLog = NULL;
			char szNewFile[PATH_SIZE] = {0}, szOldFile[PATH_SIZE] = {0};
			for(i=9; i>0; i--)
			{
				sprintf_s(szOldFile, PATH_SIZE, ".\\Log_%d.txt", i-1);
				sprintf_s(szNewFile, PATH_SIZE, ".\\Log_%d.txt", i);
				if(i == 9) remove(szNewFile);
				FILE *fTemp = NULL;
				nRet = fopen_s(&fTemp, szOldFile, "rt");
				if(fTemp)
				{
					fclose(fTemp);
					fTemp = NULL;
					rename(szOldFile, szNewFile);
				}
				if(i == 1) rename(sFile, szOldFile);
			}
			nRet = fopen_s(&fLog, sFile, "a+");
		}
		if(fLog)
		{
			fwrite((void*)szBuf, 1, nCnt, fLog);
			fflush(fLog);
			fclose(fLog);
		}
	}
	return 0;
}

CLog::CLog(void)
{
	m_nExit = 0;
	m_hHandle = NULL;
	m_pLogQueue = new CQueue();
}

CLog::~CLog(void)
{
	delete m_pLogQueue;
}

int CLog::IsExit()
{
	return m_nExit;
}

CQueue *CLog::GetQueue()
{
	return m_pLogQueue;
}

int CLog::Output(char *pLog, int nLen)
{
	CBase *pRec = new CBase();
	pRec->SetData(pLog, nLen);
	CNode *pNode = new CNode();
	pNode->m_pObject = pRec;
	m_pLogQueue->Push(pNode);
	return 0;
}

int CLog::StartHandle()
{
	m_hHandle = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))HandleLog, this, 0, 0);
	return 0;
}

int CLog::StopHandle()
{
	m_nExit = 1;
	WaitForSingleObject(m_hHandle, INFINITE);
    CloseHandle(m_hHandle);
	return 0;
}