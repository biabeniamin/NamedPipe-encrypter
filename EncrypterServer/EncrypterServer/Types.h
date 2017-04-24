#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include "Log.h"
//dimension of text per worker
#define LENGHT_PER_WORKER 100
//buffer size,the lenght of text which will be sended by client
#define MAX_BUFFER 5000
//timeout time
#define SECONDS_TIMEOUT 5
//minimum between 2 values
#define MIN(a,b) ((a)<(b)?(a):(b))
//type of packages
typedef enum
{
	initializing = 0,
	authentication = 1,
	encryption = 2,
	closing = 3,
	authenticationResponse=4,
	initializingResponse=5,
	encryptionResponse=6,
} packageType;
//authentication credentials
typedef struct
{
	TCHAR username[25];
	TCHAR password[25];
} authenticationValues;
//request answer values,isSuccessful is 1 if server accept connection
typedef struct
{
	BOOL isSuccesful;
	TCHAR buffer[100];
} requestAnswerValues;
typedef struct
{
	BOOL isAccepted;
} initializingValues;
//package with encryption values,text which will be encrypted
typedef struct
{
	TCHAR buffer[MAX_BUFFER+2];
	TCHAR key[1000];
	DWORD dOrder;
	int fIsLast;
} encryptionValues;
//response of encryption
typedef struct
{
	DWORD bufferLenght;
	TCHAR buffer[MAX_BUFFER+2];
	TCHAR key[10];
	DWORD dOrder;
	int fIsLast;
} encryptionResponseValues;
//after authentication,client receives new pipenames for pipes
typedef struct
{
	TCHAR serverPipename[1000];
	TCHAR clientPipename[1000];
	DWORD isSuccessful;
} authenticationResponseValues;
//struct of package,buffer is a generic type,it is encryptionReponse,...
typedef struct
{
	packageType type;
	void *buffer;
} package;
//the new address of pipes
typedef struct
{
	TCHAR serverPipeName[100];
	TCHAR clientPipeName[100];
} pipeNameConnection;
//structure of a thread,data which is passed to thread
struct threadStructure
{
	HANDLE thread;
	pipeNameConnection connectionPipeName;
	DWORD dThreadId;
	int isRunning;
	int hasFinished;
	struct threadStructure *next;
};
typedef struct threadStructure threadStruc;
//struct of worker,what data is passed to worker
struct workerThreadStructure
{
	HANDLE thread;
	PTCHAR text;
	PTCHAR key;
	DWORD keyStartPosition;
	int isRunning;
	int hasFinished;
	int canBeReused;
	struct workerThreadStructure *next;
};
typedef struct workerThreadStructure workerThreadStruc;
//struct which server is initializng
typedef struct
{
	DWORD dNrWorkers;
	DWORD dNrThreads;
	PINT piIsStopped;
	HANDLE mainThread;
} connectionParamaters;
#endif // TYPES_H_INCLUDED