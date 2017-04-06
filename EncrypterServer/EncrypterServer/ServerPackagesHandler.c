#include "ServerPackagesHandler.h"
#include "Connection.h"
#include "Encryptor.h"
#define MAX_CLIENTS 2
#define TIMEOUT 5
HANDLE threads[MAX_CLIENTS];
threadStruc *firstThread;
pipeNameConnection threadValues[MAX_CLIENTS];
DWORD threadCount = 0;
int packageReceived(package *pack, HANDLE responsePipe);
threadStruc *getAvailableThread();
DWORD WINAPI ClientThread(PVOID threadId);
void createThreadForConnection(threadStruc *threadS)
{
	threadS->isRunning = 0;
	threadS->hasFinished = 0;
	threadS->thread= CreateThread(
		NULL,
		0,
		ClientThread,
		threadS,
		CREATE_SUSPENDED,
		NULL);
}
void initializingCommunication()
{
	HANDLE hPipe = initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"));
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeA");
	firstThread = malloc(sizeof(threadStruc));
	threadStruc *currentThread = firstThread;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		createThreadForConnection(currentThread);
		if(i!=MAX_CLIENTS-1)
			currentThread->next = malloc(sizeof(threadStruc));
		else
			currentThread->next = NULL;
		_tcscpy(currentThread->connectionPipeName.serverPipeName, TEXT("\\\\.\\pipe\\PipeThXS"));
		_tcscpy(currentThread->connectionPipeName.clientPipeName, TEXT("\\\\.\\pipe\\PipeThXC"));
		currentThread->connectionPipeName.serverPipeName[15] = i + 97;
		currentThread->connectionPipeName.clientPipeName[15] = i + 97;
		currentThread = currentThread->next;
	}
	initializingServer(hPipe, clientPipeName, &packageReceived);
}
BOOL authenticateClient(authenticationValues *authValues)
{
	if (_tccmp(authValues->username, TEXT("username")) == 0
		&& _tccmp(authValues->password, TEXT("password")) == 0)
		return TRUE;
	return FALSE;
}
DWORD WINAPI ClientThread(PVOID threadSt)
{
	threadStruc *threadS = threadSt;
	threadS->isRunning = 1;
	HANDLE hPipe = initializingPipeAsServer(threadS->connectionPipeName.serverPipeName);
	initializingServer(hPipe, threadS->connectionPipeName.clientPipeName, &packageReceived);
	threadS->isRunning = 0;
	threadS->hasFinished = 1;
}
void clientAuthenticated(package *pack, HANDLE responsePipe)
{
	threadStruc* threadS = getAvailableThread();
	authenticationResponseValues authResponse;
	_tcscpy(authResponse.serverPipename, threadS->connectionPipeName.serverPipeName);
	_tcscpy(authResponse.clientPipename, threadS->connectionPipeName.clientPipeName);
	ResumeThread(threadS->thread);
	Sleep(10);
	package packageToBeSend;
	packageToBeSend.type = authenticationResponse;
	authResponse.isSuccessful = 1;
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
threadStruc *getLastThreadS(threadStruc *threadS)
{
	if (threadS->next != NULL)
		return getLastThreadS(threadS->next);
	return threadS;
}
threadStruc *getAvailableThread()
{
	if (firstThread->hasFinished)
	{
		threadStruc *threadSRe = firstThread;
		firstThread = firstThread->next;
		CloseHandle(threadSRe->thread);
		createThreadForConnection(threadSRe);
		threadSRe->next = NULL;
		getLastThreadS(firstThread)->next = threadSRe;
	}
	threadStruc *threadS = firstThread;
	while (threadS != NULL)
	{
		if (threadS->isRunning == 0 && threadS->hasFinished==0)
			return threadS;
		threadS = threadS->next;
	}
	return NULL;
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
		if (getAvailableThread == NULL)
			initValues.isAccepted = 0;
		if (initValues.isAccepted == 0)
			_tprintf(TEXT("Connection was denied!\n"));
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