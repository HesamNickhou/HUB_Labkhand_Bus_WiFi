#include "config.h"

#if (DeviceType==BUSDOOR)
#include "DIALOG.h"
#include "futil.h"

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_LISTVIEW_0 (GUI_ID_USER + 0x02)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "", ID_FRAMEWIN_0, 0, 0, 270, 480, 0, 0x0, 0 },
  { LISTVIEW_CreateIndirect, "Listview", ID_LISTVIEW_0, 0, 0, 265, 460, 0, 0x0, 0 },
};

//==================================================================
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
	int i;
	char str[5];
  

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Listview'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetTitleHeight(hItem, 20);

  	hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_0);
    LISTVIEW_AddColumn(hItem, 65, "مانده", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 50, "تراکنش", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 110, "زمان", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 40, "کد", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_SetGridVis(hItem, 1);
    LISTVIEW_SetAutoScrollV(hItem, 1);
    LISTVIEW_SetHeaderHeight(hItem, 25);
    LISTVIEW_SetRowHeight(hItem, 25);
    break;
  case WM_KEY:
    switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) {
    case GUI_KEY_ESCAPE:
			Released=1;
      break;
    case GUI_KEY_ENTER:
    	hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_0);
		  //KalaCode= LISTVIEW_GetSel(hItem);
		  Released=1;
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

//==================================================================
unsigned char frmTransactionsList(void) {
WM_HWIN hWin;
WM_HWIN hItem;
unsigned short i, Idx, ID;
unsigned char j, k;
char str[40];

  if (0)
	{
		//ShowMessageDlg(mtWarning, "رکوردي وجود ندارد",0);
		GUI_Delay(2000);
    return 255;
	}
	//LoadFont(FontTahoma20);
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);

  hItem = WM_GetDialogItem(hWin, ID_LISTVIEW_0);
	//LISTVIEW_SetFont(hItem,Font12x12);
	i=0;
	Idx=0;
	
	for (i=0; i<30; i++)
	{
		//if (Kalas[i].ID==0)
		//	break;
		
    LISTVIEW_AddRow(hItem, NULL);

    LISTVIEW_SetItemText(hItem, 0, i, "5000");
    LISTVIEW_SetItemText(hItem, 1, i, "-750"); 

   // ConvertWin1256TOUTF8(Kalas[i].Title, (unsigned char *)str);
		sprintf(str, "11/25 10:59", 123);		
    LISTVIEW_SetItemText(hItem, 2, i, str);
    sprintf(str, "%d", 123);		
    LISTVIEW_SetItemText(hItem, 3, i, str);
	}
	
	HandleForm(hWin, 0);
	return 0;
}
#endif

