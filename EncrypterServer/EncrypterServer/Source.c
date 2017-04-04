#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Connection.h"
#include "PackagesHandler.h"
int main(void)
{
	HANDLE hPipe =initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"));
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeA");
	initializingServer(hPipe, clientPipeName,&packageReceived);
	return 0;
}