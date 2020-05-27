#ifndef UTFT_h
#define UTFT_h

#include "stm32f4xx.h"


#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)

//#define cbi(reg, bitmask) *reg &= ~bitmask
//#define sbi(reg, bitmask) *reg |= bitmask

//#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
//#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

//#define cport(port, data) port &= data
//#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}
/*
#define fontbyte(x) (&cfont.font[x])

#define regtype volatile uint8_t
#define regsize uint8_t*/



#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define PORTRAIT 0
#define LANDSCAPE 1

#define HX8347A			0
#define ILI9327			1
#define SSD1289			2
#define ILI9325C		3
#define ILI9325D_8		4
#define ILI9325D_16		5
#define HX8340B_8		6
#define HX8340B_S		7
#define HX8352A			8
#define ST7735			9
#define PCF8833			10
#define S1D19122		11
#define SSD1963_480		12
#define SSD1963_800		13
#define S6D1121_8		14
#define S6D1121_16		15
#define	SSD1289LATCHED	16
#define ILI9320_8		17
#define ILI9320_16		18
#define SSD1289_8		19
#define	SSD1963_800ALT	20
#define ILI9481			21

#define ITDB32			0	// HX8347-A (16bit)
#define ITDB32WC		1	// ILI9327  (16bit)
#define TFT01_32W		1	// ILI9327	(16bit)
#define ITDB32S			2	// SSD1289  (16bit)
#define TFT01_32		2	// SSD1289  (16bit)
#define CTE32			2	// SSD1289  (16bit)
#define GEEE32			2	// SSD1289  (16bit)
#define ITDB24			3	// ILI9325C (8bit)
#define ITDB24D			4	// ILI9325D (8bit)
#define ITDB24DWOT		4	// ILI9325D (8bit)
#define ITDB28			4	// ILI9325D (8bit)
#define TFT01_24_8		4	// ILI9325D (8bit)
#define TFT01_24_16		5	// ILI9325D (16bit)
#define ITDB22			6	// HX8340-B (8bit)
#define GEEE22			6	// HX8340-B (8bit)
#define ITDB22SP		7	// HX8340-B (Serial)
#define ITDB32WD		8	// HX8352-A (16bit)
#define TFT01_32WD		8	// HX8352-A	(16bit)
#define ITDB18SP		9	// ST7735   (Serial)
#define LPH9135			10	// PCF8833	(Serial)
#define ITDB25H			11	// S1D19122	(16bit)
#define ITDB43			12	// SSD1963	(16bit) 480x272
#define ITDB50			13	// SSD1963	(16bit) 800x480
#define TFT01_50		13	// SSD1963	(16bit) 800x480
#define CTE50			13	// SSD1963	(16bit) 800x480
#define ITDB24E_8		14	// S6D1121	(8bit)
#define ITDB24E_16		15	// S6D1121	(16bit)
#define INFINIT32		16	// SSD1289	(Latched 16bit) -- Legacy, will be removed later
#define ELEE32_REVA		16	// SSD1289	(Latched 16bit)
#define GEEE24			17	// ILI9320	(8bit)
#define GEEE28			18	// ILI9320	(16bit)
#define ELEE32_REVB		19	// SSD1289	(8bit)
#define TFT01_70		20	// SSD1963	(16bit) 800x480 Alternative Init
#define CTE70			20	// SSD1963	(16bit) 800x480 Alternative Init
#define CTE32HR			21	// ILI9481	(16bit)

#define SERIAL_4PIN		4
#define SERIAL_5PIN		5
#define LATCHED_16		17

//*********************************
// COLORS
//*********************************
// VGA color palette
#define VGA_BLACK		0x0000
#define VGA_WHITE		0xFFFF
#define VGA_RED			0xF800
#define VGA_GREEN		0x07E0
#define VGA_BLUE		0x001F
#define VGA_SILVER		0xC618
#define VGA_GRAY		0x8410
#define VGA_MAROON		0x8000
#define VGA_YELLOW		0xFFE0
#define VGA_OLIVE		0x8400
#define VGA_LIME		0x07E0
#define VGA_AQUA		0x07FF
#define VGA_TEAL		0x0410
#define VGA_NAVY		0x0010
#define VGA_FUCHSIA		0xF81F
#define VGA_PURPLE		0x8010
#define VGA_TRANSPARENT	0xFFFFFFFF



		void ILI9327_Init();
		void ILI9327_Clear();
		void ILI9327_Line(int x1, int y1, int x2, int y2);
		void ILI9327_FillRGB(uint8_t r, uint8_t g, uint8_t b);
		void ILI9327_Fill(uint16_t color);
		void ILI9327_Rect(int x1, int y1, int x2, int y2);
		void drawRoundRect(int x1, int y1, int x2, int y2);
		void ILI9327_RectFill(int x1, int y1, int x2, int y2);
		void fillRoundRect(int x1, int y1, int x2, int y2);
		void ILI9327_Circle(int x, int y, int radius);
		void ILI9327_CircleFill(int x, int y, int radius);
		void ILI9327_setColorRGB(uint8_t r, uint8_t g, uint8_t b);
		void ILI9327_setColor(uint16_t color);
		uint16_t getColor();
		void ILI9327_setBackColorRGB(uint8_t r, uint8_t g, uint8_t b);
		void ILI9327_setBackColor(uint32_t color);
		uint16_t getBackColor();
		/*void print(char *st, int x, int y, int deg=0);
		void print(String st, int x, int y, int deg=0);
		void printNumI(long num, int x, int y, int length=0, char filler=' ');
		void printNumF(double num, uint8_t dec, int x, int y, char divider='.', int length=0, char filler=' ');*/
		void ILI9327_setFont(uint8_t* font);
		uint8_t* getFont();
		uint8_t getFontXsize();
		uint8_t getFontYsize();
		void ILI9327_Bitmap(int x, int y, int sx, int sy, unsigned int* data, int scale);
		//void drawBitmap2(int x, int y, int sx, int sy, unsigned int* data, int deg, int rox, int roy);
		void lcdOff();
		void lcdOn();
		void setContrast(char c);
		int  getDisplayXSize();
		int	 getDisplayYSize();

/*
	The functions and variables below should not normally be used.
	They have been left publicly available for use in add-on libraries
	that might need access to the lower level functions of UTFT.

	Please note that these functions and variables are not documented
	and I do not provide support on how to use them.
*/


		void ILI9327_Writ_Bus(char VH,char VL, uint8_t mode);
		void ILI9327_WriteCom(char VL);
		void ILI9327_WriteData(char VH,char VL);
		void ILI9327_WriteParam(char VL);
		void ILI9327_Pixel(uint16_t color);
		void ILI9327_HLine(int x, int y, int l);
		void ILI9327_VLine(int x, int y, int l);
		void ILI9327_Char(char c, int x, int y);
		void setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		void clrXY();
		void rotateChar(uint8_t c, int x, int y, int pos, int deg);
		//void _set_direction_registers(uint8_t mode);
		void _fast_fill_16(int ch, int cl, long pix);
		void _fast_fill_8(int ch, long pix);
		//void _convert_float(char *buf, double num, int width, uint8_t prec);

#endif
