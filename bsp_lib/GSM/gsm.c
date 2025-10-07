
#include <includes.h>
#include <math.h>
#include "Config.h"
#include "usart/usart.h"
#include "gsm.h"
#include "rtc/rtc.h"
#include "gps/gps.h"
#include "beep/beep.h"
#include "key/Keypad.h"
#include "BusDoor.h"

unsigned char GPRSStatus=Connecting;
unsigned char CurrentConnectionStep=90;
unsigned char GPRSCHECK=0;
unsigned char CSQValue=99;

unsigned int ConnectionError;

extern unsigned long int LivingTime;
extern signed long int Charge;

extern unsigned char MapView;
extern int InfoCounter;

unsigned char DataBuffer[1024];
unsigned int DataBufferLen=0;
unsigned char DataIsAvailable=0;

extern unsigned char ActivePort;
unsigned char Get_GPRSStatusText(unsigned char *Status);

// Farsi keyboard map based on ISIRI-2901
const unsigned char FarsiUnicode[][7] = {
   "0x0020", // " "
   "0x0021", // "!"
   "0x061B", // ";"
   "0x066B", // ","
   "0x00A4", // " "
   "0x066A", // "%"
   "0x060C", // "°"
   "0x06AF", // "ê"
   "0x0029", // "("
   "0x0028", // ")"
   "0x002A", // "*"
   "0x002B", // "+"
   "0x0648", // "Ê"
   "0x002D", // "-"
   "0x002E", // "."
   "0x002F", // "/"
   "0x06F0", // "?"
   "0x06F1", // "?"
   "0x06F2", // "?"
   "0x06F3", // "?"
   "0x06F4", // "?"
   "0x06F5", // "?"
   "0x06F6", // "?"
   "0x06F7", // "?"
   "0x06F8", // "?"
   "0x06F9", // "?"
   "0x003A", // ":"
   //"0x0643", // "ﬂ"
   "0x06A9", // "ò"
   "0x003E", // "<"
   "0x003D", // "="
   "0x003C", // ">"
   "0x061F", // "?"
   "0x066C", // "°"
   "0x0624", // "ƒ"
   "0x200C", // " "
   "0x0698", // "é"
   "0x0649", // "Ì"
   "0x064D", // " "
   "0x0625", // " "
   "0x0623", // "ÒÚ≈"
   "0x0622", // " "
   "0x0651", // " "
   "0x0629", // "Ò…"
   "0x00BB", // "´"
   "0x00AB", // "ª"
   "0x0621", // "¡"
   "0x004E", // "N"
   "0x005D", // "["
   "0x005B", // "]"
   "0x0652", // " "
   "0x064B", // " "
   "0x0626", // "∆"
   "0x064F", // " "
   "0x064E", // " "
   "0x0056", // "V"
   "0x064C", // " "
   "0x0058", // "X"
   "0x0650", // " "
   "0x0643", // "ﬂ"
   "0x062C", // "Ã"
   "0x005C", // "\"
   "0x0686", // "ç"
   "0x00D7", // "x"
   "0x0640", // "-"
   "0x200D", // " "
   "0x0634", // "‘"
   "0x0630", // "–"
   "0x0632", // "“"
   "0x06CC", // "Ì" 
   // "0x064A" = Ì
   "0x062B", // "À"
   "0x0628", // "»"
   "0x0644", // "·"
   "0x0627", // "«"
   "0x0647", // "Â"
   "0x062A", // " "
   "0x0646", // "‰"
   "0x0645", // "„"
   //"0x067E", // "Å"
   "0x0626", // "∆"
   "0x062F", // "œ"
   "0x062E", // "Œ"
   "0x062D", // "Õ"
   "0x0636", // "÷"
   "0x0642", // "ﬁ"
   "0x0633", // "”"
   "0x0641", // "›"
   "0x0639", // "⁄"
   "0x0631", // "—"
   "0x0635", // "’"
   "0x0637", // "ÿ"
   "0x063A", // "€"
   "0x0638", // "Ÿ"
   "0x007D", // "{"
   "0x007C", // "|"
   "0x007B", // "}"
   "0x007E"  // "~"
   };

unsigned char ChargeCode[50];
unsigned char Get_GPRSStatusText(unsigned char *Status);

unsigned int rxd3_counter = 0;
unsigned int rxd3_wr_index = 0;
unsigned int rxd3_rd_index = 0;

#define BUFFER3_SIZE         1200
unsigned char RXD3Buffer[BUFFER3_SIZE];	 

typedef struct{
  char *Message;
  unsigned char Len;
  unsigned char CurLoc;
  unsigned char *Received;
  //char *Data;
}TGSMMessage;

TGSMMessage GSMMessages[] =
{
  {"+CMTI: \0", 7, 0, &SMSReceived},
  //{"+CMTI: \"SM\",\0", 12, 0, &SMSReceived},
  {"SEND OK\0", 7, 0, &DataSent},
  {"Call Ready\0", 10, 0, &CallReady},
  {"NO CARRIER\0", 10, 0, &NoCarrier},
  {"NOT INSERTED\0", 12, 0, &HaveNotSIMCARD}, 
  {"+CBM:\0", 5, 0, &GetBtsName},
  {"TCP CONNECTING\0", 14, 0, &GPRSConnecting},
  {"TCP CLOSEING\0", 12, 0, &GPRSClose},
  {"TCP CLOSED\0", 10, 0, &GPRSClose},
  {"PDP DEACT\0", 9, 0, &GPRSDeact},
  {"CONNECT OK\0", 10, 0, &GPRSConnectOK},
  {"+CUSD:\0", 6, 0, &CUSDReady},
  {"RING\0", 4, 0, &CallReceived},
  {"+CSQ:\0", 5, 0, &CSQ}
};


const char *IPD="+IPD,";
const unsigned char IPDLen=5;
unsigned char IPDLoc=0;

unsigned int DataSize=0;
	 
//=============================================================================
void USART3_IRQHandler(void) {
	u8 Byte;
	unsigned char i;
	volatile unsigned int IIR;
	
  IIR = USART3->SR;
  if (IIR & USART_FLAG_ORE) 
	{        
    USART3->SR &= ~USART_FLAG_ORE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_TXE) 
	{        
    USART3->SR &= ~USART_FLAG_TXE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_RXNE) 
	{     
		Byte=(USART3->DR & 0x1FF);
    RXD3Buffer[rxd3_wr_index] =Byte;
		//USART_SendData(USART1, Byte);//////////////////////////

      //Process Data .... 
      //---------------------------
      if (IPDLoc<IPDLen)
        IPDLoc=(Byte==IPD[IPDLoc])?IPDLoc+1:0;
      if (IPDLoc>=IPDLen)
      {
        if (IPDLoc>=10)
        {  
          DataBuffer[DataSize++] =Byte;
          if (DataSize>=DataBufferLen)
          {
            IPDLoc=0;
            DataSize=0;
            rxd3_rd_index=rxd3_counter=rxd3_wr_index=0;
            DataIsAvailable=1;
            USART3->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
						//OSIntExit(); 
            return;
          }
        }
        else
        {
          switch (Byte)
          {
            case ':':
              DataBufferLen=DataSize;
              if (DataBufferLen>1024)
                DataBufferLen=1024;
              IPDLoc=10;
              DataSize=0;
              break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              DataSize*=10;
              DataSize+=Byte-'0';
              break;
          }//switch    
        }
      }
    
      //---------------------------
      //Process Messages .... 
      for (i=0; i<14; i++)
      {
        GSMMessages[i].CurLoc=(Byte==GSMMessages[i].Message[GSMMessages[i].CurLoc])?GSMMessages[i].CurLoc+1:0;
        if (GSMMessages[i].CurLoc>=GSMMessages[i].Len)
        {
          *GSMMessages[i].Received=1;
					//printf("\n\rMessage: %s ",GSMMessages[i].Message);
          GSMMessages[i].CurLoc=0;
					if (i!=13)
					{
            rxd3_rd_index=rxd3_counter=rxd3_wr_index=0;
					}
  				//OSIntExit(); 
          USART3->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
          return;
        }
      }
			
      if (++rxd3_wr_index == BUFFER3_SIZE) rxd3_wr_index=0;
      if (++rxd3_counter == BUFFER3_SIZE)
      {
        rxd3_rd_index=rxd3_counter=rxd3_wr_index=0;
      }
  } 
	
  USART3->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
	//OSIntExit();  	
} 
	 
