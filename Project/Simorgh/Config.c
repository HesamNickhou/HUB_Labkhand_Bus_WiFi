#include <includes.h>
#include <stddef.h>
#include <stdio.h>
#include "Config.h"
#include "app_cfg.h"
#include "key/Keypad.h"
#include "GUI.h"
#include "gps/gps.h"
#include "fatfs/ff.h"
#include "flash/flash.h"
#include "beep/beep.h"
#include "menu3d.h"

unsigned char BOARD_VER = VII;
unsigned char LCDType   = LCD480_272;

TConfig Config;
TIndicators Indicators;

#ifdef Simorgh50N
	unsigned char *FontBKoodak40   = (unsigned char *)(0x08064000);
	unsigned char *FontBNazanin200 = (unsigned char *)(0x08064000 + 54924);
	unsigned char *FontBNazanin60  = (unsigned char *)(0x08064000 + 86664);
	unsigned char *FontTahoma20    = (unsigned char *)(0x08064000 + 90328);
	unsigned char *FontBNazanin140 = (unsigned char *)(0x08064000 + 38392);
#endif

GUI_FONT ExtFont;

unsigned char PowerMode = ON;
unsigned char SDCardPresent = 0;

unsigned char _acBuffer[DATA_SIZE];

#ifdef Simorgh50N
const TResource Resources[] = 
{
{"0:ani/A0000/0028.bmp",0,20792},
{"0:desktop/00010.bmp",20792,6968},
{"0:desktop/00011.bmp",27760,6968},
{"0:desktop/00012.bmp",34728,6968},
{"0:desktop/0002.bmp",41696,26168},
{"0:desktop/ant0.bmp",67864,3128},
{"0:desktop/ant1.bmp",70992,3128},
{"0:desktop/ant2.bmp",74120,3128},
{"0:desktop/ant3.bmp",77248,3128},
{"0:desktop/ant4.bmp",80376,3128},
{"0:desktop/bat0.bmp",83504,3128},
{"0:desktop/bat1.bmp",86632,3128},
{"0:desktop/bat2.bmp",89760,3128},
{"0:desktop/bat3.bmp",92888,3128},
{"0:desktop/bat4.bmp",96016,3128},
{"0:desktop/EDlg.bmp",99144,4856},
{"0:desktop/enseraf.bmp",104000,5816},
{"0:desktop/gps0.bmp",109816,3128},
{"0:desktop/gps1.bmp",112944,3128},
{"0:desktop/gps2.bmp",116072,3128},
{"0:desktop/gsm0.bmp",119200,3128},
{"0:desktop/gsm1.bmp",122328,3128},
{"0:desktop/gsm2.bmp",125456,3128},
{"0:desktop/IDlg.bmp",128584,4856},
{"0:desktop/InputBox.bmp",133440,19256},
{"0:desktop/spk0.bmp",152696,3128},
{"0:desktop/spk1.bmp",155824,3128},
{"0:desktop/taeed.bmp",158952,5816},
{"0:desktop/WDlg.bmp",164768,5816},
{"0:driver.bmp",170584,27702},
{"0:family.bmp",198286,16854},
{"0:help/0000.bmp",215140,230456},
{"0:icons/back.bmp",445596,6968},
{"0:icons/c0.bmp",452564,1784},
{"0:icons/c1.bmp",454348,1784},
{"0:icons/frame.bmp",456132,6968},
{"0:icons/header1.bmp",463100,15416},
{"0:icons/M0000/000.bmp",478516,6968},
{"0:icons/M0000/001.bmp",485484,6968},
{"0:icons/M0000/002.bmp",492452,6968},
{"0:icons/M0000/003.bmp",499420,6968},
{"0:icons/M0001/000.bmp",506388,6968},
{"0:icons/M0001/001.bmp",513356,6968},
{"0:icons/M0001/002.bmp",520324,6968},
{"0:icons/M0002/000.bmp",527292,6968},
{"0:icons/M0002/001.bmp",534260,6968},
{"0:icons/M0002/002.bmp",541228,6968},
{"0:icons/M0002/003.bmp",548196,6968},
{"0:icons/M0002/004.bmp",555164,6968},
{"0:icons/M0002/005.bmp",562132,6968},
{"0:icons/M0003/000.bmp",569100,6968},
{"0:icons/M0003/001.bmp",576068,6968},
{"0:icons/M0003/002.bmp",583036,6968},
{"0:icons/M0003/003.bmp",590004,6968},
{"0:icons/M0004/000.bmp",596972,6968},
{"0:icons/M0004/001.bmp",603940,6968},
{"0:icons/M0005/M001.bmp",610908,6968},
{"0:icons/M0005/M002.bmp",617876,6968},
{"0:icons/r0.bmp",624844,1784},
{"0:icons/r1.bmp",626628,1784},
{"0:left.bmp",628412,19254},
{"0:map.bmp",647666,30678},
{"0:mode.bmp",678344,30678},
{"0:navi/1.bmp",709022,824},
{"0:navi/2.bmp",709846,824},
{"0:navi/3.bmp",710670,824},
{"0:navi/4.bmp",711494,824},
{"0:navi/5.bmp",712318,824},
{"0:navi/6.bmp",713142,824},
{"0:navi/7.bmp",713966,824},
{"0:navi/8.bmp",714790,824},
{"0:rep.bmp",715614,30678},
{"0:repair.bmp",746292,17094},
{"0:speed.bmp",763386,31254},
{"\0",0,0}
};
#endif
#ifdef Torgheh
const TResource Resources[] = 
{
{"0:ani/A0000/0028.bmp",0,20790},
{"0:Cent20.sif",20790,2812},
{"0:desktop/00010.bmp",23602,6968},
{"0:desktop/00011.bmp",30570,6968},
{"0:desktop/00012.bmp",37538,6968},
{"0:desktop/0002.bmp",44506,26168},
{"0:desktop/ant0.bmp",70674,3128},
{"0:desktop/ant1.bmp",73802,3128},
{"0:desktop/ant2.bmp",76930,3128},
{"0:desktop/ant3.bmp",80058,3128},
{"0:desktop/ant4.bmp",83186,3128},
{"0:desktop/bat0.bmp",86314,3128},
{"0:desktop/bat1.bmp",89442,3128},
{"0:desktop/bat2.bmp",92570,3128},
{"0:desktop/bat3.bmp",95698,3128},
{"0:desktop/bat4.bmp",98826,3128},
{"0:desktop/EDlg.bmp",101954,4856},
{"0:desktop/enseraf.bmp",106810,5816},
{"0:desktop/gps0.bmp",112626,3128},
{"0:desktop/gps1.bmp",115754,3128},
{"0:desktop/gps2.bmp",118882,3128},
{"0:desktop/gsm0.bmp",122010,3128},
{"0:desktop/gsm1.bmp",125138,3128},
{"0:desktop/gsm2.bmp",128266,3128},
{"0:desktop/IDlg.bmp",131394,4856},
{"0:desktop/InputBox.bmp",136250,19256},
{"0:desktop/spk0.bmp",155506,3128},
{"0:desktop/spk1.bmp",158634,3128},
{"0:desktop/taeed.bmp",161762,5816},
{"0:desktop/WDlg.bmp",167578,5816},
{"0:driver.bmp",173394,27702},
{"0:icons/012.bmp",201096,2934},
{"0:icons/013.bmp",204030,2934},
{"0:icons/back.bmp",206964,3126},
{"0:icons/c0.bmp",210090,1784},
{"0:icons/c1.bmp",211874,1784},
{"0:icons/frame.bmp",213658,3126},
{"0:icons/header1.bmp",216784,15416},
{"0:icons/M0000/000.bmp",232200,3126},
{"0:icons/M0000/001.bmp",235326,3126},
{"0:icons/M0000/002.bmp",238452,3126},
{"0:icons/M0000/003.bmp",241578,3126},
{"0:icons/M0001/000.bmp",244704,3126},
{"0:icons/M0001/001.bmp",247830,3126},
{"0:icons/M0001/002.bmp",250956,3126},
{"0:icons/M0002/000.bmp",254082,3126},
{"0:icons/M0002/001.bmp",257208,3126},
{"0:icons/M0002/002.bmp",260334,3126},
{"0:icons/M0002/003.bmp",263460,3126},
{"0:icons/M0002/004.bmp",266586,3126},
{"0:icons/M0002/005.bmp",269712,3126},
{"0:icons/M0003/000.bmp",272838,3126},
{"0:icons/M0003/001.bmp",275964,3126},
{"0:icons/M0003/002.bmp",279090,3126},
{"0:icons/M0003/003.bmp",282216,3126},
{"0:icons/M0004/000.bmp",285342,3126},
{"0:icons/M0004/001.bmp",288468,3126},
{"0:icons/M0005/M001.bmp",291594,3126},
{"0:icons/M0005/M002.bmp",294720,3126},
{"0:icons/r0.bmp",297846,1784},
{"0:icons/r1.bmp",299630,1784},
{"0:navi/1.bmp",301414,824},
{"0:navi/2.bmp",302238,824},
{"0:navi/3.bmp",303062,824},
{"0:navi/4.bmp",303886,824},
{"0:navi/5.bmp",304710,824},
{"0:navi/6.bmp",305534,824},
{"0:navi/7.bmp",306358,824},
{"0:navi/8.bmp",307182,824},
{"0:Yekan30.sif",308006,10236},
{"0:Yekan80.sif",318242,3688},
{"\0",0,0}
};
#endif

