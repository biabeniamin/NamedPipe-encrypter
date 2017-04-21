#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include "Encryptor.h"
#include "ClientPackagesHandler.h"
#include "..\\..\\EncrypterServer\EncrypterServer\Connection.h"
#include "..\\..\\EncrypterServer\EncrypterServer\Types.h"
void encrypt(PTCHAR username,PTCHAR password,PTCHAR text,PTCHAR key)
{
	initializingCommunication();
	initializingConnection();
	authenticateConnection(username, password);
	encryptData(text,key);
	closeCommunication();
}