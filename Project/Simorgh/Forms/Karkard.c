#include "DIALOG.h"
#include "futil.h"
#include <stdio.h>
#include "config.h"
#include "key/keypad.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0     (GUI_ID_USER + 0x00)
#define ID_LISTVIEW_0     (GUI_ID_USER + 0x01)


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 480, 272, 0, 0x0, 0 },
  { LISTVIEW_CreateIndirect, "Listview", ID_LISTVIEW_0, 0, 40, 470, 260, 0, 0x0, 0 },
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
	unsigned char str[10];
  int     NCode;
  int     Id;
	int i, j;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    
	  hItem = pMsg->hWin;
    FRAMEWIN_SetTitleVis(hItem, 0);

	
  	hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_0);
    LISTVIEW_AddColumn(hItem, 50, "مرخصي", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 50, "ماموريت", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 50, "اضافه", GUI_TA_HCENTER | GUI_TA_VCENTER);
	  LISTVIEW_AddColumn(hItem, 50, "غيبت", GUI_TA_HCENTER | GUI_TA_VCENTER);
	  LISTVIEW_AddColumn(hItem, 50, "تاخير", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 70, "کارکرد", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 23, "ش", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 120, "تاريخ", GUI_TA_HCENTER | GUI_TA_VCENTER);
		LISTVIEW_SetGridVis(hItem, 1);
    LISTVIEW_SetRowHeight(hItem, 25);

  	LISTVIEW_AddRow(hItem, NULL);
  	LISTVIEW_SetItemText(hItem, 0, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 1, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 2, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 3, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 4, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 5, 0, "00:10");
  	LISTVIEW_SetItemText(hItem, 6, 0, "A");
  	LISTVIEW_SetItemText(hItem, 7, 0, "چهارشنبه 12/10");


    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
	case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_LISTVIEW_0: // Notifications sent by 'Listview'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_SEL_CHANGED:
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
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

//====================================================================
void frmKarkerd(void)
{
WM_HWIN hWin;
	
	LoadFont("0:fonts/Tahoma20.sif");
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);

  HandleForm(hWin, 0);
}

/*************************** End of file ****************************/
