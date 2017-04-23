#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Encryptor.h"
int main()
{
	TCHAR key[100];
	TCHAR filePath[100];
	TCHAR outputFilePath[100];
	TCHAR text[50000];
	HANDLE hInputFile,hOutputFile;
	DWORD bytesReaded;
	_tcscpy(filePath, TEXT("input.txt"));
	_tprintf(TEXT("file path:"));
	_tscanf(TEXT("%s"), filePath);
	_tcscpy(outputFilePath, TEXT("output.txt"));
	_tprintf(TEXT("output file path:"));
	_tscanf(TEXT("%s"), outputFilePath);
	_tcscpy(key, TEXT("123"));
	_tprintf(TEXT("key:"));
	_tscanf(TEXT("%s"), key);
	for (int i = 0; i < 50000; i++)
	{
		text[i] = 'a';
		text[i] += i / 5000;
	}
	text[1] = 'b';
	text[40000] = '\0';
	hInputFile = CreateFile(filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	hOutputFile = CreateFile(outputFilePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	//WriteFile(hInputFile, text, 50000, &bytesReaded, NULL);
	ReadFile(hInputFile,
		text,
		50000,
		&bytesReaded,
		NULL);
	CloseHandle(hInputFile);
	
	encrypt(TEXT("username"), TEXT("password"), text,key);
	_tprintf(TEXT("final text:%s"), text);
	WriteFile(hOutputFile,
		text,
		50000,
		&bytesReaded, 
		NULL);
	return (0);
}