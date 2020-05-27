#include "UTFT.h"

#include "STM32F4xx_gpio.h"
#include "STM32F4xx_rcc.h"



extern void __delay_ms( volatile uint32_t nTime );


//#define MODE_8_BIT	//else - MODE_16_BIT
//430kHz !!!!!!!!!!!!!1

struct _current_font
{
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
}cfont;


uint8_t fillColorH, fillColorL, backColorH, backColorL;
uint8_t orient;
long disp_x_size, disp_y_size;
uint8_t display_model, display_transfer_mode, display_serial_mode;


uint8_t _transparent;


#define RESET_H	BitSet(GPIOC->ODR, 0)
#define RESET_L BitReset(GPIOC->ODR, 0)

#define CS_H	BitSet(GPIOC->ODR, 3)
#define CS_L	BitReset(GPIOC->ODR, 3)

#define DC_H	BitSet(GPIOC->ODR, 2)
#define DC_L	BitReset(GPIOC->ODR, 2)

#define WR_H	BitSet(GPIOC->ODR, 1)
#define WR_L	BitReset(GPIOC->ODR, 1)

#define RD_H	BitSet(GPIOC->ODR, 4)
#define RD_L	BitReset(GPIOC->ODR, 4)

#define WR_Pulse_H 	{WR_L;WR_H;}


void ILI9327_Init()
{ 
	//RD_H;// Read disabled

	disp_x_size=239; // ILI9327
	disp_y_size=399;
	display_transfer_mode=8;//8

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	CS_H;

	orient=PORTRAIT;
	//orient=LANDSCAPE;//

	RESET_H;
	__delay_ms(5);
	RESET_L;
	__delay_ms(10);
	RESET_H;
	__delay_ms(10);

	CS_L;

	/* Initialize LCD */
	//ILI9327_WriteCom(0xE9);		//???
	//ILI9327_WriteParam(0x20);

	ILI9327_WriteCom(0x11); 		//Exit Sleep
	__delay_ms(100);

	ILI9327_WriteCom(0xD1);		//VCOM Control
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x71);
	ILI9327_WriteParam(0x19);

	ILI9327_WriteCom(0xD0);		//Power_Setting
	ILI9327_WriteParam(0x07);
	ILI9327_WriteParam(0x01);
	ILI9327_WriteParam(0x08);

	ILI9327_WriteCom(0x36);		//set_address_mode
	ILI9327_WriteParam(0x48);

	ILI9327_WriteCom(0x3A);		//set_pixel_format
	ILI9327_WriteParam(0x05);

	ILI9327_WriteCom(0xC1);		//Display_Timing_Setting for Normal/Partial Mode
	ILI9327_WriteParam(0x10);
	ILI9327_WriteParam(0x10);
	ILI9327_WriteParam(0x02);
	ILI9327_WriteParam(0x02);

	ILI9327_WriteCom(0xC5); 		//Set frame rate
	ILI9327_WriteParam(0x04);
	ILI9327_WriteCom(0xD2); 		//Power_Setting for Normal Mode
	ILI9327_WriteParam(0x01);
	ILI9327_WriteParam(0x44);

	ILI9327_WriteCom(0xC0); 		//Set Default Gamma
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x35);
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x01);
	ILI9327_WriteParam(0x02);

	ILI9327_WriteCom(0xC8); 		//Set Gamma
	ILI9327_WriteParam(0x04);
	ILI9327_WriteParam(0x67);
	ILI9327_WriteParam(0x35);
	ILI9327_WriteParam(0x04);
	ILI9327_WriteParam(0x08);
	ILI9327_WriteParam(0x06);
	ILI9327_WriteParam(0x24);
	ILI9327_WriteParam(0x01);
	ILI9327_WriteParam(0x37);
	ILI9327_WriteParam(0x40);
	ILI9327_WriteParam(0x03);
	ILI9327_WriteParam(0x10);
	ILI9327_WriteParam(0x08);
	ILI9327_WriteParam(0x80);
	ILI9327_WriteParam(0x00);

	ILI9327_WriteCom(0x29); 		//display on

	/*ILI9327_WriteCom(0x2A);		//set_column_address - 4
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0xeF);
	ILI9327_WriteCom(0x2B);		//set_page_address - 4
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x00);
	ILI9327_WriteParam(0x01);
	ILI9327_WriteParam(0x8F);*/

//	ILI9327_WriteCom(0x2C); 		//write_memory_start

	CS_H;

	ILI9327_setColorRGB(255, 255, 255);
	ILI9327_setBackColorRGB(0, 0, 0);
	cfont.font=0;

	_transparent = DISABLE;
}


