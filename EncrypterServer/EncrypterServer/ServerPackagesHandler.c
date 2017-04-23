#include "ServerPackagesHandler.h"
#include "Connection.h"
#include "Log.h"
#include "Encryptor.h"
#include <tchar.h>
#include <windows.h> 
#include <stdio.h> 
#include<string.h>
workerThreadStruc *firstWorkerThread;
threadStruc *firstThread;
DWORD threadCount = 0;
DWORD dMaxWorkers;
HANDLE workerThreadMutex;
int packageReceived(package *pack, HANDLE responsePipe);
threadStruc *getAvailableThread();
DWORD WINAPI ClientThread(PVOID threadId);
DWORD WINAPI EncryptText(PVOID workerThreadSt)
{
	workerThreadStruc *workerThreadS = workerThreadSt;
	workerThreadS->isRunning = 1;
	encrypt(workerThreadS->text, workerThreadS->key,workerThreadS->keyStartPosition);
	workerThreadS->isRunning = 0;
	workerThreadS->canBeReused = 0;
	workerThreadS->hasFinished = 1;
}
void createThreadForWorker(workerThreadStruc *worker)
{
	worker->isRunning = 0;
	worker->hasFinished = 0;
	worker->canBeReused = 1;
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
	dMaxWorkers = nrWorkers;
	HANDLE hPipe = initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"),INFINITE);
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
	threadS->dThreadId = GetCurrentThreadId();
	HANDLE hPipe = initializingPipeAsServer(threadS->connectionPipeName.serverPipeName, SECONDS_TIMEOUT);
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
void printOpenedConnections()
{
	threadStruc *tSCurrent = firstThread;
	_tprintf(TEXT("Threads list:\n"));
	do
	{
		_tprintf(TEXT("thread with number %d is running:%d has finished:%d \n"), tSCurrent->dThreadId,tSCurrent->isRunning,tSCurrent->hasFinished);
		if(tSCurrent->next!=NULL)
			tSCurrent = tSCurrent->next;
	} while (tSCurrent->next != NULL);
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
		if(firstThread->next!=NULL)
			firstThread = firstThread->next;
		CloseHandle(threadSRe->thread);
		createThreadForConnection(threadSRe);
		threadSRe->next = NULL;
		if (firstThread->next != NULL)
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
			if(firstWorkerThread->next!=NULL)
				firstWorkerThread = firstWorkerThread->next;
			CloseHandle(threadSRe->thread);
			createThreadForWorker(threadSRe);
			threadSRe->next = NULL;
			if (firstWorkerThread->next != NULL)
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
void encryptPackage(PTCHAR text, PTCHAR key)
{
	PTCHAR *textSegments;
	DWORD dNrOfTextSegments;
	workerThreadStruc **workers;
	DWORD dCurrentSegment;
	DWORD dTextLenght = _tcslen(text);
	WaitForSingleObject(workerThreadMutex,
		INFINITE);
	//divide buffer per workers
	dNrOfTextSegments = dTextLenght / LENGHT_PER_WORKER;
	workers = malloc(dNrOfTextSegments * sizeof(workerThreadStruc*));
	textSegments = malloc(dNrOfTextSegments * sizeof(PTCHAR));
	for (int i = 0; i < dTextLenght; i += LENGHT_PER_WORKER)
	{
		dCurrentSegment = i / LENGHT_PER_WORKER;
		workerThreadMutex = OpenMutex(SYNCHRONIZE,
			TRUE,
			TEXT("worker"));
		do
		{
			workers[dCurrentSegment] = getAvailableWorkerThread();
			for (int j = 0; j < dCurrentSegment; j++)
			{
				if (workers[j]->hasFinished && workers[j]->canBeReused == 0)
				{
					for (int k = 0; k < LENGHT_PER_WORKER; k++)
					{
						text[j*LENGHT_PER_WORKER+k] = textSegments[j][k];
					}
					workers[j]->canBeReused = 1;
				}
			}
		} while (workers[dCurrentSegment] == NULL);
		workers[dCurrentSegment]->isRunning = 1;
		workers[dCurrentSegment]->keyStartPosition = i%_tcslen(key);
		ReleaseMutex(workerThreadMutex);
		textSegments[dCurrentSegment] = malloc((LENGHT_PER_WORKER+5) * sizeof(TCHAR));
		textSegments[dCurrentSegment][LENGHT_PER_WORKER] = '\0';
		_tcsncpy(textSegments[dCurrentSegment],text+ dCurrentSegment*LENGHT_PER_WORKER, LENGHT_PER_WORKER);
		workers[dCurrentSegment]->text = textSegments[dCurrentSegment];
		workers[dCurrentSegment]->key = key;
		ResumeThread(workers[dCurrentSegment]->thread);
	}
	for (int i = 0; i < MIN(dMaxWorkers, dTextLenght / LENGHT_PER_WORKER + 1); i++)
	{
		if (workers[i]->hasFinished == 0)
			WaitForSingleObject(workers[i]->thread, INFINITE);
		if (workers[i]->hasFinished && workers[i]->canBeReused == 0)
		{
			for (int j = 0; j < LENGHT_PER_WORKER; j++)
			{
				text[i*LENGHT_PER_WORKER + j] = textSegments[i][j];
			}
		}
		workers[i]->canBeReused = 1;
		CloseHandle(workers[i]->thread);
		createThreadForWorker(workers[i]);
		//free(textSegments[i]);
	}
	for (int i = 0; i < MIN(dMaxWorkers, dTextLenght / LENGHT_PER_WORKER + 1); i++)
	{
		textSegments[i][5] = '\0';
		free(textSegments[i]);
	}
	//free(textSegments);
	//free(workers);
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
		encryptPackage(encValues->buffer, encValues->key);
		logWriteWord(TEXT("encrypted text %ls\n"));
		for (int i = 0; i < _tcslen(encValues->buffer); i++)
		{
			logWriteChar(encValues->buffer[i]);
		}
		_tcscpy(encResponseValues.key, TEXT("key"));
		_tcscpy(encResponseValues.buffer, encValues->buffer);
		encResponseValues.dOrder = encValues->dOrder;
		encResponseValues.fIsLast = encValues->fIsLast;
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