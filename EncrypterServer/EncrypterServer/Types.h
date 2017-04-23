#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include "Log.h"
#define LENGHT_PER_WORKER 50
#define MAX_BUFFER 5000
#define SECONDS_TIMEOUT 5
#define MIN(a,b) ((a)<(b)?(a):(b))
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
typedef struct
{
	TCHAR username[25];
	TCHAR password[25];
} authenticationValues;
typedef struct
{
	BOOL isSuccesful;
	TCHAR buffer[100];
} requestAnswerValues;
typedef struct
{
	BOOL isAccepted;
} initializingValues;
typedef struct
{
	TCHAR buffer[MAX_BUFFER+2];
	TCHAR key[1000];
	DWORD dOrder;
	int fIsLast;
} encryptionValues;
typedef struct
{
	DWORD bufferLenght;
	TCHAR buffer[MAX_BUFFER+2];
	TCHAR key[10];
	DWORD dOrder;
	int fIsLast;
} encryptionResponseValues;
typedef struct
{
	TCHAR serverPipename[1000];
	TCHAR clientPipename[1000];
	DWORD isSuccessful;
} authenticationResponseValues;
typedef struct
{
	packageType type;
	void *buffer;
} package;
typedef struct
{
	TCHAR serverPipeName[100];
	TCHAR clientPipeName[100];
} pipeNameConnection;
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
typedef struct
{
	DWORD dNrWorkers;
	DWORD dNrThreads;
	PINT piIsStopped;
	HANDLE mainThread;
} connectionParamaters;
#endif // TYPES_H_INCLUDED