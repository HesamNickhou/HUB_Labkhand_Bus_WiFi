#include "touch.h" 
#include "GUI.h"
#include "stdlib.h"
#include "config.h"
#include "math.h"

#define Tabriz

#define ERR_RANGE 50 						//ϳӮ׶Χ 
#define SAVE_ADDR_BASE 50
#define READ_TIMES 50 						//ׁȡՎ˽
#define LOST_VAL 5	 						//֪Ǻֵ
_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0,
 	0,
	0,
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};					
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

void TP_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for (count=0;count<8;count++)  
	{ 	  
		if (num&0x80)TDIN=1;  
		else TDIN=0;   
		num <<= 1;    
		TCLK=0; 	 
		__nop();__nop();
		__nop();__nop();
		TCLK=1;		   
		__nop();__nop();
		__nop();__nop();
	}		 			    
} 		 
 
uint16_t ADS_Read_AD(uint8_t CMD)	  
{ 	 
	
	u8 count=0; 	  
	u16 Num=0; 
	
	WDTR;
	TCLK=0;		 	 
	TDIN=0; 	 
	TCS=0; 		 
	TP_Write_Byte(CMD); 
	TCLK=1;		    	    
  __nop();__nop();
	__nop();__nop();
	TCLK=0; 	     	    
	for (count=0;count<16;count++) 
	{ 				  
		Num <<= 1; 	 
		TCLK=0;	  	   
		__nop();__nop();
		__nop();__nop();
		TCLK=1;
  	__nop();__nop();
		if (DOUT)Num++; 		 
	}  	
	Num >>= 4;   	 
	TCS=1;		  
	return Num;    
}
    
 
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for (i=0;i<10;i++)buf[i] =ADS_Read_AD(xy);		 		    
	for (i=0;i<READ_TIMES;i++)buf[i] =ADS_Read_AD(xy);		 		    
	for (i=0;i<READ_TIMES-1; i++)//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if (xtemp<100||ytemp<100)return 0; 
	#ifdef Tabriz
	*x=ytemp;
	*y=xtemp;
	#else
	*x=xtemp;
	*y=ytemp;
	#endif
	return 1;
}

u8 TP_Read_XY2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if (flag==0)return 0;
    flag=TP_Read_XY(&x2,&y2);	   
    if (flag==0)return 0;   
    if (((x2<=x1&&x1<x2+ERR_RANGE) || (x1<=x2&&x2<x1+ERR_RANGE))
    &&((y2<=y1&&y1<y2+ERR_RANGE) || (y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 

void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	GUI_SetColor(color);
	GUI_DrawLine(x-12,y,x+13,y);
	GUI_DrawLine(x,y-12,x,y+13);
	GUI_DrawPoint(x+1,y+1);
	GUI_DrawPoint(x-1,y+1);
	GUI_DrawPoint(x+1,y-1);
	GUI_DrawPoint(x-1,y-1);
	GUI_DrawCircle(x,y,6);
}	  

void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	GUI_SetColor(color);
	GUI_DrawPoint(x,y); 
	GUI_DrawPoint(x+1,y);
	GUI_DrawPoint(x,y+1);
	GUI_DrawPoint(x+1,y+1);	 	  	
}	


//////////////////////////////////////////////////////////////////////////////////		  

u8 TP_Scan(u8 tp)
{			   
	if (PEN==0)
	{
		if (tp)TP_Read_XY2(&tp_dev.x,&tp_dev.y);
		else if (TP_Read_XY2(&tp_dev.x,&tp_dev.y))
		{
	 		tp_dev.x=tp_dev.xfac*tp_dev.x+tp_dev.xoff;
			tp_dev.y=tp_dev.yfac*tp_dev.y+tp_dev.yoff;  
	 	} 
		if ((tp_dev.sta&TP_PRES_DOWN)==0)
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;  
			tp_dev.x0=tp_dev.x;
			tp_dev.y0=tp_dev.y;  	   			 
		}			   
	}else
	{
		if (tp_dev.sta&TP_PRES_DOWN)
		{
			tp_dev.sta&=~(1<<7);	
		}else
		{
			tp_dev.x0=0;
			tp_dev.y0=0;
			tp_dev.x=0xffff;
			tp_dev.y=0xffff;
		}	    
	}
	#ifdef Tabriz
	tp_dev.x=tp_dev.x;
	tp_dev.y=tp_dev.y;
	#endif
	
	return tp_dev.sta&TP_PRES_DOWN;
}	  
 					  



