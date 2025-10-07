#include "DIALOG.h"
#include "futil.h"
#include <stdio.h>
#include "config.h"
#include "usart/usart.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0     (GUI_ID_USER + 0x01)
#define ID_EDIT_0     (GUI_ID_USER + 0x02)
#define ID_BUTTON_0     (GUI_ID_USER + 0x03)
#define ID_BUTTON_1     (GUI_ID_USER + 0x04)
#define ID_TEXT_0     (GUI_ID_USER + 0x05)


static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 120, 40, 220, 160, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_0, 24, 48, 162, 25, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 9, 92, 93, 30, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 111, 92, 93, 30, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_0, 13, 9, 185, 25, 0, 0x64, 0 },
};

unsigned char *message;
unsigned char pass;
unsigned int defValue;

//====================================================================
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
	char i, str[15];
  // USER START (Optionally insert additional variables)
  // USER END


	switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Framewin'
    //
    hItem = pMsg->hWin;
 	  FRAMEWIN_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    hItem = pMsg->hWin;
    FRAMEWIN_SetTitleHeight(hItem, 20);
    //FRAMEWIN_SetText(hItem, "منو");
    //
    // Initialization of 'Edit'
    //
	  if (defValue==0)
			str[0] =0; 
		else
  	  sprintf(str, "%d", defValue);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
    EDIT_SetText(hItem, str);
    EDIT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetText(hItem, "انصراف");
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetText(hItem, "تاييد");
    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_VCENTER);
    TEXT_SetText(hItem, (const char *)message);
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (Id) {
    case ID_EDIT_0: // Notifications sent by 'Edit'
      switch (NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch (NCode) {
      case WM_NOTIFICATION_CLICKED:
			  Released=1;
  			break;
      case WM_NOTIFICATION_RELEASED:
        break;
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch (NCode) {
      case WM_NOTIFICATION_CLICKED:
        hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
        EDIT_GetText(hItem, str, 15);
			  defValue=0;
			  for (i=0; (i<15) && (str[i]!=0); i++)
			  {
			    defValue*=10;
					defValue+=(str[i]-48);
        }
			  Released=1;
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
	case WM_KEY:
    switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
		{
      case GUI_KEY_ESCAPE:
				Released=1;
        break;
      case GUI_KEY_ENTER:
        hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
        EDIT_GetText(hItem, str, 15);
			  defValue=0;
			  for (i=0; (i<15) && (str[i]!=0); i++)
			  {
			    defValue*=10;
					defValue+=(str[i]-48);
        }
				Released=1;
        break;
    }
    break;				
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

//====================================================================
unsigned int frmGetNumberBox(unsigned char *text, unsigned int DefaultValue, unsigned char Password){
WM_HWIN hWin;
	
 	LoadFont(FontBKoodak40);
  message=text;
	defValue= DefaultValue;
	pass= Password;
	
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	//GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  HandleForm(hWin, 0);
	return defValue;
}


/*************************** End of file ****************************/
