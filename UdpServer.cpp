#include "StdAfx.h"
#include "UdpServer.h"

int DataRecv(void* param)
{
	CUdpServer *pUs = (CUdpServer *)param;
	SOCKET udpSocket = pUs->GetSocket();
	CQueue *pRecvQueue = pUs->GetRecvQueue();
	struct sockaddr_in from;
	int fromlen = sizeof(from);
	int nErr = 0;
	char szBuf[RX_SIZE] = {0};

	fd_set fdRead;
	timeval time;
	time.tv_sec = 10;//INFINITE;
	time.tv_usec = 0;

	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv()\n");
	pUs->PrintLog(szLog);

	while(pUs->IsRecvExit() == 0)
	{
		FD_ZERO(&fdRead);
		FD_SET(udpSocket, &fdRead);
		sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() select begin\n");
		pUs->PrintLog(szLog);
		nErr = select(0, &fdRead, NULL, NULL, &time);
		sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() select end nErr=%d\n", nErr);
		pUs->PrintLog(szLog);
		if(0 == nErr)
		{
			sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() select timeout\n");
			pUs->PrintLog(szLog);
			continue;
		}
		else if(SOCKET_ERROR == nErr)
		{
			nErr = WSAGetLastError();
			sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() select error %d\n", nErr);
			pUs->PrintLog(szLog);
			continue;
		}
		if(FD_ISSET(udpSocket, &fdRead))
		{
			memset(szBuf, 0, RX_SIZE);
			nErr = recvfrom(udpSocket, szBuf, RX_SIZE, 0, (struct sockaddr*)&from, &fromlen);
			if(SOCKET_ERROR == nErr || 0 == nErr)
			{
				nErr = WSAGetLastError();
				sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() recvfrom error %d\n", nErr);
				pUs->PrintLog(szLog);
				continue;
			}
			if(RX_SIZE <= nErr)
			{
				nErr = RX_SIZE - 1;
				szBuf[nErr] = '\0';
			}
			int nIp = ntohl(from.sin_addr.s_addr);
			int nPort = ntohs(from.sin_port);
			char *sIp = CUtil::Int2Ip(nIp);
			char *pStr = CUtil::Data2String(szBuf, nErr);
			sprintf_s(szLog, BUF_SIZE, "CUdpServer DataRecv() recvfrom(%s:%d) size:%d %s\n", sIp, nPort, nErr, pStr);
			pUs->PrintLog(szLog);
			delete pStr;
			delete sIp;
			CPeer *p = new CPeer();
			p->SetIp(nIp);
			p->SetPort(nPort);
			p->SetAddr(&from);
			p->SetData(szBuf, nErr);
			CNode *node = new CNode();
			node->m_pObject = (CBase *)p;
			if(pRecvQueue != NULL)
			{
				pRecvQueue->Push(node);
			}
		}
	}
	return 0;
}

int DataSend(void* param)
{
	CUdpServer *pUs = (CUdpServer *)param;
	SOCKET udpSocket = pUs->GetSocket();
	CQueue *pSendQueue = pUs->GetSendQueue();
	CNode *pNode = NULL;
	CPeer *pPeer = NULL;
	int nCnt = 0;
	int nErr = 0;

	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUdpServer DataSend()\n");
	pUs->PrintLog(szLog);
	while(pUs->IsSendExit() == 0)
	{
		nCnt = pSendQueue->GetCount();
		if(nCnt > 0)
		{
			pNode = pSendQueue->Pop();
			pPeer = (CPeer *)(pNode->m_pObject);
			char *pData = pPeer->GetData();
			int nSize = pPeer->GetSize();
			int nIp = pPeer->GetIp();
			int nPort = pPeer->GetPort();
			struct sockaddr_in saFrom;
			int nFromSize = sizeof(saFrom);
			pPeer->GetAddr(&saFrom);
			char *sIp = CUtil::Int2Ip(nIp);
			char *sCmd = CUtil::Data2String(pData, nSize);
			sprintf_s(szLog, BUF_SIZE, "CUdpServer DataSend() to(%s:%d) nSize=%d sCmd=%s\n", sIp, nPort, nSize, sCmd);
			pUs->PrintLog(szLog);
			delete sCmd;
			//from.sin_family = AF_INET;
			//from.sin_addr.s_addr = htonl(nIp);
			//from.sin_port = htons(nPort);
			nErr = sendto(udpSocket, pData, nSize, 0, (struct sockaddr*)&saFrom, nFromSize);
			delete pNode;
			if(nErr == SOCKET_ERROR)
			{
				nErr = WSAGetLastError();
				sprintf_s(szLog, BUF_SIZE, "CUdpServer DataSend() sendto(%s:%d) error %d\n", sIp, nPort, nErr);
				pUs->PrintLog(szLog);
			}
			delete sIp;
		}
		else
		{
			Sleep(10);
		}
	}
	return 0;
}

