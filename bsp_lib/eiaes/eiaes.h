#ifndef EIAES_H
#define EIAES_H


extern void ExpandAESKeyForEncryption(const unsigned char Key[16], unsigned long int ExpandedKey[44]);
extern void EncryptAES(const unsigned char InBuf[16], const unsigned long int  Key[44], unsigned char OutBuf[16]);
extern void ExpandAESKeyForDecryption(unsigned long int ExpandedKey[44]); 
extern void ExpandAESKeyForDecryption2(const unsigned char Key[16], unsigned long int ExpandedKey[44]);
extern void DecryptAES(const unsigned char InBuf[16], const unsigned long int Key[44], unsigned char OutBuf[16]);


#endif