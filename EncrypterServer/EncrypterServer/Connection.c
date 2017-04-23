#include "Connection.h"
DWORD getBufferSizeForType(packageType type)
{
	DWORD octetsToBeWritted=0;
	switch (type)
	{
	case initializing:
		octetsToBeWritted =1;
		break;
	case initializingResponse:
		octetsToBeWritted = sizeof(initializingValues);
		break;
	case authentication:
		octetsToBeWritted = sizeof(authenticationValues);
		break;
	case authenticationResponse:
		octetsToBeWritted = sizeof(authenticationResponseValues);
		break;
	case encryption:
		octetsToBeWritted = sizeof(encryptionValues);
		break;
	case encryptionResponse:
		octetsToBeWritted = sizeof(encryptionResponseValues);
		break;
	case closing:
		octetsToBeWritted = sizeof(package)+2*sizeof(TCHAR);
	}
	return octetsToBeWritted;
}
void writePackage(HANDLE hPipe, package *pack)
{
	DWORD dwWritten;
	WriteFile(hPipe,
		&pack->type,
		sizeof(packageType),
		&dwWritten,
		NULL);
	
	WriteFile(hPipe,
		pack->buffer,
		getBufferSizeForType(pack->type),
		&dwWritten,
		NULL);
}
HANDLE initializingPipeAsServer(PTCHAR pipeName,DWORD dTimeout)
{
	HANDLE hPipe;
	hPipe = CreateNamedPipe(pipeName,
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_WAIT,
		1,
		1024 * 16,
		1024 * 16,
		dTimeout,
		NULL);
	DWORD error = GetLastError();
	return hPipe;
}
HANDLE initializingPipeAsClient(PTCHAR pipeName)
{
	HANDLE hPipe;
	hPipe = CreateFile(pipeName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	return hPipe;
}
BOOL readPackage(HANDLE hPipe, package *pack)
{
	DWORD dwRead;
	DWORD dwSecondRead=0;
	BOOL result;
	result = ReadFile(hPipe,
		&pack->type,
		sizeof(packageType),
		&dwRead,
		NULL);
	pack->buffer = malloc(getBufferSizeForType(pack->type));
	result = ReadFile(hPipe,
		pack->buffer,
		getBufferSizeForType(pack->type),
		&dwSecondRead,
		NULL);
	//pack->buffer[dwSecondRead / sizeof(TCHAR)] = TEXT('\0');
	dwRead += dwSecondRead;
	return result;

}
void waitAnswer(HANDLE hPipe, int(*packageReceived)(package*))
{
	package p;
	DWORD dwRead;
	BOOL isServerClosed = FALSE;
			while (readPackage(hPipe, &p) != FALSE && !isServerClosed)
			{
				packageReceived(&p) == 1;
				isServerClosed = TRUE;
				break;
				//free(p.buffer);
			}

	
}
void initializingServer(HANDLE hPipe,PTCHAR tClientPipeName, int(*packageReceived)(package*, HANDLE))
{
	package p;
	DWORD dwRead;
	HANDLE responsePipe;
	BOOL isServerClosed = FALSE;
	
	while (hPipe != INVALID_HANDLE_VALUE && !isServerClosed)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)
		{
			do
			{
				responsePipe = initializingPipeAsClient(tClientPipeName);
			} while (responsePipe ==-1);
			while (readPackage(hPipe, &p) != FALSE && !isServerClosed)
			{
				if (packageReceived(&p, responsePipe) == 1)
				{
					isServerClosed = TRUE;
					break;
				}
				//free(p.buffer);
			}
		}

		DisconnectNamedPipe(hPipe);
		DisconnectNamedPipe(responsePipe);
		
	}
	CloseHandle(hPipe);
}