//==============================================================================
unsigned char GSM_GCL(unsigned char *c)
{
int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) 
			return 0;
	  if (OS_TimeMS-i>=6000) //6 sec
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  *c=RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) rxd3_rd_index=0;
  if (rxd3_counter) rxd3_counter--;
  return 1;
}

//==============================================================================
unsigned char GSM_GCM(unsigned char *c)
{
int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) 
			return 0;
	  if (OS_TimeMS-i>=1000) //6 sec
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  *c=RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) rxd3_rd_index=0;
  if (rxd3_counter) rxd3_counter--;
  return 1;
}

//==============================================================================
unsigned char GSM_GCF(unsigned char *c)
{
unsigned int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=200)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  *c=RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) rxd3_rd_index=0;
  if (rxd3_counter) rxd3_counter--;
  return 1;
}

//==============================================================================
unsigned char GSM_GCVF(unsigned char *c)
{
unsigned int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=50) //50 msec
			return 0;
    if (rxd3_counter!=0) 
      break;
		WDTR;
	}		
  *c=RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) rxd3_rd_index=0;
  if (rxd3_counter) rxd3_counter--;
  return 1;
}
	 
//==============================================================================
void EmptyGSMRXBuffer(void)
{
  rxd3_rd_index=rxd3_counter=rxd3_wr_index=0;
}

//==================================================================
unsigned char GetGSMResponse(const char *Response)
{
int i, j, ReceiveBufLen;
unsigned char Byte;
unsigned char ReceiveBuf[101];

  ReceiveBufLen=0;
	while (ReceiveBufLen==0)
	{
    if (GSM_GCL(&Byte))
  	{
      if ((Byte!=13) && (Byte!=10))
		    ReceiveBuf[ReceiveBufLen++] =Byte;  
    }else 
      return 1;
  }
      
  while ((GSM_GCF(&Byte)) && (ReceiveBufLen<100)) 
    ReceiveBuf[ReceiveBufLen++] =Byte;

  ReceiveBuf[ReceiveBufLen] =0;
  
  //printf("\n\rResponse[%d] : ",ReceiveBufLen);/////////////
  j=0;
  for (i=0; i<ReceiveBufLen; i++)
  {
    //printf("%c",ReceiveBuf[i]);//////////////////////////
    if (Response[j] =='\0') return 0;  
    if (Response[j] ==ReceiveBuf[i]) j++;
    else j=0;
  }          
  return 2;
}

//==================================================================
unsigned char GetGSMResponse2(const char *Response)
{
int i, j, ReceiveBufLen;
unsigned char Byte;
unsigned char ReceiveBuf[101];

  ReceiveBufLen=0;
  while ((GSM_GCF(&Byte)) && (ReceiveBufLen<100)) 
    ReceiveBuf[ReceiveBufLen++] =Byte;
    
  ReceiveBuf[ReceiveBufLen] =0;
  
  if (ReceiveBufLen==0)
    return 1;
  
  //printf("\n\rResponse2: ");/////////////
  j=0;
  for (i=0; i<ReceiveBufLen; i++)
  {
    //printf("%c",ReceiveBuf[i]);///////////////////////////////////////
    if (Response[j] =='\0') return 0;  
    if (Response[j] ==ReceiveBuf[i]) j++;
    else j=0;
  }          
  return 2;
}

//========================================================================================================
void USART_SendStr(USART_TypeDef* USARTx, char *str)
{
int i=0;

	  //printf("Send:");////////////////////////////////
    //while (str[i]!=0)
    //  USART_SendData(USART1, str[i++]);
		
  	i=0;
    while (str[i]!=0)
    {
      // Send character
      switch (str[i])
      {
        case '\n':
          USART_SendData(USARTx, 13);
          break;
        case '\r':
          USART_SendData(USARTx, 10);
          break;
        case '\"':
          USART_SendData(USARTx, 34);
          break;
        default:
          USART_SendData(USARTx, str[i]);
      } 
      i++;
    }  
}

//========================================================================================================
char SendSMS(char *TelNum,char *DataToSend)
{
  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse2("OK")) return 1;
  USART_SendStr(USART3, "AT+CSMP=17,167,0,0\n");
  if (GetGSMResponse2("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse2("CREG: 0,1")) return 2;  
  // We have excuted at+creg?
  // if GSM part is active and attached to the network, we should recieve AT+CREG: 0,1
  // if GSM part is not atatached to the net , we will recieve AT+CREG: 0,n    ( n is 0 or 2 )
  // if GSM part or interface board is off, we will recieve nothing	 		

  USART_SendStr(USART3, "at+cmgf=1\n");
  if (GetGSMResponse("OK")) return 4;
	
  USART_SendStr(USART3, "at+cmgs=\"");
  USART_SendStr(USART3, TelNum);
  USART_SendStr(USART3, "\"\n");
  GetGSMResponse(">");
  USART_SendStr(USART3, DataToSend);
  USART_SendData(USART3, 26);

  if (GetGSMResponse("+CMGS")) return 5;
  return 0;

}

unsigned char CUSDReady=0;
unsigned char CallReceived=0;
unsigned char CallReady=0;
unsigned char SMSReceived=0;
unsigned char NoCarrier=0;
unsigned char HaveNotSIMCARD=0;
unsigned char GetBtsName=0;
unsigned char DataSent=0;
unsigned char GPRSConnecting=0;
unsigned char GPRSClose=0;
unsigned char GPRSDeact=0;
unsigned char GPRSConnectOK=0;
unsigned char GPRSRetry=0;
unsigned char DataInBuffer=0;
unsigned char CSQ=0;

//=========================================================================
unsigned char ExecuteCommandBySMS(unsigned char *Res)
{                         
unsigned char Idx, i=0,Byte;

  //printf("\n\rChargeCode=%s",ChargeCode);
  
  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

  for (i=0; (i<50) && (ChargeCode[i]!=0); i++) 
     USART_SendData(USART3,ChargeCode[i]);
  USART_SendData(USART3,'#'); 
  USART_SendData(USART3,13);  
  
   if (GetGSMResponse("OK")) return 3;     
  
   Res[0] =0;
  
	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
  }
 
  Idx=0;

  while (GSM_GCF(&Byte) && (Idx<150)) 
  {  
    Res[Idx++] =Byte;
  }

  Res[Idx] =0;

  OkBip(1);
}

