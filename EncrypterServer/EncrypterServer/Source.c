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
	con.dNrThreads = 2;
	con.dNrWorkers = 5;
	hServer = CreateThread(NULL,
		0,
		run,
		&con,
		0,
		NULL);
	WaitForSingleObject(hServer, INFINITE);
	return 0;
}