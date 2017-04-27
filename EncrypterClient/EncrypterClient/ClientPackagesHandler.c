#include "ClientPackagesHandler.h"
HANDLE h;
HANDLE response;
int isOpen = 0;
PTCHAR ptText;
PTCHAR ptKey;
void initializingCommunication()
{
	//initializng pipes
	h = initializingPipeAsClient(TEXT("\\\\.\\pipe\\Pipe"));
	response = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeA"),INFINITE);
	//connect to pipe
	ConnectNamedPipe(response,NULL);
	if (h != NULL)
	{
		_tprintf(TEXT("Connection cannot be established!"));
		isOpen = 0;
	}
	isOpen = 1;
}
void initializingConnection()
{
	//request to server to accept connection
	initializingValues initVal = { .isAccepted = TRUE };
	package p;
	p.type = initializing;
	p.buffer = malloc(1);
	if (isOpen)
	{
		//send package
		writePackage(h, &p);
		//wait answer
		waitAnswer(response, &packageReceived);
	}
}
void authenticateConnection(PTCHAR username, PTCHAR password)
{
	//authenticatem on server
	package p;
	p.type = authentication;
	authenticationValues authVal;
	//copy credentials to package
	_tcscpy(authVal.username, username);
	_tcscpy(authVal.password, password);
	p.buffer = &authVal;
	//check if server is open
	if (isOpen)
	{
		//send package
		writePackage(h, &p);
		//wait for response
		waitAnswer(response, &packageReceived);
	}
}
void sendPackage(DWORD start)
{
	//send a package with text
	package p;
	encryptionValues data;
	DWORD lenght;
	DWORD dKeyPosition;
	DWORD dKeyLenght;
	p.type = encryption;
	p.buffer = &data;
	//calculate lenght of key
	dKeyLenght = _tcslen(ptKey);
	lenght = _tcslen(ptText);
	//copy text to buffer
	_tcsncpy(data.buffer, ptText + start, MAX_BUFFER);
	data.buffer[ MAX_BUFFER] = '\0';
	//sync key with text
	//the text is divided in pieces and key must be sync
	dKeyPosition = start % (_tcslen(ptKey));
	_tcscpy(data.key, ptKey + dKeyPosition);
	_tcsncpy(data.key + dKeyLenght - dKeyPosition, ptKey, dKeyLenght - (dKeyLenght - dKeyPosition));
	//set end of key
	data.key[dKeyLenght] = '\0';
	//set package order
	data.dOrder = start / MAX_BUFFER;
	if (lenght / MAX_BUFFER == start + 1)
		data.fIsLast = 1;
	else
		data.fIsLast = 0;
	//write package
	writePackage(h, &p);
}
void encryptData(PTCHAR text,PTCHAR key)
{
	//get lenght of text
	DWORD textLenght = _tcslen(text);
	//check if server is opened
	if (isOpen)
	{
		ptText = text;
		ptKey = key;
		//divide text in small pieces of MAX_BUFFER size
		for (DWORD i = 0; i < textLenght; i+=MAX_BUFFER)
		{
			//send package
			sendPackage(i);
			//wait for answer
			waitAnswer(response, &packageReceived);
		}
	}
}
void closeCommunication()
{
	//close connection
	package p;
	p.type = closing;
	TCHAR buf[] = TEXT("");
	p.buffer = &buf;
	//if connection is open,send package
	if(isOpen)
		writePackage(h, &p);
}
void closePipes()
{
	//disconnect named pipes
	DisconnectNamedPipe(h);
	DisconnectNamedPipe(response);
	//set is open as 0
	isOpen = 0;
}
int packageReceived(package *pack)
{
	requestAnswerValues *requestAnswerVal;
	initializingValues *initVal;
	encryptionResponseValues *encResponseVal;
	authenticationResponseValues *authResponseValues;
	package packageToBeSend;
	//case pack type
	switch (pack->type)
	{
	case initializingResponse:
		//received an answer from server if connection is accepted or declined
		initVal = pack->buffer;
		_tprintf(TEXT("%d is succesful %d\n"), pack->type,initVal->isAccepted);
		if (initVal->isAccepted == 0)
		{
			//if declined,close pipes
			closePipes();
			return 1;
		}
		break;

	case authenticationResponse:
		//authentication response,if credentials was ok or not
		authResponseValues = pack->buffer;
		_tprintf(TEXT("%d is succes=%d message=%ls \n"), pack->type, authResponseValues->isSuccessful, authResponseValues->serverPipename);
		if (authResponseValues->isSuccessful)
		{
			//if authentication was ok,connect to new pipes
			h = initializingPipeAsClient(authResponseValues->serverPipename);
			response = initializingPipeAsServer(authResponseValues->clientPipename,INFINITE);
			ConnectNamedPipe(response, NULL);
			
		}
		//otherwise close server
		else
		{
			_tprintf(TEXT("Invalid credentials!"));
			isOpen = 0;
			return 1;
		}
		break;
	case encryptionResponse:
		//new encryption respose received
		encResponseVal = pack->buffer;
		_tprintf(TEXT("%d was encrypted.The result is %ls with key %s with lenght of %d\n"), pack->type, encResponseVal->buffer,encResponseVal->key,encResponseVal->bufferLenght);
		//set in original position the encrypted content
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