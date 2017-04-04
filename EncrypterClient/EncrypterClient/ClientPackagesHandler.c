#include "ClientPackagesHandler.h"
HANDLE h;
HANDLE response;
void initializingCommunication()
{
	h = initializingPipeAsClient(TEXT("\\\\.\\pipe\\Pipe"));
	response = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeA"));
	ConnectNamedPipe(response,NULL);
}
void initializingConnection()
{
	initializingValues initVal = { .isAccepted = TRUE };
	package p;
	p.type = initializing;
	p.buffer = malloc(1);
	writePackage(h, &p);
	waitAnswer(response,&packageReceived);
}
void authenticateConnection(PTCHAR username, PTCHAR password)
{
	package p;
	p.type = authentication;
	authenticationValues authVal;
	_tcscpy(authVal.username, username);
	_tcscpy(authVal.password, password);
	p.buffer = &authVal;
	writePackage(h, &p);
	waitAnswer(response, &packageReceived);
}
void encryptData(PTCHAR text)
{
	package p;
	encryptionValues data;
	p.type = encryption;
	_tcscpy(data.buffer, text);
	p.buffer = &data;
	writePackage(h, &p);
	waitAnswer(response, &packageReceived);
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
		//if (initVal->isAccepted == 0)
		return 1;
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