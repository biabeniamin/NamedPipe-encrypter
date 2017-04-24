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
HANDLE mainThread;
int packageReceived(package *pack, HANDLE responsePipe);
threadStruc *getAvailableThread();
DWORD WINAPI ClientThread(PVOID threadId);
DWORD WINAPI EncryptText(PVOID workerThreadSt)
{
	//encrypt text
	workerThreadStruc *workerThreadS = workerThreadSt;
	//set thread as running
	workerThreadS->isRunning = 1;
	//encrypt text
	encrypt(workerThreadS->text, workerThreadS->key,workerThreadS->keyStartPosition);
	//set as stopped
	workerThreadS->isRunning = 0;
	//must get date from it
	workerThreadS->canBeReused = 0;
	//set as finished
	workerThreadS->hasFinished = 1;
}
void createThreadForWorker(workerThreadStruc *worker)
{
	//create a thread and initializng it for worker
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
DWORD WINAPI ServerStopper(PVOID par)
{
	//check if stop flag was activated
	while (1)
	{
		//check flag
		if (isServerRunning() == 0)
		{
			//wait threads to finish
			threadStruc *current = firstThread;
			do
			{
				if (current->isRunning == 1)
					WaitForSingleObject(current->thread, 10000);
				if(current->next!=NULL)
					current = current->next;
			} while (current->next != NULL);
			//terminate main thread
			TerminateThread(mainThread, 0);
		}
		//sleep 100mS for performance optimization
		Sleep(100);
	}
}
void createThreadForConnection(threadStruc *threadS)
{
	//create a thread for connection
	threadS->isRunning = 0;
	threadS->hasFinished = 0;
	threadS->thread = CreateThread(
		NULL,
		0,
		ClientThread,
		threadS,
		CREATE_SUSPENDED,
		NULL);
}
void createThreadForStopFlag(HANDLE *hThread)
{
	//create a thread for stop flag
	*hThread = CreateThread(
		NULL,
		0,
		ServerStopper,
		NULL,
		0,
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
void initializingCommunication(DWORD nrClients, DWORD nrWorkers,HANDLE hMainThread)
{
	mainThread = hMainThread;
	HANDLE hStopFlagThread;
	//create and run thread for stop flag
	createThreadForStopFlag(&hStopFlagThread);
	//load users
	loadUsers();
	dMaxWorkers = nrWorkers;
	//alocate pipe where server receives
	HANDLE hPipe = initializingPipeAsServer(TEXT("\\\\.\\pipe\\Pipe"),INFINITE);
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeA");
	//alloc memory for threads
	firstThread = malloc(sizeof(threadStruc));
	//alloc memory for worker
	firstWorkerThread = malloc(sizeof(workerThreadStruc));
	threadStruc *currentThread = firstThread;
	workerThreadStruc *currentWorker = firstWorkerThread;
	//create mutex to be use later
	workerThreadMutex = CreateMutex(NULL,
		FALSE,
		NULL);
	for (int i = 0; i < nrClients; i++)
	{
		//initializing threads to be ready to go next time
		createThreadForConnection(currentThread);
		if (i != nrClients - 1)
		{
			currentThread->next = malloc(sizeof(threadStruc));
		}
		else
		{
			currentThread->next = NULL;
		}
		//copy pipe names for it
		_tcscpy(currentThread->connectionPipeName.serverPipeName, TEXT("\\\\.\\pipe\\PipeThXS"));
		_tcscpy(currentThread->connectionPipeName.clientPipeName, TEXT("\\\\.\\pipe\\PipeThXC"));
		currentThread->connectionPipeName.serverPipeName[15] = i + 97;
		currentThread->connectionPipeName.clientPipeName[15] = i + 97;
		//set next thread
		currentThread = currentThread->next;
	}
	for (int i = 0; i < nrWorkers; i++)
	{
		//initializng worker
		createThreadForWorker(currentWorker);
		if (i != nrWorkers - 1)
		{
			//allocate it
			currentWorker->next = malloc(sizeof(workerThreadStruc));
		}
		else
		{
			currentWorker->next = NULL;
		}
		//set worker's parameters
		currentWorker->canBeReused = 1;
		currentWorker = currentWorker->next;
	}
	//initialize server and waits connections
	initializingServer(hPipe, clientPipeName, &packageReceived);
	//terminate thread which checks stop flag
	TerminateThread(hStopFlagThread, 0);
}
BOOL authenticateClient(authenticationValues *authValues)
{
	//authenticate client
	if (_tccmp(authValues->username, TEXT("username")) == 0
		&& _tccmp(authValues->password, TEXT("password")) == 0)
		return TRUE;
	return FALSE;
}
DWORD WINAPI ClientThread(PVOID threadSt)
{
	//after client was logged in
	//set thread
	threadStruc *threadS = threadSt;
	threadS->isRunning = 1;
	threadS->dThreadId = GetCurrentThreadId();
	//initializng new pipe for server
	HANDLE hPipe = initializingPipeAsServer(threadS->connectionPipeName.serverPipeName, SECONDS_TIMEOUT);
	//initializng a new server instance and wait for commands
	initializingServer(hPipe, threadS->connectionPipeName.clientPipeName, &packageReceived);
	//set thread as finished
	threadS->isRunning = 0;
	threadS->hasFinished = 1;
}
void clientAuthenticated(package *pack, HANDLE responsePipe)
{
	//get a available thread from queque
	threadStruc* threadS = getAvailableThread();
	if (threadS == NULL)
		return;
	authenticationResponseValues authResponse;
	//copy pipenames to thread
	_tcscpy(authResponse.serverPipename, threadS->connectionPipeName.serverPipeName);
	_tcscpy(authResponse.clientPipename, threadS->connectionPipeName.clientPipeName);
	//start thread
	ResumeThread(threadS->thread);
	Sleep(10);
	package packageToBeSend;
	//responde to client with new pipe names
	packageToBeSend.type = authenticationResponse;
	//set response as successful
	authResponse.isSuccessful = 1;
	packageToBeSend.buffer = &authResponse;
	//responde to client
	writePackage(responsePipe, &packageToBeSend);
}
void printOpenedConnections()
{
	//walk through all threads
	threadStruc *tSCurrent = firstThread;
	_tprintf(TEXT("Threads list:\n"));
	do
	{
		//print current thread information
		_tprintf(TEXT("thread with number %d is running:%d has finished:%d \n"), tSCurrent->dThreadId,tSCurrent->isRunning,tSCurrent->hasFinished);
		//get next thread
		if(tSCurrent->next!=NULL)
			tSCurrent = tSCurrent->next;
	} while (tSCurrent->next != NULL);
}
int isServerRunning()
{
	//check if stop flag is activate
	//open file
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
	//read from file
	ReadFile(
		hFile,
		&isStopped,
		sizeof(char),
		&bytesReaded,
		NULL);
	//convert from ascii to integer
	isStopped -= 48;
	//close file
	CloseHandle(hFile);
	if (isStopped == 0)
		return 1;
	return 0;
}
threadStruc *getLastThreadS(threadStruc *threadS)
{
	//get last thread
	if (threadS->next != NULL)
		return getLastThreadS(threadS->next);
	return threadS;
}
workerThreadStruc *getLastWorker(workerThreadStruc *worker)
{
	//get last worker
	if (worker->next != NULL)
		return getLastWorker(worker->next);
	return worker;
}
threadStruc *getAvailableThread()
{
	//get an available thread
	//if there is a thread which can be reused,reinitializing it
	if (firstThread->hasFinished)
	{
		//get first in queque
		threadStruc *threadSRe = firstThread;
		//set the top of queque the next one
		if(firstThread->next!=NULL)
			firstThread = firstThread->next;
		//close last thread,it is stopped
		CloseHandle(threadSRe->thread);
		//create new thread
		createThreadForConnection(threadSRe);
		threadSRe->next = NULL;
		//add it back in queque on last position
		if (firstThread->next != NULL)
			getLastThreadS(firstThread)->next = threadSRe;
	}
	//walk through all threads until it finds one available
	threadStruc *threadS = firstThread;
	while (threadS != NULL)
	{
		//check if it is running
		if (threadS->isRunning == 0 && threadS->hasFinished == 0)
			return threadS;
		threadS = threadS->next;
	}
	return NULL;
}
workerThreadStruc *getAvailableWorkerThread()
{
	//get an available worker
	__try
	{
		//reused threads which was used
		if (firstWorkerThread->hasFinished && firstWorkerThread->canBeReused==1)
		{
			//make the first worker ,second worker
			workerThreadStruc *threadSRe = firstWorkerThread;
			if(firstWorkerThread->next!=NULL)
				firstWorkerThread = firstWorkerThread->next;
			CloseHandle(threadSRe->thread);
			//create new thread for worker
			createThreadForWorker(threadSRe);
			threadSRe->next = NULL;
			//add it back in queque in last position
			if (firstWorkerThread->next != NULL)
				getLastWorker(firstWorkerThread)->next = threadSRe;
		}
		//walk through all workers to find one available
		workerThreadStruc *threadS = firstWorkerThread;
		while (threadS != NULL)
		{
			//check if it is available
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
	DWORD workersCount=0;
	//wait for mutext
	WaitForSingleObject(workerThreadMutex,
		INFINITE);
	//divide buffer per workers
	dNrOfTextSegments = dTextLenght / LENGHT_PER_WORKER+1;
	//alloc workers
	workers = malloc(dNrOfTextSegments * sizeof(workerThreadStruc*));
	//alloc text segments for every worker
	textSegments = malloc(dNrOfTextSegments * sizeof(PTCHAR));
	for (int i = 0; i < dTextLenght; i += LENGHT_PER_WORKER)
	{
		//calculate current segmnet
		dCurrentSegment = i / LENGHT_PER_WORKER;
		//open mutex
		workerThreadMutex = OpenMutex(SYNCHRONIZE,
			TRUE,
			TEXT("worker"));
		do
		{
			//increment workers
			workersCount++;
			//get an available worker
			workers[dCurrentSegment] = getAvailableWorkerThread();
			//walk through all workers
			for (int j = 0; j < dCurrentSegment; j++)
			{
				//if one has finished and can get date out from it
				if (workers[j]->hasFinished && workers[j]->canBeReused == 0)
				{
					for (int k = 0; k < LENGHT_PER_WORKER; k++)
					{
						//replace in original text with encrypted one
						text[j*LENGHT_PER_WORKER+k] = textSegments[j][k];
					}
					workers[j]->canBeReused = 1;
				}
			}
			//repet if no worker was founded
		} while (workers[dCurrentSegment] == NULL);
		//set worker as running
		workers[dCurrentSegment]->isRunning = 1;
		//set position on key,because text is divided in pieces and key must be sync
		workers[dCurrentSegment]->keyStartPosition = i%_tcslen(key);
		//release mutex to be used by other thread
		ReleaseMutex(workerThreadMutex);
		//alloc space for text segment
		textSegments[dCurrentSegment] = malloc((LENGHT_PER_WORKER*5) * sizeof(TCHAR));
		//mark end of it
		textSegments[dCurrentSegment][LENGHT_PER_WORKER] = '\0';
		//copy original text in text segment
		_tcsncpy(textSegments[dCurrentSegment],text+ dCurrentSegment*LENGHT_PER_WORKER, LENGHT_PER_WORKER);
		//set worker text
		workers[dCurrentSegment]->text = textSegments[dCurrentSegment];
		//set worker key
		workers[dCurrentSegment]->key = key;
		//start worker
		ResumeThread(workers[dCurrentSegment]->thread);
	}
	//itterate all workers to find one that finished,otherwise wait
	for (int i = 0; i < workersCount; i++)
	{
		//check if it finished and wait if needed
		if (workers[i]->isRunning == 1 && workers[i]->hasFinished == 0)
			WaitForSingleObject(workers[i]->thread, INFINITE);
		//check if it finished and the content wasn't extracted
		if (workers[i]->hasFinished && workers[i]->canBeReused == 0)
		{
			//copy encrypted content in original 
			for (int j = 0; j < LENGHT_PER_WORKER; j++)
			{
				text[i*LENGHT_PER_WORKER + j] = textSegments[i][j];
			}
		}
		//set worker as available
		workers[i]->canBeReused = 1;
		//close handle
		CloseHandle(workers[i]->thread);
		//create new thread for this worker
		createThreadForWorker(workers[i]);
		//deallocate memory 
		free(textSegments[i]);

	}
	//deallocate memory
	free(textSegments);
	free(workers);
}
int packageReceived(package *pack,HANDLE responsePipe)
{
	//new request from client
	authenticationValues *authenticationVal;
	requestAnswerValues requestAnswerVal;
	initializingValues initValues;
	encryptionValues *encValues;
	encryptionResponseValues encResponseValues;
	package packageToBeSend;
	
	//HANDLE responsePipe = initializingPipeAsClient(TEXT("\\\\.\\pipe\\PipeA"));
	//case request type
	switch (pack->type)
	{
	case initializing:
		//initializng request,server accept or decline connection
		logWriteLine(TEXT("%d \n"), pack->type);
		//response with initializng response
		packageToBeSend.type = initializingResponse;
		//accept it if server is running
		initValues.isAccepted = isServerRunning();
		//if it is an available thread,accept it
		if (getAvailableThread == NULL)
			initValues.isAccepted = 0;
		if (initValues.isAccepted == 0)
			logWriteLine(TEXT("Connection was denied!"));
		packageToBeSend.buffer = &initValues;
		//response to client
		writePackage(responsePipe, &packageToBeSend);
		break;
	case authentication:
		//authentication reuqest
		authenticationVal = pack->buffer;
		logNewLine();
		logWriteNumber(pack->type);
		logWriteWord(TEXT(" "));
		logWriteWord(authenticationVal->username);
		logWriteWord(TEXT(" "));
		logWriteLine(authenticationVal->password);
		//check credentials
		if (authenticateClient(authenticationVal))
		{
			logWriteLine(TEXT("User was loged on!\n"));
			requestAnswerVal.isSuccesful = TRUE;
			//authentication was successful,responde to client
			clientAuthenticated(pack, responsePipe);
		}
		else
		{
			//authentication failed
			logWriteLine(TEXT("Invalid credentials!\n"));
			packageToBeSend.type = authenticationResponse;
			requestAnswerVal.isSuccesful = FALSE;
			_tcscpy(requestAnswerVal.buffer, TEXT("not OK"));
			packageToBeSend.buffer = &requestAnswerVal;
			//response to client
			writePackage(responsePipe, &packageToBeSend);
			return 1;
		}
		break;
	case encryption:
		//encryption request,client send data to decrypt
		packageToBeSend.type = encryptionResponse;
		encValues = pack->buffer;
		logNewLine();
		logWriteNumber(pack->type);
		logWriteWord(TEXT(" message received to be decrypted "));
		logWriteLine(encValues->buffer);
		encResponseValues.bufferLenght = _tcslen(encValues->buffer);
		//encrypt data
		encryptPackage(encValues->buffer, encValues->key);
		logWriteWord(TEXT("encrypted text %ls\n"));
		for (int i = 0; i < _tcslen(encValues->buffer); i++)
		{
			logWriteChar(encValues->buffer[i]);
		}
		//set content of response
		_tcscpy(encResponseValues.key, TEXT("key"));
		_tcscpy(encResponseValues.buffer, encValues->buffer);
		encResponseValues.dOrder = encValues->dOrder;
		encResponseValues.fIsLast = encValues->fIsLast;
		packageToBeSend.buffer = &encResponseValues;
		//responde to client
		writePackage(responsePipe, &packageToBeSend);
		break;
	case closing:
		//closing request
		logNewLine();
		logWriteLine(TEXT("Connection closed"));
		return 1;
	}
	free(pack->buffer);
	return 0;
}