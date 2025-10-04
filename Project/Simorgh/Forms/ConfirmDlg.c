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
#define ID_FRAMEWIN_0     (GUI_ID_USER + 0x00)
#define ID_BUTTON_0     (GUI_ID_USER + 0x01)
#define ID_BUTTON_1     (GUI_ID_USER + 0x02)
#define ID_TEXT_0     (GUI_ID_USER + 0x03)


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

unsigned int ConfirmDlgResult;

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 40, 30, 400, 200, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 59, 123, 110, 30, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 214, 125, 110, 30, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_0, 13, 21, 362, 82, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Framewin'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetTitleHeight(hItem, 25);
    //FRAMEWIN_SetText(hItem, "پيام");
    
   	hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetText(hItem, "انصراف");

  	hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetText(hItem, "تاييد");
	  WM_SetFocus(hItem);
	  
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
			  Released=1;
			  ConfirmDlgResult=0;
        break;
      }
      break;
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
			  Released=1;
			  ConfirmDlgResult=1;
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

//====================================================================
unsigned int frmConfirmDlg(unsigned char *text)
{
WM_HWIN hWin;
WM_HWIN hItem;
	
 	LoadFont(FontTahoma20);

	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);

  hItem = WM_GetDialogItem(hWin, ID_TEXT_0);
  TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_VCENTER);
  TEXT_SetText(hItem, (const char *)text);

  HandleForm(hWin, 0);
	
	return(ConfirmDlgResult);
}

/*************************** End of file ****************************/
