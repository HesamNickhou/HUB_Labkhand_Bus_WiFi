#include "sys.h"
#include "voice.h"	  
#include "includes.h"				
#include "Config.h"				
#include <fatfs/ff.h>

#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_44100                   44100

extern char CheckReader;
/*
01  خدانگهدار
02  دوباره سعي کنيد
03  خطا
04  اعتبار کافي نيست
05  خدمات
06  خارج شهر
07  خسته نباشيد
08  خوش آمديد
09  خروج شهري
10  محاسبه ترافيک
11  محاسبه توقف
12  سلام
13  سلام بر راننده 
14  سرعت زياد 
15  تاييد شد
16  تعرفه برگشت
93  لغو گرديد
94  خدمات سرويس
95  سرويس برگشت
90  سرويس خارج شهر
91  کاربري دربستي
92  کاربري گردشي
*/
const TResource Voices[]= 
{
{"01.wav",0,25050},
{"10.wav",25050,42256},
{"11.wav",67306,41118},
{"12.wav",108424,13240},
{"13.wav",121664,58346},
{"14.wav",180010,34674},
{"15.wav",214684,29544},
{"16.wav",244228,42204},
{"17.wav",286432,25436},
{"02.wav",311868,37998},
{"03.wav",349866,11102},
{"04.wav",360968,50020},
{"05.wav",410988,24940},
{"06.wav",435928,34768},
{"07.wav",470696,35162},
{"08.wav",505858,21090},
{"09.wav",526948,23688},
{"90.wav",550636,46370},
{"91.wav",597006,37336},
{"92.wav",634342,36856},
{"93.wav",671198,24636},
{"94.wav",695834,46256},
{"95.wav",742090,35264},
{"\0",0,0}
};

//18  دربستي
//19  گردشي


//==============================================================================
unsigned char GetVoiceAddress(char *title, unsigned int *StartAddress, unsigned int *Len)
{
unsigned char i=0;
unsigned char j=0;

  *Len=0; 
  for(i=0; (Voices[i].Title[0]!=0) && (i<200); i++)
  {
    for(j=0; j<250; j++)
    {
      if(title[j]==0)
      {
        *Len=Voices[i].Len;
				*StartAddress=addVoices+Voices[i].StartAddress;
        return(0);
      }
      if(Voices[i].Title[j]!=title[j])
        break;
    }
  }

  return(1);
}

//==============================================================================
void PlayVoice(const char* FileName)
{
const unsigned char chkLIST[]="LIST";
unsigned int i,j,k;
unsigned short ByteRead;
unsigned int StartAddress, Len;
unsigned char EndCounter2=0, FirstSector=1,snr[10];
unsigned short b=0;
unsigned int TIM6ARRValue = 1088,ReadedSector=0;
	
		
	  if(GetVoiceAddress((char *)FileName, &StartAddress, &Len))
			return;

		
		VOICEEN=1; 

    Len+=StartAddress;
    while(1)
    {
			if(FirstSector)
			{
        LoadFromDFToRam(StartAddress, 44, _acBuffer);

				
				
				StartAddress+=44;
				i=_acBuffer[24]; i<<=8;
				i+=_acBuffer[25]; i<<=8;
				i+=_acBuffer[26]; i<<=8;
				i+=_acBuffer[27];
        switch (i)
        {
          case SAMPLE_RATE_8000 :
            TIM6ARRValue = 3000;
            break; /* 8KHz = 24MHz / 3000 */
          case SAMPLE_RATE_11025:
            TIM6ARRValue = 2176;
            break; /* 11.025KHz = 24MHz / 2176 */
          case SAMPLE_RATE_22050:
            TIM6ARRValue = 1088;
            break; /* 22.05KHz = 24MHz / 1088 */
          case SAMPLE_RATE_44100:
            TIM6ARRValue = 544;
            break; /* 44.1KHz = 24MHz / 544 */
					default:
            TIM6ARRValue = 3100;
						break;
	   		}
				//printf("\n\rVoice Playing");
				FirstSector=0;
			}
 			ByteRead=512;
			if(StartAddress+512>Len)
  			ByteRead=Len-StartAddress;
      LoadFromDFToRam(StartAddress, ByteRead, _acBuffer);

			for(i=0; i<ByteRead; i++)
				if(_acBuffer[i]==chkLIST[EndCounter2])
				{
					if(++EndCounter2>=4) ByteRead=i-4;
				}else
					EndCounter2=0;

			for(i=0; i<ByteRead; i++)
			{
  			DAC_SetChannel2Data(DAC_Align_8b_R, _acBuffer[i]);	
					//	for(j=1; j<340; j++)
					for(j=1; j<350; j++)
		  __NOP();
			}

			ReadedSector++;
			#if(DeviceType==BUSDOOR)
			if(CheckReader)
			{
				
				if((ReadedSector%50)==0)
				{
					#ifdef WithSAMCARD
					#ifdef CLRC_CHIP
					k=ISO14443_SingleTagSelect2(snr);
					#else
					if(!ISO14443_SingleTagSelect(snr))
						k=2;
					else
						k=0;
					#endif
					
				  if((k==2)||(k==3)) break;
					#endif
				}
			}
			#endif
			
			StartAddress+=ByteRead;
      if(ByteRead<512)
        break;
    }


	VOICEEN=0; 
}




