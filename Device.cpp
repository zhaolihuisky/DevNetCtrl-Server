#include "StdAfx.h"
#include "Device.h"


CDevice::CDevice(void)
{
	memset(m_szMac, 0, MAC_SIZE);
	memset(m_szOwner, 0, NAME_SIZE);
	memset(m_szDir, 0, PATH_SIZE);
}

CDevice::~CDevice(void)
{
}

char *CDevice::GetMac()
{
	return m_szMac;
}

int CDevice::SetMac(char *pMac, int nLen)
{
	memcpy(m_szMac, pMac, nLen);
	return 0;
}

int CDevice::SetOwner(char *pName, int nLen)
{
	if(pName != NULL)
	{
		memcpy(m_szOwner, pName, nLen);
	}
	else
	{
		memset(m_szOwner, 0, NAME_SIZE);
	}
	
	return 0;
}

char *CDevice::GetOwner()
{
	return m_szOwner;
}

int CDevice::SetDir(char *pPath, int nLen)
{
	memcpy(m_szDir, pPath, nLen);
	return 0;
}

int CDevice::MakeDir(char *pDir)
{
	char szDir[PATH_SIZE] = {0};
	int nErr = 0;
	char *b = pDir;
	while(*b != '\0')
	{
		memset(szDir, 0, PATH_SIZE);
		char *e = strchr(b, '\\');
		if(e == b)
		{
			b++;
		}
		else
		{
			if(e == NULL)
			{
				sprintf_s(szDir, PATH_SIZE, "%s", pDir);
			}
			else
			{
				memcpy(szDir, pDir, e-pDir);
				b = e;
			}
			if(strcmp(szDir, ".") != 0)
			{
				nErr = _mkdir(szDir);
			}
			if(nErr == -1)
			{
				if(errno == EEXIST)
				{
				}
				else if(errno == ENOENT)
				{
					return -1;
				}
			}
			if(e == NULL)
			{
				break;
			}
		}
	}
	return 0;
}

int CDevice::Save()
{
	int nCnt = 0;
	int nErr = 0;
	char szPath[PATH_SIZE] = {0};
	char szText[BUF_SIZE]= {0};
	FILE *fFile = NULL;
	MakeDir(m_szDir);
	sprintf_s(szPath, PATH_SIZE, "%s\\%s.ini", m_szDir, m_szMac);
	int nRet = fopen_s(&fFile, szPath, "w");
	if(fFile)
	{
		time_t tFirst = GetFirstTime();
		struct tm tmFirst;
		localtime_s(&tmFirst, &tFirst);
		char szTime[BUF_SIZE] = {0};
		sprintf_s(szTime, BUF_SIZE, "%04d%02d%02d%02d%02d%02d", tmFirst.tm_year+1900, tmFirst.tm_mon+1, tmFirst.tm_mday, tmFirst.tm_hour, tmFirst.tm_min, tmFirst.tm_sec);
		nCnt = sprintf_s(szText, BUF_SIZE, "[DEV]\nMAC=%s\nOWNER=%s\nTIME=%s\n", m_szMac, m_szOwner, szTime);
		fwrite((void*)szText, 1, nCnt, fFile);
		fflush(fFile);
		fclose(fFile);
	}
	else
	{
		nErr = -1;
	}
	return nErr;
}

int CDevice::Load()
{
	int nCnt = 0;
	int nErr = 0;
	char *pKey = NULL;
	char *pVal = NULL;
	char szPath[PATH_SIZE] = {0};
	char szText[BUF_SIZE]= {0};
	char szTime[BUF_SIZE] = {};
	FILE *fFile = NULL;
	sprintf_s(szPath, PATH_SIZE, "%s\\%s.ini", m_szDir, m_szMac);
	int nRet = fopen_s(&fFile, szPath, "r");
	if(fFile)
	{
		memset(szText, 0, BUF_SIZE);
		fread(szText, 1, BUF_SIZE, fFile);
		fclose(fFile);
		nErr = sscanf_s(szText, "[DEV]\nMAC=%s\nOWNER=%s\nTIME=%s\n", m_szMac, MAC_SIZE, m_szOwner, NAME_SIZE, szTime, BUF_SIZE);
		if(nErr == 3)
		{
			struct tm tmFirst;
			sscanf_s(szTime, "%04d%02d%02d%02d%02d%02d", &tmFirst.tm_year, &tmFirst.tm_mon, &tmFirst.tm_mday, &tmFirst.tm_hour, &tmFirst.tm_min, &tmFirst.tm_sec);
			tmFirst.tm_year -= 1900;
			tmFirst.tm_mon -= 1;
			time_t tFirst = mktime(&tmFirst);
			SetFirstTime(tFirst);
		}
		else
		{
			nErr = -1;
		}
		/*
		pKey = strstr(szText, "MAC=");
		pKey = pKey + 4;
		pVal = strchr(pKey, '\n');
		memcpy(m_szMac, pKey, pVal-pKey);

		pKey = strstr(pVal, "OWNER=");
		pKey = pKey + 6;
		pVal = strchr(pKey, '\n');
		memcpy(m_szOwner, pKey, pVal-pKey);

		pKey = strstr(pVal, "TIME=");
		pKey = pKey + 5;
		pVal = strchr(pKey, '\n');
		char szTime[BUF_SIZE] = {};
		memcpy(szTime, pKey, pVal-pKey);
		struct tm tmFirst;
		sscanf_s(szTime, "%04d%02d%02d%02d%02d%02d", &tmFirst.tm_year, &tmFirst.tm_mon, &tmFirst.tm_mday, &tmFirst.tm_hour, &tmFirst.tm_min, &tmFirst.tm_sec);
		tmFirst.tm_year -= 1900;
		tmFirst.tm_mon -= 1;
		time_t tFirst = mktime(&tmFirst);
		SetFirstTime(tFirst);
		*/
	}
	else
	{
		nErr = -1;
	}
	return 0;
}

int CDevice::Delete()
{
	char szPath[PATH_SIZE] = {0};
	sprintf_s(szPath, PATH_SIZE, "%s\\%s.ini", m_szDir, m_szMac);
	remove(szPath);
	return 0;
}