#ifndef MODE_8_BIT
void _fast_fill_16(int colorH, int colorL, long pix)
{
	GPIOA->ODR = (uint16_t)((colorH<<8)|colorL);

	for (int i=0; i<16*(pix/16); i++)
		WR_Pulse_H;

	if ((pix % 16) != 0)
	{
		for (int i=0; i<(pix % 16); i++)
			WR_Pulse_H;
	}

}
#else
void _fast_fill_8(int color, long pix)
{
	GPIOA->ODR = color;

	for(uint32_t i=0; i<32*(pix/16); i++)
		WR_Pulse_H;

	if((pix % 16) != 0)
	{
		for (int i=0; i<2*(pix % 16); i++)
			WR_Pulse_H;
	}
}
#endif

void ILI9327_WriteCom(char VL)
{   
	DC_L;
	GPIOA->ODR = VL;
	WR_Pulse_H;
}

void ILI9327_WriteData(char VH, char VL)
{
	DC_H;

#ifdef MODE_8_BIT
	GPIOA->ODR = VH;
	WR_Pulse_H;
	GPIOA->ODR = VL;
	WR_Pulse_H;
#else
	GPIOA->ODR = (uint16_t)((VH<<8)|VL);
	WR_Pulse_H;
#endif
}

void ILI9327_WriteParam(char VL)
{
	DC_H;
	GPIOA->ODR = VL;
	WR_Pulse_H;
}


void setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (orient==LANDSCAPE)
	{
		swap(uint16_t, x1, y1);
		swap(uint16_t, x2, y2);
		y1=disp_y_size-y1;
		y2=disp_y_size-y2;
		swap(uint16_t, y1, y2);
	}

	ILI9327_WriteCom(0x2a);		//set_column_address
	ILI9327_WriteParam(x1>>8);
	ILI9327_WriteParam(x1);
	ILI9327_WriteParam(x2>>8);
	ILI9327_WriteParam(x2);

	ILI9327_WriteCom(0x2b);		//set_page_address
	ILI9327_WriteParam(y1>>8);
	ILI9327_WriteParam(y1);
	ILI9327_WriteParam(y2>>8);
	ILI9327_WriteParam(y2);

	ILI9327_WriteCom(0x2c);		//write_memory_start
}

void clrXY()
{
	if(orient==PORTRAIT)
		setXY(0, 0, disp_x_size, disp_y_size);
	else
		setXY(0, 0, disp_y_size, disp_x_size);
}

void ILI9327_Rect(int x1, int y1, int x2, int y2)
{
	if(x1>x2)
		swap(int, x1, x2);
	if(y1>y2)
		swap(int, y1, y2);

	ILI9327_HLine(x1, y1, x2-x1);
	ILI9327_HLine(x1, y2, x2-x1);
	ILI9327_VLine(x1, y1, y2-y1);
	ILI9327_VLine(x2, y1, y2-y1);
}

void drawRoundRect(int x1, int y1, int x2, int y2)
{
	if(x1>x2)
		swap(int, x1, x2);
	if(y1>y2)
		swap(int, y1, y2);

	if((x2-x1)>4 && (y2-y1)>4)
	{
		CS_L;
			setXY(x1+1, y1+1, x1+1, y1+1);
			ILI9327_Pixel((fillColorH<<8)|fillColorL);
		//CS_H;
		//clrXY();
		CS_L;
			setXY(x2-1, y1+1, x2-1, y1+1);
			ILI9327_Pixel((fillColorH<<8)|fillColorL);
		//CS_H;
		//clrXY();
		CS_L;
			setXY(x1+1, y2-1, x1+1, y2-1);
			ILI9327_Pixel((fillColorH<<8)|fillColorL);
		//CS_H;
		//clrXY();
		CS_L;
			setXY(x2-1, y2-1, x2-1, y2-1);
			ILI9327_Pixel((fillColorH<<8)|fillColorL);
		CS_H;
		clrXY();

		ILI9327_HLine(x1+2, y1, x2-x1-4);
		ILI9327_HLine(x1+2, y2, x2-x1-4);
		ILI9327_VLine(x1, y1+2, y2-y1-4);
		ILI9327_VLine(x2, y1+2, y2-y1-4);
	}
}