//=======================================================================================
unsigned char ProcessSMS(void)
{
char Password[20];
char TelNumber[20],Counter,Which_Output;
char CommandNo[20];
char Parameters[10][20];
unsigned char ServerStatus[50],Ip_Temp[20];
char _snr[6];
unsigned int GpsSpeed=0;
unsigned int i;
unsigned char Byte, OldByte;
unsigned char CommandStarted=0;
unsigned char CommandPosition=0;
unsigned int  CurIndex=0;
unsigned long int time=0;
unsigned int TelIdx=0;
unsigned int I,D,M;
float F, F1;
char str[400], str3[100];
signed long int SimcardCharge=0;
unsigned long int Li=0;
unsigned char SendConfig=0;	

	//printf("\n\rSMS Received");
  Parameters[0][0] =0;
  Parameters[1][0] =0;
  Parameters[2][0] =0;
  Parameters[3][0] =0;
  Parameters[4][0] =0;
  Parameters[5][0] =0;
  Parameters[6][0] =0;
  Parameters[7][0] =0;
  Parameters[8][0] =0;
  Parameters[9][0] =0;
	USART_SendStr(USART3, "AT\n");
  if (GetGSMResponse2("OK")) return 1;
	USART_SendStr(USART3, "at+cmgf=1\n");
  if (GetGSMResponse2("OK")) return 1;

  USART_SendStr(USART3, "at+cmgr=1\n");
  
  while (GSM_GCM(&Byte))
  {
    //SendByte(Byte);
    if (!CommandStarted)
    {
      if (TelIdx)
      {
        switch (Byte)
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            TelNumber[TelIdx++] =Byte;
					  if (TelIdx>=20) TelIdx=0;
            TelNumber[TelIdx] =0;
            break;
          default:
            TelIdx=0;
        }
      }
      if ((Byte=='+') && (OldByte=='"'))
      {
        TelIdx=1;
        TelNumber[0] ='+';
        TelNumber[1] =0;
      }
      if ((Byte=='W') || (Byte=='w'))
        CommandStarted=1;
      CommandPosition=0;
      OldByte=Byte;
    }else{
      if (Byte==','||(Byte==10) || (Byte==13))
      {
        CommandPosition++;
        CurIndex=0;
      }else{
        if (CurIndex<20)
        {
          switch (CommandPosition)
          {
            case 0:  //Password
              Password[CurIndex++] =Byte;
              Password[CurIndex] =0;
              break;
            case 1:  //Command #
              CommandNo[CurIndex++] =Byte;
              CommandNo[CurIndex] =0;
              break;
            case 2:  //Parameter 1
            case 3:  //Parameter 2
            case 4:  //Parameter 3
            case 5:  //Parameter 4
            case 6:  //Parameter 5
            case 7:  //Parameter 6
              Parameters[CommandPosition-2][CurIndex++] =Byte;
              Parameters[CommandPosition-2][CurIndex] =0;
              break;
          }//switch
        }//if  
      }//if
    }//if
  }//while
  
  DeleteAllMessages();
  
  //printf("\n\r==================");
  Config.MainPassword[6] =0;
  //printf("\n\rTelNumber: %s",TelNumber);
  //printf("\n\rMainPassword: %s",Config.MainPassword);
  //printf("\n\rPassword: %s",Password);
  //printf("\n\rCommandNo: %s",CommandNo);
  //printf("\n\rParameter1: %s",Parameters[0]);
  //printf("\n\rParameter2: %s",Parameters[1]);
  //printf("\n\rParameter3: %s",Parameters[2]);
  //printf("\n\rParameter4: %s",Parameters[3]);
  //printf("\n\rParameter5: %s",Parameters[4]);
  
  
  for (i=0; (i<6) && (Password[i]!=0); i++)
    if (Password[i]!=Config.MainPassword[i])
      break;
  
  if ((i<6) || (Password[6]!=0))
  {
    sprintf(str,"Incorrect Password"); 
    str[160] =0;
    SendSMS(TelNumber, str); 
    //ErrorBip(0); //Password Incorrect
    return 0;
  }
  
  if (!strcmp("001",CommandNo))  //Change Password
  {
    for (i=0; (i<20) && (Parameters[0][i]!=0); i++);
    if (i==6)
    {
      strcpy(Config.MainPassword, Parameters[0]);
      //i=SaveConfiguration();
      //LoadConfiguration();
      str[160] =0;
      SendSMS(TelNumber, str);
      OkBip(1);
    }else{
      ErrorBip(1); //Invalid New Password
      return 0;
    }
  }
  
  if (!strcmp("064",CommandNo))  
  {
    SendConfig=1;
  }

  if (!strcmp("088",CommandNo))
  {
    if ((Parameters[0][0]!=0) && (Parameters[0][0]>='0') && (Parameters[0][0]<='9'))
      strcpy(Config.HostIP, Parameters[0]);
    
    SaveConfiguration();
    LoadConfiguration();
    OkBip(1);
		SendConfig=1;
  }

  if (!strcmp("020",CommandNo))
  {
    if ((Parameters[0][0]!=0) && (Parameters[0][0]>='0') && (Parameters[0][0]<='9'))
      Config.DeviceID=atoi(Parameters[0]);
    
    SaveConfiguration();
    LoadConfiguration();
    OkBip(1);
		SendConfig=1;
  }
	
	if (SendConfig==1)
	{
    sprintf(str3,"%c[%d,%d,%d]",GPSData.Status[0],SattelitesInView,PositionFix,HDOP);    
    for (Counter=0;Counter<15;Counter++) Ip_Temp[Counter] =Config.HostIP[Counter];
    Ip_Temp[Counter] =0;
    sprintf(str,"Ver:%d-%d.%d\rID: %d\rUC: %d\r%02d:%02d:%02d , %02d/%02d/%02d\rCharge:%d\r%s\r%s",DeviceType,Ver,Release,Config.DeviceID,Config.UC,Hour,Min,Sec,(Year%100),Month,Day,DeviceInfo.Charge/10,Ip_Temp,str3); 
    str[160] =0;
    SendSMS(TelNumber, str); 
  }
	
  //.........................................................................................
  if (!strcmp("100",CommandNo))  //Track on Demand
  {
    F=atof(GPSData.Lat);
    I=floor(F);
    F-=I;
    D=I/100;
    M=I%100;
    F=D+(float)(M*0.0167)+(float)(F*0.0167*0.0167); 
    
    F1=atof(GPSData.Lon);
    I=floor(F1);
    F1-=I;
    D=I/100;
    M=I%100;
    F1=D+(float)(M*0.0167)+(float)(F1*0.0167*0.0167); 

    sprintf(str, "http://maps.google.com/maps?f=q&hl=en&q=%f,%f&ie=UTF8&z=16&iwloc=addr&om=1",F,F1);
    SendSMS(TelNumber, str); 
  }
	return 0;
}

