#include <includes.h>
#include <usart/usart.h>
#include <config.h>

#ifdef WithSAMCARD

unsigned char ta1, td1, tc3;
extern unsigned int rxd2_counter;
extern unsigned int rxd2_wr_index;
extern unsigned int rxd2_rd_index;

#define BUFFER2_SIZE         250
extern unsigned char RXD2Buffer[BUFFER2_SIZE];


//=============================================================================
void SAM_EmptyRXBuffer(void)
{
unsigned char c;
unsigned long int i;
  /*
	i=OS_TimeMS;
	while (1)
	{
		if (USART2->SR & USART_FLAG_RXNE)
		{
  	  USART2->SR &= ~USART_FLAG_RXNE;
      c=(USART2->DR & 0x1FF);
			i=OS_TimeMS;
		}
  	if (i>OS_TimeMS) return;
	  if (OS_TimeMS-i>=5) 
			return;
	}
  */
	rxd2_rd_index=rxd2_counter=rxd2_wr_index=0;
}

//=============================================================================
unsigned char SAM_GCL(unsigned char *c)
{
unsigned long int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=2000) //5 Sec
			return 0;
    if (rxd2_counter != 0) 
      break;
		GUI_Delay(1);
  	WDTR;
  }		
  *c=RXD2Buffer[rxd2_rd_index];
  if (++rxd2_rd_index >= BUFFER2_SIZE) rxd2_rd_index=0;
  if (rxd2_counter) rxd2_counter--;
  return 1;

  /*
  i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=2000)
			return 0;
    if (USART2->SR & USART_FLAG_RXNE) 
		{
			USART2->SR &= ~USART_FLAG_RXNE;
      *c=(USART2->DR & 0x1FF);
			return 1;
		}
  	WDTR;
  }	
  */
}

//=============================================================================
unsigned char SAM_GCF(unsigned char *c)
{
unsigned long int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=300) 
			return 0;
    if (rxd2_counter != 0) 
      break;
		GUI_Delay(1);
  	WDTR;
  }		
  *c=RXD2Buffer[rxd2_rd_index];
  if (++rxd2_rd_index >= BUFFER2_SIZE) rxd2_rd_index=0;
  if (rxd2_counter) rxd2_counter--;
  return 1;

	/*
	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=50) //5 Sec
			return 0;
    if (USART2->SR & USART_FLAG_RXNE) 
		{
			USART2->SR &= ~USART_FLAG_RXNE;
      *c=(USART2->DR & 0x1FF);
			return 1;
		}
  	WDTR;
  }		
	*/
}

//=============================================================================
void SAM_SendByte(unsigned char c)
{
  USART_SendData(USART2, c);
}

//=============================================================================
void SmartCardUsartEnable(unsigned char pinConfig){
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
USART_ClockInitTypeDef USART_ClockInitStructure; 
GPIO_InitTypeDef GPIO_InitStructure;
	 
  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
  USART_DeInit(USART2);   
	
  if (pinConfig)
	{
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	 //SAM CLK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  //SAM RST
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	 
	}
	
	USART_InitStructure.USART_BaudRate = 8064;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  /* SC_USART Clock set to 4.5MHz (PCLK1 = 36 MHZ / 8) */
  USART_SetPrescaler(USART2, 6);//6
  /* SC_USART Guard Time set to 2 Bit */
  USART_SetGuardTime(USART2, 2);//16
  /* Enable the NACK Transmission */
	
  USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
  USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
  USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
  USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
  USART_ClockInit(USART2, &USART_ClockInitStructure);	

  USART_Init(USART2, &USART_InitStructure);
 
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
  USART_Cmd(USART2, ENABLE);    

  /* Enable the NACK Transmission */
  USART_SmartCardNACKCmd(USART2, ENABLE);

  /* Enable the Smartcard Interface */
  USART_SmartCardCmd(USART2, ENABLE);
	
	SAM_EmptyRXBuffer();
}

