#include "Encryptor.h"
PTCHAR getEncryptionKey()
{
	return TEXT("aaa");
}
PTCHAR encrypt(PTCHAR text, PTCHAR key, DWORD keyPosition)
{
	int intPositionOnKey = keyPosition;
	int intKeyLenght = _tcslen(key);
	DWORD dTestLenght= _tcslen(text);
	intPositionOnKey %= intKeyLenght;
	for (DWORD i = 0; i < dTestLenght; i++)
	{
		text[i]=text[i]^ key[intPositionOnKey];
		intPositionOnKey++;
		if (intPositionOnKey >= intKeyLenght)
			intPositionOnKey = 0;
	}
}