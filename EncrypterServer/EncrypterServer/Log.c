#include "Log.h"
HANDLE _file=NULL;
void initializing()
{
	if(_file==NULL)
		_file = CreateFile(TEXT("log.txt"),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
}
void logWriteLine(PTCHAR text)
{
	initializing();
	DWORD bytesWritted;
	WriteFile(_file,
		text,
		_tcslen(text),
		&bytesWritted,
		NULL);
}