void ILI9327_RectFill(int x1, int y1, int x2, int y2)
{

	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

#ifndef MODE_8_BIT
		CS_L;
		setXY(x1, y1, x2, y2);
		DC_H;
		_fast_fill_16(fillColorH, fillColorL, ((x2-x1)+1)*((y2-y1)+1));
		CS_H;
#else
	if(fillColorH == fillColorL)
	{
		CS_L;
		setXY(x1, y1, x2, y2);
		DC_H;
		_fast_fill_8(fillColorH,((x2-x1)+1)*((y2-y1)+1));
		CS_H;
	}
	else
	{
		if(orient==PORTRAIT)
		{
			for (int i=0; i<((y2-y1)/2)+1; i++)
			{
				ILI9327_HLine(x1, y1+i, x2-x1);
				ILI9327_HLine(x1, y2-i, x2-x1);
			}
		}
		else
		{
			for (int i=0; i<((x2-x1)/2)+1; i++)
			{
				ILI9327_VLine(x1+i, y1, y2-y1);
				ILI9327_VLine(x2-i, y1, y2-y1);
			}
		}
	}
#endif
}

void fillRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
		swap(int, x1, x2);

	if (y1>y2)
		swap(int, y1, y2);

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				ILI9327_HLine(x1+2, y1+i, x2-x1-4);
				ILI9327_HLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				ILI9327_HLine(x1+1, y1+i, x2-x1-2);
				ILI9327_HLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				ILI9327_HLine(x1, y1+i, x2-x1);
				ILI9327_HLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void ILI9327_Circle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
 
	CS_L;

	setXY(x, y + radius, x, y + radius);
	ILI9327_WriteData(fillColorH,fillColorL);
	setXY(x, y - radius, x, y - radius);
	ILI9327_WriteData(fillColorH,fillColorL);
	setXY(x + radius, y, x + radius, y);
	ILI9327_WriteData(fillColorH,fillColorL);
	setXY(x - radius, y, x - radius, y);
	ILI9327_WriteData(fillColorH,fillColorL);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x + x1, y + y1, x + x1, y + y1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x - x1, y + y1, x - x1, y + y1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x + x1, y - y1, x + x1, y - y1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x - x1, y - y1, x - x1, y - y1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x + y1, y + x1, x + y1, y + x1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x - y1, y + x1, x - y1, y + x1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x + y1, y - x1, x + y1, y - x1);
		ILI9327_WriteData(fillColorH,fillColorL);
		setXY(x - y1, y - x1, x - y1, y - x1);
		ILI9327_WriteData(fillColorH,fillColorL);
	}
	CS_H;
	clrXY();
}

void ILI9327_CircleFill(int x, int y, int radius)
{
	for(int y1=-radius; y1<=0; y1++) 
		for(int x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius) 
			{
				ILI9327_HLine(x+x1, y+y1, 2*(-x1));
				ILI9327_HLine(x+x1, y-y1, 2*(-x1));
				break;
			}
}

void ILI9327_Clear()
{
	CS_L;

	clrXY();
	DC_H;

#ifndef MODE_8_BIT
		_fast_fill_16(0, 0, ((disp_x_size+1)*(disp_y_size+1)));
#else
		_fast_fill_8(0, ((disp_x_size+1)*(disp_y_size+1)));
#endif

	CS_H;
}

void ILI9327_FillRGB(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
	ILI9327_Fill(color);
}

void ILI9327_Fill(uint16_t color)
{
	uint8_t colorH = (uint8_t)(color>>8);
	uint8_t colorL = (uint8_t)color;

	CS_L;

	clrXY();

	DC_H;

#ifndef MODE_8_BIT
		_fast_fill_16(colorH, colorL, ((disp_x_size+1)*(disp_y_size+1)));
#else
	if(colorH == colorL)
		_fast_fill_8(colorH,((disp_x_size+1)*(disp_y_size+1)));
	else
	{
		for(long i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
		{
			GPIOA->ODR = colorH;
			WR_Pulse_H;
			GPIOA->ODR = colorL;
			WR_Pulse_H;
		}
	}
#endif

	CS_H;
}

void ILI9327_setColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	fillColorH=((r&248)|g>>5);
	fillColorL=((g&28)<<3|b>>3);
}

void ILI9327_setColor(uint16_t color)
{
	fillColorH=(uint8_t)(color>>8);
	fillColorL=(uint8_t)(color & 0xFF);
}

uint16_t getColor(){
	return (fillColorH<<8) | fillColorL;
}

void ILI9327_setBackColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	backColorH=((r&248)|g>>5);
	backColorL=((g&28)<<3|b>>3);
	_transparent=DISABLE;
}

void ILI9327_setBackColor(uint32_t color)
{
	if (color==VGA_TRANSPARENT)
		_transparent=ENABLE;
	else
	{
		backColorH=(uint8_t)(color>>8);
		backColorL=(uint8_t)(color & 0xFF);
		_transparent=DISABLE;
	}
}

