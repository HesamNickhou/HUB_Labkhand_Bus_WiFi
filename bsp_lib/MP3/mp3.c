#include <includes.h>
#include "sys.h"
#include "config.h"	

#ifdef Torgheh

#include <mp3/mp3.h>


//=================================================================
//=================================================================
//                              USART4
//=================================================================
//=================================================================
//=================================================================
#define BUFFER4_SIZE 100
volatile unsigned int rxd4_counter;
volatile unsigned int rxd4_wr_index;
volatile unsigned int rxd4_rd_index;
unsigned char RXD4Buffer[BUFFER4_SIZE];

//=============================================================================
void N9200SendByte(unsigned char c)
{
  USART_SendData(UART4, c);
}

//=============================================================================
void N9200EmptyRXBuffer(void)
{
  rxd4_rd_index=rxd4_counter=rxd4_wr_index=0;
}

//=============================================================================
unsigned char N9200_GCL(unsigned char *c)
{
unsigned int i;
	
	i=OS_TimeMS;
	while(1)
	{
  	if(i>OS_TimeMS) return(0);
	  if(OS_TimeMS-i>=5000) //5 Sec
			return(0);
    if(rxd4_counter!=0) 
      break;
  	WDTR;
}		
  *c=RXD4Buffer[rxd4_rd_index];
  if(++rxd4_rd_index >= BUFFER4_SIZE) rxd4_rd_index=0;
  if(rxd4_counter) rxd4_counter--;
  return(1);
}

//=============================================================================
unsigned char N9200_GCF(unsigned char *c)
{
unsigned int i;

	i=OS_TimeMS;
	while(1)
	{
  	if(i>OS_TimeMS) return(0);
	  if(OS_TimeMS-i>=100) //100 msec
			return(0);
    if(rxd4_counter!=0) 
      break;
		WDTR;
	}		
  *c=RXD4Buffer[rxd4_rd_index];
  if(++rxd4_rd_index >= BUFFER4_SIZE) rxd4_rd_index=0;
  if(rxd4_counter) rxd4_counter--;
  return(1);
}