//==============================================================================
void ISO7816_Datablock_ATR( unsigned char* p_atr, unsigned char* p_length )
{
unsigned int i;
unsigned int j;
unsigned char uc_value;

	*p_length = 0;
   
	ta1=0;
	td1=0;
	tc3=0;

	/* Read ATR TS. */
	SAM_GCL( &p_atr[0]);

	/* Read ATR T0. */
	SAM_GCF( &p_atr[1]);

	uc_value = p_atr[1] & 0xF0;
	i = 2;

	/* Read ATR T1. */
	if (uc_value & 0x10) { /* TA[1] */
		SAM_GCF( &p_atr[i++]);
		ta1 = p_atr[2];
	}

	if (uc_value & 0x20) { /* TB[1] */
		SAM_GCF( &p_atr[i++]);
	}

	if (uc_value & 0x40) { /* TC[1] */
		SAM_GCF( &p_atr[i++]);
	}

	if (uc_value & 0x80) { /* TD[1] */
		SAM_GCF( &p_atr[i]);
		td1 = p_atr[i];
		uc_value = p_atr[i++] & 0xF0;
	} else {
		uc_value = 0;
	}

	if (uc_value) {
		if (uc_value & 0x10) { /* TA[2] */
			SAM_GCF( &p_atr[i++]);
		}

		if (uc_value & 0x20) { /* TB[2] */
			SAM_GCF( &p_atr[i++]);
		}

		if (uc_value & 0x40) { /* TC[2] */
			SAM_GCF( &p_atr[i++]);
		}

		if (uc_value & 0x80) { /* TD[2] */
			SAM_GCF( &p_atr[i]);
			uc_value = p_atr[i++] & 0xF0;
		} else {
			uc_value = 0;
		}
	}

	if (uc_value) {
		if (uc_value & 0x10) { /* TA[3] */
			SAM_GCF( &p_atr[i++]);
		}

		if (uc_value & 0x20) { /* TB[3] */
			SAM_GCF( &p_atr[i++]);
		}

		if (uc_value & 0x40) { /* TC[3] */
			SAM_GCF( &p_atr[i++]);
			tc3 = p_atr[i - 1];
		}

		if (uc_value & 0x80) { /* TD[3] */
			SAM_GCF( &p_atr[i]);
			uc_value = p_atr[i++] & 0xF0;
		} else {
			uc_value = 0;
		}
	}

	/* Historical Bytes. */
	uc_value = p_atr[1] & 0x0F+1;
	for (j = 0; j < uc_value; j++) {
		SAM_GCF( &p_atr[i++]);
	}

	*p_length = i;
        
	/*
    printf("Length = %d . (%d-%d-%d)", i,ta1,td1,tc3);
    printf("ATR = ");

    for (j=0; j < i; j++) {
        printf("%02x ", p_atr[j]);
    }
*/
}

//==============================================================================
unsigned char ISO7816_Decode_ATR( unsigned char* p_atr )
{
	unsigned int ftable[16] = {0, 372, 558, 744, 1116, 1488, 1860, 0,
			       0, 512, 768, 1024, 1536, 2048, 0, 0};
	unsigned int dtable[10] = {0, 1, 2, 4, 8, 16, 32, 64, 12, 20};
	unsigned char ppss[4];

	unsigned int i;
	unsigned int j;
	unsigned int y;
	unsigned char uc_offset;
	unsigned int fidi;

	i = 2;
	y = p_atr[1] & 0xF0;

	/* Read ATR Ti. */
	uc_offset = 1;

	if (y & 0x10) {
		while (y) {
			if (y & 0x10) { /* TA[i] */
				i++;
			}

			if (y & 0x20) { /* TB[i] */
				i++;
			}

			if (y & 0x40) { /* TC[i] */
				i++;
			}

			if (y & 0x80) { /* TD[i] */
				y = p_atr[i++] & 0xF0;
			} else {
				y = 0;
			}

			uc_offset++;
		}
	}

	y = p_atr[1] & 0x0F;
	for (j = 0; j < y; j++) {
		i++;
	}

	if (td1 & 0x10) {
		return 0;
	}

	if (ta1 && ((ta1 & 0xf) != 0)) {
		fidi = ftable[(ta1 >> 4) & 0xf] / dtable[ta1 & 0xf];
	}

	if (td1 & 0xf) {
		return (td1 & 0xf);
	}

	SAM_SendByte( 0xff);
	SAM_SendByte( 0x10);
	SAM_SendByte( ta1);
	SAM_SendByte( 0xff ^ 0x10 ^ ta1);

	SAM_GCF(&ppss[0]);
	SAM_GCF(&ppss[1]);
	SAM_GCF(&ppss[2]);
	SAM_GCF(&ppss[3]);

	if ((ppss[0] != 0xff) || (ppss[1] != 0x10)) {
		return 0;
	}

	if (ppss[2] != ta1) {
		if (ppss[3] == (0xff ^ 0x10 ^ ppss[2])) {
			fidi = ftable[(ppss[3] >>
					4) & 0xf] / dtable[ppss[3] & 0xf];
		} else {
			return 0;
		}
	}

	return 0;
}

