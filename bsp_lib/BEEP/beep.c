#include <includes.h>
#include "beep.h"
#include "app_cfg.h"

int speakerPin = 9;

unsigned char notes[] = "CbgCbeRCcgaC";
int beats[]  = { 16, 16, 16,  8,  8,  16, 32, 16, 16, 16, 8, 8 }; 
int tempo = 300;

void delayMicroseconds(int d)
{
unsigned int i, j;
  for (i=0; i<d; i++)	
	  for (j=0; j<2; j++) __NOP();
}

void playTone(int tone, int duration) {
	long i;
	
  for (i = 0; i < duration * 1000L; i += tone * 2) {
    BUZZER=1;
    delayMicroseconds(tone);
    BUZZER=0;
    delayMicroseconds(tone);
  }
}
/*
#define  c     3830    // 261 Hz 
#define  d     3400    // 294 Hz 
#define  e     3038    // 329 Hz 
#define  f     2864    // 349 Hz 
#define  g     2550    // 392 Hz 
#define  a     2272    // 440 Hz 
#define  b     2028    // 493 Hz 
#define  C     1912    // 523 Hz 
*/
void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  //int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  int tones[] = { 3830, 3400, 3038, 2864, 2550, 2272, 2028, 1912 };
	int i;

	// play the tone corresponding to the note name
  for (i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void loop() {
int i;
	
  for (i = 0; i < sizeof(notes); i++) {
    if (notes[i] == ' ') {
      delayMicroseconds(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause between notes
    GUI_Delay(tempo / 2); 
  }
}

void Bizzer_delay(U32 delay)
{
U32 i;
	
  #ifdef Torgheh
	ShiftChanged();
	#endif
  for (i=0;i<delay*30;i++)
  {
    __NOP();
  }
}

void Play_Bizzer(unsigned char type)
{
U16 i;
	
    for (i=0;i<200;i++)
    {
      BUZZER=1;
      Bizzer_delay(80);
      BUZZER=0;
      Bizzer_delay(20);
    }
    for (i=0;i<200;i++)
    {
      BUZZER=1;
      Bizzer_delay(40);
      BUZZER=0;
      Bizzer_delay(5);
    }
		if (type==2) return;
    for (i=0;i<200;i++)
    {
      BUZZER=1;
      Bizzer_delay(40);
      BUZZER=0;
      Bizzer_delay(40);
    }
    for (i=0;i<200;i++)
    {
      BUZZER=1;
      Bizzer_delay(100);
      BUZZER=0;
      Bizzer_delay(80);
    }
}

//========================================================================
void Sound(unsigned int Delay) 
{
unsigned int i, j;
  
  BUZZER=1;
	#ifdef Torgheh
	ShiftChanged();
	#endif
  GUI_Delay(Delay);
}

//========================================================================
void OkBip(unsigned char LongBip) {
unsigned char i;
unsigned int Fr;
	
  switch (LongBip)
  {
		case 1:
			Play_Bizzer(1);
		  break;
		case 0:	
      for (Fr=15; Fr>0; Fr--) Sound(15);
      BUZZER=0;
			#ifdef Torgheh
			ShiftChanged();
			#endif
      GUI_Delay(5);
      for (Fr=15; Fr>0; Fr--) Sound(15); 
      BUZZER=0;
			#ifdef Torgheh
			ShiftChanged();
			#endif
		  break;
		case 2:
			Play_Bizzer(2);
		  break;
  }   
  BUZZER=0;
	#ifdef Torgheh
	ShiftChanged();
	#endif

}

//========================================================================
void ErrorBip(unsigned char LongBip) {
unsigned int i;

  for (i=0;i<400;i++)
  {
    BUZZER=1;
    Bizzer_delay(200);
    BUZZER=0;
    Bizzer_delay(80);
  }
}

//========================================================================
void PlayBip(unsigned char Type) {
unsigned int i;

  for (i=0;i<100;i++)
  {
    BUZZER=1;
    Bizzer_delay(100);
    BUZZER=0;
    Bizzer_delay(80);
  }
}

//========================================================================
void keyboard_Beep(void)
{
unsigned short i;
	
  for (i=0;i<100;i++)
  {
    BUZZER=1;
    Bizzer_delay(50);
    BUZZER=0;
    Bizzer_delay(10);
  }
}   
