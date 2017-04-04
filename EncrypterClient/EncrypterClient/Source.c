#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "ClientPackagesHandler.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
int main(void)
{
	initializingCommunication();
	
	initializingConnection();
	
	authenticateConnection(TEXT("username"), TEXT("password"));
	encryptData( TEXT("Ana are mere de vanzare"));
	closeCommunication();
	return (0);
}