CUdpServer::CUdpServer(void)
{
	m_nRecvExit = 0;
	m_nSendExit = 0;
	m_sUdp = 0;
	m_hRecv = NULL;
	m_hSend = NULL;
	m_pRecvQueue = NULL;
	m_pSendQueue = NULL;
	m_pLog = NULL;
}

CUdpServer::~CUdpServer(void)
{
	closesocket(m_sUdp);
}

int CUdpServer::InitSocket()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUdpServer::InitSocket()\n");
	PrintLog(szLog);
	int nErr = 0;
	WSADATA wsaData;
	nErr = WSAStartup(MAKEWORD(2, 1), &wsaData);
	if(nErr)
	{
		sprintf_s(szLog, BUF_SIZE, "CUdpServer::InitSocket() WSAStartup() failed nErr=%d\n", nErr);
		PrintLog(szLog);
		WSACleanup();
		return nErr;
	}
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	m_sUdp = socket(AF_INET, SOCK_DGRAM, 0);
	nErr = bind(m_sUdp, (struct sockaddr*)&server, sizeof(server));
	sprintf_s(szLog, BUF_SIZE, "CUdpServer::InitSocket() bind() nErr=%d\n", nErr);
	PrintLog(szLog);
	return 0;
}

SOCKET CUdpServer::GetSocket()
{
	return m_sUdp;
}

int CUdpServer::SetRecvQueue(CQueue *pQueue)
{
	m_pRecvQueue = pQueue;
	return 0;
}

int CUdpServer::SetSendQueue(CQueue *pQueue)
{
	m_pSendQueue = pQueue;
	return 0;
}

CQueue *CUdpServer::GetRecvQueue()
{
	return m_pRecvQueue;
}

CQueue *CUdpServer::GetSendQueue()
{
	return m_pSendQueue;
}

int CUdpServer::SetLog(CLog *pLog)
{
	m_pLog = pLog;
	return 0;
}

CLog *CUdpServer::GetLog()
{
	return m_pLog;
}

int CUdpServer::PrintLog(char *pStr)
{
	if(m_pLog != NULL)
	{
		m_pLog->Output(pStr, strlen(pStr));
	}
	return 0;
}

int CUdpServer::StartRecv()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUdpServer::StartRecv()\n");
	PrintLog(szLog);
	m_hRecv = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))DataRecv, this, 0, 0);
	return 0;
}

int CUdpServer::StopRecv()
{
	char szLog[BUF_SIZE] = {0};

	m_nRecvExit = 1;
    WaitForSingleObject(m_hRecv, INFINITE);
    CloseHandle(m_hRecv);

	sprintf_s(szLog, BUF_SIZE, "CUdpServer::StopRecv()\n");
	PrintLog(szLog);
	return 0;
}

int CUdpServer::StartSend()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CUdpServer::StartSend()\n");
	PrintLog(szLog);
	m_hSend = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))DataSend, this, 0, 0);
	return 0;
}

int CUdpServer::StopSend()
{
	char szLog[BUF_SIZE] = {0};

	m_nSendExit = 1;
    WaitForSingleObject(m_hSend, INFINITE);
    CloseHandle(m_hSend);

	sprintf_s(szLog, BUF_SIZE, "CUdpServer::StopSend()\n");
	PrintLog(szLog);
	return 0;
}

int CUdpServer::IsRecvExit()
{
	return m_nRecvExit;
}

int CUdpServer::IsSendExit()
{
	return m_nSendExit;
}