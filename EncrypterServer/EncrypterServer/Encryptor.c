#include "Encryptor.h"
PTCHAR getEncryptionKey()
{
	return TEXT("aaa");
}
PTCHAR encrypt(PTCHAR text, PTCHAR key, DWORD keyPosition)
{
	int intPositionOnKey = keyPosition;
	int intKeyLenght = _tcslen(key);
	intPositionOnKey %= intKeyLenght;
	for (int i = 0; i < _tcslen(text); i++)
	{
		text[i]=text[i]^ key[intPositionOnKey];
		intPositionOnKey++;
		if (intPositionOnKey >= intKeyLenght)
			intPositionOnKey = 0;
	}
}