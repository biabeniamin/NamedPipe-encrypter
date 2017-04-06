#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include "Encryptor.h"
#include "ClientPackagesHandler.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
void encrypt(PTCHAR username,PTCHAR password,PTCHAR text)
{
	initializingCommunication();
	initializingConnection();
	authenticateConnection(TEXT("username"), TEXT("password"));
	encryptData(TEXT("Ana are mere de vanzare"));
	closeCommunication();
}