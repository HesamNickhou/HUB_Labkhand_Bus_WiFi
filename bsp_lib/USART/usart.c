#include "sys.h"
#include "usart.h"	  
#include "includes.h"				
#include "config.h"				

#pragma import(__use_no_semihosting)             
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
FILE __stdout;       
int _sys_exit(int x) { x = x; } 

int fputc(int ch, FILE *f) {      
	while ((USART1->SR & 0X40) == 0);
	USART1->DR = (u8) ch;
	return ch;
}

unsigned int CheckSum=0;
volatile unsigned int rxd_counter;
volatile unsigned int rxd_wr_index;
volatile unsigned int rxd_rd_index;
unsigned char RXDBuffer[BUFFER_SIZE];

//=============================================================================
void SendByteE(unsigned char c)
{
  CheckSum+=c;
  USART_SendData(USART1, c);
}

//=============================================================================
void SendByte(unsigned char c)
{
  USART_SendData(USART1, c);
}

//=============================================================================
void EmptyRXBuffer(void)
{
  rxd_rd_index=rxd_counter=rxd_wr_index=0;
}

//=============================================================================
unsigned char WaitForC(void)
{
unsigned int i;
	
	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=500) //5 Sec
			return 0;
    if (rxd_counter!=0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  return 1;
}

//=============================================================================
unsigned char USART_GCL(unsigned char *c)
{
unsigned int i;
	
	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=5000) //5 Sec
			return 0;
    if (rxd_counter!=0) 
      break;
  	WDTR;
}		
  *c=RXDBuffer[rxd_rd_index];
  if (++rxd_rd_index >= BUFFER_SIZE) rxd_rd_index=0;
  if (rxd_counter) rxd_counter--;
  return 1;
}

//=============================================================================
unsigned char USART_GCM(unsigned char *c)
{
unsigned int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=2000) //2 sec
			return 0;
    if (rxd_counter!=0) 
      break;
		WDTR;
	}		
  *c=RXDBuffer[rxd_rd_index];
  if (++rxd_rd_index >= BUFFER_SIZE) rxd_rd_index=0;
  if (rxd_counter) rxd_counter--;
  return 1;
}

//=============================================================================
unsigned char USART_GCF(unsigned char *c)
{
unsigned int i;

	i=OS_TimeMS;
	while (1)
	{
  	if (i>OS_TimeMS) return 0;
	  if (OS_TimeMS-i>=100) //100 msec
			return 0;
    if (rxd_counter!=0) 
      break;
		WDTR;
	}		
  *c=RXDBuffer[rxd_rd_index];
  if (++rxd_rd_index >= BUFFER_SIZE) rxd_rd_index=0;
  if (rxd_counter) rxd_counter--;
  return 1;
}

//=============================================================================
void USART1_IRQHandler(void)  
{
volatile unsigned int IIR;
u8 Byte;
	
  IIR = USART1->SR;
  if (IIR & USART_FLAG_ORE) 
	{        
    USART1->SR &= ~USART_FLAG_ORE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_TXE) 
	{        
    USART1->SR &= ~USART_FLAG_TXE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_RXNE) 
	{     
    //USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
		Byte=(USART1->DR & 0x1FF);
    RXDBuffer[rxd_wr_index] =Byte;
    if (++rxd_wr_index >= BUFFER_SIZE) rxd_wr_index=0;
    USART1->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
		//USART_SendData(USART2, Byte);
    if (++rxd_counter >= BUFFER_SIZE)
    {
       rxd_rd_index=0;
			 rxd_counter=0;
			 rxd_wr_index=0;
    };
    //USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  } 
} 
									 
//=============================================================================
void uart_init(u32 bound){
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);                    

}

//=============================================================================
void uart2_init(u32 bound){
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
  USART_DeInit(USART2);   
  USART_Cmd(USART2, DISABLE);   
  USART_SmartCardNACKCmd(USART2, DISABLE);
  USART_SmartCardCmd(USART2, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
  USART_Cmd(USART2, ENABLE);                    

}

//=============================================================================
void uart3_init(u32 bound){
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStruct;

  /** USART3 GPIO Configuration  
  PB10   ------> USART3_TX
  PB11   ------> USART3_RX
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
	
  //USART_Cmd(USART3, DISABLE);                    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART3, &USART_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
  USART_Cmd(USART3, ENABLE);                    

}