uint16_t ADS_Read_XY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for (i=0;i<READ_TIMES;i++)
	{				 
		buf[i] =ADS_Read_AD(xy);	    
	}				    
	for (i=0;i<READ_TIMES-1; i++)			//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])				//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

/*********************************************************************************************************
** Functoin name:       Read_ADSX
** Descriptions:        ׁXքֵ
** input paraments:     Ϟ
** output paraments:    x:ׁȡ۳քֵѣզքַ֘    
** Returned values:    	1
*********************************************************************************************************/
uint8_t Read_ADSX(uint16_t *x)
{
	uint16_t xtemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);	  												   
	if (xtemp<100)return 0;					//ׁ˽ʧќ
	*x=xtemp;

	return 1;								//ׁ˽ԉ٦
}
/*********************************************************************************************************
** Functoin name:       Read_ADSY
** Descriptions:        ׁYքֵ
** input paraments:     Ϟ
** output paraments:    y:ׁȡ۳քֵѣզքַ֘    
** Returned values:    	1
*********************************************************************************************************/
uint8_t Read_ADSY(uint16_t *y)
{
	uint16_t ytemp;			 	 		  
	ytemp=ADS_Read_XY(CMD_RDY);	  												   
	if (ytemp<100)return 0;					//ׁ˽ʧќ
	*y=ytemp;

	return 1;								//ׁ˽ԉ٦
}

/*********************************************************************************************************
** Functoin name:       Read_ADSX
** Descriptions:        
				2ՎׁȡADS7846,lѸׁȡ2ՎԐЧքADֵ,Ȓ֢}Վքƫ
				ӮһŜӬڽ50,úأ͵ݾ,ղɏΪׁ˽ֽȷ,رղׁ˽խϳ.
				كگ˽Ŝճճٟ͡׼ȷ׈
** input paraments:     ext:ˇربɻʽәطèبɻúֈսֱսдЂԥǁì؇بɻú׵ܘڌֵ֨é
** output paraments:    x:ׁȡ۳քֵѣզքַ֘    
** Returned values:    	1
*********************************************************************************************************/

