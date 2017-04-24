#include "Encryptor.h"
PTCHAR getEncryptionKey()
{
	return TEXT("aaa");
}
PTCHAR encrypt(PTCHAR text, PTCHAR key, DWORD keyPosition)
{
	//get position on key
	int intPositionOnKey = keyPosition;
	//get key lenght
	int intKeyLenght = _tcslen(key);
	DWORD dTestLenght= _tcslen(text);
	//make sure position on key isn't bigger than lenght of it
	intPositionOnKey %= intKeyLenght;
	//itterate all text characters
	for (DWORD i = 0; i < dTestLenght; i++)
	{
		//make xor between text and key
		text[i]=text[i]^ key[intPositionOnKey];
		//increment position on key
		intPositionOnKey++;
		//if key position is bigger then lenght,set to 0
		if (intPositionOnKey >= intKeyLenght)
			intPositionOnKey = 0;
	}
}