#define SIZE_UTF8Table 36
static const u16 TableUTF8[SIZE_UTF8Table] = {
	0xD8A7 , 0xD8A2 , 0xD8A8 , 0xD9BE , 0xD8AA , 0xD8AB ,
	0xD8AC , 0xD8AD , 0xD8AE , 0xDA86 , 0xD8AF , 0xD8B0 ,
	0xD8B1 , 0xD8B2 , 0xDA98 , 0xD8B3 , 0xD8B4 , 0xD8B5 ,
	0xD8B6 , 0xD8B7 , 0xD8B8 , 0xD8B9 , 0xD8BA , 0xD981 , 
	0xD982 , 0xDAA9 , 0xDAAF , 0xD984 , 0xD985 , 0xD986 , 
	0xD988 , 0xD987 , 0xD98A , 0xD8A4 , 0xD8A1 , 0xDB8C 
};

static const u8 TableWin1256[SIZE_UTF8Table] = {
	0xC7 , 0xC2 , 0xC8 , 0x81 , 0xCA , 0xCB ,
	0xCC , 0xCD , 0xCE , 0x8d , 0xCF , 0xD0 , 
	0xD1 , 0xD2 , 0x8E , 0xD3 , 0xD4 , 0xD5 ,
	0xD6 , 0xD8 , 0xD9 , 0xDA , 0xDB , 0xDD ,
	0xDE , 0x98 , 0x90 , 0xE1 , 0xE3 , 0xE4 ,
	0xE6 , 0xE5 , 0xED , 0xC4 , 0xC1 , 0x89
};