uint8_t Touch_GexX(uint16_t *x,uint8_t ext) 
{
	uint16_t x1;
 	uint16_t x2;
 	uint8_t flag; 

	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;

	if (ext){
		/*ֈսԥǁдЂ*/
		while (PEN);
	}    
	
	for (i=0;i<40;i++)
		Read_ADSX(&x1);   		 		    
	for (i=0;i<READ_TIMES;i++)
	{
		Read_ADSX(&x1);   		 		    
	  buf[i] =x1;
	}
	for (i=0;i<READ_TIMES-1; i++)//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)	sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	x1=temp;   	
	
	
	
	for (i=0;i<40;i++)
		Read_ADSX(&x1);   		 		    
	for (i=0;i<READ_TIMES;i++)
	{
		Read_ADSX(&x2);   		 		    
	  buf[i] =x2;
	}
	for (i=0;i<READ_TIMES-1; i++)//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	x2=temp; 

	
	/*
    flag=Read_ADSX(&x1);   
    if (flag==0)return 0;
    flag=Read_ADSX(&x2);	   
    if (flag==0)return 0;  
	*/
	
	
	
	
	
	
	if (ext){
		/*ֈսԥǁ̉ߪ*/
		while (!(PEN));
	} 
    if (((x2<=x1&&x1<x2+ERR_RANGE) || (x1<=x2&&x2<x1+ERR_RANGE)))//ǰ۳}ՎӉҹ՚+-50Ś
    {
        *x=(x1+x2)/2;
//		Printf("x=0x%x\r\n",*x);
        return 1;
    }else return 0;	  
} 
/*********************************************************************************************************
** Functoin name:       Read_ADSY
** Descriptions:        
				2ՎׁȡADS7846,lѸׁȡ2ՎԐЧքADֵ,Ȓ֢}Վքƫ
				ӮһŜӬڽ50,úأ͵ݾ,ղɏΪׁ˽ֽȷ,رղׁ˽խϳ.
				كگ˽Ŝճճٟ͡׼ȷ׈
** input paraments:     ext:ˇربɻʽәطèبɻúֈսֱսдЂԥǁì؇بɻú׵ܘڌֵ֨é
** output paraments:    y:ׁȡ۳քֵѣզքַ֘    
** Returned values:    	1
*********************************************************************************************************/
uint8_t Touch_GexY(uint16_t *y,uint8_t ext) 
{
	uint16_t y1;
 	uint16_t y2;
 	uint8_t flag; 
	
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	
	if (ext){
		/*ֈսԥǁдЂ*/
		while (PEN);
	}   
	
	
	
	
	
	
	
	for (i=0;i<READ_TIMES;i++)
	{
		Read_ADSY(&y1);   		 		    
	  buf[i] =y1;
	}
	for (i=0;i<READ_TIMES-1; i++)//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	y1=temp;   	
	
	
	
	for (i=0;i<READ_TIMES;i++)
	{
		Read_ADSY(&y2);   		 		    
	  buf[i] =y2;
	}
	for (i=0;i<READ_TIMES-1; i++)//ƅѲ
	{
		for (j=i+1;j<READ_TIMES;j++)
		{
			if (buf[i]>buf[j])//ʽѲƅ
			{
				temp=buf[i];
				buf[i] =buf[j];
				buf[j] =temp;
			}
		}
	}	  
	sum=0;
	for (i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum += buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	y2=temp;   	


   /*	
	
    flag=Read_ADSY(&y1);   
    if (flag==0)return 0;
    flag=Read_ADSY(&y2);	   
    if (flag==0)return 0;  
	*/
	if (ext){
		/*ֈսԥǁ̉ߪ*/
		while (!(PEN));
	} 
    if (((y2<=y1&&y1<y2+ERR_RANGE) || (y1<=y2&&y2<y1+ERR_RANGE)))//ǰ۳}ՎӉҹ՚+-50Ś
    {
        *y=(y1+y2)/2;
//		Printf("x=0x%x\r\n",*x);
        return 1;
    }else return 0;	  
		
}
/*********************************************************************************************************
** Functoin name:       Touch_GextXY
** Descriptions:        
						փսXYքADֵ
** input paraments:     ext:ˇربɻʽәطèبɻúֈսֱսдЂԥǁì؇بɻú׵ܘڌֵ֨é
** output paraments:    xy:ׁȡ۳քֵѣզքַ֘    
** Returned values:    	Ϟ
*********************************************************************************************************/
void Touch_GetXY(uint16_t *x,uint16_t* y,uint8_t ext)
{
	if (ext){
		while (PEN) WDTR;
	}
	Touch_GexX(y,0); /////////////////////////////////////////////////////
	Touch_GexY(x,0);
	if (ext){
		
		while (!(PEN)) WDTR;
	}	
}


void Drow_Touch_Point(u16 x,u16 y)
{
  GUI_SetColor(GUI_BLUE);  
	GUI_DrawLine(x-12,y,x+13,y);		//ۡП
	GUI_DrawLine(x,y-12,x,y+13);		//˺П
	GUI_DrawPoint(x+1,y+1);
	GUI_DrawPoint(x-1,y+1);
	GUI_DrawPoint(x+1,y-1);
	GUI_DrawPoint(x-1,y-1);
	GUI_DrawCircle(x,y,6);					//ۭאфȦ
}	  
/*********************************************************************************************************
** Functoin name:       Draw_Big_Point
** Descriptions: 		ۭһٶճ֣ ռ4ٶλ׃
** input paraments:     xy:ۭ֣քظҪ
** output paraments:    Ϟ 
** Returned values:    	Ϟ
*********************************************************************************************************/			   
void Draw_Big_Point(u16 x,u16 y)
{	  
  GUI_SetColor(GUI_BLUE);  
	GUI_DrawPoint(x,y);//אф֣ 
	GUI_DrawPoint(x+1,y);
	GUI_DrawPoint(x,y+1);
	GUI_DrawPoint(x+1,y+1);	 	  	
}


void TP_Adjust(void)
{	
	u8 i;
						 
	u16 pos_temp[4][2];											//ظҪۺզֵ
	u16 d1,d2;
	u32 tem1,tem2;
	float fac; 

	while (1){
								 
		for (i=0;i<4;i++){
		
			GUI_Clear();
			if (i==0){
				Drow_Touch_Point(20,20);
				Touch_GetXY(&pos_temp[0][0],&pos_temp[0][1],1);
				//printf("\n\rX1:%d , Y1:%d",pos_temp[0][0],pos_temp[0][1]);
			}
			else if (i == 1){
				Drow_Touch_Point(460,20);	
				Touch_GetXY(&pos_temp[1][0],&pos_temp[1][1],1);	
				//printf("\n\rX1:%d , Y1:%d",pos_temp[1][0],pos_temp[1][1]);
			}
			else if (i == 2){
				Drow_Touch_Point(20,246-10);
				Touch_GetXY(&pos_temp[2][0],&pos_temp[2][1],1);	
				//printf("\n\rX1:%d , Y1:%d",pos_temp[2][0],pos_temp[2][1]);
			}
			else {
			
				Drow_Touch_Point(460,246-10);
				Touch_GetXY(&pos_temp[3][0],&pos_temp[3][1],1);	
				//printf("\n\rX1:%d , Y1:%d",pos_temp[3][0],pos_temp[3][1]);
			}
		}

			tem1=abs(pos_temp[0][0]-pos_temp[1][0]);					//x1-x2
			tem2=abs(pos_temp[0][1]-pos_temp[1][1]);					//y1-y2
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);											//փս1,2քߠk
			
			tem1=abs(pos_temp[2][0]-pos_temp[3][0]);					//x3-x4
			tem2=abs(pos_temp[2][1]-pos_temp[3][1]);					//y3-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);											//փս3,4քߠk
			fac=(float)d1/d2;
			if (fac<0.90||fac>1.10||d1==0||d2==0)						//һۏٱ
			{
				//continue;
			}
			tem1=abs(pos_temp[0][0]-pos_temp[2][0]);					//x1-x3
			tem2=abs(pos_temp[0][1]-pos_temp[2][1]);					//y1-y3
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);											//փս1,3քߠk
			
			tem1=abs(pos_temp[1][0]-pos_temp[3][0]);					//x2-x4
			tem2=abs(pos_temp[1][1]-pos_temp[3][1]);					//y2-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);											//փս2,4քߠk
			fac=(float)d1/d2;
			if (fac<0.90||fac>1.10)										//һۏٱ
			{
				//continue;
			}
			//ֽȷ
						   
			//הއПРֈ
			tem1=abs(pos_temp[1][0]-pos_temp[2][0]);				//x1-x3
			tem2=abs(pos_temp[1][1]-pos_temp[2][1]);				//y1-y3
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);										//փս1,4քߠk

			tem1=abs(pos_temp[0][0]-pos_temp[3][0]);				//x2-x4
			tem2=abs(pos_temp[0][1]-pos_temp[3][1]);				//y2-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);										//փս2,3քߠk
			fac=(float)d1/d2;
			if (fac<0.90||fac>1.10)//һۏٱ
			{
				//continue;
			}
			//ֽȷ

			//݆̣ޡڻ
			tp_dev.xfac=(float)460/(pos_temp[1][0]-pos_temp[0][0]);			//փսxfac		 
			tp_dev.xoff=(480-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;			//փսxoff
				  
			tp_dev.yfac=(float)(246-20)/(pos_temp[2][1]-pos_temp[1][1]);			//փսyfac
			tp_dev.yoff=(246-tp_dev.yfac*(pos_temp[2][1]+pos_temp[1][1]))/2;			//փսyoff  

			GUI_Clear();
			//printf("\n\r%f,%f   %d,%d",tp_dev.xfac,tp_dev.yfac,tp_dev.xoff,tp_dev.yoff);
			break;	
	}

	/*
	u8 i;
						 
	u16 pos_temp[4][2];											//ظҪۺզֵ
	u16 d1,d2;
	u32 tem1,tem2;
	float fac; 

	while (1){
								 
		for (i=0;i<4;i++){
		
			GUI_Clear();
			if (i==0){
				Drow_Touch_Point(20,20);
				Touch_GetXY(&pos_temp[0][0],&pos_temp[0][1],1);
			}
			else if (i == 1){
				Drow_Touch_Point(460,20);	
				Touch_GetXY(&pos_temp[1][0],&pos_temp[1][1],1);	
			}
			else if (i == 2){
				Drow_Touch_Point(20,252);
				Touch_GetXY(&pos_temp[2][0],&pos_temp[2][1],1);	
			}
			else {
			
				Drow_Touch_Point(460,252);
				Touch_GetXY(&pos_temp[3][0],&pos_temp[3][1],1);	
			}
		}

			tem1=abs(pos_temp[0][0]-pos_temp[1][0]);					//x1-x2
			tem2=abs(pos_temp[0][1]-pos_temp[1][1]);					//y1-y2
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);											//փս1,2քߠk
			
			tem1=abs(pos_temp[2][0]-pos_temp[3][0]);					//x3-x4
			tem2=abs(pos_temp[2][1]-pos_temp[3][1]);					//y3-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);											//փս3,4քߠk
			fac=(float)d1/d2;
			if (fac<0.95||fac>1.05||d1==0||d2==0)						//һۏٱ
			{
				//continue;
			}
			tem1=abs(pos_temp[0][0]-pos_temp[2][0]);					//x1-x3
			tem2=abs(pos_temp[0][1]-pos_temp[2][1]);					//y1-y3
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);											//փս1,3քߠk
			
			tem1=abs(pos_temp[1][0]-pos_temp[3][0]);					//x2-x4
			tem2=abs(pos_temp[1][1]-pos_temp[3][1]);					//y2-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);											//փս2,4քߠk
			fac=(float)d1/d2;
			if (fac<0.95||fac>1.05)										//һۏٱ
			{
				//continue;
			}
			//ֽȷ
						   
			//הއПРֈ
			tem1=abs(pos_temp[1][0]-pos_temp[2][0]);				//x1-x3
			tem2=abs(pos_temp[1][1]-pos_temp[2][1]);				//y1-y3
			tem1*=tem1;
			tem2*=tem2;
			d1=sqrt(tem1+tem2);										//փս1,4քߠk

			tem1=abs(pos_temp[0][0]-pos_temp[3][0]);				//x2-x4
			tem2=abs(pos_temp[0][1]-pos_temp[3][1]);				//y2-y4
			tem1*=tem1;
			tem2*=tem2;
			d2=sqrt(tem1+tem2);										//փս2,3քߠk
			fac=(float)d1/d2;
			if (fac<0.95||fac>1.05)//һۏٱ
			{
				//continue;
			}
			//ֽȷ

			//݆̣ޡڻ
			tp_dev.xfac=(float)460/(pos_temp[1][0]-pos_temp[0][0]);			//փսxfac		 
			tp_dev.xoff=(480-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;			//փսxoff
				  
			tp_dev.yfac=(float)252/(pos_temp[2][1]-pos_temp[1][1]);			//փսyfac
			tp_dev.yoff=(272-tp_dev.yfac*(pos_temp[2][1]+pos_temp[1][1]))/2;			//փսyoff  

			GUI_Clear();
			printf("\n\r%f,%f   %d,%d",tp_dev.xfac,tp_dev.yfac,tp_dev.xoff,tp_dev.yoff);
			break;	
	}
*/	
}

u8 TP_Init(void)
{			    		   
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF, ENABLE);	 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_1);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_13);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				  
 	GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_SetBits(GPIOF,GPIO_Pin_9);

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		  
 	GPIO_Init(GPIOF, &GPIO_InitStructure);

  TP_Read_XY(&tp_dev.x,&tp_dev.y);  

	
 	//AT24CXX_Init(); 
  //TP_Adjust();   
	/*
	if (TP_Get_Adjdata())return 0; 
	else			   
	{ 										    
	  TP_Adjust();   
		TP_Save_Adjdata();	 
	}			
	TP_Get_Adjdata();	
	*/
	return 1; 									 
}


