#pragma once

class CNode
{
public:
	CNode(void);
	~CNode(void);
	CBase *m_pObject;
	class CNode *m_pNext;
};

class CQueue
{
public:
	CQueue(void);
	~CQueue(void);
private:
	int m_nCount;
	CNode *m_pFront;
	CNode *m_pRear;
	CNode *m_pNext;
	CRITICAL_SECTION m_csLock;
public:
	int Push(CNode *pNode);
	CNode *Pop();
	int Remove(CNode *pNode);
	int GetCount();
	int ClearQueue();
	CNode *GetFirst();
	CNode *GetNext();
};