//=============================================================================
void UART4_IRQHandler(void)  
{
volatile unsigned int IIR;
u8 Byte;
	
  IIR = UART4->SR;
  if (IIR & USART_FLAG_ORE) 
	{        
    UART4->SR &= ~USART_FLAG_ORE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_TXE) 
	{        
    UART4->SR &= ~USART_FLAG_TXE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_RXNE) 
	{     
    //USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
		Byte=(UART4->DR & 0x1FF);
    RXD4Buffer[rxd4_wr_index]=Byte;
    if(++rxd4_wr_index >= BUFFER4_SIZE) rxd4_wr_index=0;
    UART4->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
		USART_SendData(USART1, Byte); //////////
    if(++rxd4_counter >= BUFFER4_SIZE)
    {
       rxd4_rd_index=0;
			 rxd4_counter=0;
			 rxd4_wr_index=0;
    };
    //USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  } 
} 

//=============================================================================
void uart4_init(u32 bound){
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(UART4, &USART_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  USART_Cmd(UART4, ENABLE);                    

}

//=================================================================
//=================================================================
//                       N9200 MP3 Player
//=================================================================
//=================================================================
void InitN9200(void)
{
	uart4_init(9600);
	
  //Switch to MUSIC
	N9200SendByte(0x03);	
  N9200SendByte(0xFC);	
  N9200SendByte(0x02);	
  N9200SendByte(0x01);	
  N9200SendByte(0x00);	
  N9200SendByte(0x02);	
	GUI_Delay(1000);

  //Query Current Mode
  N9200SendByte(0x03);	
  N9200SendByte(0xFC);	
  N9200SendByte(0x01);	
  N9200SendByte(0x00);	
  N9200SendByte(0x00);	
	GUI_Delay(1000);
	
	
  N9200SendByte(0x04);	
  N9200SendByte(0xFB);	
  N9200SendByte(0x01);	
  N9200SendByte(0x0A);	
  N9200SendByte(0x0A);	
	GUI_Delay(1000);
	
  N9200SendByte(0x06);	
  N9200SendByte(0xF9);	
  N9200SendByte(0x02);	
  N9200SendByte(0x01);	
  N9200SendByte(0x14);	
  N9200SendByte(0x16);	
	GUI_Delay(1000);
	

/*
  N9200SendByte(0x06);	
  N9200SendByte(0xF9);	
  N9200SendByte(0x02);	
  N9200SendByte(0x01);	
  N9200SendByte(0x05);	
  N9200SendByte(0x07);	
	GUI_Delay(1000);
*/
  N9200SendByte(0x04);	
  N9200SendByte(0xFB);	
  N9200SendByte(0x01);	
  N9200SendByte(0x01);	
  N9200SendByte(0x01);	
}

//=================================================================
//=================================================================
//                              TWI
//=================================================================
//=================================================================
//#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=8<<12;}
//#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}

void SDA_IN(void) 
{    
GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}
	
void SDA_OUT(void) 
{    
GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//===========================================================
void delay_us(unsigned char d)
{
unsigned int i;
	
  for(i=0; i<d*1000; i++) 
	  __NOP();
}

//===========================================================
void IIC_Start(void)
{
	SDA_OUT();     //sdaدكتن³ِ
	SDA=1;	  	  
	SCL=1;
	delay_us(4);
 	SDA=0;
	delay_us(4);
	SCL=0;
}	  

//===========================================================
void IIC_Stop(void)
{
	SDA_OUT();
	SCL=0;
	SDA=0;
 	delay_us(4);
	SCL=1; 
	SDA=1;
	delay_us(4);							   	
}

//===========================================================
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN(); 
	SDA=1;delay_us(1);	   
	SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL=0;
	return 0;  
} 

//===========================================================
void IIC_Ack(void)
{
	SCL=0;
	SDA_OUT();
	SDA=0;
	delay_us(2);
	SCL=1;
	delay_us(2);
	SCL=0;
}

//===========================================================
void IIC_NAck(void)
{
	SCL=0;
	SDA_OUT();
	SDA=1;
	delay_us(2);
	SCL=1;
	delay_us(2);
	SCL=0;
}					 				     

//===========================================================
void IIC_Send_Byte(u8 txd)
{                        
u8 t;   
	SDA_OUT(); 	    
  SCL=0;
  for(t=0;t<8;t++)
  {              
    SDA=(txd&0x80)>>7;
    txd<<=1; 	  
		delay_us(2);  
		SCL=1;
		delay_us(2); 
		SCL=0;	
		delay_us(2);
 }	 
} 	    

//===========================================================
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDAةèضأخھتنبë
  for(i=0; i<8; i++)
	{
    SCL=0; 
    delay_us(2);
		SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
  	delay_us(1); 
  }					 
  if(!ack)
    IIC_NAck();
  else
    IIC_Ack(); 
  return receive;
}


//=================================================================
//=================================================================
//                        FM Transceiver
//=================================================================
//=================================================================
u8 RDA5820_Init(void)
{
	u16 id;
	id=RDA5820_RD_Reg(RDA5820_R00);	//ׁȡID =0X5805
	if(id==0X5805)					//ׁȡIDֽȷ
	{
	 	RDA5820_WR_Reg(RDA5820_R02,0x0002);	//ɭشλ
		GUI_Delay(50);
	 	RDA5820_WR_Reg(RDA5820_R02,0xC001);	//bͥʹ,ʏ֧
		GUI_Delay(600);						//ֈսʱדψ֨ 
	 	RDA5820_WR_Reg(RDA5820_R05,0X884F);	//̷̑ǿ׈8,LNAN,1.8mA,VOLخճ
 	 	RDA5820_WR_Reg(0X07,0X7800);		// 
	 	RDA5820_WR_Reg(0X13,0X0008);		// 
	 	RDA5820_WR_Reg(0X15,0x1420);		//VCOʨ׃  0x17A0/0x1420 
	 	RDA5820_WR_Reg(0X16,0XC000);		//  
	 	RDA5820_WR_Reg(0X1C,0X3126);		// 
	 	RDA5820_WR_Reg(0X22,0X9C24);		//fm_true 
	 	RDA5820_WR_Reg(0X47,0XF660) ;		//tx rds 
 	}else return 1;//Եʼۯʧќ
	return 0;
}

//=================================================================
void RDA5820_WR_Reg(u8 addr,u16 val)
{
    IIC_Start();  			   
	IIC_Send_Byte(RDA5820_WRITE);	//ע̍дļ®   	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(addr);   			//עַ̍֘
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(val>>8);     		//עٟ̍ؖޚ							   
	IIC_Wait_Ack();  		    	   
 	IIC_Send_Byte(val&0XFF);     	//ע̍֍ؖޚ							   
 	IIC_Wait_Ack();  		    	   
    IIC_Stop();						//Ӻʺһٶֹͣ͵ݾ 	 
}

//=================================================================
u16 RDA5820_RD_Reg(u8 addr)
{
	u16 res;
    IIC_Start();  			   
	IIC_Send_Byte(RDA5820_WRITE);	//ע̍дļ®   	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(addr);   			//עַ̍֘
	IIC_Wait_Ack(); 	 										  		   
    IIC_Start();  			   
	IIC_Send_Byte(RDA5820_READ);    //עׁ̍ļ®							   
	IIC_Wait_Ack();  		    	   
 	res=IIC_Read_Byte(1);     		//ׁٟؖޚ,ע̍ACK	  
  	res<<=8;
  	res|=IIC_Read_Byte(0);     		//ׁ֍ؖޚ,ע̍NACK							   
  	IIC_Stop();						//Ӻʺһٶֹͣ͵ݾ 
	return res;						//׵ܘׁսք˽ߝ
}

//=================================================================
void RDA5820_RX_Mode(void)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X40);		//ׁȡ0X40քŚɝ
	temp&=0xfff0;					//RX ģʽ   
	RDA5820_WR_Reg(0X40,temp) ;		//FM RXģʽ 
}			