//=========================================================================
unsigned char DeleteAllMessages(void)
{
  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  //USART_SendStr(USART3, "at+cmgd=0,4\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  USART_SendStr(USART3, "at+cmgd=1,4\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("OK")) return 2;  
  return 0;
}

//=========================================================================
char GPRSSend(unsigned int len, unsigned char *DataToSendRec)
{
unsigned char ch=0;
int i=0;

  ConnectionError++;
  EmptyGSMRXBuffer();
	/*
  USART_SendStr(USART3, "AT\n");
  if (GetGSMResponse("OK")) 
  {
    //printf("\n\rAT Error");
    return 1;
  }
	*/
  
  USART_SendStr(USART3, "at+cipsend=");
  
  ch=(len/1000)%10;
  if (ch!=0) { USART_SendData(USART3, ch+'0'); i=1; } else { if (i==1) USART_SendData(USART3, '0');}
  ch=(len/100)%10;
  if (ch!=0) { USART_SendData(USART3, ch+'0'); i=1; } else { if (i==1) USART_SendData(USART3, '0');}
  ch=(len/10)%10;
  if (ch!=0) { USART_SendData(USART3, ch+'0'); i=1; } else { if (i==1) USART_SendData(USART3, '0');}
  ch=(len/1)%10;
  if (ch!=0) { USART_SendData(USART3, ch+'0'); i=1; } else { if (i==1) USART_SendData(USART3, '0');}
  USART_SendData(USART3, 13);
  
  
  if (GetGSMResponse(">")) 
  {
    //printf("\n\r> Error");
    return 2;
  }
	
	//GUI_Delay(100);

  for (i=0; i<len; i++) 
    USART_SendData(USART3, DataToSendRec[i]);
  
  //printf("\n\rGPRS Send: ");
  //for (i=0; i<len; i++) 
  //  printf("%d ", DataToSendRec[i]);
  //if (GetGSMResponse("OK")) return 3;
 	GUI_SetColor(GUI_ORANGE);
	GUI_FillCircle(300, 11, 4);

  return 0;
}

//------------------------------------------------------------------------------
void GSM_PowerOFF(void)
{
  if (GSMSTATUS)  //Device is currently on
  {
    GSMPWR=1;
    GUI_Delay(1000);
    GSMPWR=0;
  }
}

//==================================================================
unsigned char GetCSQ(void)
{
int j,ReceiveBufLen;
unsigned char ReceiveBuf[101];

  ReceiveBufLen=0;
  while (rxd3_counter)
  {
    if (((ReceiveBufLen==0) && (RXD3Buffer[rxd3_rd_index] =='+')) || (ReceiveBufLen>0))
    {
      ReceiveBuf[ReceiveBufLen++] =RXD3Buffer[rxd3_rd_index];
    }
    if (++rxd3_rd_index >= BUFFER3_SIZE) rxd3_rd_index=0;
    rxd3_counter--;
    if (ReceiveBufLen>=10) break;
  }  
 
  //printf("\n\rReceived Data: ");
  //for (j=0; j<ReceiveBufLen; j++)
  //  printf("%c",ReceiveBuf[j]);
  ReceiveBuf[ReceiveBufLen] =0;
  
  if (ReceiveBufLen<7)
    return 101;
  
  if ((ReceiveBuf[0]!='+') || (ReceiveBuf[1]!='C') || (ReceiveBuf[2]!='S') || (ReceiveBuf[3]!='Q') || (ReceiveBuf[4]!=' '))
    return 102;
  
  if (ReceiveBuf[6] ==',')
    j=ReceiveBuf[5]-'0';
  else if (ReceiveBuf[7] ==',')
    j=((ReceiveBuf[5]-'0')*10)+(ReceiveBuf[6]-'0');
  else
    return 103;
  CSQValue=j;
  return j;
}

