#pragma once

#define STATUS_OFFLINE 0
#define STATUS_ONLINE 1
#define STATUS_FRESH 18000

class CPeer : public CBase
{
public:
	CPeer(void);
	~CPeer(void);

	int GetIp();
	int SetIp(int ip);
	int GetPort();
	int SetPort(int port);
	int SetAddr(struct sockaddr_in *pFrom);
	int GetAddr(struct sockaddr_in *pFrom);
	int SetStatus(int nStatus);
	int GetStatus();
	int SetLastTime(time_t tLast);
	time_t GetLastTime();
	int SetFirstTime(time_t tFirst);
	time_t GetFirstTime();
private:
	int m_nIp;
	int m_nPort;
	struct sockaddr_in m_saFrom;
	int m_nStatus;
	time_t m_tFirst;
	time_t m_tLast;
};

