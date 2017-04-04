#include "PackagesHandler.h"
#include "Connection.h"
#include "Encryptor.h"
DWORD threadCount = 0;
BOOL authenticateClient(authenticationValues *authValues)
{
	if (_tccmp(authValues->username, TEXT("username")) == 0
		&& _tccmp(authValues->password, TEXT("password")) == 0)
		return TRUE;
	return FALSE;
}
DWORD WINAPI ClientThread(PVOID pipeName)
{
	pipeNameConnection *pipeNameC = pipeName;
	HANDLE hPipe = initializingPipeAsServer(pipeNameC->serverPipeName);
	initializingServer(hPipe, pipeNameC->clientPipeName, &packageReceived);
	free(pipeName);
}
void clientAuthenticated(package *pack, HANDLE responsePipe)
{
	pipeNameConnection *pipeName=malloc(sizeof(pipeNameConnection));
	_tcscpy(pipeName->serverPipeName, TEXT("\\\\.\\pipe\\PipeThXS"));
	_tcscpy(pipeName->clientPipeName, TEXT("\\\\.\\pipe\\PipeThXC"));
	pipeName->serverPipeName[15] = threadCount + 97;
	pipeName->clientPipeName[15] = threadCount + 97;
	threadCount++;
	HANDLE thread = CreateThread(NULL, 0, ClientThread,pipeName, 0, NULL);
	Sleep(10);
	authenticationResponseValues authResponse;
	package packageToBeSend;
	packageToBeSend.type = authenticationResponse;
	authResponse.isSuccessful = 1;
	_tcscpy(authResponse.serverPipename, pipeName->serverPipeName);
	_tcscpy(authResponse.clientPipename, pipeName->clientPipeName);
	packageToBeSend.buffer = &authResponse;
	writePackage(responsePipe, &packageToBeSend);
}
int isServerRunning()
{
	HANDLE hFile = CreateFile(
		TEXT("StopFlag.txt"),
		GENERIC_READ,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	int isStopped = 0;
	DWORD bytesReaded;
	ReadFile(
		hFile,
		&isStopped,
		sizeof(char),
		&bytesReaded,
		NULL);
	isStopped -= 48;
	CloseHandle(hFile);
	if (isStopped == 0)
		return 1;
	return 0;
}
int packageReceived(package *pack,HANDLE responsePipe)
{
	authenticationValues *authenticationVal;
	requestAnswerValues requestAnswerVal;
	initializingValues initValues;
	encryptionValues *encValues;
	encryptionResponseValues encResponseValues;
	package packageToBeSend;
	//HANDLE responsePipe = initializingPipeAsClient(TEXT("\\\\.\\pipe\\PipeA"));
	switch (pack->type)
	{
	case initializing:
		_tprintf(TEXT("%d \n"), pack->type);
		packageToBeSend.type = initializingResponse;
		initValues.isAccepted = isServerRunning();
		packageToBeSend.buffer = &initValues;
		writePackage(responsePipe, &packageToBeSend);
		break;
	case authentication:
		authenticationVal = pack->buffer;
		_tprintf(TEXT("%d %ls %ls \n"), pack->type, authenticationVal->username, authenticationVal->password);
		if (authenticateClient(authenticationVal))
		{
			_tprintf(TEXT("User was loged on!\n"));
			requestAnswerVal.isSuccesful = TRUE;
			clientAuthenticated(pack, responsePipe);
		}
		else
		{
			_tprintf(TEXT("Invalid credentials!\n"));
			packageToBeSend.type = authenticationResponse;
			requestAnswerVal.isSuccesful = FALSE;
			_tcscpy(requestAnswerVal.buffer, TEXT("not OK"));
			packageToBeSend.buffer = &requestAnswerVal;
			writePackage(responsePipe, &packageToBeSend);
			return 1;
		}
		break;
	case encryption:
		packageToBeSend.type = encryptionResponse;
		encValues = pack->buffer;
		_tprintf(TEXT("%d message received to be decrypted %ls\n"), pack->type, encValues->buffer);
		encResponseValues.bufferLenght = _tcslen(encValues->buffer);
		encrypt(encValues->buffer, encResponseValues.key);
		_tprintf(TEXT("encrypted text %ls\n"),encValues->buffer);
		_tcscpy(encResponseValues.key, TEXT("key"));
		_tcscpy(encResponseValues.buffer, encValues->buffer);
		packageToBeSend.buffer = &encResponseValues;
		writePackage(responsePipe, &packageToBeSend);
		break;
	case closing:
		_tprintf(TEXT("Connection closed"));
		return 1;
	}
	free(pack->buffer);
	return 0;
}