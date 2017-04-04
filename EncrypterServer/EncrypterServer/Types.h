#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

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
	TCHAR buffer[1000];
} encryptionValues;
typedef struct
{
	DWORD bufferLenght;
	TCHAR buffer[1000];
	TCHAR key[10];
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
#endif // TYPES_H_INCLUDED