//==============================================================================
unsigned short SAM_SendAPDU_T0_Send(const unsigned char *pAPDU, 
                                        unsigned short wLength)
{
unsigned char i;
	
  //printf("\n\r[S>] ");
  //for (i=0; i<wLength; i++)
  //  printf("%X ", pAPDU[i]);
	
  for (i=0; i<5; i++)
    USART_SendData(USART2, pAPDU[i]);  
	GUI_Delay(10);//50
  for (i=5; i<wLength; i++)
	  USART_SendData(USART2, pAPDU[i]);
}

//==============================================================================
unsigned short SAM_SendAPDU_T0(const unsigned char *pAPDU, 
                                        unsigned char *pMessage, 
                                        unsigned short wLength, unsigned short wRxLength)
{
unsigned short NeNc;
unsigned char Byte, i, j;
unsigned char sendstr[256];
unsigned char index;
unsigned long int Li;
	
  SAM_EmptyRXBuffer();
	//printf("\n\rSendDate: ");
	SAM_SendAPDU_T0_Send(pAPDU, wLength);
  
  index=0;
	Li=OS_TimeMS;
	while (1)
	{
  	if (Li>OS_TimeMS) return 0;
	  if (OS_TimeMS-Li>=1000)
		{		
      //printf("\n\rTimeOut RxC: %d Data: %d",rxd2_wr_index,wRxLength+wLength);			
			return 0;
		}
    if (rxd2_wr_index>wRxLength+wLength) 
		{
			for (index=0; index<=wRxLength+wLength; index++)
        sendstr[index] =RXD2Buffer[rxd2_rd_index++];
  	  break;
		}
		GUI_Delay(1);
  	WDTR;
  }

  //printf("\n\rXXX: ");
  
	//for (i=0; i<index; i++)
    //SendByte(sendstr[i]);
	
  //printf("\n\r[S<] ");
	
	j=0;
	i=wLength+1;
  for (; i<index; i++)
  {
    pMessage[j++] =sendstr[i];
    //printf("%X ", sendstr[i]);
  }

  return j;
}

//=============================================================================
void SAM_Init(void)
{
unsigned char p_atr[256];
unsigned char i, size;
unsigned char ApduCmd[100] ={0};
unsigned char ResBuff[256] ={0};
	
	SmartCardUsartEnable(1);
	GUI_Delay(500);
	SAM_EmptyRXBuffer();
	
  SAM_RST = 0;
	GUI_Delay(100);
	SAM_RST = 1;
  
 // ISO7816_Datablock_ATR(p_atr, &size);

	GUI_Delay(2000);
  SAM_EmptyRXBuffer();

  /*
  printf("-I- ATR: ");
  for (i = 0; i < size; i++) 
    printf("0x%02X ", p_atr[i]);
  printf("\n\r");
  */
  //size = ISO7816_Decode_ATR(p_atr);

  //printf("-I- Card type: T= %d\n\r", size);



  //GUI_Delay(5000);

  //uart2_init(9600);
	
}

#endif