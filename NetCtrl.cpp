// NetCtrl.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//if test memory leak, you should insert MemLeak.h to cpp file only
//#include "MemLeak.h"

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

	CLog *pLog = new CLog();
	pLog->StartHandle();
	char szLog[BUF_SIZE] = {0};
	sprintf_s(szLog, BUF_SIZE, "Network remote control server begin\n");
	pLog->Output(szLog, strlen(szLog));
	Sleep(1000);
	int i = 0;
	for(i=0; i<10; i++)
	{
		sprintf_s(szLog, BUF_SIZE, "Test log %d\n", i);
		pLog->Output(szLog, strlen(szLog));
	}
	
	CProtocol *pPro = new CProtocol();
	pPro->SetLog(pLog); //must begin at pPro->Init()
	pPro->Init();
	pPro->StartHandle();
	while(1)
	{
		char szBuf[BUF_SIZE] = {0};
		printf("If you want quit, type bye or quit or exit\n");
		scanf_s("%s", szBuf, BUF_SIZE);
		if(strcmp(szBuf, "bye") == 0 || strcmp(szBuf, "quit") == 0 || strcmp(szBuf, "exit") == 0)
		{
			printf("exit\n");
			sprintf_s(szLog, BUF_SIZE, "Network remote control server quit!\n");
			pLog->Output(szLog, strlen(szLog));
			Sleep(100);
			break;
		}
	}
	pPro->StopHandle();
	delete pPro;

	sprintf_s(szLog, BUF_SIZE, "Network remote control server end\n");
	pLog->Output(szLog, strlen(szLog));
	Sleep(1000);
	pLog->StopHandle();
	delete pLog;
	return 0;
}

