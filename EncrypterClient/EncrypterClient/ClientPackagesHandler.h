#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
int
packageReceived(package *pack);
void 
initializingConnection(HANDLE hPipe);
void
authenticateConnection(HANDLE hPipe, PTCHAR username, PTCHAR password);
void 
encryptData(HANDLE hPipe, PTCHAR text);