uint16_t getBackColor(){
	return (backColorH<<8) | backColorL;
}

void ILI9327_Pixel(uint16_t color)
{
	ILI9327_WriteData((color>>8),(color&0xFF));	// rrrrrggggggbbbbb
}


void ILI9327_Line(int x1, int y1, int x2, int y2)
{
	if (y1==y2)
		ILI9327_HLine(x1, y1, x2-x1);
	else if (x1==x2)
		ILI9327_VLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				color = x1, row = y1;

		CS_L;
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (ENABLE)
			{
				setXY (color, row, color, row);
				ILI9327_WriteData (fillColorH, fillColorL);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					color += xstep;
					t   -= dy;
				}
			} 
		}
		else
		{
			int t = - (dx >> 1);
			while (ENABLE)
			{
				setXY (color, row, color, row);
				ILI9327_WriteData (fillColorH, fillColorL);
				if (color == x2)
					return;
				color += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			} 
		}
		CS_H;
	}
	clrXY();
}

void ILI9327_HLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		x -= l;
	}
	CS_L;
	setXY(x, y, x+l, y);

#ifndef MODE_8_BIT
		DC_H;
		_fast_fill_16(fillColorH, fillColorL, l);
#else
	if(fillColorH == fillColorL)
	{
		DC_H;
		_fast_fill_8(fillColorH,l);
	}
	else
	{
		for(int i=0; i<l+1; i++)
			ILI9327_WriteData(fillColorH, fillColorL);
	}
#endif
	CS_H;
	clrXY();
}

void ILI9327_VLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		y -= l;
	}
	CS_L;
	setXY(x, y, x, y+l);
#ifndef MODE_8_BIT
		DC_H;
		_fast_fill_16(fillColorH,fillColorL,l);
#else
	if(fillColorH == fillColorL)
	{
		DC_H;
		_fast_fill_8(fillColorH,l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
			ILI9327_WriteData(fillColorH, fillColorL);
	}
#endif
	CS_H;
	clrXY();
}

void ILI9327_Char(char c, int x, int y)
{
	char colorH;
	uint16_t temp;

	CS_L;
  
	if(!_transparent)
	{
		if(orient==PORTRAIT)
		{
			setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);
	  
			temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(uint16_t j=0; j<((cfont.x_size/8)*cfont.y_size); j++)
			{
				colorH = cfont.font[temp];

				for(uint8_t i=0; i<8; i++)
				{   
					if((colorH & (1<<(7-i))) != 0)
					{
						ILI9327_Pixel((fillColorH<<8)|fillColorL);
					} 
					else
					{
						ILI9327_Pixel((backColorH<<8)|backColorL);
					}   
				}
				temp++;
			}
		}
		else
		{
			temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(uint16_t j=0; j<((cfont.x_size/8)*cfont.y_size); j+=(cfont.x_size/8))
			{
				setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));

				for (int zz=(cfont.x_size/8)-1; zz>=0; zz--)
				{
					colorH = cfont.font[temp+zz];

					for(uint8_t i=0; i<8; i++)
					{   
						if((colorH & (1<<i)) != 0)
							ILI9327_Pixel((fillColorH<<8)|fillColorL);
						else
							ILI9327_Pixel((backColorH<<8)|backColorL);
					}
				}
				temp+=(cfont.x_size/8);
			}
		}
	}
	else
	{
		temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for(uint16_t j=0; j<cfont.y_size; j++)
		{
			for(int zz=0; zz<(cfont.x_size/8); zz++)
			{
				colorH = cfont.font[temp+zz];
				for(uint8_t i=0; i<8; i++)
				{   
					setXY(x+i+(zz*8), y+j, x+i+(zz*8)+1, y+j+1);
				
					if((colorH & (1<<(7-i))) != 0)
						ILI9327_Pixel((fillColorH<<8)|fillColorL);
				}
			}
			temp += (cfont.x_size/8);
		}
	}

	CS_H;
	clrXY();
}
/*
void rotateChar(uint8_t c, int x, int y, int pos, int deg)
{
	uint8_t i,j,colorH;
	uint16_t temp;
	int newx,newy;
	double radian;
	radian=deg*0.0175;  

	CS_L;

	temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0;j<cfont.y_size;j++) 
	{
		for (int zz=0; zz<(cfont.x_size/8); zz++)
		{
			colorH=(&cfont.font[temp+zz]);
			for(i=0;i<8;i++)
			{   
				newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

				setXY(newx,newy,newx+1,newy+1);
				
				if((colorH&(1<<(7-i)))!=0)
				{
					ILI9327_Pixel((fillColorH<<8)|fillColorL);
				} 
				else  
				{
					if (!_transparent)
						ILI9327_Pixel((backColorH<<8)|backColorL);
				}   
			}
		}
		temp+=(cfont.x_size/8);
	}
	CS_H;
	clrXY();
}

void print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (orient==PORTRAIT)
	{
	if (x==RIGHT)
		x=(disp_x_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_x_size+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x==RIGHT)
		x=(disp_y_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_y_size+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg==0)
			ILI9327_Char(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void print(String st, int x, int y, int deg)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	print(buf, x, y, deg);
}

void printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	uint8_t neg=DISABLE;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=ENABLE;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	print(st,x,y);
}

void printNumF(double num, uint8_t dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	uint8_t neg=DISABLE;

	if (dec<1)
		dec=1;
	else if (dec>5)
		dec=5;

	if (num<0)
		neg = ENABLE;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (int i=0; i<sizeof(st); i++)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (int i=1; i<sizeof(st); i++)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (int i=0; i<sizeof(st); i++)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	print(st,x,y);
}
*/
void ILI9327_setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=cfont.font[0];
	cfont.y_size=cfont.font[1];
	cfont.offset=cfont.font[2];
	cfont.numchars=cfont.font[3];
}

