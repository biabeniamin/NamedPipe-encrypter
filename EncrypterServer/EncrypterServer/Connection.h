#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Types.h"
void
writePackage(HANDLE hPipe, package *pack);
HANDLE
initializingPipeAsServer(PTCHAR pipeName);
HANDLE
initializingPipeAsClient(PTCHAR pipeName);
BOOL
readPackage(HANDLE hPipe, package *pack);
void
waitAnswer(HANDLE hPipe, int(*packageReceived)(package*));
void 
initializingServer(HANDLE, int (*packageReceived)(package*, HANDLE));//returns 0 when the connection will be closed,0 otherwise