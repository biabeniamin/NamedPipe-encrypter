#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "ServerPackagesHandler.h"
DWORD WINAPI run(PVOID parameters)
{
	connectionParamaters *connection = parameters;
	initializingCommunication(connection->dNrThreads,connection->dNrWorkers,connection->mainThread);
}
void writeToFlagFile(PTCHAR text)
{
	HANDLE hFile = CreateFile(
		TEXT("StopFlag.txt"),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	DWORD bytesReaded;
	WriteFile(
		hFile,
		text,
		(_tcslen(text)+1) * sizeof(TCHAR),
		&bytesReaded,
		NULL);
	CloseHandle(hFile);
}
void closeServer()
{
	writeToFlagFile(TEXT("1"));
}
int main(void)
{
	writeToFlagFile(TEXT("0"));
	HANDLE hServer; 
	logWriteLine(TEXT("dfhdf"));
	connectionParamaters con;
	con.dNrThreads = 3;
	con.dNrWorkers = 100;
	/*_tprintf(TEXT("number of threads for clients:"));
	_tscanf(TEXT("%d"), &con.dNrThreads);
	_tprintf(TEXT("number of workers:"));
	_tscanf(TEXT("%d"), &con.dNrWorkers);*/
	hServer = CreateThread(NULL,
		0,
		run,
		&con,
		CREATE_SUSPENDED,
		NULL);
	con.mainThread = hServer;
	ResumeThread(hServer);
	while (1)
	{
		TCHAR com[100];
		_tscanf(TEXT("%ls"), com);
		if (_tcscmp(com, TEXT("list")) == 0)
		{
			printOpenedConnections();
		}
		if (_tcscmp(com, TEXT("exit")) == 0)
		{
			closeServer();
			break;
		}
	}

	WaitForSingleObject(hServer, INFINITE);
	return 0;
}