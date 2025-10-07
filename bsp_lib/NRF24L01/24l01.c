#include "24l01.h"
#include "config.h"
#include <includes.h>

u8 TX_ADDRESS[TX_ADR_WIDTH] ={0x34,0x43,0x10,0x10,0x01}; 
u8 RX_ADDRESS[RX_ADR_WIDTH] ={0x34,0x43,0x10,0x10,0x01}; 

#define NRF24L01_SCK    PCout(4)
#define NRF24L01_CSN    PFout(11)
#define NRF24L01_MOSI   PAout(7)
#define NRF24L01_CE     PAout(8)
#define NRF24L01_MISO   PAin(6)
#define NRF24L01_IRQ    PBin(0)

unsigned char NRF24L01_SPI(unsigned char data)
{
unsigned char count=0; 	  
unsigned char Num=0;
	
	NRF24L01_SCK=0;	
	NRF24L01_MOSI=0;
	for (count=0;count<8;count++)  
	{ 	  
		if (data&0x80)  {NRF24L01_MOSI=1;  }
		else           {NRF24L01_MOSI=0;  }
		
		data<<=1;    
		NRF24L01_SCK=1; 	 
		Num<<=1; 	 
		if (NRF24L01_MISO) Num++; 		 
		NRF24L01_SCK=0;		       
	}		 			    
	//Num>>=4;
	return Num; 
}

//========================================================================
void NRF24L01_Init(void)
{ 	
GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOC, &GPIO_InitStructure);			

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOF, &GPIO_InitStructure);			
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);			
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOB, &GPIO_InitStructure);			

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);			
	

	NRF24L01_CE=0; 		
	NRF24L01_CSN=1;		
		GUI_Delay(1000);
}

//========================================================================
unsigned char NRF24L01_Check(void)
{
 unsigned char buf[5] ={0XA5,0XA5,0XA5,0XA5,0XA5};
 unsigned char i;

 //NRF24L01_SPI1_SetSpeed(NRF24L01_SPI_BaudRatePrescaler_4); 
 NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,buf,5);
 NRF24L01_Read_Buf(TX_ADDR,buf,5); 
 printf("\n\rbuf[0--4] =%x-%x-%x-%x-%x",buf[0],buf[1],buf[2],buf[3],buf[4]);
	for (i=0;i<5;i++)  if (buf[i]!=0XA5)break;	 							   
 if (i!=5)  return 1;
 return 0;		 
}

//========================================================================
unsigned char NRF24L01_Write_Reg(unsigned char reg,unsigned char value)
{
unsigned char status;	
 	NRF24L01_CSN=0;                 
 	status =NRF24L01_SPI(reg);
 	NRF24L01_SPI(value);      
 	NRF24L01_CSN=1;                 
 	return status;       			
}

//========================================================================
unsigned char NRF24L01_Read_Reg(unsigned char reg)
{
unsigned char reg_val;	
	
 	NRF24L01_CSN=0;          
 	NRF24L01_SPI(reg);   
 	reg_val=NRF24L01_SPI(0XFF);
 	NRF24L01_CSN=1;       
 	return reg_val;        
}	

//========================================================================
unsigned char NRF24L01_Read_Buf(unsigned char reg,unsigned char*pBuf,unsigned char len)
{
unsigned char status,u8_ctr;	       
 	NRF24L01_CSN=0;          
 	status=NRF24L01_SPI(reg);
 	for (u8_ctr=0;u8_ctr<len;u8_ctr++) pBuf[u8_ctr] =NRF24L01_SPI(0XFF);
 	NRF24L01_CSN=1;       
 	return status;       
}

//========================================================================
unsigned char NRF24L01_Write_Buf(unsigned char reg, unsigned char*pBuf, unsigned char len)
{
unsigned char status,u8_ctr;	    
 	NRF24L01_CSN=0;       
 	status = NRF24L01_SPI(reg);
 	for (u8_ctr=0; u8_ctr<len; u8_ctr++)  NRF24L01_SPI(*pBuf++);
 	NRF24L01_CSN=1;     
 	return status;        
}				   

//========================================================================
unsigned char NRF24L01_TxPacket(unsigned char * txbuf)
{
unsigned char sta;

 	//NRF24L01_SPI1_SetSpeed(NRF24L01_SPI_BaudRatePrescaler_8);

	NRF24L01_CE=0;
  NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
 	NRF24L01_CE=1;
	
	while (NRF24L01_IRQ!=0);
	//HAL_Delay(50);
	
	sta=NRF24L01_Read_Reg(STATUS); 
	NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta);
	
	if (sta&MAX_TX)
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);
		return MAX_TX; 
	}
	if (sta&TX_OK)
	{
		return TX_OK;
	}
	return 0xff;
}

//========================================================================
unsigned char  NRF24L01_RxPacket(unsigned char * rxbuf)
{
unsigned char sta;		    							   
	
	//NRF24L01_SPI1_SetSpeed(NRF24L01_SPI_BaudRatePrescaler_8); 
	sta=NRF24L01_Read_Reg(STATUS); 
	NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta);
//printf(" sta=%x ",sta);	
	if (sta&RX_OK)
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);
		NRF24L01_Write_Reg(FLUSH_RX,0xff);
		return 0; 
	}	   
	return 1;
}			

//========================================================================
void NRF24L01_RX_Mode(unsigned char *RxAddress)
{
	NRF24L01_CE=0;	  
	RX_ADDRESS[0] =RxAddress[0];
	RX_ADDRESS[1] =RxAddress[1];
	RX_ADDRESS[2] =RxAddress[2];
	RX_ADDRESS[3] =RxAddress[3];
	RX_ADDRESS[4] =RxAddress[4];

 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)RX_ADDRESS,RX_ADR_WIDTH);
	  
 	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);    
 	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);
 	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);	
 	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);
 	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);
 	NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG, 0x0f);
 	NRF24L01_CE=1; 
}						 

//========================================================================
void NRF24L01_TX_Mode(unsigned char *TxAddress)
{														 
	NRF24L01_CE=0;	    
	TX_ADDRESS[0] =TxAddress[0];
	TX_ADDRESS[1] =TxAddress[1];
	TX_ADDRESS[2] =TxAddress[2];
	TX_ADDRESS[3] =TxAddress[3];
	TX_ADDRESS[4] =TxAddress[4];

 	NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)TX_ADDRESS,TX_ADR_WIDTH);
 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)TX_ADDRESS,RX_ADR_WIDTH);

 	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);    
 	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);
 	NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x1a);
 	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       
 	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  
 	NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    
	NRF24L01_CE=1;
}		  
