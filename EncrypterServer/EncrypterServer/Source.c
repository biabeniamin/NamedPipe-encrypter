#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "ServerPackagesHandler.h"
DWORD WINAPI run(PVOID parameters)
{
	connectionParamaters *connection = parameters;
	//initializing server
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
	//set stop flag to 1
	writeToFlagFile(TEXT("0"));
	HANDLE hServer; 
	logWriteLine(TEXT("dfhdf"));
	connectionParamaters con;
	con.dNrThreads = 3;
	con.dNrWorkers = 1000;
	/*_tprintf(TEXT("number of threads for clients:"));
	_tscanf(TEXT("%d"), &con.dNrThreads);
	_tprintf(TEXT("number of workers:"));
	_tscanf(TEXT("%d"), &con.dNrWorkers);*/
	//create thread for server
	hServer = CreateThread(NULL,
		0,
		run,
		&con,
		CREATE_SUSPENDED,
		NULL);
	con.mainThread = hServer;
	//start server thread
	ResumeThread(hServer);
	while (1)
	{
		//check command line for commands
		TCHAR com[100];
		_tscanf(TEXT("%ls"), com);
		if (_tcscmp(com, TEXT("list")) == 0)
		{
			//print connections
			printOpenedConnections();
		}
		if (_tcscmp(com, TEXT("exit")) == 0)
		{
			//close server
			closeServer();
			break;
		}
	}
	//wait server to finish
	WaitForSingleObject(hServer, INFINITE);
	return 0;
}