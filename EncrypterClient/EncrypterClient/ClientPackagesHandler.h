#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
void 
initializingCommunication();
int
packageReceived(package *pack);
void 
initializingConnection();
void
authenticateConnection(PTCHAR username, PTCHAR password);
void 
encryptData(PTCHAR text,PTCHAR key);
void
closeCommunication();