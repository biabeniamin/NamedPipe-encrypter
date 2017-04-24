#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Types.h"
//initializing server
void
initializingCommunication(DWORD nrClients, DWORD nrWorkers, HANDLE hMainThread);
//print opened connections
void
printOpenedConnections();