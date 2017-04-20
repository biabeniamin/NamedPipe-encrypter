#include "ServerPackagesHandler.h"
#include "Connection.h"
#include "Log.h"
#include "Encryptor.h"
#include <tchar.h>
#include <windows.h> 
#include <stdio.h> 
#include<string.h>
#define TIMEOUT 5
workerThreadStruc *firstWorkerThread;
threadStruc *firstThread;
DWORD threadCount = 0;
HANDLE workerThreadMutex;
int packageReceived(package *pack, HANDLE responsePipe);
threadStruc *getAvailableThread();
DWORD WINAPI ClientThread(PVOID threadId);
DWORD WINAPI EncryptText(PVOID workerThreadSt)
{
	workerThreadStruc *workerThreadS = workerThreadSt;
	workerThreadS->isRunning = 1;
	encrypt(workerThreadS->text, workerThreadS->key);
	workerThreadS->isRunning = 0;
	workerThreadS->canBeReused = 0;
	workerThreadS->hasFinished = 1;
}
void createThreadForWorker(workerThreadStruc *worker)
{
	worker->isRunning = 0;
	worker->hasFinished = 0;
	worker->thread = CreateThread(NULL,
		0,
		EncryptText,
		worker,
		CREATE_SUSPENDED,
		NULL);
}
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
void loadUsers()
{
	TCHAR user[100];
	TCHAR password[100];
	DWORD bytesReaded;
	//_tcscpy(user,)
	HANDLE usersFile = CreateFile(TEXT("users.txt"),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	ReadFile(usersFile,
		user,
		100,
		&bytesReaded,
		NULL);
}
void initializingCommunication(DWORD nrClients, DWORD nrWorkers)
{
	loadUsers();
	HANDLE hPipe = initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"));
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeA");
	firstThread = malloc(sizeof(threadStruc));
	firstWorkerThread = malloc(sizeof(workerThreadStruc));
	threadStruc *currentThread = firstThread;
	workerThreadStruc *currentWorker = firstWorkerThread;
	workerThreadMutex = CreateMutex(NULL,
		FALSE,
		NULL);
	for (int i = 0; i < nrClients; i++)
	{
		createThreadForConnection(currentThread);
		if (i != nrClients - 1)
		{
			currentThread->next = malloc(sizeof(threadStruc));
		}
		else
		{
			currentThread->next = NULL;
		}
		_tcscpy(currentThread->connectionPipeName.serverPipeName, TEXT("\\\\.\\pipe\\PipeThXS"));
		_tcscpy(currentThread->connectionPipeName.clientPipeName, TEXT("\\\\.\\pipe\\PipeThXC"));
		currentThread->connectionPipeName.serverPipeName[15] = i + 97;
		currentThread->connectionPipeName.clientPipeName[15] = i + 97;
		currentThread = currentThread->next;
	}
	for (int i = 0; i < nrWorkers; i++)
	{
		createThreadForWorker(currentWorker);
		if (i != nrWorkers - 1)
		{
			currentWorker->next = malloc(sizeof(workerThreadStruc));
		}
		else
		{
			currentWorker->next = NULL;
		}
		currentWorker->canBeReused = 1;
		currentWorker = currentWorker->next;
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
workerThreadStruc *getLastWorker(workerThreadStruc *worker)
{
	if (worker->next != NULL)
		return getLastWorker(worker->next);
	return worker;
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
		if (threadS->isRunning == 0 && threadS->hasFinished == 0)
			return threadS;
		threadS = threadS->next;
	}
	return NULL;
}
workerThreadStruc *getAvailableWorkerThread()
{
	__try
	{
		if (firstWorkerThread->hasFinished && firstWorkerThread->canBeReused==1)
		{
			workerThreadStruc *threadSRe = firstWorkerThread;
			firstWorkerThread = firstWorkerThread->next;
			CloseHandle(firstWorkerThread->thread);
			createThreadForWorker(firstWorkerThread);
			threadSRe->next = NULL;
			getLastWorker(firstWorkerThread)->next = threadSRe;
		}
		workerThreadStruc *threadS = firstWorkerThread;
		while (threadS != NULL)
		{
			if (threadS->isRunning == 0 && threadS->hasFinished == 0)
			{
				return threadS;
			}
			threadS = threadS->next;
		}
	}
	__finally
	{
		//
	}
	return NULL;
}
int packageReceived(package *pack,HANDLE responsePipe)
{
	workerThreadStruc **workers;
	authenticationValues *authenticationVal;
	requestAnswerValues requestAnswerVal;
	initializingValues initValues;
	encryptionValues *encValues;
	encryptionResponseValues encResponseValues;
	package packageToBeSend;
	PTCHAR *textSegments;
	DWORD dNrOfTextSegments;
	//HANDLE responsePipe = initializingPipeAsClient(TEXT("\\\\.\\pipe\\PipeA"));
	switch (pack->type)
	{
	case initializing:
		logWriteLine(TEXT("%d \n"), pack->type);
		packageToBeSend.type = initializingResponse;
		initValues.isAccepted = isServerRunning();
		if (getAvailableThread == NULL)
			initValues.isAccepted = 0;
		if (initValues.isAccepted == 0)
			logWriteLine(TEXT("Connection was denied!"));
		packageToBeSend.buffer = &initValues;
		writePackage(responsePipe, &packageToBeSend);
		break;
	case authentication:
		authenticationVal = pack->buffer;
		logNewLine();
		logWriteNumber(pack->type);
		logWriteWord(TEXT(" "));
		logWriteWord(authenticationVal->username);
		logWriteWord(TEXT(" "));
		logWriteLine(authenticationVal->password);
		if (authenticateClient(authenticationVal))
		{
			logWriteLine(TEXT("User was loged on!\n"));
			requestAnswerVal.isSuccesful = TRUE;
			clientAuthenticated(pack, responsePipe);
		}
		else
		{
			logWriteLine(TEXT("Invalid credentials!\n"));
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
		logNewLine();
		logWriteNumber(pack->type);
		logWriteWord(TEXT(" message received to be decrypted "));
		logWriteLine(encValues->buffer);
		encResponseValues.bufferLenght = _tcslen(encValues->buffer);
		WaitForSingleObject(workerThreadMutex,
			INFINITE);
		//divide buffer per workers
		dNrOfTextSegments = _tcslen(encValues->buffer) / LENGHT_PER_WORKER;
		workers = malloc(dNrOfTextSegments *sizeof(workerThreadStruc*));
		textSegments = malloc(dNrOfTextSegments * sizeof(PTCHAR));
		for (int i = 0; i < _tcslen(encValues->buffer); i+=LENGHT_PER_WORKER)
		{
			workerThreadMutex=OpenMutex(SYNCHRONIZE,
				TRUE,
				TEXT("worker"));
			do
			{
				workers[i] = getAvailableWorkerThread();
			} while (workers[i] == NULL);
			workers[i]->isRunning = 1;
			ReleaseMutex(workerThreadMutex);
			textSegments[i] = malloc(LENGHT_PER_WORKER * sizeof(TCHAR));
			_tcsncpy(textSegments[i] , encValues->buffer+i, LENGHT_PER_WORKER);
			workers[i]->text = textSegments[i];
			workers[i]->key = encValues->key;
			ResumeThread(workers[i]->thread);
		}
		WaitForSingleObject(workers[0]->thread,INFINITE);
		for (int i = 0; i < _tcslen(encValues->buffer); i += LENGHT_PER_WORKER)
		{
			WaitForSingleObject(workers[i]->thread, INFINITE);
			for (int j = 0; j < LENGHT_PER_WORKER; j++)
			{
				encValues->buffer[i + j] = textSegments[i][j];
			}
			workers[i]->canBeReused = 1;
		}
		logWriteWord(TEXT("encrypted text %ls\n"));
		/*for (int i = 0; i < _tcslen(encValues->buffer); i++)
		{
			logWriteChar(encValues->buffer[i]);
		}*/
		_tcscpy(encResponseValues.key, TEXT("key"));
		_tcscpy(encResponseValues.buffer, encValues->buffer);
		packageToBeSend.buffer = &encResponseValues;
		writePackage(responsePipe, &packageToBeSend);
		break;
	case closing:
		logNewLine();
		logWriteLine(TEXT("Connection closed"));
		return 1;
	}
	free(pack->buffer);
	return 0;
}