//------------------------------------------------------------------------------
void PasssGPRSConnectionSteps(void)
{
char str[100],Temp,Ip_Temp[20],Counter;
static unsigned char Command_Wait=0;
static unsigned char Command_Retry=0;
static unsigned char CIICR_ErrorResponse=0;

	  //printf("\n\rCurrentConnectionStep: %d ConnectionError: %d", CurrentConnectionStep,ConnectionError);
    switch (CurrentConnectionStep)
    {
      case 90:  //Module must be turned off  200
    Log("GPRS90")
        CIICR_ErrorResponse=0;
        if (GSMSTATUS)  //Device is currently on
        {
           //printf("Status 1 Set Power");
           GSMPWR=1;
           CurrentConnectionStep=91;
        }
        else
        {
           //printf("Status 0 Go 92");
           CurrentConnectionStep=92;
           Command_Retry=0;
        }
        break;
      case 91:
    Log("GPRS91")
        //printf("Clear Power Go 90");
        GSMPWR=0;
        CurrentConnectionStep=90;
        break; 
      case 92: //91
    Log("GPRS92")
        CurrentConnectionStep++;  //wait 1 Sec
        Command_Retry=0;
        break;
      case 93:  //Module must be turned on     92
    Log("GPRS93")
        CallReady=0;
        if (GSMSTATUS) 
        {
          //printf("Status 1 Go 95");
          CurrentConnectionStep=95;
          Command_Retry=0;
        }
        else
        {
          //printf("Status 0 Set Power");
          GSMPWR=1;
          CurrentConnectionStep=94;
        }
        break;
      case 94:
    Log("GPRS94")
        //printf("Reset Power");
        GSMPWR=0;
        CurrentConnectionStep=93;
        break; 
      case 95:           
    Log("GPRS95")
        //printf("CallReady: %d",CallReady);
        USART_SendStr(USART3, "at\n");
        //CurrentConnectionStep=1;
			  
        if (CallReady)
        {
          EmptyGSMRXBuffer();
          USART_SendStr(USART3, "at\n");
          GUI_Delay(50);
          USART_SendStr(USART3, "at\n");
          CurrentConnectionStep=1;
          Command_Retry=0;
        }
        else Command_Retry++;
     
        if (Command_Retry>=5) 
        {
          //CurrentConnectionStep=1;
          //Command_Retry=0;
          EmptyGSMRXBuffer();
          USART_SendStr(USART3, "at\n");
          GUI_Delay(50);
          USART_SendStr(USART3, "at\n");
          CurrentConnectionStep=1;
          Command_Retry=0;
        } 
        			
        break;
      case 1:
    Log("GPRS1")
        if (GetGSMResponse2("OK"))
        {
          uart3_init(19200);    
          GUI_Delay(100);
          USART_SendStr(USART3, "AT\n");
          GetGSMResponse2("OK");
          USART_SendStr(USART3, "ate0\n");
          GetGSMResponse2("OK");
          USART_SendStr(USART3, "at+ciurc=1\n");
          GetGSMResponse2("OK");
          USART_SendStr(USART3, "at+ipr=115200\n");
          GetGSMResponse2("OK");
          USART_SendStr(USART3, "at&w\n");
          GetGSMResponse2("OK");
          uart3_init(115200);         
          CurrentConnectionStep=90;
          break;
        }
        USART_SendStr(USART3, "AT+CLVL=100\n");
        GetGSMResponse2("OK");
        EmptyGSMRXBuffer();
        USART_SendStr(USART3, "AT+CMIC=0,10\n");
        GetGSMResponse2("OK");
        EmptyGSMRXBuffer();
        USART_SendStr(USART3, "AT+CUSD=1\n");
        GetGSMResponse2("OK");
        EmptyGSMRXBuffer();
        USART_SendStr(USART3, "AT+CPIN?\n");
        CurrentConnectionStep++;
        Command_Retry=0;
        break;
      case 2:
    Log("GPRS2")
        if (Command_Retry>=10) 
        {
					/*
          SetColor(RGB(84,75,80));
          G_RectangleFill(215, 220, 233, 240, 0);
          SetFgColor(RGB(255,0,0));
          LcdFont(Tahoma16);
          SetCursorX(217);
          SetCursorY(220);
          PutChar('S');
          PutChar('I');	
          PutChar('M');	
          G_RepaintArea(220,240);
					*/
          break;
        }  
      
				Temp=GetGSMResponse2("+CPIN: READY");
        if (Temp)
        {
          Command_Retry++;
          USART_SendStr(USART3, "AT+CPIN?\n");
          if ((Command_Retry>3) && (Temp==1))
            CurrentConnectionStep=90;
        }
        else
        {
          USART_SendStr(USART3, "ate0\n");
          GUI_Delay(50);
          EmptyGSMRXBuffer();
          USART_SendStr(USART3, "at+creg?\n");
          CurrentConnectionStep++;
          Command_Retry=0;
        } 
        break;
      case 3:
    Log("GPRS3")
        if (!GetGSMResponse2("CREG: 0,0"))
        {
          Command_Retry++;
          USART_SendStr(USART3, "at+creg?\n");
          //CurrentConnectionStep--;
        }
        else
        {
          CurrentConnectionStep++;
          Command_Retry=0;
        } 
        
        if (Command_Retry>=30) 
        {
          CurrentConnectionStep=90;
          Command_Retry=0;
        }        
        break;
      case 4:
			  Log("GPRS4")
        USART_SendStr(USART3, "at+cgatt?\n");
        CurrentConnectionStep++;
        break;
      case 5:
    Log("GPRS5")
        if (GetGSMResponse2("CGATT: 1"))
        {
          Command_Retry++;
          CurrentConnectionStep--;
        }
        else
        {
      		switch (Config.SIMCardType)
		      {
			      case 2: //MCI
			      case 20:
              USART_SendStr(USART3, "AT+CSTT=\"mcinet\"\n");
              break;			
			      case 3: //Rightel
              USART_SendStr(USART3, "AT+CSTT=\"RighTel\"\n");
              break;			
			      default: //Irancell
              USART_SendStr(USART3, "AT+CSTT=\"mtnirancell\"\n");
              break;			
		      }
          GUI_Delay(50);
          EmptyGSMRXBuffer();
          USART_SendStr(USART3, "at+ciicr\n");
          CurrentConnectionStep++;
          Command_Retry=0;
        } 
        
        if (Command_Retry>=15) 
        {
          CurrentConnectionStep=90;
          Command_Retry=0;
        }
        break;
      case 6:
    Log("GPRS6")
        if (GetGSMResponse2("OK"))
          Command_Retry++;
        else
        {
          USART_SendStr(USART3, "at+ciphead=1\n");
          GUI_Delay(50);
          EmptyGSMRXBuffer();
          USART_SendStr(USART3, "at+cifsr\n");
          CurrentConnectionStep++;
          Command_Retry=0;
        } 
        
        if (Command_Retry>=15) 
        {
          CurrentConnectionStep=90;
          Command_Retry=0;
        }
        break;
      case 7:
    Log("GPRS7")
        EmptyGSMRXBuffer();
        for (Counter=0;Counter<15;Counter++) Ip_Temp[Counter] =Config.HostIP[Counter];
        Ip_Temp[Counter] =0;
        sprintf(str,"at+cipstart=\"TCP\",\"%s\",\"10045\"\n",Ip_Temp);
			  #if (DeviceType==Basket)
        sprintf(str,"at+cipstart=\"TCP\",\"%s\",\"10068\"\n",Ip_Temp); 
			  #endif
        USART_SendStr(USART3, str); 
        GPRSStatus=Connected;
        CurrentConnectionStep++;
        break;
      case 13:
    Log("GPRS13")
  			EmptyGSMRXBuffer();
        USART_SendStr(USART3, "at+cipshut\n");
        GetGSMResponse2("OK");
        USART_SendStr(USART3, "at+ciicr\n");
        CurrentConnectionStep=6;
			  /*
	  		EmptyGSMRXBuffer();
        GPRSRetry++;
        if (GPRSRetry<4)
        {
          USART_SendStr(USART3, "at+cipclose\n");
          CurrentConnectionStep=7;
        }else 
          CurrentConnectionStep++;
			  */
        break;
      case 14:
    Log("GPRS14")
  			EmptyGSMRXBuffer();
        USART_SendStr(USART3, "at+cipshut\n");
        GetGSMResponse2("OK");
        USART_SendStr(USART3, "at+ciicr\n");
        CurrentConnectionStep=6;
			  /*
        EmptyGSMRXBuffer();
        GPRSRetry++;
        if (GPRSRetry<6) 
        {
          USART_SendStr(USART3, "at+cipshut\n");
          CurrentConnectionStep=4;
        }else
          CurrentConnectionStep=90;
			  */
        break;
    }//switch
      
}

