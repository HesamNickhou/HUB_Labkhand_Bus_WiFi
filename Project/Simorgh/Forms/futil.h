
#ifndef FUTIL_H
#define FUTIL_H

#include <WM.h>
#include <config.h>

#if(DeviceType==Basket)
extern unsigned char frmKalas(void);
extern unsigned char frmSahmiye(unsigned char KalaIndex);
#endif
#if(DeviceType==BUSDOOR)
unsigned char frmTransactionsList(void);
#endif

extern unsigned int frmGetNumberBox(unsigned char *text, unsigned int DefaultValue, unsigned char Password);
extern void HandleForm(WM_HWIN hWin, char (*func)(unsigned char Key));
extern unsigned int frmConfirmDlg(unsigned char *text);

extern unsigned char NumericInput;
extern unsigned char Released;
extern WM_HWIN DisplayIO;

#endif //#ifndef FUTIL_H

