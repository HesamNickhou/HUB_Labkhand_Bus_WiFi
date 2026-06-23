#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include "stm32f10x.h"
#include "sys.h"

//=======================================================
#ifdef Simorgh50N
#define LEDPWR   PAout(1)
#define LEDCON   PEout(3)
#define LEDBK    PEout(5)
#define LEDRF    PEout(6)

#define LEDPS1   PEout(3)
#define LEDPS2   PEout(5)
#define LEDPS3   PAout(1)
#define LEDPS4   PEout(6)


#define BUZZER   PEout(2)

#define ROW0     PGout(9)
#define ROW1     PGout(11)
#define ROW2     PCout(7)
#define ROW3     PDout(3)
#define ROW4     PDout(6)
#define COL0     PGin(6)
#define COL1     PGin(7)
#define COL2     PGin(8)
#define COL3     PCin(6)

#define LCDCHECK PFin(8)

#define ROW4_VII  PGout(11)
#define ROW5_VII  PDout(6)
#define ROW6_VII  PCout(6)

#define ROW0_VII  PGout(9)
#define ROW1_VII  PDout(3)
#define ROW2_VII  PGout(5)
#define ROW3_VII  PGout(6)

#define COL0_VII  PGin(11)
#define COL1_VII  PGin(10)
#define COL2_VII  PDin(6)
#define COL3_VII  PCin(6)

#define GSMPWR   PCout(1)
#define GSMEN    PCout(3)

#define WIFIRST   PCout(1)

#define GSMSTATUS PCin(2)



#define R_IRQ  	 PGin(14)  
#define MISO   	 PBin(6)  
#define MOSI 	   PBout(5) 
#define SCK   	 PBout(7) 
#define R_CS  	 PGout(13)
#define R_RST	   PGout(15)

#define VOICEEN  PEout(3)

#define SAM_RST  PCout(0)


#define LEDOK  PDout(11)
#define LEDERR  PDout(12)
#define RELAY1  PDout(13)
#define RELAY2  PGout(2)

#define DERE    PAout(1)
#define SetTX485()  DERE=1
#define SetRX485()  DERE=1

#endif

//=======================================================
#ifdef Torgheh

extern unsigned char LEDPWR;
extern unsigned char LEDCON;
extern unsigned char LEDBK;
extern unsigned char LEDRF;
extern unsigned char LEDPS4;
extern unsigned char LEDPS3;
extern unsigned char LEDPS2;
extern unsigned char LEDPS1;
extern unsigned char RELAY;
extern unsigned char BUZZER;

#define SHIFTCLK  PBout(8)
#define LATCHCLK  PBout(9)
#define SDI       PCout(13)

#define ROW0      PAout(12)
#define ROW1      PBout(7)
#define COL0      PCin(12)
#define COL1      PDin(2)
#define COL2      PBin(5)
#define COL3      PBin(6)

#define GSMPWR    PAout(6)
#define GSMSTATUS PAin(7)

#define MISO   	 PCin(8)  
#define MOSI 	   PCout(9) 
#define SCK   	 PAout(8) 
#define R_CS  	 PAout(13)
#define R_RST	   PBout(3)

#define VOICEEN  PBout(4)

#define SDA        PAout(15)
#define READ_SDA   PAin(15)
#define SCL        PAout(14)

#define SAM_RST  PAout(1)


#endif

#endif
