#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include<string.h>
#include "Encryptor.h"
int main()
{
	TCHAR text[100];
	_tcscpy(text, TEXT("bla bla bla"));
	encrypt(TEXT("username"), TEXT("password"), text,TEXT("abc"));
	return (0);
}