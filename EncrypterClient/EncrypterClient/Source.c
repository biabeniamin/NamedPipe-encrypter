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
		if(i<5000)
			text[i] = 'a';
		else
			text[i] = 'b';
	
	}
	text[1] = 'b';
	text[20001] = '\0';
	encrypt(TEXT("username"), TEXT("password"), text,TEXT("123"));
	return (0);
}