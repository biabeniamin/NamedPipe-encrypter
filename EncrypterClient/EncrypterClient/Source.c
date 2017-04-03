#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "ClientPackagesHandler.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
int main(void)
{
	HANDLE h = initializingPipeAsClient(TEXT("\\\\.\\pipe\\Pipe"));
	HANDLE response = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeA"));
	ConnectNamedPipe(response, NULL);
	initializingConnection(h);
	waitAnswer(response, &packageReceived);
	authenticateConnection(h,TEXT("username"), TEXT("password"));
	waitAnswer(response, &packageReceived);
	encryptData(h, TEXT("Ana are mere de vanzare"));
	waitAnswer(response, &packageReceived);
	return (0);
}