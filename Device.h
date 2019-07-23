#pragma once

class CDevice : public CPeer
{
public:
	CDevice(void);
	~CDevice(void);

	int SetMac(char *pMac, int nLen);
	char *GetMac();
	int SetOwner(char *pName, int nLen);
	char *GetOwner();

	int SetDir(char *pPath, int nLen);
	int MakeDir(char *pDir);
	int Save();
	int Load();
	int Delete();
private:
	char m_szMac[MAC_SIZE];
	char m_szOwner[NAME_SIZE];
	char m_szDir[PATH_SIZE];
};

