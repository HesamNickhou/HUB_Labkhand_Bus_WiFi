#include "config.h"

#if(DeviceType==Basket)
#include "DIALOG.h"
#include "futil.h"

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_LISTVIEW_0 (GUI_ID_USER + 0x02)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "", ID_FRAMEWIN_0, 0, 0, 480, 270, 0, 0x0, 0 },
  { LISTVIEW_CreateIndirect, "Listview", ID_LISTVIEW_0, 0, 0, 478, 250, 0, 0x0, 0 },
};

unsigned short KalaCode=0;

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
    LISTVIEW_AddColumn(hItem, 100, "توزيع", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 100, "واحد", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 220, "کالا", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 60, "کد", GUI_TA_HCENTER | GUI_TA_VCENTER);
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
		  KalaCode= LISTVIEW_GetSel(hItem);
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
unsigned char frmKalas(void) {
WM_HWIN hWin;
WM_HWIN hItem;
unsigned short i, Idx, ID;
unsigned char j, k;
char str[40];

  if(0)
	{
		//ShowMessageDlg(mtWarning, "رکوردي وجود ندارد",0);
		GUI_Delay(2000);
    return(255);
	}
	//LoadFont("0:fonts/Tahoma20.sif");
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);

  hItem = WM_GetDialogItem(hWin, ID_LISTVIEW_0);
	i=0;
	Idx=0;
	for(i=0; i<30; i++)
	{
		if(Kalas[i].ID==0)
			break;
		
    LISTVIEW_AddRow(hItem, NULL);
		
		switch(Kalas[i].KalaType)
		{
			case 0: LISTVIEW_SetItemText(hItem, 0, i, "آزاد"); break;
			case 1: LISTVIEW_SetItemText(hItem, 0, i, "سهميه اي"); break;
		}
    
		switch(Kalas[i].KalaUnit)
		{
			case 0: LISTVIEW_SetItemText(hItem, 1, i, "کيسه"); break;
			case 1: LISTVIEW_SetItemText(hItem, 1, i, "کيلو"); break;
			case 2: LISTVIEW_SetItemText(hItem, 1, i, "ليتر"); break;
			case 3: LISTVIEW_SetItemText(hItem, 1, i, "تن"); break;
			case 4: LISTVIEW_SetItemText(hItem, 1, i, "مترمکعب"); break;
		}
    ConvertWin1256TOUTF8(Kalas[i].Title, (unsigned char *)str);
    LISTVIEW_SetItemText(hItem, 2, i, str);
    sprintf(str, "%d", Kalas[i].ID);		
    LISTVIEW_SetItemText(hItem, 3, i, str);
	}
	KalaCode=255;
	HandleForm(hWin, 0);
	return(KalaCode);
}
#endif

