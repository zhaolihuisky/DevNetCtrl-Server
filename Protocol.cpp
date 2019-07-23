#include "StdAfx.h"
#include "Protocol.h"

int HandleData(void* param)
{
	CProtocol *pPro = (CProtocol *)param;
	CQueue *pRecvQueue = pPro->GetRecvQueue();
	CNode *pNode = NULL;
	int nCnt = 0;

	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CProtocol HandleData()\n");
	pPro->PrintLog(szLog);

	while(pPro->IsExit() == 0)
	{
		nCnt = pRecvQueue->GetCount();
		if(nCnt > 0)
		{
			pNode = pRecvQueue->Pop();
			if(pNode != NULL)
			{
				pPro->NetControl(pNode);
				delete pNode;
			}
		}
		else
		{
			Sleep(100);
		}
	}
	return 0;
}

CProtocol::CProtocol(void)
{
	m_nExit = 0;
	m_pLog = NULL;
	m_pRecvQueue = new CQueue();
	m_pSendQueue = new CQueue();
	m_pUdpServer = new CUdpServer();
	m_pDevMgr = new CDevMgr();
	m_pUserMgr = new CUserMgr();
}

CProtocol::~CProtocol(void)
{
	delete m_pRecvQueue;
	delete m_pSendQueue;
	delete m_pUdpServer;
	delete m_pDevMgr;
	delete m_pUserMgr;
}
int CProtocol::Init()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CProtocol::Init() begin\n");
	PrintLog(szLog);
	m_pUdpServer->SetLog(m_pLog);
	m_pUdpServer->SetRecvQueue(m_pRecvQueue);
	m_pUdpServer->SetSendQueue(m_pSendQueue);
	m_pUdpServer->InitSocket();
	m_pDevMgr->SetLog(m_pLog);
	m_pUserMgr->SetLog(m_pLog);

	sprintf_s(szLog, BUF_SIZE, "CProtocol::Init() end\n");
	PrintLog(szLog);
	return 0;
}

int CProtocol::StartHandle()
{
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CProtocol::StartHandle()\n");
	PrintLog(szLog);
	m_hHandle = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall *)(void *))HandleData, this, 0, 0);
	m_pUdpServer->StartRecv();
	m_pUdpServer->StartSend();
	m_pDevMgr->StartFresh();
	m_pUserMgr->StartFresh();
	return 0;
}

int CProtocol::StopHandle()
{
	char szLog[BUF_SIZE] = {0};
	m_pUdpServer->StopRecv();
	m_pUdpServer->StopSend();
	m_pDevMgr->StopFresh();
	m_pUserMgr->StopFresh();

	m_nExit = 1;
	WaitForSingleObject(m_hHandle, INFINITE);
    CloseHandle(m_hHandle);

	sprintf_s(szLog, BUF_SIZE, "CProtocol::StopHandle()\n");
	PrintLog(szLog);
	return 0;
}

int CProtocol::IsExit()
{
	return m_nExit;
}

int CProtocol::SetLog(CLog *pLog)
{
	m_pLog = pLog;
	return 0;
}

CLog *CProtocol::GetLog()
{
	return m_pLog;
}

int CProtocol::PrintLog(char *pStr)
{
	if(m_pLog != NULL)
	{
		m_pLog->Output(pStr, strlen(pStr));
	}
	return 0;
}

CQueue *CProtocol::GetRecvQueue()
{
	return m_pRecvQueue;
}

CQueue *CProtocol::GetSendQueue()
{
	return m_pSendQueue;
}

