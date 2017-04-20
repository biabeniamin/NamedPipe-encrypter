#include "Log.h"
HANDLE _file=NULL;
void initializing()
{
	if(_file==NULL || _file ==INFINITE)
		_file = CreateFile(TEXT("log.txt"),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
}
void logWrite(LPCVOID buffer, DWORD dSize)
{
	PTCHAR aux = buffer;
	initializing();
	DWORD bytesWritted;
	WriteFile(_file,
		aux,
		dSize,
		&bytesWritted,
		NULL);
}
void logWriteNr(DWORD buffer, DWORD dSize)
{
	initializing();
	DWORD bytesWritted;
	WriteFile(_file,
		buffer,
		dSize,
		&bytesWritted,
		NULL);
}
void logWriteLine(PTCHAR text)
{
	logWrite(text, _tcslen(text));
	_tprintf(TEXT("%s\n"), text);
}
void logWriteNumber(DWORD dNr)
{
	logWriteNr(dNr, sizeof(DWORD));
	_tprintf(TEXT("%d "), dNr);
}
void logWriteWord(PTCHAR word)
{
	logWrite(word, _tcslen(word));
	_tprintf(TEXT("%s "), word);
}
void logNewLine()
{
	_tprintf(TEXT("thread %d "), GetCurrentThreadId());
}
void logWriteChar(TCHAR c)
{
	logWrite(&c, sizeof(TCHAR));
	_tprintf(TEXT("%c "), c);
}