#pragma once
class CUtil
{
public:
	CUtil(void);
	~CUtil(void);
	static int Int2Bytes(int n, unsigned char *b);
	static int Bytes2Int(unsigned char *b);
	static short Short2Bytes(short n, unsigned char *b);
	static short Bytes2Short(unsigned char *b);
	static char *Int2Ip(int ip);
	static int Ip2Int(char *pIp);
	static unsigned short Crc16(char * pData, int nCnt);
	static char *Data2String(char *pData, int nLen);
};

