#include "..\\..\\EncrypterServer\\EncrypterServer\\Connection.h"
#include "..\\..\\EncrypterServer\\EncrypterServer\\Types.h"
//initializing communication with server
void 
initializingCommunication();
//triggered when a package was received
int
packageReceived(package *pack);
//initializing connection
void 
initializingConnection();
//authenticate on server
void
authenticateConnection(PTCHAR username, PTCHAR password);
//send data to be decrypted
void 
encryptData(PTCHAR text,PTCHAR key);
//close communication
void
closeCommunication();
