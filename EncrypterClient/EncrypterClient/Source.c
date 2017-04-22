#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Encryptor.h"
int main()
{
	TCHAR text[9000];
	_tcscpy(text, TEXT("bla bla bla"));
	for (int i = 0; i < 9000; i++)
	{
		if(i<5000)
			text[i] = 'a';
		else
			text[i] = 'b';
	}
	text[8999] = '\0';
	encrypt(TEXT("username"), TEXT("password"), text,TEXT("abc"));
	return (0);
}