uint8_t* getFont(){
	return cfont.font;
}

uint8_t getFontXsize(){
	return cfont.x_size;
}

uint8_t getFontYsize(){
	return cfont.y_size;
}

void ILI9327_Bitmap(int x, int y, int sx, int sy, unsigned int* data, int scale)
{
	uint16_t color;

	if(scale==1) //default
	{
		if (orient==PORTRAIT)
		{
			CS_L;
			setXY(x, y, x+sx-1, y+sy-1);
			for(int tc=0; tc<(sx*sy); tc++)
			{
				color = data[tc];
				ILI9327_WriteData(color>>8,color & 0xff);
			}
			CS_H;
		}
		else
		{
			CS_L;
			for(int ty=0; ty<sy; ty++)
			{
				setXY(x, y+ty, x+sx-1, y+ty);
				for(int tx=sx; tx>=0; tx--)
				{
					color = data[(ty*sx)+tx];
					ILI9327_WriteData(color>>8,color & 0xff);
				}
			}
			CS_H;
		}
	}
	else
	{
		if(orient==PORTRAIT)
		{
			CS_L;
			for(int ty=0; ty<sy; ty++)
			{
				setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				for(int tsy=0; tsy<scale; tsy++)
					for(int tx=0; tx<sx; tx++)
					{
						color = data[(ty*sx)+tx];

						for(int tsx=0; tsx<scale; tsx++)
							ILI9327_WriteData(color>>8,color & 0xff);
					}
			}
			CS_H;
		}
		else
		{
			CS_L;
			for(int ty=0; ty<sy; ty++)
			{
				for(int tsy=0; tsy<scale; tsy++)
				{
					setXY(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					for(int tx=sx; tx>=0; tx--)
					{
						color = data[(ty*sx)+tx];
						for(int tsx=0; tsx<scale; tsx++)
							ILI9327_WriteData(color>>8,color & 0xff);
					}
				}
			}
			CS_H;
		}
	}
	clrXY();
}
/*
void drawBitmap2(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int color;
	int tx, ty, newx, newy;
	uint8_t r, g, b;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		ILI9327_Bitmap(x, y, sx, sy, data);
	else
	{
		CS_L;
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				color=data[(ty*sx)+tx];

				newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				setXY(newx, newy, newx, newy);
				ILI9327_WriteData(color>>8,color & 0xff);
			}
		CS_H;
	}
	clrXY();
}
*/
/*
void lcdOff()
{
	CS_L;
	switch (display_model)
	{
	case PCF8833:
		ILI9327_WriteCom(0x28);
		break;
	}
	CS_H;
}

void lcdOn()
{
	CS_L;
	switch (display_model)
	{
	case PCF8833:
		ILI9327_WriteCom(0x29);
		break;
	}
	CS_H;
}

void setContrast(char c)
{
	CS_L;
	switch (display_model)
	{
	case PCF8833:
		if (c>64) c=64;
		ILI9327_WriteCom(0x25);
		ILI9327_WriteData(0, c);
		break;
	}
	CS_H;
}
*/
int getDisplayXSize()
{
	if (orient==PORTRAIT)
		return disp_x_size+1;
	else
		return disp_y_size+1;
}

int getDisplayYSize()
{
	if (orient==PORTRAIT)
		return disp_y_size+1;
	else
		return disp_x_size+1;
}
