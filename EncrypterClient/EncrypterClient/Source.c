#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Encryptor.h"
int main()
{
	TCHAR text[1000];
	_tcscpy(text, TEXT("bla bla bla"));
	for (int i = 0; i < 1000; i++)
	{
		text[i] = 'a';
	}
	text[990] = '\0';
	encrypt(TEXT("username"), TEXT("password"), text,TEXT("abc"));
	return (0);
}