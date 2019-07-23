#pragma once
class CBase
{
public:
	CBase(void);
	~CBase(void);
	int GetSize();
	char *GetData();
	void SetData(char *data, int size);
private:
	int m_nSize;
	char *m_pData;
};