const unsigned int CRC16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

const unsigned char KeyTable[256] = {
    0xf4, 0x41, 0x17, 0x27, 0xab, 0x9d, 0xfa, 0xe3, 0x30, 0x76, 0xcc, 0x02, 0xe5, 0x2a, 0x35, 0x62,
    0xb1, 0xba, 0xea, 0xfe, 0x2f, 0x4c, 0x46, 0xd3, 0x8f, 0x92, 0x6d, 0x52, 0xbe, 0x74, 0xe0, 0xc9,
    0xc2, 0x8e, 0x58, 0xb9, 0xe1, 0x88, 0x20, 0xce, 0xdf, 0x1a, 0x51, 0x53, 0x7c, 0x42, 0x84, 0x00,
    0x64, 0x6b, 0x81, 0x08, 0xa4, 0x3f, 0xa5, 0xa2,
    0x48, 0x45, 0xde, 0x7b, 0x73, 0x4b, 0x1f, 0x55, 0xeb, 0xb5, 0xc5, 0x37, 0x28, 0xbf, 0x03, 0x16,
    0xcf, 0x79, 0x07, 0x69, 0xda, 0x05, 0x34, 0xa6, 0x2e, 0xf3, 0x8a, 0xf6, 0x83, 0x60, 0x71, 0x6e,
    0x21, 0xdd, 0x3e, 0xe6, 0x54, 0xc4, 0x06, 0x50, 0x98, 0xbd, 0x40, 0xd9, 0xe8, 0x89, 0x19, 0xc8,
    0x80, 0x2b, 0x11, 0x5a, 0x0e, 0x85, 0xae, 0x2d, 0x0f, 0x5c, 0x5b, 0x36, 0x0a, 0x57, 0xee, 0x9b,
    0xc0, 0xdc, 0x77, 0x12, 0x93, 0xa0, 0x22, 0x1b, 0x09, 0x8b, 0xb6, 0x1e, 0xf1, 0x75, 0x99, 0x7f,
    0x43, 0x23, 0xed, 0xe4, 0x01, 0x72, 0x66, 0xfb, 0x31, 0x63, 0x97, 0xc6, 0x4a, 0xbb, 0xf9, 0x29,
    0x9e, 0xb2, 0x86, 0xc1, 0xb3, 0x70, 0x94, 0xe9, 0xfc, 0xf0, 0x7d, 0x33, 0x49, 0x38, 0xca, 0xd4, 
    0x3a, 0x78, 0x5f, 0x7e, 0xf5, 0x7a, 0xb7, 0xad, 0x8d, 0xd8, 0x39, 0xc3, 0x5d, 0xd0, 0xd5, 0x25,
    0xac, 0x18, 0x9c, 0x3b, 0x26, 0x59, 0x9a, 0x4f, 0x95, 0xff, 0xbc, 0x15, 0xe7, 0x6f, 0x9f, 0xb0, 
    0x4e, 0x82, 0x90, 0xa7, 0x04, 0xec, 0xcd, 0x91, 0x4d, 0xef, 0xaa, 0x96, 0xd1, 0x6a, 0x2c, 0x65,
    0x5e, 0x8c, 0x87, 0x0b, 0xcb, 0x32, 0x6c, 0xb8, 0x67, 0xdb, 0x10, 0xd6, 0xd7, 0xa1, 0xf8, 0x13,
    0xa9, 0x61, 0x1c, 0x47, 0xd2, 0xf2, 0x14, 0xc7, 0xf7, 0xfd, 0x3d, 0x44, 0xaf, 0x68, 0x24, 0xa3,
    0x1d, 0xe2, 0x3c, 0x0d, 0xa8, 0x0c, 0xb4, 0x56,
};

