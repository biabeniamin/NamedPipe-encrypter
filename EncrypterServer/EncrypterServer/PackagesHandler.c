#include "PackagesHandler.h"
#include "Connection.h"
#include "Encryptor.h"
BOOL authenticateClient(authenticationValues *authValues)
{
	if (_tccmp(authValues->username, TEXT("username")) == 0
		&& _tccmp(authValues->password, TEXT("password")) == 0)
		return TRUE;
	return FALSE;
}
DWORD WINAPI ClientThread(PVOID pipeName)
{
	HANDLE hPipe = initializingPipeAsServer(TEXT("\\\\.\\pipe\\PipeThS"));
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeThC");
	initializingServer(hPipe, clientPipeName, &packageReceived);
	
}
void clientAuthenticated(package *pack, HANDLE responsePipe)
{
	HANDLE thread = CreateThread(NULL, 0, ClientThread, NULL, 0, NULL);
	Sleep(10);
	authenticationResponseValues authResponse;
	package packageToBeSend;
	TCHAR serverPipeName[] = TEXT("\\\\.\\pipe\\PipeThS");
	TCHAR clientPipeName[] = TEXT("\\\\.\\pipe\\PipeThC");
	packageToBeSend.type = authenticationResponse;
	authResponse.isSuccessful = 1;
	_tcscpy(authResponse.serverPipename, serverPipeName);
	_tcscpy(authResponse.clientPipename, clientPipeName);
	packageToBeSend.buffer = &authResponse;
	writePackage(responsePipe, &packageToBeSend);
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
		_tprintf(TEXT("%d \n"), pack->type);
		packageToBeSend.type = initializingResponse;
		initValues.isAccepted = 1;
		packageToBeSend.buffer = &initValues;
		writePackage(responsePipe, &packageToBeSend);
		break;
	case authentication:
		authenticationVal = pack->buffer;
		_tprintf(TEXT("%d %ls %ls \n"), pack->type, authenticationVal->username, authenticationVal->password);
		if (authenticateClient(authenticationVal))
		{
			_tprintf(TEXT("User was loged on!\n"));
			requestAnswerVal.isSuccesful = TRUE;
			clientAuthenticated(pack, responsePipe);
		}
		else
		{
			_tprintf(TEXT("Invalid credentials!\n"));
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
		_tprintf(TEXT("%d message received to be decrypted %ls\n"), pack->type, encValues->buffer);
		encResponseValues.bufferLenght = _tcslen(encValues->buffer);
		encrypt(encValues->buffer, encResponseValues.key);
		_tprintf(TEXT("encrypted text %ls\n"),encValues->buffer);
		_tcscpy(encResponseValues.key, TEXT("key"));
		_tcscpy(encResponseValues.buffer, encValues->buffer);
		packageToBeSend.buffer = &encResponseValues;
		writePackage(responsePipe, &packageToBeSend);
	}
	return 0;
}