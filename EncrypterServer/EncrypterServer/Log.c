#include "Log.h"
HANDLE _file=NULL;
HANDLE _hSemaphore;
void initializing()
{
	if (_file == NULL || _file == INFINITE)
	{
		_file = CreateFile(TEXT("log.txt"),
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		_hSemaphore = CreateSemaphore(NULL,
			1,
			1,
			NULL);
	}
}
void logWrite(LPCVOID buffer, DWORD dSize)
{
	WaitForSingleObject(_hSemaphore, 5000);
	PTCHAR aux = buffer;
	initializing();
	DWORD bytesWritted;
	WriteFile(_file,
		aux,
		dSize,
		&bytesWritted,
		NULL);
	ReleaseSemaphore(_hSemaphore,
		1,
		NULL);
}
void logWriteLine(PTCHAR text)
{
	logWrite(text, _tcslen(text) * sizeof(TCHAR));
	logWriteChar('\r');
	logWriteChar('\n');
	_tprintf(TEXT("%s\n"), text);
}
void logWriteNumber(DWORD dNr)
{
	TCHAR buf[10];
	int pos = 0;
	TCHAR dReversedNumber[10];
	int lenght;
	//reverse number
	pos = 0;
	while (dNr > 0)
	{
		dReversedNumber[pos++] = dNr % 10 + '0';
		dNr /= 10;
	}
	//convert number to buffer
	lenght = pos;
	pos = 0;
	while (pos<lenght)
	{
		buf[pos] = dReversedNumber[lenght-pos-1];
		pos++;
	}
	buf[lenght] = ' ';
	buf[lenght+1] = '\0';
	logWriteWord(buf);
	_tprintf(TEXT("%d "), dNr);
}
void logWriteWord(PTCHAR word)
{
	logWrite(word, _tcslen(word) * sizeof(TCHAR));
	_tprintf(TEXT("%s "), word);
}
void logNewLine()
{
	logWriteChar('\r');
	logWriteChar('\n');
	logWriteWord(TEXT("thread "));
	logWriteNumber(GetCurrentThreadId());
	_tprintf(TEXT("thread %d "), GetCurrentThreadId());
}
void logWriteChar(TCHAR c)
{
	logWrite(&c, sizeof(TCHAR));
	_tprintf(TEXT("%c "), c);
}