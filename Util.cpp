#include "StdAfx.h"
#include "Util.h"


CUtil::CUtil(void)
{
}

CUtil::~CUtil(void)
{
}

int CUtil::Int2Bytes(int n, unsigned char *b)
{
	//unsigned char *b = new unsigned char[4];
	b[3] = (unsigned char) (n & 0xff);
	b[2] = (unsigned char) (n >> 8 & 0xff);
	b[1] = (unsigned char) (n >> 16 & 0xff);
	b[0] = (unsigned char) (n >> 24 & 0xff);
	return 0;
}

int CUtil::Bytes2Int(unsigned char *b)
{
	return b[3] & 0xff | (b[2] & 0xff) << 8 | (b[1] & 0xff) << 16 | (b[0] & 0xff) << 24;
}

short CUtil::Short2Bytes(short n, unsigned char *b)
{
	b[1] = (unsigned char) (n & 0xff);
	b[0] = (unsigned char) (n >> 8 & 0xff);
	return 0;
}

short CUtil::Bytes2Short(unsigned char *b)
{
	return (short)(b[1] & 0xff | (b[0] & 0xff) << 8);
}

char *CUtil::Int2Ip(int ip)
{
	char *pIp = new char[16];
	memset(pIp, 0, 16);
	sprintf_s(pIp, 16, "%d.%d.%d.%d", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, ip & 0x000000ff);
	return pIp;
}

int CUtil::Ip2Int(char *pIp)
{
	int i = 3;
	int nIp = 0;
	int nTmp = 0;
	char szTmp[4] = {0};
	char *pFront = pIp;
	char *pRear = pFront;
	while(*pRear != '\0')
	{
		if(*pRear == '.')
		{
			memset(szTmp, 0, 4);
			memcpy(szTmp, pFront, pRear-pFront);
			int nTmp = atoi(szTmp);
			nIp = nIp | ((nTmp & 0xff) << (i*8));
			pFront = pRear + 1;
			i--;
		}
		pRear++;
	}
	memset(szTmp, 0, 4);
	memcpy(szTmp, pFront, pRear-pFront);
	nTmp = atoi(szTmp);
	nIp = nIp | (nTmp & 0xff);
	return nIp;
}

unsigned short CUtil::Crc16(char * pData, int nCnt)
{
	int i = 0, j = 0;
	unsigned short usCrc = 0xffff;
	unsigned short usPoly = 0xa001;
	for(i=0; i<nCnt; i++)
	{
		usCrc = usCrc ^ (*(pData+i) & 0xFF);
		for(j=0; j<8; j++)
		{
			if ((usCrc & 0x0001) == 0x00001)
				usCrc = (usCrc>>1) ^ usPoly;
			else
				usCrc = usCrc>>1;
		}
	}
	return usCrc;
}

char *CUtil::Data2String(char *pData, int nLen)
{
	int i = 0, j = 0;
	int nCnt = nLen*2+1;
	char *pStr = new char[nCnt];
	memset(pStr, 0, nCnt);
	for(i=0; i<nLen; i++)
	{
		j += sprintf_s(pStr+j, nCnt-j, "%02X", pData[i] & 0xFF);
	}
	return pStr;
}