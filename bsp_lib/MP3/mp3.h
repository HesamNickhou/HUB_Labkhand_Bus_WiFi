#ifndef __MP3_H
#define __MP3_H

#include "includes.h"

#define RDA5820_R00		0X00  //¶ء³ِ16¸ِخ»µؤID =0X5820
#define RDA5820_R02		0X02  //DHIZ[15],DMUTE[14],MONO[13],BASS[12],SEEKUP[9],SEEK[8],SKMODE[7],CLK_MODE[6:4],ANTENNA[2],SOFTRESET[1],ENABLE[0]
#define RDA5820_R03		0X03  //CHAN[15:6],TUNE[4],BAND[3:2],SPACE[1:0]
#define RDA5820_R04		0X04  //STCIEN[14](ثرث÷حê³ةضذ¶د),DE[11],I2S_ENABLE[6],GPIO3~1[5:0]
#define RDA5820_R05		0X05  //INT_MODE[15],SEEKTH[14:8](ةè¶¨ذإ؛إا؟¶ب),LNA_PORT_SEL[7:6],LNA_ICSEL_BIT[5:4],VOLUME[3:0];
#define RDA5820_R0A		0X0A  //STC[14](ثرث÷حê³ة±êض¾),SF[13](ثرث÷ت§°ـ±êض¾),ST[10](ء¢جهةù±êض¾),READCHAN[9:0](ئµآت).
#define RDA5820_R0B		0X0B  //RSSI[15:9](ذإ؛إا؟¶ب),FM_TRUE[8](ص¾µم±êض¾),FM_READY[7](fm¾حذ÷±êض¾)
#define RDA5820_R40		0X40  //AUTO_SEEK[15](ب«/°ë×ش¶¯ثرج¨),CHAN_SCAN[14](ثر؟ص/أ¦ج¨),CHIP_FUN[3:0](¹¤×÷ؤ£ت½)
#define RDA5820_R41		0X41  //MEM_CLR[15](ؤع´واه³‎),RPT_TIME[14:9](RDSس¦´ً´خت‎),MEM_DEPTH[8:0](FIFO´َذ،,×î´َ256)
#define RDA5820_R42		0X42  //TX_SIG_GAIN[10:8],TX_PA_GAIN[5:0](·¢ةن¹¦آت).
#define RDA5820_R4A		0X4A  //MEM_INTMOD[4:0]
#define RDA5820_R4B		0X4B  //CHAN_NUM[7:0](½ِشعب«×ش¶¯ثرج¨سذذ§,´ْ±يµçج¨ت‎ء؟).
#define RDA5820_R4C		0X4C  //TX_AUDIO_DEV[15:0]
#define RDA5820_R4D		0X4D  //TX_PILOT_DEV[15:0]
#define RDA5820_R4E		0X4E  //TX_RDS_DEV[15:0]
#define RDA5820_R53		0X53  //CHAN_BOTTON[10:0](ح¨µہ×îµحئµآت,100kHZخھµ¥خ»)
#define RDA5820_R54		0X54  //CHAN_TOP[10:0](ح¨µہ×î¸كئµآت,100kHZخھµ¥خ»)
#define RDA5820_R64		0X64  //RDS_RXMOD[4:3](FIFOؤ£ت½[10]»ٍصك¼ؤ´وئ÷ؤ£ت½[11])		 

#define RDA5820_READ    0X23  //¶ءRDA5820
#define RDA5820_WRITE	0X22  //ذ´RDA5820



u8 RDA5820_Init(void);					//³ُت¼»¯		 				    
void RDA5820_WR_Reg(u8 addr,u16 val);	//ذ´RDA5820¼ؤ´وئ÷
u16 RDA5820_RD_Reg(u8 addr);			//¶ءRDA5820¼ؤ´وئ÷
void RDA5820_RX_Mode(void);				//ةèضأRDA5820خھRXؤ£ت½
void RDA5820_TX_Mode(void);				//ةèضأRDA5820خھTXؤ£ت½
u8 RDA5820_Rssi_Get(void);
void RDA5820_Mute_Set(u8 mute);
void RDA5820_Rssi_Set(u8 rssi);
void RDA5820_Vol_Set(u8 vol);
void RDA5820_TxPAG_Set(u8 gain);
void RDA5820_TxPGA_Set(u8 gain);
void RDA5820_Band_Set(u8 band);
void RDA5820_Space_Set(u8 spc);
void RDA5820_Freq_Set(u16 freq);
u16 RDA5820_Freq_Get(void);

#endif
