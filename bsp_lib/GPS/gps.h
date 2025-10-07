#ifndef __GPS_H
#define __GPS_H

typedef struct 
{ 
  char UTC[20];
  char Status[20];
  char Lat[20];
  char NS[20];
  char Lon[20];
  char COG[20];
  char EW[20];
  char Speed[20];
  char DATE[20];
  char ANTSTATUS;
}TGPSData;
extern unsigned char GPSDataWrite;
extern unsigned long int GPSDataWaitTimeout;
extern TGPSData GPSData;
extern unsigned char GPSTimeAcquired;
extern unsigned int GPSSpeed;
#define GPSDataFreez                              \
          {                                       \
            GPSDataWaitTimeout=0;                 \
            while (GPSDataWrite)                   \
              if (GPSDataWaitTimeout++>1000000)    \
                break;                            \
            GPSDataWrite=1;                       \
          }
#define GPSDataRelease  GPSDataWrite=0;

extern unsigned char GGAReady;
extern unsigned char GPRMCReady;
extern unsigned char GGAMessage[];
extern unsigned char GPRMCMessage[];

extern unsigned int GPSValidData;
extern unsigned int HDOP;
extern unsigned int SattelitesInView;
extern unsigned char PositionFix;
extern void GPSHandler(void *p_arg) ;
extern int GPSInitialize(void);
extern void ProcessGPRMCMessage(void);
extern void ProcessGPGGAMessage(void);
extern void DisplayLocation(unsigned char ForceShow);
extern float GpsDist(float Lat1, float Long1, float Lat2, float Long2);
extern unsigned char GPS_CheckSystemTime(void);
extern void DisplayGpsLocation(void);
extern void ProcessGPTXTMessage(char  Status);
extern void ProcessGPRMCMessage(void);
extern void ProcessGPGGAMessage(void);

#endif
