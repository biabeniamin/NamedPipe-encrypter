#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Encryptor.h"
int main()
{
	TCHAR text[50000];
	_tcscpy(text, TEXT("bla bla bla"));
	for (int i = 0; i < 50000; i++)
	{
			text[i] = 'a';
			text[i] += i / 5000;
	}
	text[1] = 'b';
	text[49000] = '\0';
	encrypt(TEXT("username"), TEXT("password"), text,TEXT("123"));
	_tprintf(TEXT("final text:%s"), text);
	return (0);
}