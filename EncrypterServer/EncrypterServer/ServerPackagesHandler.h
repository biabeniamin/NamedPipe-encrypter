#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Types.h"
void
initializingCommunication(DWORD nrClients, DWORD nrWorkers, HANDLE hMainThread);
void
printOpenedConnections();