const unsigned int UCMap[255] = {
	2937, //Kashan
	5436, //Kerman
	5540, //Rasht
	5750, //Zahedan
	5780, //Shahin Shahr
	5925, //Bandar Abbas
	5950, //Shahriar
	6010, //Bam
	6025, //Saveeh
	6040, //Bandar Anzali
	6105, //Sabzevar
	6110, //Khoramshahr
	6115, //Mahshahr
	6135, //Marvdasht
	6165, //Malard
	6250, //Jahrom
	6285, //Dezfool
	6310, //Broujerd
	6325, //Bojnoord
	6330, //Neishaboor
	6340,
	6341, 6342, 6343, 6344, 6345, 6346, 6347, 6348, 6349, 6350,
	6351, 6352, 6353, 6354, 6355, 6356, 6357, 6358, 6359, 6360,
	6361, 6362, 6363, 6364, 6365, 6366, 6367, 6368, 6369, 6370,
	6371, 6372, 6373, 6374, 6375, 6376, 6377, 6378, 6379, 6380,
	6381, 6382, 6383, 6384, 6385, 6386, 6387, 6388, 6389, 6390,
	6391, 6392, 6393, 6394, 6395, 6396, 6397, 6398, 6399, 6400,
	6401, 6402, 6403, 6404, 6405, 6406, 6407, 6408, 6409, 6410,
	6411, 6412, 6413, 6414, 6415, 6416, 6417, 6418, 6419, 6420,
	6421, 6422, 6423, 6424, 6425, 6426, 6427, 6428, 6429, 6430,
	6431, 6432, 6433, 6434, 6435, 6436, 6437, 6438, 6439, 6440,
	6441, 6442, 6443, 6444, 6445, 6446, 6447, 6448, 6449, 6450,
	6451, 6452, 6453, 6454, 6455, 6456, 6457, 6458, 6459, 6460,
	6461, 6462, 6463, 6464, 6465, 6466, 6467, 6468, 6469, 6470,
	6471, 6472, 6473, 6474, 6475, 6476, 6477, 6478, 6479, 6480,
	6481, 6482, 6483, 6484, 6485, 6486, 6487, 6488, 6489, 6490,
	6491, 6492, 6493, 6494, 6495, 6496, 6497, 6498, 6499, 6500,
	6501, 6502, 6503, 6504, 6505, 6506, 6507, 6508, 6509, 6510,
	6511, 6512, 6513, 6514, 6515, 6516, 6517, 6518, 6519, 6520,
	6521, 6522, 6523, 6524, 6525, 6526, 6527, 6528, 6529, 6530,
	6531, 6532, 6533, 6534, 6535, 6536, 6537, 6538, 6539, 6540,
	6541, 6542, 6543, 6544, 6545, 6546, 6547, 6548, 6549, 6550,
	6551, 6552, 6553, 6554, 6555, 6556, 6557, 6558, 6559, 6560,
	6561, 6562, 6563, 6564, 6565, 6566, 6567, 6568, 6569,
	6570, //Tabriz
	6571, //Kashan 
	6572, 6573, 6574
};