//=======================================================================================================================
unsigned int LastSIMCardChargeRequest=4000;
unsigned char ChargeRequest=0;
void CheckGPRS(void)
{
static unsigned char cntGPRSLowError=0;
static unsigned char GSMResp=0;  
char str[60];

  //SetFgColor(RGB(255,255,255));
  //LcdFont(Tahoma16);
  //G_DrawGradianRectangle(255,255,255,84,75,80,170,25,20,1);
  //sprintf(str, "%d [D:%d,S:%d,C:%d*%d] E:%d R:%d k:%d",GPRSStatus,GPRSDeact,GPRSClose,GPRSConnecting,cntGPRSLowError,ConnectionError,GPRSRetry,GPRSCHECK);
  //PutText(4, 170,0,0, str, ALINE_LEFT);
  //G_Repaint();
  //printf("\n\r%d %d [OK:%d,D:%d,S:%d,C:%d*%d] E:%d R:%d k:%d Res:%d",CurrentConnectionStep,GPRSStatus,GPRSConnectOK,GPRSDeact,GPRSClose,GPRSConnecting,cntGPRSLowError,ConnectionError,GPRSRetry,GPRSCHECK,GSMResp);

    Log("CheckGPRSEnmter")
  LastSIMCardChargeRequest++;
	
  if (GPRSConnectOK)
  {
    Log("GPRSConnectOK")
    GSMResp=0;  
    GPRSStatus=Connected;
    GPRSRetry=0;
    GPRSConnectOK=0;
    GPRSConnecting=0;
    GPRSClose=0;
    GPRSDeact=0;
    cntGPRSLowError=0;
  }
  
  if (GPRSConnecting)
  {
    Log("GPRSConnecting")
    GSMResp=0;  
    if (++cntGPRSLowError>2)
    {
      CurrentConnectionStep=13;
      GPRSStatus=Connecting;
      cntGPRSLowError=0;
    }
    GPRSConnecting=0;
  }
  
  if (GPRSClose)
  {
    Log("GPRSClose")
    GSMResp=0;  
    CurrentConnectionStep=13;
    GPRSStatus=Connecting;
    GPRSClose=0;
  }
  
  if (GPRSDeact)
  {
    Log("GPRSDeact")
    GSMResp=0;  
    CurrentConnectionStep=14;
    GPRSStatus=Connecting;
    GPRSDeact=0;
  }

  if ((ConnectionError>3) && (cntGPRSLowError==0))
  {
    Log("cntGPRSLowError")
    CurrentConnectionStep=14;
    GPRSStatus=Connecting;
    ConnectionError=0;
  }
  
  if (GPRSStatus==Connecting)
	{
    Log("DISP3")
  	DisplayToolbar(3);
    PasssGPRSConnectionSteps();
	}
  
  if (GPRSStatus==Connected)
  {
    Log("Connected")
    if (LastSIMCardChargeRequest>3600)
    {
      Log("GetSimCardChargeInt")
      LastSIMCardChargeRequest=0;
      GPRSStatus=ChargeQuering;
      GetSimCardChargeInt(1);
    }
  }
    
  if (CUSDReady)
  {
    Log("CUSDReady")
    CUSDReady=0;
    GPRSStatus=Connected;
    Charge=GetSimCardChargeInt(2);
		//printf("\n\rNew Charge: %d", Charge);
    ChargeRequest=0;
		if (Charge>=0)
			DeviceInfo.Charge=Charge;
		#if (DeviceUC!=UC_KASHAN)
    if ((Charge>=0) && (Charge<4000))
      ChargeRequest=1;
		#endif
  }
    
  if (GPRSStatus==ChargeQuering)
  {
    Log("ChargeQuering")
    if (LastSIMCardChargeRequest>10)
    {
      GPRSStatus=Connected;
      Charge=-1;
    }
    return;
  }

  if (++GPRSCHECK==3)
    USART_SendStr(USART3, "AT+CSQ\n");
  
  if (GPRSStatus==Connected)
    if (++GPRSCHECK>6)
    {
      if (++GSMResp>3)
      {
        Log("++GSMResp>3")
        CurrentConnectionStep=90;
        GPRSStatus=Connecting;
        GSMResp=0;
      	DisplayToolbar(3);
        PasssGPRSConnectionSteps();
      }
      //GPRSCHECK=0;
    Log("USART3 AT+")
      USART_SendStr(USART3, "AT+CIPSTATUS\n");
    }
  Log("GPRSStatus")
	switch (GPRSStatus)
	{
		case Connected:	GUI_SetColor(GUI_GREEN); break;
		case ChargeQuering:	GUI_SetColor(GUI_BLUE); break;
		case Connecting:	GUI_SetColor(GUI_RED); break;
	}
	GUI_FillCircle(300, TFT_START_Y+11, 4);
  if (GPRSCHECK>6)
    GPRSCHECK=0;
}

//=======================================================================================================================
unsigned char GSMGetTime(void)
{
unsigned char CurLoc=0;
unsigned char Byte, Idx=0;
unsigned char str[128];
unsigned char Message[] ="+CCLK: \"";
const unsigned char Len=8;
unsigned long int time=0;

  USART_SendStr(USART3, "at+cclk?\n");
  
  while ((GSM_GCF(&Byte)) && (Idx<128)) 
  {
    if (CurLoc<Len)
      CurLoc=(Byte==Message[CurLoc])?CurLoc+1:0;
    else if (CurLoc==Len) CurLoc++;
    if (CurLoc>Len)
      str[Idx++] =Byte;
  }
  
  if (Idx<21) return 1;
  if ((str[2]!='/') || (str[5]!='/') || (str[8]!=',') || (str[11]!=':') || (str[14]!=':') || (str[17]!='+'))
    return 2;
  
  Year=(str[0]-'0')*10+(str[1]-'0'); Year+=2000;
  Month=(str[3]-'0')*10+(str[4]-'0');
  Day=(str[6]-'0')*10+(str[7]-'0');
  Hour=(str[9]-'0')*10+(str[10]-'0');
  Min=(str[12]-'0')*10+(str[13]-'0');
  Sec=(str[15]-'0')*10+(str[16]-'0');
  
  gregorian_to_jalali(Year,Month,Day,&CurrentDate);
  Year=CurrentDate.year;
  Month=CurrentDate.month;
  Day=CurrentDate.day;
  DayOfWeek=GetDayOfWeek(Year,Month,Day);
  return 0;
}

//==================================================================================
unsigned char GSMSetTime(void)
{
char str[100];
unsigned char Byte;

  jalali_to_gregorian(Year,Month,Day,&CurrentDate);
  DayOfWeek=GetDayOfWeek(Year,Month,Day);
  sprintf(str,"at+cclk=\"%1d%1d/%1d%1d/%1d%1d,%1d%1d:%1d%1d:%1d%1d+00\"\n",(CurrentDate.year/10)%10,CurrentDate.year%10,
                                                                           CurrentDate.month/10,CurrentDate.month%10,
                                                                           CurrentDate.day/10,CurrentDate.day%10,
                                                                           Hour/10,Hour%10,Min/10,Min%10,Sec/10,Sec%10);
  USART_SendStr(USART3, str);
  if (GetGSMResponse("OK")) return 2;
  
  return 0;
}

//==================================================================================
unsigned char _IsPrintable(unsigned char ch)
{
  if (ch<32) return 0;
  if (ch>200) return 0;
  return 1;
}
//==================================================================================
void GetGPRSStatus(void)
{
int i, k, ReceiveBufLen;
unsigned char Byte,temp;
unsigned char ReceiveBuf[100];

  USART_SendStr(USART3, "at+cipstatus\n");
  ReceiveBufLen=0;
  if (GSM_GCL(&Byte))
    ReceiveBuf[ReceiveBufLen++] =Byte;  
  else 
    return;
      
  while ((GSM_GCF(&Byte)) && (ReceiveBufLen<70)) 
    ReceiveBuf[ReceiveBufLen++] =Byte;

  ReceiveBuf[ReceiveBufLen] =0; 
  
}

//==============================================================================
unsigned char Get_GPRSStatusText(unsigned char *Status)
{
int i, k, ReceiveBufLen;
unsigned char Byte,temp;
unsigned char ReceiveBuf[100];

  USART_SendStr(USART3, "at+cipstatus\n");
  ReceiveBufLen=0;
  if (GSM_GCL(&Byte))
    ReceiveBuf[ReceiveBufLen++] =Byte;  
  else 
    return 0;
      
  while ((GSM_GCF(&Byte)) && (ReceiveBufLen<70)) 
    ReceiveBuf[ReceiveBufLen++] =Byte;

  ReceiveBuf[ReceiveBufLen] =0; 
  
  Byte=0; 
  for (k=0,i=0; i<ReceiveBufLen; i++)
  {
    if ((Byte==1)&&_IsPrintable(ReceiveBuf[i]))
      Status[k++] =ReceiveBuf[i];
    if (ReceiveBuf[i] ==':')
    {
      Byte=1;
      if ((ReceiveBufLen-i)>16) ReceiveBufLen=i+16;
      i++;
    }  
  }  
  Status[k++] =0;
  
  return 1;
}

