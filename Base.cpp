#include "StdAfx.h"
#include "Base.h"


CBase::CBase(void)
{
	m_nSize = 0;
	m_pData = NULL;
}

CBase::~CBase(void)
{
	if (m_pData != NULL)
	{
		delete m_pData;
		m_pData = NULL;
	}
}

int CBase::GetSize()
{
	return m_nSize;
}

char *CBase::GetData()
{
	return m_pData;
}

void CBase::SetData(char *data, int size)
{
	m_nSize = size;
	m_pData = new char[m_nSize+1];
	memset(m_pData, 0, m_nSize+1);
	memcpy(m_pData, data, m_nSize);
}