unsigned char  TFT_START_Y = 0; 
unsigned short TFT_HEIGHT  = 480;

#if (DeviceType == Basket)
TPerson Person;
TSeries Series[30];
TKalas Kalas[30];
#endif

               
//------------------------------------------------------------------------------
void IntToByte(unsigned int INT,unsigned char *BYTE) {
  BYTE[0] =  INT  			& 0xFF;
  BYTE[1] = (INT >> 8)  & 0xFF;
  BYTE[2] = (INT >> 16) & 0xFF;
  BYTE[3] = (INT >> 24) & 0xFF; 
}
//================================================================================
void GenerateCardIdUC(unsigned char *buf, unsigned char *snr, unsigned int *UC, 
	unsigned int *CardID) {
	unsigned char 
		BCC, 
		data[16];
	unsigned int i;

  for (i=0; i<16; i++)
    data[i] = buf[i];

  BCC = 0xC9;
  for (i=0; i<15; i++)
    BCC ^= data[i];
    if (BCC == data[15]) {
      for (i=0; i<15; i++)
        data[i] ^= KeyTable[snr[i % 4] / ((i / 4) + 1)] ^ snr[3 - (i % 4)];

      BCC = 0xE2;
      for (i=0; i<14; i++)
        BCC ^= data[i];
      if (BCC != data[14]) {
        for (i=0; i<16; i++)
          data[i] = buf[i];
        BCC = data[15] + 1;
      }
			else {
        *UC  = data[1]; *UC <<= 8;
        *UC += data[0];

        *CardID  = data[5]; *CardID <<= 8;
        *CardID += data[4]; *CardID <<= 8;
        *CardID += data[3]; *CardID <<= 8;
        *CardID += data[2];
        BCC 		 = data[15];
      }
    }

    if (BCC != data[15]) {
      *UC  = buf[1];  *UC <<= 8;
      *UC += buf[0];
      *CardID  = buf[5]; *CardID <<= 8;
      *CardID += buf[4]; *CardID <<= 8;
      *CardID += buf[3]; *CardID <<= 8;
      *CardID += buf[2];
      if ((*UC & 0x8000) == 0x8000) {
        *UC = *UC & 0x7FFF;
        *UC = *UC ^ 0x6A29;
        *CardID = *CardID ^ 0xB2A6;
      }
    }
}

//==============================================================================
unsigned char LoadFromDFToRam(unsigned int memoryOffset, unsigned int sizeToCopy, 
	unsigned char *pDest) {
	WDTR;
	SPI_Flash_Read(pDest, memoryOffset, sizeToCopy);
}
//==============================================================================
unsigned char SaveFromRamToDF(unsigned int memoryOffset, 
	unsigned int sizeToCopy, unsigned char *pDest) {
	WDTR;
	SPI_Flash_Write(pDest, memoryOffset, sizeToCopy);
}

//==============================================================================
union {
  unsigned char Buf[4 * 512];
  TConfig Cfg;
} ConfigTransfer;
unsigned char SaveConfiguration(void) {
	#if (DeviceType==BUSDOOR)
  Config.Type=DeviceType+0xCA;
  ConfigTransfer.Cfg=Config;
  SaveFromRamToDF(addConfig2, 4*512, ConfigTransfer.Buf);
	#else
	FLASH_UNLOCK
  Config.Type=DeviceType+0xCA;
  ConfigTransfer.Cfg=Config;
  SaveFromRamToDF(addConfig, 4*512, ConfigTransfer.Buf);
	FLASH_LOCK
	#endif
}

