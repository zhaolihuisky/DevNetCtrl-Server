#pragma once

class CUser : public CPeer
{
public:
	CUser(void);
	~CUser(void);

	char *GetName();
	int SetName(char *pName, int nLen);
	char *GetPwd();
	int SetPwd(char *pPwd, int nLen);

	int SetDir(char *pPath, int nLen);
	int MakeDir(char *pDir);
	int Save();
	int Load();
	int Delete();
private:
	char m_szName[NAME_SIZE];
	char m_szPwd[PWD_SIZE];
	char m_szDir[PATH_SIZE];
};

