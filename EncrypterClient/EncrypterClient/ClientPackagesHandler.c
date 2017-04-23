#include "ClientPackagesHandler.h"
HANDLE h;
HANDLE response;
int isOpen = 0;
PTCHAR ptText;
PTCHAR ptKey;
void initializingCommunication()
{
	h = initializingPipeAsClient(TEXT("\\\\.\\pipe\\Pipe"));
	response = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeA"),INFINITE);
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
void sendPackage(DWORD start)
{
	package p;
	encryptionValues data;
	DWORD lenght;
	DWORD dKeyPosition;
	DWORD dKeyLenght;
	p.type = encryption;
	p.buffer = &data;
	dKeyLenght = _tcslen(ptKey);
	lenght = _tcslen(ptText);
	_tcsncpy(data.buffer, ptText + start, MAX_BUFFER);
	data.buffer[start + MAX_BUFFER] = '\0';
	//sync key with text
	dKeyPosition = start % (_tcslen(ptKey));
	_tcscpy(data.key, ptKey + dKeyPosition);
	_tcsncpy(data.key + dKeyLenght - dKeyPosition, ptKey, dKeyLenght - (dKeyLenght - dKeyPosition));
	data.key[dKeyLenght] = '\0';
	data.dOrder = start / MAX_BUFFER;
	if (lenght / MAX_BUFFER == start + 1)
		data.fIsLast = 1;
	else
		data.fIsLast = 0;
	writePackage(h, &p);
}
void encryptData(PTCHAR text,PTCHAR key)
{
	DWORD textLenght = _tcslen(text);
	if (isOpen)
	{
		ptText = text;
		ptKey = key;
		for (DWORD i = 0; i < textLenght; i+=MAX_BUFFER)
		{
			sendPackage(i);
			waitAnswer(response, &packageReceived);
		}
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
			response = initializingPipeAsServer(authResponseValues->clientPipename,INFINITE);
			ConnectNamedPipe(response, NULL);
			
		}
		else
			return 1;
		break;
	case encryptionResponse:
		encResponseVal = pack->buffer;
		_tprintf(TEXT("%d was encrypted.The result is %ls with key %s with lenght of %d\n"), pack->type, encResponseVal->buffer,encResponseVal->key,encResponseVal->bufferLenght);
		for (DWORD i = encResponseVal->dOrder*MAX_BUFFER; i < encResponseVal->dOrder*MAX_BUFFER +MAX_BUFFER; i++)
		{
			ptText[i] = encResponseVal->buffer[i%MAX_BUFFER];
		}
		//close server if it is the last package
		return encResponseVal->fIsLast;
		break;
	}
	return 0;
}