//================================================================================================
unsigned char GetSimCardCharge(void)
{                         
unsigned char i=0, j=0, Byte,k=0;
unsigned char Idx;
unsigned long int Li;
    
  EmptyGSMRXBuffer();
  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

	switch (Config.SIMCardType)
  {
    case 2: //MCI
    case 20:
      USART_SendStr(USART3, "atd*140*11#\n");
      break;			
    case 3: //Rightel
      USART_SendStr(USART3, "atd*140#\n");
      break;			
    default: //Irancell
      USART_SendStr(USART3, "atd*141*1#\n");
      break;			
  }

  if (GetGSMResponse("OK")) return 3;
  
	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
  }
  
  j=0;
  i=0;
  k=0;
  Idx=0;
  while (GSM_GCL(&Byte) && (Idx<220)) 
  {  
   if (k++<10) continue;
   if (!_IsPrintable(Byte)) continue;
    if (++j>17)
    {
      if (++i>7) break;
      j=0;
    }
    Idx++;
  }
  
  if (Idx==0) return 4;

	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
    i=ScanKeyboard();
    if ((i==BOK) || (i==BCANCEL)) break;
  }                    
                      
}

//================================================================================================
unsigned char ChargeSimCard(unsigned char JustChargeQuery)
{                         
unsigned char i=0, j=0, Byte,Idx,k=0,ResText[200],m;
unsigned long int Li,NewCharge;
unsigned char  NumberDone=0, RialPosition=0,Temp[60];
signed long int N;
char Charge[20];

  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

  if (JustChargeQuery==0)
    if (GetChargeCode()==0) 
			return 1;  
  
	switch (Config.SIMCardType)
  {
    case 2: //MCI
    case 20:
      USART_SendStr(USART3, "atd*140*#");
      break;			
    case 3: //Rightel
      USART_SendStr(USART3, "atd*141*");
      break;			
    default: //Irancell
      USART_SendStr(USART3, "atd*141*");
      break;			
  }
  
  if (JustChargeQuery==0)
  {
    for (i=0; (i<16) && (ChargeCode[i]!=0); i++) 
      USART_SendData(USART3,ChargeCode[i]); 
  }else{
    USART_SendData(USART3,'1'); 
  }
  USART_SendData(USART3,'#'); 
  USART_SendData(USART3,13); 
  if (GetGSMResponse("OK")) return 3;    
  
	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
  }
  
  j=0;
  i=0;
  k=0;
  Idx=4;
  m=0;
  while (GSM_GCL(&Byte) && (Idx<200)) 
  {  
    if (k++<10) continue;
    if (!_IsPrintable(Byte)) continue;
    ResText[m++] =Byte;
    ResText[m] =0;
    if (++j>17)
    {
      if (++i>7) break;
      j=0;
    }
    Idx++;
  }
  
  if (Idx==4) return 4;
  
  //G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
  //SetFgColor(WHITE);
  //LcdFont(Tahoma16);
  //PutText(0,30,320,0,ResText,ALINE_LEFT);  
  //G_Repaint();

 j=0;
 i=0;
 NewCharge=0;
 while ((ResText[i]!=0) && (j<20))
 {
   Byte=ResText[i];
   i++;
    switch (Byte)
      {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':  
          if (NumberDone==1)
          {
            NumberDone=0;
            j=0;
          }  
          Charge[j++] =Byte;
          break;                 
        case 'R':
        case 'r':
          RialPosition=1;
          break;  
        case 'I':
        case 'i':
          if (RialPosition==1)
            RialPosition=2;
          else
            RialPosition=0;
          break;  
        case 'A':
        case 'a':
          if (RialPosition==2)
            RialPosition=3;
          else
            RialPosition=0;
          break;  
        case 'L':
        case 'l':
          if (RialPosition==3)
            RialPosition=4;
          else
            RialPosition=0;
          break;          
        default:
          RialPosition=0;
          NumberDone=1;
          break;
      }//switch

      if (RialPosition==4)
      {
        Charge[j] =0;
        NewCharge=atol(Charge);
      }  
 }

  //GetTimeStamp();
  //Li=timestamp; 
  while (1)                
  {
    i=ScanKeyboard();
    //GetTimeStamp();
    //if (timestamp-Li>60000) {if (NewCharge)  return 5;}
    if ((i==BOK) || (i==BCANCEL) || (i==120) || (i==BLEFT) || (i=='A')) break;
  } 

  if (NewCharge) return 5;  
  return 4;

}  

//=========================================================================
unsigned char GetSimCardChargeAuto(void)
{                         
unsigned char Idx, i=0, j=0, Byte,SendBuf[250];
unsigned long int Li;
unsigned int crc=0;

  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

	switch (Config.SIMCardType)
  {
    case 2: //MCI
    case 20:
      USART_SendStr(USART3, "atd*140*11#\n");
      break;			
    case 3: //Rightel
      USART_SendStr(USART3, "atd*140#\n");
      break;			
    default: //Irancell
      USART_SendStr(USART3, "atd*141*1#\n");
      break;			
  }
  if (GetGSMResponse("OK")) return 3;      
  
	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
  }

  SendBuf[0] =STX;
  SendBuf[1] =11; //Result
  SendBuf[2] =Config.DeviceID%256;
  SendBuf[3] =Config.DeviceID/256;
  SendBuf[4] =0;
  SendBuf[5] =0;
  SendBuf[6] =30;
  Idx=7;   

  while (GSM_GCL(&Byte) && (Idx<200)) 
  {  
   if (j++<10) continue;
   if (!_IsPrintable(Byte)) continue;
   SendBuf[Idx++] =Byte;
  }
  
  SendBuf[4] =(Idx-6)%256;
  SendBuf[5] =(Idx-6)/256;
  //crc=crc16(0, SendBuf+1, Idx-1);
  SendBuf[Idx++] =crc%256;
  SendBuf[Idx++] =crc/256;
  SendBuf[Idx++] =ETX;

  switch (ActivePort)
  {
    case 0:
      for (i=0; i<Idx; i++)
        USART_SendData(USART1, SendBuf[i]);
      break;
    case 1:
      GPRSSend(Idx, SendBuf);
      break;  
  }
  OkBip(1);
}

