#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Types.h"
//write a package
void
writePackage(HANDLE hPipe, package *pack);
//initialize a named pipe as server
HANDLE
initializingPipeAsServer(PTCHAR pipeName,DWORD timeout);
//initialize a named pipe as client
HANDLE
initializingPipeAsClient(PTCHAR pipeName);
//read a package
BOOL
readPackage(HANDLE hPipe, package *pack);
//wait for a package
void
waitAnswer(HANDLE hPipe, int(*packageReceived)(package*));
//initializing server and waits for packages
//returns 0 when the connection will be closed,0 otherwise
void 
initializingServer(HANDLE,PTCHAR, int (*packageReceived)(package*, HANDLE));