#include "Encryptor.h"
PTCHAR getEncryptionKey()
{
	return TEXT("aaa");
}
PTCHAR encrypt(PTCHAR text, PTCHAR key)
{
	_tcscpy(key, getEncryptionKey());
	int intPositionOnKey = 0;
	int intKeyLenght = _tcslen(key);
	for (int i = 0; i < _tcslen(text); i++)
	{
		text[i]=text[i]^ key[intPositionOnKey];
		intPositionOnKey++;
		if (intPositionOnKey >= intKeyLenght)
			intPositionOnKey = 0;
	}
}