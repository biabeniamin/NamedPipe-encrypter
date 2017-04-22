#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "ServerPackagesHandler.h"
DWORD WINAPI run(PVOID parameters)
{
	connectionParamaters *connection = parameters;
	initializingCommunication(connection->dNrThreads,connection->dNrWorkers);
}
int main(void)
{
	HANDLE hServer; 
	logWriteLine(TEXT("dfhdf"));
	connectionParamaters con;
	con.dNrThreads = 1;
	con.dNrWorkers = 100;
	/*_tprintf(TEXT("number of threads for clients:"));
	_tscanf(TEXT("%d"), &con.dNrThreads);
	_tprintf(TEXT("number of workers:"));
	_tscanf(TEXT("%d"), &con.dNrWorkers);*/
	hServer = CreateThread(NULL,
		0,
		run,
		&con,
		0,
		NULL);

	while (1)
	{
		TCHAR com[100];
		_tscanf(TEXT("%ls"), com);
		if (_tcscmp(com, TEXT("list")) == 0)
		{
			printOpenedConnections();
		}
	}

	WaitForSingleObject(hServer, INFINITE);
	return 0;
}