#ifndef MENU3D_H
#define MENU3D_H


#define mtError       0
#define mtWarning     1
#define mtInformation 2

#define RGB(red, green, blue)	(unsigned int)(((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF))

struct T3DMenuStruct{
  const char *ICON[2];
  const char *HotICON[2];
  const char *Title[2];
  unsigned char (*fun)(void);
  const unsigned char Select;
};

typedef const struct T3DMenuStruct MenuStruct;

extern MenuStruct const MainMenu[];

extern char CalcWeek(int year, int month, int day);
extern unsigned char ExecMenu(MenuStruct const *Menu);
extern unsigned char Setup(void);
extern void ShowMessageDlg(unsigned char mType, unsigned char *mMessage, unsigned char repaint, unsigned char showpage, unsigned int delay, unsigned char alarm);
extern unsigned char G_LoadBMPPart(unsigned int X,unsigned int Y, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, const char* FileName);
extern unsigned char G_LoadBMP(unsigned int X,unsigned int Y, const char* FileName, char LoadTransparent);
extern unsigned char G_LoadMapPart(unsigned int X,unsigned int Y, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, const char* FileName);
extern unsigned char G_LoadMap(unsigned int X,unsigned int Y, const char* FileName, char LoadTransparent);

#endif


