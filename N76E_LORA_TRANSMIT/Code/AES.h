#include "string.h"
void Shift_Left(unsigned char *Data);
void XOR(unsigned char *New_Data,unsigned char *Old_Data);
void AES_Add_Round_Key(unsigned char *Round_Key, unsigned char (*State)[4]);
unsigned char AES_Sub_Byte(unsigned char Byte);
void AES_Shift_Rows(unsigned char (*State)[4]);
void AES_Mix_Collums(unsigned char (*State)[4]);
void AES_Calculate_Round_Key(unsigned char Round, unsigned char *Round_Key);
void AES_Encrypt(unsigned char *Data, unsigned char *Key);