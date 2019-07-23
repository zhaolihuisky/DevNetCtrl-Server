#include "StdAfx.h"
#include "Queue.h"

CNode::CNode(void)
{
	m_pObject = NULL;
	m_pNext = NULL;
}

CNode::~CNode(void)
{
	if(m_pObject != NULL)
	{
		delete m_pObject;
		m_pObject = NULL;
	}
	m_pNext = NULL;
}

CQueue::CQueue(void)
{
	InitializeCriticalSection(&m_csLock);
	m_pFront = NULL;
	m_pRear = NULL;
	m_nCount = 0;
}

CQueue::~CQueue(void)
{
	ClearQueue();
	DeleteCriticalSection(&m_csLock);
}

int CQueue::Push(CNode *pNode)
{
	EnterCriticalSection(&m_csLock);
	if(m_pRear == NULL)
	{
		m_pFront = pNode;
		m_pRear = m_pFront;
	}
	else
	{
		m_pRear->m_pNext = pNode;
		m_pRear = pNode;
	}
	m_pRear->m_pNext = NULL;
	m_nCount++;
	LeaveCriticalSection(&m_csLock);
	return 0;
}

CNode *CQueue::Pop()
{
	CNode *pNode = NULL;
	EnterCriticalSection(&m_csLock);
	if(m_pFront)
	{
		pNode = m_pFront;
		m_pFront = m_pFront->m_pNext;
		pNode->m_pNext = NULL;
		m_nCount--;
	}
	if(m_nCount == 0)
	{
		m_pFront = NULL;
		m_pRear = NULL;
	}
	LeaveCriticalSection(&m_csLock);
	return pNode;
}

int CQueue::Remove(CNode *pNode)
{
	EnterCriticalSection(&m_csLock);
	CNode *pf = m_pFront;
	CNode *p = pf->m_pNext;
	if(m_pFront == pNode)
	{
		m_pFront = pf->m_pNext;
		if(m_pRear == pNode)
		{
			m_pRear = NULL;
		}
		delete pf;
		m_nCount--;
	}
	else
	{
		while(p)
		{
			if(p == pNode)
			{
				if(p == m_pRear)
				{
					m_pRear = pf;
				}
				pf->m_pNext = p->m_pNext;
				delete p;
				m_nCount--;
				break;
			}
			pf = pf->m_pNext;
			p = pf->m_pNext;
		}
	}
	LeaveCriticalSection(&m_csLock);
	return 0;
}

int CQueue::GetCount()
{
	int nCount = 0;
	EnterCriticalSection(&m_csLock);
	nCount = m_nCount;
	LeaveCriticalSection(&m_csLock);
	return nCount;
}

int CQueue::ClearQueue()
{
	EnterCriticalSection(&m_csLock);
	CNode *pNode = NULL;
	while(m_pFront)
	{
		pNode = m_pFront;
		m_pFront = m_pFront->m_pNext;
		delete pNode;
	}
	m_nCount = 0;
	m_pFront = NULL;
	m_pRear = NULL;
	m_pNext = NULL;
	LeaveCriticalSection(&m_csLock);
	return 0;
}

CNode *CQueue::GetFirst()
{
	EnterCriticalSection(&m_csLock);
	m_pNext = m_pFront;
	LeaveCriticalSection(&m_csLock);
	return m_pNext;
}

CNode *CQueue::GetNext()
{
	EnterCriticalSection(&m_csLock);
	m_pNext = m_pNext->m_pNext;
	LeaveCriticalSection(&m_csLock);
	return m_pNext;
}