//=================================================================
void RDA5820_TX_Mode(void)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X40);		//ׁȡ0X40քŚɝ
	temp&=0xfff0;
	temp|=0x0001;				    //TX ģʽ
	RDA5820_WR_Reg(0X40,temp) ;		//FM TM ģʽ 
}

//=================================================================
u8 RDA5820_Rssi_Get(void)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X0B);		//ׁȡ0X0BքŚɝ
	return temp>>9;                 //׵ܘхۅǿ׈
}

//=================================================================
void RDA5820_Vol_Set(u8 vol)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X05);		//ׁȡ0X05քŚɝ
	temp&=0XFFF0;
	temp|=vol&0X0F;				   		 
	RDA5820_WR_Reg(0X05,temp) ;		//ʨ׃Ӵ	
}

//=================================================================
void RDA5820_Mute_Set(u8 mute)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X02);		//ׁȡ0X02քŚɝ
	if(mute)temp|=1<<14;
	else temp&=~(1<<14);	    		   		 
	RDA5820_WR_Reg(0X02,temp) ;		//ʨ׃MUTE	
}

//=================================================================
void RDA5820_Rssi_Set(u8 rssi)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X05);		//ׁȡ0X05քŚɝ
	temp&=0X80FF;
	temp|=(u16)rssi<<8;				   		 
	RDA5820_WR_Reg(0X05,temp) ;		//ʨ׃RSSI	
}

//=================================================================
void RDA5820_TxPAG_Set(u8 gain)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X42);		//ׁȡ0X42քŚɝ
	temp&=0XFFC0;
	temp|=gain;				   		//GAIN
	RDA5820_WR_Reg(0X42,temp) ;		//ʨ׃PAք٦Ê
}

//=================================================================
void RDA5820_TxPGA_Set(u8 gain)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X42);		//ׁȡ0X42քŚɝ
	temp&=0XF8FF;
	temp|=gain<<8;			    	//GAIN
	RDA5820_WR_Reg(0X42,temp) ;		//ʨ׃PGA
}

//=================================================================
void RDA5820_Band_Set(u8 band)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X03);		//ׁȡ0X03քŚɝ
	temp&=0XFFF3;
	temp|=band<<2;			     
	RDA5820_WR_Reg(0X03,temp) ;		//ʨ׃BAND
}

//=================================================================
void RDA5820_Space_Set(u8 spc)
{
	u16 temp;
	temp=RDA5820_RD_Reg(0X03);		//ׁȡ0X03քŚɝ
	temp&=0XFFFC;
	temp|=spc;			     
	RDA5820_WR_Reg(0X03,temp) ;		//ʨ׃BAND
}

//=================================================================
void RDA5820_Freq_Set(u16 freq)
{
	u16 temp;
	u8 spc=0,band=0;
	u16 fbtm,chan;
	temp=RDA5820_RD_Reg(0X03);	//ׁȡ0X03քŚɝ
	temp&=0X001F;
	band=(temp>>2)&0x03;		//փսƵո
	spc=temp&0x03; 				//փսؖҦÊ
	if(spc==0)spc=10;
	else if(spc==1)spc=20;
	else spc=5;
	if(band==0)fbtm=8700;
	else if(band==1||band==2)fbtm=7600;
	else 
	{
		fbtm=RDA5820_RD_Reg(0X53);//փսbottomƵÊ
		fbtm*=10;
	}
	if(freq<fbtm)return;
	chan=(freq-fbtm)/spc;		//փսCHANӦكдɫքֵ
	chan&=0X3FF;				//ȡ֍10λ	  
	temp|=chan<<6;
	temp|=1<<4;					//TONE ENABLE			     
	RDA5820_WR_Reg(0X03,temp) ;	//ʨ׃ƵÊ
	GUI_Delay(20);				//ֈս20ms
	while((RDA5820_RD_Reg(0X0B)&(1<<7))==0);//ֈսFM_READY
	
}

//=================================================================
u16 RDA5820_Freq_Get(void)
{
	u16 temp;
	u8 spc=0,band=0;
	u16 fbtm,chan;
	temp=RDA5820_RD_Reg(0X03);		//ׁȡ0X03քŚɝ
	chan=temp>>6;   
	band=(temp>>2)&0x03;		//փսƵո
	spc=temp&0x03; 				//փսؖҦÊ
	if(spc==0)spc=10;
	else if(spc==1)spc=20;
	else spc=5;
	if(band==0)fbtm=8700;
	else if(band==1||band==2)fbtm=7600;
	else 
	{
		fbtm=RDA5820_RD_Reg(0X53);//փսbottomƵÊ
		fbtm*=10;
	}
 	temp=fbtm+chan*spc;				 
	return temp;//׵ܘƵÊֵ
}

#endif