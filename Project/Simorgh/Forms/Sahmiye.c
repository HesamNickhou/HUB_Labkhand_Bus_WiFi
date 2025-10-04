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

unsigned short SahmiyeCode=0;

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
    LISTVIEW_AddColumn(hItem, 100, "مقدار", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 100, "واحد", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 120, "تاريخ انقضاء", GUI_TA_HCENTER | GUI_TA_VCENTER);
    LISTVIEW_AddColumn(hItem, 160, "کد", GUI_TA_HCENTER | GUI_TA_VCENTER);
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
		  SahmiyeCode= LISTVIEW_GetSel(hItem);
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
unsigned char frmSahmiye(unsigned char KalaIndex) {
WM_HWIN hWin;
WM_HWIN hItem;
unsigned short i, Idx, ID;
unsigned char j, k;
unsigned char S[30];
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
		if(Series[i].ID==0)
			break;
		if(Series[i].KalaCode!=Kalas[KalaIndex].ID)
			continue;
    LISTVIEW_AddRow(hItem, NULL);
		S[Idx]=i;
		switch(Series[i].Status)
		{
	    case 2: for(j=0; j<4; j++) LISTVIEW_SetItemBkColor(hItem, j, Idx, 0, GUI_DARKGRAY); break;
	    case 3: for(j=0; j<4; j++) LISTVIEW_SetItemBkColor(hItem, j, Idx, 0, GUI_RED); break;
		}		
    sprintf(str, "%d", Series[i].Amount[Person.Members-1]);		
		LISTVIEW_SetItemText(hItem, 0, Idx, str);

		switch(Kalas[KalaIndex].KalaUnit)
		{
			case 0: LISTVIEW_SetItemText(hItem, 1, Idx, "کيسه"); break;
			case 1: LISTVIEW_SetItemText(hItem, 1, Idx, "کيلو"); break;
			case 2: LISTVIEW_SetItemText(hItem, 1, Idx, "ليتر"); break;
			case 3: LISTVIEW_SetItemText(hItem, 1, Idx, "تن"); break;
			case 4: LISTVIEW_SetItemText(hItem, 1, Idx, "مترمکعب"); break;
		}
    sprintf(str, "%d/%d/%d", Series[i].ExpirationDate[0],Series[i].ExpirationDate[1],Series[i].ExpirationDate[2]);		
    LISTVIEW_SetItemText(hItem, 2, Idx, str);
    sprintf(str, "%d", Series[i].ID);		
    LISTVIEW_SetItemText(hItem, 3, Idx, str);
		Idx++;
	}
	SahmiyeCode=255;
	HandleForm(hWin, 0);
	if(SahmiyeCode==255)
	  return(255);
	else
	  return(S[SahmiyeCode]);
}
#endif

