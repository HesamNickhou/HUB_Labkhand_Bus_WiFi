
#ifndef  __SMARTCARD_H__
#define  __SMARTCARD_H__


unsigned short SAM_SendAPDU_T0(const unsigned char *pAPDU, 
                                        unsigned char *pMessage, 
                                        unsigned short wLength, unsigned short wRxLength);


#endif