//==============================================================================
unsigned char LoadConfiguration(void) {
	#if (DeviceType==BUSDOOR)
  LoadFromDFToRam(addConfig2, 4*512, ConfigTransfer.Buf);
  Config=ConfigTransfer.Cfg;
	if (Config.Type != DeviceType+0xCA)
	{
		LoadFromDFToRam(addConfig, 4*512, ConfigTransfer.Buf);
		Config=ConfigTransfer.Cfg;
		SaveConfiguration();
	}
	#else
  LoadFromDFToRam(addConfig, 4*512, ConfigTransfer.Buf);
  Config=ConfigTransfer.Cfg;
	#endif

  if (Config.Type != DeviceType+0xCA)
  {
    Indicators.Transactions=0;
    Indicators.Locations=0;
    Indicators.OffTransactions=0;
    Indicators.OffLocations=0;

    SaveIndicators();
    
    Config.DeviceID=1;
    strcpy(Config.MainPassword,"111111");
    Config.Voice=1;
    Config.Beep=1;
		sprintf(Config.HostIP,"178.131.057.058");
				Config.Password=1;
    SaveConfiguration();
  }
	
	if (Config.TFTType == 2)	{
		TFT_START_Y = 12; 
    TFT_HEIGHT  = 240;
	}	

  return 0;
}

//==============================================================================
unsigned char SaveIndicators(void) {return 0;}
//==============================================================================
unsigned char LoadIndicators(void) {return 0;}
//==============================================================================
unsigned char GetResourceAddress(char *title, unsigned int *StartAddress, 
	unsigned int *Len) {
	unsigned int i  = 0;
	unsigned char j = 0;

  *Len = 0;   
  for (i=0; (Resources[i].Title[0] != 0) && (i < 200); i++) {
    for (j=0; j<250; j++) {
      if (title[j] == 0) {
        *Len = Resources[i].Len;
				*StartAddress = addResources+Resources[i].StartAddress;
        return 0;
      }
      if (Resources[i].Title[j] != title[j])
        break;
    }
  }
  return 1;
}


#ifdef Torgheh
//==============================================================================
extern unsigned char RequestType;
unsigned char fontBuffer[10*1024];
void LoadFont(unsigned char FileType)
{
char FileName[20];
unsigned int StartAddress, Len;

	if (RequestType==0xF1) 
		return;
	
	switch (FileType)
	{
		case 0: sprintf(FileName, "0:Yekan30.sif"); break;
		case 1: sprintf(FileName, "0:Yekan80.sif"); break;
		default: sprintf(FileName, "0:Cent20.sif"); break;
	}
  if (GetResourceAddress((char *)FileName, &StartAddress, &Len))
	  return;
	
	if (Len>10*1024)
    Len=10*1024; 
		//return;
	
  GUI_SIF_DeleteFont(&ExtFont);

  SPI_FLASH_CS=0;      
  SPI2_ReadWriteByte(W25X_ReadData);    
  SPI2_ReadWriteByte((u8)((StartAddress) >> 16)); 
  SPI2_ReadWriteByte((u8)((StartAddress) >> 8));   
  SPI2_ReadWriteByte((u8)StartAddress);   

	for (StartAddress=0;StartAddress<Len; StartAddress++)	
	{
    while ((SPI2->SR&SPI_I2S_FLAG_TXE) != SPI_I2S_FLAG_TXE);
    SPI2->DR = 0xFF; 
    while ((SPI2->SR&SPI_I2S_FLAG_RXNE) != SPI_I2S_FLAG_RXNE);
    fontBuffer[StartAddress] =SPI2->DR;  
  }
  SPI_FLASH_CS=1;      

	if ((fontBuffer[0] != 'G') || (fontBuffer[1] != 'U') || (fontBuffer[2] != 'I') || (fontBuffer[3] != 'P'))
		return;
	
  GUI_SIF_CreateFont(fontBuffer, &ExtFont, GUI_SIF_TYPE_PROP);
}
#endif

#ifdef Simorgh50N
//==============================================================================
void LoadFont(const unsigned char* Data)
{
	if ((Data[0] != 'G') || (Data[1] != 'U') || (Data[2] != 'I') || (Data[3] != 'P'))
	{
		if (Data==FontBNazanin140)
			Data=FontBKoodak40;
		//else if (Data==FontTahoma20)
		//	Data=FontTahoma20;
		else
		  GUI_SetFont(&GUI_Font6x8);

		return;
	}
	
  GUI_SIF_CreateFont(Data, &ExtFont, GUI_SIF_TYPE_PROP_AA2);
}
#endif

