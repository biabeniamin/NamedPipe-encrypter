#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Connection.h"
#include "PackagesHandler.h"
int main(void)
{
	HANDLE hPipe =initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"));
	initializingServer(hPipe,&packageReceived);
	return 0;
}