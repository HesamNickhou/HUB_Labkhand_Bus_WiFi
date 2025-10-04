
#ifndef  __KEYPAD_H__
#define  __KEYPAD_H__

#ifdef Simorgh50N
#define BA1        1
#define BA2        2
#define BA3        3
#define BA4        4

#define BA5        5
#define BA6        6
#define BA7        7
#define BA8        8
#define BA9        9
#define BA10       10

#define BUP        11
#define BDOWN      12
#define BOK        13
#define BCANCEL    14


#define BLEFT      15
#define BRIGHT     16


#define BMUTE      17
#define BMAP       18
#define BPRINT     19
#define BHELP      20
#define BTOOLS     21
#define BCARDINFO  22
#define BPAYMENT   23
#define BPAYMENT2  24

#define BP1        25
#define BP2        26
#define BP3        27
#define BP4        28
#define BF1        29
#define BF2        30
#define BF3        31
#endif


#ifdef Torgheh
#define BP1        1
#define BP2        2
#define BP3        3
#define BP4        4
#define BF1        9
#define BF2        11
#define BF3        10

#define BUP        11
#define BDOWN      10

#define BLEFT      2
#define BCANCEL    9
#define BRIGHT     1

#define BOK        6
#define BMUTE      16
#define BMAP       15
#define BPRINT     14
#define BHELP      13
#define BTOOLS     17
#define BCARDINFO  18
#define BPAYMENT   19
#define BPAYMENT2  30
#endif

extern unsigned char OldKey_Pressed;


unsigned char AppTaskKbd(void);
unsigned char ScanKeyboard(void);

#endif