//==============================================================================
void LoadResourceFromSD2DF(void) {
	unsigned int i,j;
	unsigned short Loc;
	unsigned int ByteRead;
	FRESULT res;
	DIR dirs;
	FIL FileObject;
	unsigned int address=addResources;
   
    // Open the file
	/*
	  res = f_open(&FileObject, "0:Simorgh.res", FA_OPEN_EXISTING|FA_READ);
    if ( res != FR_OK ) 
      return;

    ShowMessageDlg(mtError, "در حال انتقال",0,0,0,0);  
    while (1)
    {
      res = f_read(&FileObject, _acBuffer, DATA_SIZE, &ByteRead);
			WDTR;
      if (res==FR_OK) 
      {
        SaveFromRamToDF(address, ByteRead, _acBuffer);
				address += ByteRead;
      }
      if (ByteRead<DATA_SIZE)
        break;
    }

    //ShowMessageDlg(mtError, "انجام شد",1,1,2000,3);  
    // Close the file
    res = f_close(&FileObject);
    if ( res != FR_OK )
      return;
		
		ShowMessageDlg(mtError, "انجام شد",1,1,2000,3);  
		*/
		//..................................      ........................................
		address=addVoices;
		
    // Open the file
	  res = f_open(&FileObject, "0:Simorgh.res", FA_OPEN_EXISTING|FA_READ);
    if ( res != FR_OK ) 
      return;

    ShowMessageDlg(mtError, "در حال انتقال صدا",0,0,0,0);  
		
			FLASH_UNLOCK
		
    while (1)
    {
      res = f_read(&FileObject, _acBuffer, DATA_SIZE, &ByteRead);
			WDTR;
      if (res==FR_OK) 
      {
        SaveFromRamToDF(address, ByteRead, _acBuffer);
				address += ByteRead;
      }
      if (ByteRead<DATA_SIZE)
        break;
    }

			FLASH_LOCK
    //ShowMessageDlg(mtError, "انجام شد",1,1,2000,3);  
    // Close the file
    res = f_close(&FileObject);
    if ( res != FR_OK )
      return;
		
		ShowMessageDlg(mtError, "انجام شد",1,1,2000,3); 		
}

//==============================================================================
s8 ConvertUTF8TOWin1256(u8 *StringUTF8, u8 *BuffStrWin1256) {
	u16 i=0, j=0;
	u16 CountLen=0;
	u16 CStrUtf8=0;

	for (i=0; (StringUTF8[i] != 0) && (i<100); )
	{
		if (StringUTF8[i]<=122)
		{
			BuffStrWin1256[CountLen++] =StringUTF8[i];
			i++;
			continue;
		}
		CStrUtf8= ((u16)(StringUTF8[i] << 8)) + StringUTF8[i+1];
		for (j=0; j<SIZE_UTF8Table; j++)
		{
			if (CStrUtf8 == TableUTF8[j])
				break;
		}
		BuffStrWin1256[CountLen++] = TableWin1256[j];
		i += 2;
	}
	BuffStrWin1256[CountLen++] = 0;
	return 0;
}

//========= = == = == = ==============================================================
s8 ConvertWin1256TOUTF8(u8 *StringWin1256, u8 *BuffStrUTF8) {
	u16 i = 0 , 
			j = 0;
	u16 CountLen = 0;
	u16 CStrUtf8 = 0;
	
	for (i=0; (StringWin1256[i] != 0) && (i < 200); i++) {
		if (StringWin1256[i] <= 122) {
			BuffStrUTF8[CountLen++] = StringWin1256[i];
			continue;
		}
		for (j=0; j<SIZE_UTF8Table; j++)
			if (StringWin1256[i] == TableWin1256[j]) 
				break;
		if (j>=SIZE_UTF8Table) {
			BuffStrUTF8[CountLen++] = ' ';
			continue;
		}
		CStrUtf8 = TableUTF8[j];
		BuffStrUTF8[CountLen++] = CStrUtf8 >> 8;
		BuffStrUTF8[CountLen++] = CStrUtf8 & 0x00FF;
	}
	BuffStrUTF8[CountLen++] = 0;
	return 0;
}