int CProtocol::NetControl(CNode *pNode)
{
	CPeer *pPeer = NULL;
	char *pBuf = NULL;
	int nSize = 0;
	char szLog[BUF_SIZE] = {0};
	pPeer = (CPeer *)(pNode->m_pObject);
	nSize = pPeer->GetSize();
	pBuf = pPeer->GetData();
	memset(szLog, 0, BUF_SIZE);
	char *pStr = CUtil::Data2String(pBuf, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::NetControl() Data(%d): %s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)
	if(nSize > CMD_MIN_SIZE)
	{
		unsigned short usAddr = 0;
		((char *)&usAddr)[0] = pBuf[1];
		((char *)&usAddr)[1] = pBuf[0];
		unsigned short usSize = 0;
		((char *)&usSize)[0] = pBuf[3];
		((char *)&usSize)[1] = pBuf[2];
		unsigned char ucCode = pBuf[4];
		unsigned short usOffset = 0;
		((char *)&usOffset)[0] = pBuf[6];
		((char *)&usOffset)[1] = pBuf[5];
		sprintf_s(szLog, BUF_SIZE, "CProtocol::NetControl() usAddr=0x%04X usSize=0x%04X ucCode=0x%02X usOffset=0x%04X\n", usAddr, usSize, ucCode, usOffset);
		PrintLog(szLog);
		if((ucCode == 0x7F) && (usSize == nSize))
		{
			switch(usOffset)
			{
			case CMD_TRANSLATE:
				CmdTranslate(pPeer);
				break;
			case USR_REGISTER:
				UsrRegister(pPeer);
				break;
			case USR_UNREGISTER:
				UsrUnregister(pPeer);
				break;
			case USR_LOGON:
				UsrLogon(pPeer);
				break;
			case USR_LOGOFF:
				UsrLogoff(pPeer);
				break;
			case USR_DEV_REGISTER:
				UsrDevRegister(pPeer);
				break;
			case USR_DEV_UNREGISTER:
				UsrDevUnregister(pPeer);
				break;
			case DEV_CONNECT:
				DevConnect(pPeer);
				break;
			case DEV_DISCONNECT:
				DevDisconnect(pPeer);
				break;
			case USR_DEV_LIST:
				UsrDevList(pPeer);
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

int CProtocol::CmdTranslate(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|maclen(1B)|mac(12B)|cmdlen(1B)|cmd(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szLog[BUF_SIZE] = {0};
	unsigned char ucNameLen = pVal[0];
	if(ucNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, ucNameLen);
		unsigned char ucMacLen = pVal[1+ucNameLen];
		if(ucMacLen < MAC_SIZE)
		{
			char szMac[MAC_SIZE] = {0};
			memset(szMac, 0, MAC_SIZE);
			memcpy(szMac, pVal+1+ucNameLen+1, ucMacLen);
			//char *sMac = CUtil::Data2String(szMac, nMacLen);

			int nCmdLen = pVal[1+ucNameLen+1+ucMacLen];
			char szCmd[CMD_SIZE] = {0};
			memcpy(szCmd, pVal+1+ucNameLen+1+ucMacLen+1, nCmdLen);
			
			char *pStr = CUtil::Data2String(szCmd, nCmdLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::CmdTranslate() name=%s mac=%s cmd=%s\n", szName, szMac, pStr);
			delete pStr;
			PrintLog(szLog);

			CUser *pUser = m_pUserMgr->GetUser(szName);
			CDevice *pDev = m_pDevMgr->GetDev(szMac);
			//delete sMac;
			sprintf_s(szLog, BUF_SIZE, "CProtocol::CmdTranslate() pUser=%p pDev=%p\n", pUser, pDev);
			PrintLog(szLog);
			if((pUser != NULL) && (pDev != NULL) && (strcmp(pDev->GetOwner(), szName) == 0))
			{
				CNode *pDevNode = new CNode();
				CPeer *pDevPeer = new CPeer();
				pDevNode->m_pObject = (CBase *)pDevPeer;
				pDevPeer->SetData(szCmd, nCmdLen);
				pDevPeer->SetIp(pDev->GetIp());
				pDevPeer->SetPort(pDev->GetPort());
				memset(&saFrom, 0, sizeof(&saFrom));
				pDev->GetAddr(&saFrom);
				pDevPeer->SetAddr(&saFrom);
				m_pSendQueue->Push(pDevNode);
			}
		}
	}

	return 0;
}

int CProtocol::UsrRegister(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|pwdlen(1B)|pwd(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[2] = {0x01, 0x01};
	char szLog[BUF_SIZE] = {0};
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nPwdLen = pVal[1+nNameLen];
		if(nPwdLen < PWD_SIZE)
		{
			char szPwd[PWD_SIZE] = {0};
			memcpy(szPwd, pVal+1+nNameLen+1, nPwdLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrRegister() name=%s pwd=%s\n", szName, szPwd);
			PrintLog(szLog);
			int nValid = m_pUserMgr->IsNameValid(szName);
			if(nValid == 0)
			{
				CUser *pUser = m_pUserMgr->GetUser(szName);
				sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrRegister() pUser=%p\n", pUser);
				PrintLog(szLog);
				if(pUser == NULL)
				{
					CUser *pNewUser = new CUser();
					pNewUser->SetName(szName, nNameLen);
					pNewUser->SetPwd(szPwd, nPwdLen);
					pNewUser->SetIp(nIp);
					pNewUser->SetPort(nPort);
					pNewUser->SetAddr(&saFrom);
					m_pUserMgr->AddUser(pNewUser);
					szParam[1] = 0x00; //user register success
				}
				else
				{
					szParam[1] = 0x03; //user register failed, name is already exist
				}
			}
			else
			{
				szParam[1] = 0x02;  //user register failed, name is invalid
			}
		}
		else
		{
			szParam[1] = 0x04;  //user register failed, password is too long
		}
	}
	else
	{
		szParam[1] = 0x01;  //user register failed, name is too long
	}
	//addr(2B)|size(2B)|code(1B)|offset(2B)|resultlen(1B)|result(1B)
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 2, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrRegister() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::UsrUnregister(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|pwdlen(1B)|pwd(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[2] = {0x01, 0x01};
	char szLog[BUF_SIZE] = {0};
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nPwdLen = pVal[1+nNameLen];
		if(nPwdLen < PWD_SIZE)
		{
			char szPwd[PWD_SIZE] = {0};
			memcpy(szPwd, pVal+1+nNameLen+1, nPwdLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrUnregister() name=%s pwd=%s\n", szName, szPwd);
			PrintLog(szLog);
			int nValid = m_pUserMgr->IsNameValid(szName);
			if(nValid == 0)
			{
				CUser *pUser = m_pUserMgr->GetUser(szName);
				sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrUnregister() pUser=%p\n", pUser);
				PrintLog(szLog);
				if((pUser != NULL) && (strcmp(szPwd, pUser->GetPwd()) == 0))
				{
					m_pDevMgr->DelDevByOwner(szName);
					m_pUserMgr->DelUser(szName);
					szParam[1] = 0x00; //user unregister success
				}
			}
		}
	}
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 2, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrUnregister() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::UsrLogon(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|pwdlen(1B)|pwd(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[2] = {0x01, 0x01};
	char szLog[BUF_SIZE] = {0};
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nPwdLen = pVal[1+nNameLen];
		if(nPwdLen < PWD_SIZE)
		{
			char szPwd[PWD_SIZE] = {0};
			memcpy(szPwd, pVal+1+nNameLen+1, nPwdLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogon() name=%s pwd=%s\n", szName, szPwd);
			PrintLog(szLog);

			CUser *pUser = m_pUserMgr->GetUser(szName);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogon() pUser=%p\n", pUser);
			PrintLog(szLog);
			if(pUser != NULL && (strcmp(szPwd, pUser->GetPwd()) == 0))
			{
				pUser->SetStatus(STATUS_ONLINE);
				pUser->SetLastTime(time(NULL));
				pUser->SetIp(nIp);
				pUser->SetPort(nPort);
				pUser->SetAddr(&saFrom);
				szParam[1] = 0x00; //user logon success
			}
		}
	}
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 2, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogon() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::UsrLogoff(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|pwdlen(1B)|pwd(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[2] = {0x01, 0x01};
	char szLog[BUF_SIZE] = {0};
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nPwdLen = pVal[1+nNameLen];
		if(nPwdLen < PWD_SIZE)
		{
			char szPwd[PWD_SIZE] = {0};
			memcpy(szPwd, pVal+1+nNameLen+1, nPwdLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogoff() name=%s pwd=%s\n", szName, szPwd);
			PrintLog(szLog);

			CUser *pUser = m_pUserMgr->GetUser(szName);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogoff() pUser=%p\n", pUser);
			PrintLog(szLog);
			if(pUser != NULL && (strcmp(szPwd, pUser->GetPwd()) == 0))
			{
				//pUser->SetLastTime(time(NULL));
				//pUser->SetIp(nIp);
				//pUser->SetPort(nPort);
				//pUser->SetAddr(&saFrom);
				pUser->SetStatus(STATUS_OFFLINE);
				szParam[1] = 0x00; //user logon success
			}
		}
	}
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 2, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrLogoff() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::UsrDevRegister(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|maclen(1B)|mac(12B)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[15] = {0};
	char szLog[BUF_SIZE] = {0};
	memset(szParam, 0, 15);
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nMacLen = pVal[1+nNameLen];
		if(nMacLen < MAC_SIZE)
		{
			char szMac[MAC_SIZE] = {0};
			memcpy(szMac, pVal+1+nNameLen+1, nMacLen);
			szParam[0] = nMacLen;
			memcpy(szParam+1, szMac, nMacLen);
			szParam[1+nMacLen] = 0x01;
			//char *sMac = CUtil::Data2String(szMac, nMacLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevRegister() name=%s mac=%s\n", szName, szMac);
			PrintLog(szLog);
			CUser *pUser = m_pUserMgr->GetUser(szName);
			CDevice *pDev = m_pDevMgr->GetDev(szMac);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevRegister() pUser=%p pDev=%p\n", pUser, pDev);
			PrintLog(szLog);
			if(pUser != NULL)
			{
				if(pDev == NULL)
				{
					CDevice *pNewDev = new CDevice();
					pNewDev->SetMac(szMac, nMacLen);
					pNewDev->SetOwner(szName, nNameLen);
					m_pDevMgr->AddDev(pNewDev);
					szParam[1+nMacLen+1] = 0x00; //user device register success
				}
				else
				{
					szParam[1+nMacLen+1] = 0x01; //user device already register
				}
			}
			else
			{
				szParam[1+nMacLen+1] = 0x02; //user not exist
			}
			//delete sMac;
		}
	}
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|maclen(1B)|mac(12B)|resultlen(1B)|result(1B)|crc(2B)
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 15, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevRegister() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::UsrDevUnregister(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|maclen(1B)|mac(12B)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[15] = {0};
	memset(szParam, 0, 15);
	char szLog[BUF_SIZE] = {0};
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		unsigned char nMacLen = pVal[1+nNameLen];
		if(nMacLen < MAC_SIZE)
		{
			char szMac[MAC_SIZE] = {0};
			memset(szMac, 0, MAC_SIZE);
			memcpy(szMac, pVal+1+nNameLen+1, nMacLen);
			szParam[0] = nMacLen;
			memcpy(szParam+1, szMac, nMacLen);
			szParam[1+nMacLen] = 0x01;
			szParam[1+nMacLen+1] = 0x01;
			//char *sMac = CUtil::Data2String(szMac, nMacLen);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevUnregister() name=%s sMac=%s\n", szName, szMac);
			PrintLog(szLog);
			CUser *pUser = m_pUserMgr->GetUser(szName);
			CDevice *pDev = m_pDevMgr->GetDev(szMac);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevUnregister() pUser=%p pDev=%p\n", pUser, pDev);
			PrintLog(szLog);
			if((pUser != NULL) && (pDev != NULL))
			{
				m_pDevMgr->DelDev(szMac);
				szParam[1+nMacLen+1] = 0x00; //user device unregister success
			}
			//delete sMac;
		}
	}
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|maclen(1B)|mac(12B)|resultlen(1B)|result(1B)|crc(2B)
	CNode *pUsrNode = new CNode();
	CPeer *pUsrPeer = new CPeer();
	pUsrNode->m_pObject = (CBase *)pUsrPeer;
	char szCmd[CMD_SIZE] = {0};
	int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 15, szCmd, CMD_SIZE);
	char *pStr = CUtil::Data2String(szCmd, nSize);
	sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevUnregister() return nSize=%d cmd=%s\n", nSize, pStr);
	delete pStr;
	PrintLog(szLog);
	pUsrPeer->SetData(szCmd, nSize);
	pUsrPeer->SetIp(nIp);
	pUsrPeer->SetPort(nPort);
	pUsrPeer->SetAddr(&saFrom);
	m_pSendQueue->Push(pUsrNode);
	return 0;
}

int CProtocol::DevConnect(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|maclen(1B)|mac(6B)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[CMD_SIZE] = {0};
	char szLog[BUF_SIZE] = {0};
	char szCmd[CMD_SIZE] = {0};
	unsigned char nMacLen = pVal[0];
	if(nMacLen < MAC_SIZE)
	{
		char szMac[MAC_SIZE] = {0};
		memset(szMac, 0, MAC_SIZE);
		memcpy(szMac, pVal+1, nMacLen);
		char *sMac = CUtil::Data2String(szMac, nMacLen);
		CDevice *pDev = m_pDevMgr->GetDev(sMac);
		sprintf_s(szLog, BUF_SIZE, "CProtocol::DevConnect() sMac=%s pDev=%p\n", sMac, pDev);
		PrintLog(szLog);
		delete sMac;
		if(pDev != NULL)
		{
			pDev->SetLastTime(time(NULL));
			pDev->SetIp(nIp);
			pDev->SetPort(nPort);
			pDev->SetAddr(&saFrom);

			char *pName = pDev->GetOwner();
			CUser *pUser = m_pUserMgr->GetUser(pName);

			//send data to device
			//szParam = usriplen|usrip|usrportlen|usrport
			memset(szParam, 0, CMD_SIZE);
			int nUsrIpLen = 4;
			int nUsrPortLen = 4;
			int nParamCnt = 1+nUsrIpLen+1+nUsrPortLen;
			int nUsrIp = 0;
			int nUsrPort = 0;
			if(pUser != NULL)
			{
				nUsrIp = pUser->GetIp();
				nUsrPort = pUser->GetPort();
			}

			szParam[0] = nUsrIpLen;
			unsigned char szUsrIp[4] = {0};
			CUtil::Int2Bytes(nUsrIp, szUsrIp);
			memcpy(szParam+1, szUsrIp, nUsrIpLen);

			szParam[1+nUsrIpLen] = nUsrPortLen;
			unsigned char szUsrPort[4] = {0};
			CUtil::Int2Bytes(nUsrPort, szUsrPort);
			memcpy(szParam+1+nUsrIpLen+1, szUsrPort, nUsrPortLen);

			//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|usriplen(1B)|usrip(4B)|usrportlen(1B)|usrport(4B)|crc(2B)
			memset(szCmd, 0, CMD_SIZE);
			int nSize = DataGenerate(usAddr, ucCode, DEV_CON_USR, szParam, nParamCnt, szCmd, CMD_SIZE);
			char *pStr = CUtil::Data2String(szCmd, nSize);
			sprintf_s(szLog, BUF_SIZE, "CProtocol::DevConnect() sendto device nSize=%d cmd=%s\n", nSize, pStr);
			delete pStr;
			PrintLog(szLog);
			CNode *pDevNode = new CNode();
			CPeer *pDevPeer = new CPeer();
			pDevNode->m_pObject = (CBase *)pDevPeer;
			pDevPeer->SetData(szCmd, nSize);
			pDevPeer->SetIp(nIp);
			pDevPeer->SetPort(nPort);
			pDevPeer->SetAddr(&saFrom);
			m_pSendQueue->Push(pDevNode);

			if((pUser != NULL) && (pUser->GetStatus() == STATUS_ONLINE))
			{
				//send data to user
				//szParam = maclen|mac|deviplen|devip|devportlen|devport
				memset(szParam, 0, CMD_SIZE);
				char *pMac = pDev->GetMac();
				nMacLen = strlen(pMac);
				szParam[0] = nMacLen;
				memcpy(szParam+1, pMac, nMacLen);

				int nDevIp = pDev->GetIp();
				char *pIp = CUtil::Int2Ip(nDevIp);
				int nDevIpLen = strlen(pIp);
				szParam[1+nMacLen] = nDevIpLen;
				memcpy(szParam+1+nMacLen+1, pIp, nDevIpLen);
				delete pIp;

				int nDevPortLen = 4;
				unsigned char szDevPort[4] = {0};
				int nDevPort = pDev->GetPort();
				szParam[1+nMacLen+1+nDevIpLen] = nDevPortLen;
				CUtil::Int2Bytes(nDevPort, szDevPort);
				memcpy(szParam+1+nMacLen+1+nDevIpLen+1, szDevPort, nDevPortLen);

				//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|maclen(1B)|mac(12B)|deviplen(1B)|devip(xB)|devportlen(1B)|devport(4B)|crc(2B)
				memset(szCmd, 0, CMD_SIZE);
				int nParamCnt = 1+nMacLen+1+nDevIpLen+1+nDevPortLen;
				int nSize = DataGenerate(usAddr, ucCode, USR_CON_DEV, szParam, nParamCnt, szCmd, CMD_SIZE);
				char *pStr = CUtil::Data2String(szCmd, nSize);
				sprintf_s(szLog, BUF_SIZE, "CProtocol::DevConnect() sendto user nSize=%d cmd=%s\n", nSize, pStr);
				delete pStr;
				PrintLog(szLog);
				CNode *pUsrNode = new CNode();
				CPeer *pUsrPeer = new CPeer();
				pUsrNode->m_pObject = (CBase *)pUsrPeer;
				pUsrPeer->SetData(szCmd, nSize);
				pUsrPeer->SetIp(pUser->GetIp());
				pUsrPeer->SetPort(pUser->GetPort());
				pUser->GetAddr(&saFrom);
				pUsrPeer->SetAddr(&saFrom);
				m_pSendQueue->Push(pUsrNode);
			}
		}
	}
	return 0;
}

int CProtocol::DevDisconnect(CPeer *pPeer)
{
	return 0;
}

int CProtocol::UsrDevList(CPeer *pPeer)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|namelen(1B)|name(xB)|crc(2B)
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	struct sockaddr_in saFrom;
	pPeer->GetAddr(&saFrom);
	unsigned short usAddr = 0;
	((char *)&usAddr)[0] = pBuf[1];
	((char *)&usAddr)[1] = pBuf[0];
	unsigned short usSize = 0;
	((char *)&usSize)[0] = pBuf[3];
	((char *)&usSize)[1] = pBuf[2];
	unsigned char ucCode = pBuf[4];
	unsigned short usOffset = 0;
	((char *)&usOffset)[0] = pBuf[6];
	((char *)&usOffset)[1] = pBuf[5];
	unsigned char ucVallen = pBuf[7];
	char *pVal = pBuf + 8;
	char szParam[15] = {0};
	char szLog[BUF_SIZE] = {0};
	memset(szParam, 0, 15);
	unsigned char nNameLen = pVal[0];
	if(nNameLen < NAME_SIZE)
	{
		char szName[NAME_SIZE] = {0};
		memcpy(szName, pVal+1, nNameLen);
		CUser *pUser = m_pUserMgr->GetUser(szName);
		sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevList() name=%s pUser=%p\n", szName, pUser);
		PrintLog(szLog);
		if(pUser != NULL)
		{
			CDevice *pUserDevs[USR_MAX_DEV] = {0};
			int nCnt = m_pDevMgr->GetDevsByOwner(szName, pUserDevs);
			for(int i=0; i<nCnt; i++)
			{
				//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|maclen(1B)|mac(12B)|resultlen(1B)|result(1B)|crc(2B)
				memset(szParam, 0, 15);
				CDevice *d = pUserDevs[i];
				char *sMac = d->GetMac();
				int nMacLen = strlen(sMac);
				szParam[0] = nMacLen;
				memcpy(szParam+1, sMac, nMacLen);
				szParam[1+nMacLen] = 0x01;
				szParam[1+nMacLen+1] = 0x00;
				CNode *pUsrNode = new CNode();
				CPeer *pUsrPeer = new CPeer();
				pUsrNode->m_pObject = (CBase *)pUsrPeer;
				char szCmd[CMD_SIZE] = {0};
				int nSize = DataGenerate(usAddr, ucCode, usOffset, szParam, 15, szCmd, CMD_SIZE);
				char *pStr = CUtil::Data2String(szCmd, nSize);
				sprintf_s(szLog, BUF_SIZE, "CProtocol::UsrDevList() return nSize=%d cmd=%s\n", nSize, pStr);
				delete pStr;
				PrintLog(szLog);
				pUsrPeer->SetData(szCmd, nSize);
				pUsrPeer->SetIp(nIp);
				pUsrPeer->SetPort(nPort);
				pUsrPeer->SetAddr(&saFrom);
				m_pSendQueue->Push(pUsrNode);
			}
		}
	}
	return 0;
}

int CProtocol::Echo2Client(CPeer *pPeer)
{
	int nIp = pPeer->GetIp();
	int nPort = pPeer->GetPort();
	char *pBuf = pPeer->GetData();
	int nSize = pPeer->GetSize();
	struct sockaddr_in from;
	pPeer->GetAddr(&from);
	char *sIp = CUtil::Int2Ip(nIp);
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "CProtocol::Echo2Client() sIp=%s port=%d\n", sIp, nPort);
	PrintLog(szLog);
	delete sIp;
	CNode *pNewNode = new CNode();
	CPeer *pNewPeer = new CPeer();
	pNewNode->m_pObject = (CBase *)pNewPeer;
	pNewPeer->SetData(pBuf, nSize);
	pNewPeer->SetIp(nIp);
	pNewPeer->SetPort(nPort);
	pNewPeer->SetAddr(&from);
	m_pSendQueue->Push(pNewNode);
	return 0;
}

int CProtocol::DataGenerate(unsigned short usAddr, char ucCode, unsigned short usOffset, char *pParam, unsigned char ucParlen, char *pTx, int nSize)
{
	//addr(2B)|size(2B)|code(1B)|offset(2B)|vallen(1B)|...|crc(2B)
	short sCnt = (short)(2 + 2 + 1 + 2 + 1 + ucParlen + 2);
	if(sCnt < nSize)
	{
		memset(pTx, 0, nSize);
		unsigned char szAddr[2];
		CUtil::Short2Bytes(usAddr, szAddr);
		pTx[0] = szAddr[0];
		pTx[1] = szAddr[1];
		unsigned char szSize[2];
		CUtil::Short2Bytes(sCnt, szSize);
		pTx[2] = szSize[0];
		pTx[3] = szSize[1];
		pTx[4] = ucCode;
		unsigned char szOffset[2];
		CUtil::Short2Bytes(usOffset, szOffset);
		pTx[5] = szOffset[0];
		pTx[6] = szOffset[1];
		pTx[7] = ucParlen;
		memcpy(pTx+8, pParam, ucParlen);
		short sCrc = CUtil::Crc16(pTx, sCnt-2);
		unsigned char szCrc[2];
		CUtil::Short2Bytes(sCrc, szCrc);
		pTx[8+ucParlen] = szCrc[0];
		pTx[8+ucParlen+1] = szCrc[1];
	}
	else
	{
		sCnt = 0;
	}
	return sCnt;
}