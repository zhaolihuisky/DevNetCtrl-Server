#include "StdAfx.h"
#include "Peer.h"

CPeer::CPeer(void)
{
	m_nIp = 0;
	m_nPort = 0;
	memset(&m_saFrom, 0, sizeof(m_saFrom));
	m_tFirst = time(NULL);
	m_tLast = m_tFirst;
	m_nStatus = STATUS_OFFLINE;
}

CPeer::~CPeer(void)
{
}

int CPeer::GetIp()
{
	return m_nIp;
}

int CPeer::SetIp(int ip)
{
	m_nIp = ip;
	return 0;
}

int CPeer::GetPort()
{
	return m_nPort;
}

int CPeer::SetPort(int port)
{
	m_nPort = port;
	return 0;
}

int CPeer::SetAddr(struct sockaddr_in *pFrom)
{
	memcpy(&m_saFrom, pFrom, sizeof(struct sockaddr_in));
	return 0;
}

int CPeer::GetAddr(struct sockaddr_in *pFrom)
{
	memcpy(pFrom, &m_saFrom, sizeof(struct sockaddr_in));
	return 0;
}

int CPeer::SetStatus(int nStatus)
{
	m_nStatus = nStatus;
	return 0;
}

int CPeer::GetStatus()
{
	return m_nStatus;
}

int CPeer::SetLastTime(time_t tLast)
{
	m_tLast = tLast;
	return 0;
}

int CPeer::SetFirstTime(time_t tFirst)
{
	m_tFirst = tFirst;
	return 0;
}

time_t CPeer::GetLastTime()
{
	return m_tLast;
}

time_t CPeer::GetFirstTime()
{
	return m_tFirst;
}