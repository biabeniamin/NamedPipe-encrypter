#include "ClientPackagesHandler.h"
HANDLE h;
HANDLE response;
int isOpen = 0;
void initializingCommunication()
{
	h = initializingPipeAsClient(TEXT("\\\\.\\pipe\\Pipe"));
	response = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeA"));
	ConnectNamedPipe(response,NULL);
	isOpen = 1;
}
void initializingConnection()
{
	initializingValues initVal = { .isAccepted = TRUE };
	package p;
	p.type = initializing;
	p.buffer = malloc(1);
	if (isOpen)
	{
		writePackage(h, &p);
		waitAnswer(response, &packageReceived);
	}
}
void authenticateConnection(PTCHAR username, PTCHAR password)
{
	package p;
	p.type = authentication;
	authenticationValues authVal;
	_tcscpy(authVal.username, username);
	_tcscpy(authVal.password, password);
	p.buffer = &authVal;
	if (isOpen)
	{
		writePackage(h, &p);
		waitAnswer(response, &packageReceived);
	}
}
void encryptData(PTCHAR text,PTCHAR key)
{
	package p;
	encryptionValues data;
	DWORD lenght;
	DWORD dKeyPosition;
	DWORD dKeyLenght;
	p.type = encryption;
	p.buffer = &data;
	dKeyLenght = _tcslen(key);
	if (isOpen)
	{
		lenght = _tcslen(text);
		for (DWORD i = 0; i < lenght; i+=MAX_BUFFER)
		{
			_tcsncpy(data.buffer, text + i, MAX_BUFFER);
			data.buffer[i + MAX_BUFFER] = '\0';
			//sync key with text
			dKeyPosition = i % (_tcslen(key));
			_tcscpy(data.key, key+dKeyPosition);
			_tcsncpy(data.key+ dKeyLenght-dKeyPosition, key, dKeyLenght-( dKeyLenght - dKeyPosition));
			data.dOrder = i / MAX_BUFFER;
			if (lenght / MAX_BUFFER == i + 1)
				data.fIsLast = 1;
			else
				data.fIsLast = 0;
			writePackage(h, &p);
		}
		
		waitAnswer(response, &packageReceived);
	}
}
void closeCommunication()
{
	package p;
	p.type = closing;
	TCHAR buf[] = TEXT("");
	p.buffer = &buf;
	if(isOpen)
		writePackage(h, &p);
}
void closePipes()
{
	DisconnectNamedPipe(h);
	DisconnectNamedPipe(response);
	isOpen = 0;
}
int packageReceived(package *pack)
{
	requestAnswerValues *requestAnswerVal;
	initializingValues *initVal;
	encryptionResponseValues *encResponseVal;
	authenticationResponseValues *authResponseValues;
	package packageToBeSend;
	switch (pack->type)
	{
	case initializingResponse:
		initVal = pack->buffer;
		_tprintf(TEXT("%d is succesful %d\n"), pack->type,initVal->isAccepted);
		if (initVal->isAccepted == 0)
		{
			closePipes();
			return 1;
		}
		break;

	case authenticationResponse:
		authResponseValues = pack->buffer;
		_tprintf(TEXT("%d is succes=%d message=%ls \n"), pack->type, authResponseValues->isSuccessful, authResponseValues->serverPipename);
		if (authResponseValues->isSuccessful)
		{
			h = initializingPipeAsClient(authResponseValues->serverPipename);
			response = initializingPipeAsServer(authResponseValues->clientPipename);
			ConnectNamedPipe(response, NULL);
			
		}
		else
			return 1;
		break;
	case encryptionResponse:
		encResponseVal = pack->buffer;
		_tprintf(TEXT("%d was encrypted.The result is %ls with key %ls with lenght of %d\n"), pack->type, encResponseVal->buffer,encResponseVal->key,encResponseVal->bufferLenght);
		return 1;
		break;
	}
	return 0;
}