//=========================================================================
unsigned int ExecuteCommand(void)
{               
unsigned int SIMCharge=0;
char Charge[20];
unsigned char Idx, i=0, Byte;
unsigned long int Li;
unsigned int crc=0;
unsigned char j=0, NumberDone=0, RialPosition=0;
unsigned char SendBuf[300];

  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

  for (i=0; (i<50) && (ChargeCode[i]!=0); i++) 
     USART_SendData(USART3,ChargeCode[i]); 
  if (GetGSMResponse("OK")) return 3;
    
	Li=OS_TimeMS;
  while (1)
  {
    if (rxd3_counter>10)
      break;
  	if (Li>OS_TimeMS) 
			return 4;
	  if (OS_TimeMS-Li>=6000) //6 sec
			return 5;
		GUI_Delay(1);
  }
  if (rxd3_counter==0)
    return 6;
  
  SendBuf[0] =STX;
  SendBuf[1] =11; //Result
  SendBuf[2] =Config.DeviceID%256;
  SendBuf[3] =Config.DeviceID/256;
  SendBuf[4] =0;
  SendBuf[5] =0;
  SendBuf[6] =32;
  Idx=7;   
  j=0;
  while (GSM_GCF(&Byte) && (Idx<220)) 
  {  
    switch (Byte)
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':  
        if (NumberDone==1)
        {
          NumberDone=0;
          j=0;
        }  
        Charge[j++] =Byte;
        break;                 
      case 'R':
      case 'r':
        RialPosition=1;
        break;  
      case 'I':
      case 'i':
        if (RialPosition==1)
          RialPosition=2;
        else
          RialPosition=0;
        break;  
      case 'A':
      case 'a':
        if (RialPosition==2)
          RialPosition=3;
        else
          RialPosition=0;
        break;  
      case 'L':
      case 'l':
        if (RialPosition==3)
          RialPosition=4;
        else
          RialPosition=0;
        break;          
      default:
        RialPosition=0;
        NumberDone=1;
        break;
    }//switch

    if ((RialPosition==4) && (SIMCharge==0))
    {
      Charge[j] =0;
      SIMCharge=atol(Charge);
    }  
    if (!_IsPrintable(Byte)) continue;
    SendBuf[Idx++] =Byte;
  }
  
  SendBuf[4] =(Idx-6)%256;
  SendBuf[5] =(Idx-6)/256;
  //crc=crc16(0, SendBuf+1, Idx-1);
  SendBuf[Idx++] =crc%256;
  SendBuf[Idx++] =crc/256;
  SendBuf[Idx++] =ETX;
  switch (ActivePort)
  {
    case 0:
      for (i=0; i<Idx; i++)
        USART_SendData(USART1, SendBuf[i]);
      break;
    case 1:
      GPRSSend(Idx, SendBuf);
      break;  
  }
  OkBip(1);
	/*
  if (SIMCharge>10)
  {
    SaveFromRamToDF(addIndicators+20+20, 49, ChargeCode);
    SaveFromRamToDF(addIndicators+20+20+50, Idx-7, &SendBuf[7]);
  }
	*/
  return SIMCharge;

}

//=========================================================================
unsigned char ChargeSimCardAuto(void)
{                         
unsigned char Idx, i=0, j=0, Byte,SendBuf[255];
unsigned long int Li;
unsigned int crc=0;

  USART_SendStr(USART3, "at\n");
  if (GetGSMResponse("OK")) return 1;
  USART_SendStr(USART3, "at+creg?\n"); // BAYAD BA HOROOFE KOOCHAK BASHAD
  if (GetGSMResponse("CREG: 0,1")) return 2;  

	switch (Config.SIMCardType)
  {
    case 2: //MCI
    case 20:
      USART_SendStr(USART3, "atd*140*#");
      break;			
    case 3: //Rightel
      USART_SendStr(USART3, "atd*141*");
      break;			
    default: //Irancell
      USART_SendStr(USART3, "atd*141*");
      break;			
  }
  
  for (i=0; (i<12) && (ChargeCode[i]!=0); i++) 
    USART_SendData(USART3,ChargeCode[i]); 
  USART_SendData(USART3,'#'); 
  USART_SendData(USART3,13);   
  if (GetGSMResponse("OK")) return 3;      

	i=OS_TimeMS;
  while (1)
  {
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=6000)
			return 0;
    if (rxd3_counter!=0) 
      break;
		GUI_Delay(1);
  }
 
  SendBuf[0] =STX;
  SendBuf[1] =11; //Result
  SendBuf[2] =Config.DeviceID%256;
  SendBuf[3] =Config.DeviceID/256;
  SendBuf[4] =0;
  SendBuf[5] =0;
  SendBuf[6] =31;
  Idx=7;   

  while (GSM_GCL(&Byte) && (Idx<220)) 
  {  
   if (j++<10) continue;
   if (!_IsPrintable(Byte)) continue;
    SendBuf[Idx++] =Byte;
  }
  
  SendBuf[4] =(Idx-6)%256;
  SendBuf[5] =(Idx-6)/256;
  //crc=crc16(0, SendBuf+1, Idx-1);
  SendBuf[Idx++] =crc%256;
  SendBuf[Idx++] =crc/256;
  SendBuf[Idx++] =ETX;
  switch (ActivePort)
  {
    case 0:
      for (i=0; i<Idx; i++)
        USART_SendData(USART1, SendBuf[i]);
      break;
    case 1:
      GPRSSend(Idx, SendBuf);
      break;  
  }
  OkBip(1);
  return 0;
}

//=========================================================================
signed long int GetSimCardChargeInt(unsigned char background)
{                         
unsigned char j=0, Byte, NumberDone=0, RialPosition=0;
signed long int N;
unsigned long int Li;
char Charge[20];
  
  switch (background)
  {
    case 0:
      EmptyGSMRXBuffer();
    	switch (Config.SIMCardType)
      {
        case 2: //MCI
        case 20:
          USART_SendStr(USART3, "atd*140*11#\n");
          break;			
        case 3: //Rightel
          USART_SendStr(USART3, "atd*140#\n");
          break;			
        default: //Irancell
          USART_SendStr(USART3, "atd*141*1#\n");
          break;			
      }
      if (GetGSMResponse("OK")) return -1;  
      break;
    case 1:  
      EmptyGSMRXBuffer();
    	switch (Config.SIMCardType)
      {
        case 2: //MCI
        case 20:
          USART_SendStr(USART3, "atd*140*11#\n");
          break;			
        case 3: //Rightel
          USART_SendStr(USART3, "atd*140#\n");
          break;			
        default: //Irancell
          USART_SendStr(USART3, "atd*141*1#\n");
          break;			
      }
      if (GetGSMResponse("OK")) return -1;  
      if (background==1)
        return -1;
      break;
    case 2:  
      break;
  }
  
	Li=OS_TimeMS;
  while (background==0)
  {
    if (rxd3_counter>10)
      break;
  	if (Li>OS_TimeMS) 
			return 0;
	  if (OS_TimeMS-Li>=6000) //6 sec
			return 0;
    Byte=ScanKeyboard();
    if (Byte==BLEFT) 
    {
      EmptyGSMRXBuffer();
      return -1;
    }
  }
  //printf("\n\rReceived: ");/////////////
  j=0;
  while ((GSM_GCF(&Byte)) && (j<20)) 
  { 
    //printf("%c",Byte);/////////////  
    switch (Byte)
      {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':  
          if (NumberDone==1)
          {
            NumberDone=0;
            j=0;
          }  
          Charge[j++] =Byte;
          break;                 
        case 'R':
        case 'r':
          RialPosition=1;
          break;  
        case 'I':
        case 'i':
          if (RialPosition==1)
            RialPosition=2;
          else
            RialPosition=0;
          break;  
        case 'A':
        case 'a':
          if (RialPosition==2)
            RialPosition=3;
          else
            RialPosition=0;
          break;  
        case 'L':
        case 'l':
          if (RialPosition==3)
            RialPosition=4;
          else
            RialPosition=0;
          break;          
        default:
          RialPosition=0;
          NumberDone=1;
          break;
      }//switch
      
      if (RialPosition==4)
      {
        Charge[j] =0;
        N=atol(Charge);
        return N;
      }  
  }//for    
  EmptyGSMRXBuffer();
  return -1;
                      
}
