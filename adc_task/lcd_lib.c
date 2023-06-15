//*****************************************************************************
#include "lcd_lib.h"
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

const uint8_t lcd_customchar[] PROGMEM=//define 6 custom LCD chars
{
0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b0, // 0. 0/5 full progress block
0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b0, // 1. 1/5 full progress block
0b11000,0b11000,0b11000,0b11000,0b11000,0b11000,0b11000,0b0, // 2. 2/5 full progress block	
0b11100,0b11100,0b11100,0b11100,0b11100,0b11100,0b11100,0b0, // 3. 3/5 full progress block
0b11110,0b11110,0b11110,0b11110,0b11110,0b11110,0b11110,0b0, // 4. 4/5 full progress block
0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b0, // 5. 5/5 full progress block
};

//Sends Char to LCD
void lcd_dat(uint8_t ch)		
{
	LDP=(ch&0b11110000);
	LCP|=1<<LCD_RS;
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);	
	LCP&=~(1<<LCD_RS);
	_delay_ms(1);
	LDP=((ch&0b00001111)<<4);
	LCP|=1<<LCD_RS;
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);	
	LCP&=~(1<<LCD_RS);
	_delay_ms(1);
}
//Sends Command to LCD
void lcd_com(uint8_t cmd)	
{
	LDP=(cmd&0b11110000);
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);
	LDP=((cmd&0b00001111)<<4);	
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);

}
// LCD initialization
void lcd_init(void)
{
	_delay_ms(15);
	LDP=0x00;
	LCP=0x00;
	LDDR|=1<<LCD_D7|1<<LCD_D6|1<<LCD_D5|1<<LCD_D4;
	LCDR|=1<<LCD_E|1<<LCD_RW|1<<LCD_RS;
        //---------one------
	LDP=0<<LCD_D7|0<<LCD_D6|1<<LCD_D5|1<<LCD_D4; //4 bit mode
	LCP|=1<<LCD_E|0<<LCD_RW|0<<LCD_RS;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);
	//-----------two-----------
	LDP=0<<LCD_D7|0<<LCD_D6|1<<LCD_D5|1<<LCD_D4; //4 bit mode
	LCP|=1<<LCD_E|0<<LCD_RW|0<<LCD_RS;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);
	//-------three-------------
	LDP=0<<LCD_D7|0<<LCD_D6|1<<LCD_D5|0<<LCD_D4; //4 bit mode
	LCP|=1<<LCD_E|0<<LCD_RW|0<<LCD_RS;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);
	//-------four-------------
	lcd_com(0x28);
        //-----increment address, invisible cursor shift------
	lcd_com(0x0C);
	
    //init 8 custom chars
	uint8_t ch=0, chn=0;
	while(ch<64)
	{
		lcd_definechar((lcd_customchar+ch),chn++);
		ch=ch+8;
	}
}			

//LCD clear
void lcd_clr(void)				
{
	lcd_com(1<<LCD_CLR);
}

//LCD cursor home
void lcd_home(void)			
{
	lcd_com(1<<LCD_HOME);
}

//Put string to LCD
void lcd_string(uint8_t* data, uint8_t nBytes)	
{
        register uint8_t i;
	// check to make sure we have a good pointer
	if (!data) return;
	// print data
	for(i=0; i<nBytes; i++)
	{
		lcd_dat(data[i]);
	}
}

//XY Position for LCD
void lcd_gotoxy(uint8_t x, uint8_t y)	
{
	register uint8_t DDRAMAddr;
	// remap lines into proper order
	switch(y)
	{
	case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+x; break;
	case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+x; break;
	case 2: DDRAMAddr = LCD_LINE2_DDRAMADDR+x; break;
	case 3: DDRAMAddr = LCD_LINE3_DDRAMADDR+x; break;
	default: DDRAMAddr = LCD_LINE0_DDRAMADDR+x;
	}
	// set data address
	lcd_com(1<<LCD_DDRAM | DDRAMAddr);
}

void copy_string_to_lcd(const uint8_t *FlashLoc, uint8_t x, uint8_t y)
{
	uint8_t i;
	lcd_gotoxy(x,y);
	for(i=0;(uint8_t)pgm_read_byte(&FlashLoc[i]);i++)
	{
		lcd_dat((uint8_t)pgm_read_byte(&FlashLoc[i]));
	}
}

void lcd_definechar(const uint8_t *pc,uint8_t char_code)
{
	uint8_t a, pcc;
	uint16_t i;
	a=(char_code<<3)|0x40;
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&pc[i]);
		lcd_com(a++);
		lcd_dat(pcc);
		}
}

void lcd_shift_left(uint8_t n)	//Scrol n of characters Right
{
	for (uint8_t i=0;i<n;i++)
	{
		lcd_com(0x1E);
	}
}

void lcd_shift_right(uint8_t n)	//Scrol n of characters Left
{
	for (uint8_t i=0;i<n;i++)
	{
		lcd_com(0x18);
	}
}

void lcd_cursor_on(void) //displays LCD cursor
{
	lcd_com(0x0E);
}

void lcd_cursor_blink(void)	//displays LCD blinking cursor
{
	lcd_com(0x0F);
}

void lcd_cursor_off(void)	//turns OFF cursor
{
	lcd_com(0x0C);
}

void lcd_blank(void)		//blanks LCD
{
	lcd_com(0x08);
}

void lcd_visible(void)		//Shows LCD
{
	lcd_com(0x0C);
}

//Moves cursor by n poisitions left
void lcd_cursor_left(uint8_t n)	
{
	for (uint8_t i=0;i<n;i++)
	{
		lcd_com(0x10);
	}
}

//Moves cursor by n poisitions left
void lcd_cursor_right(uint8_t n)	
{
	for (uint8_t i=0;i<n;i++)
	{
		lcd_com(0x14);
	}
}

//adapted fro mAVRLIB
void lcd_progress_bar(uint8_t progress, uint8_t maxprogress, uint8_t length)
{                       //óðîâåíü øêàëû, max óðîâåíü øêàëû,äëèíà øêàëû
	uint8_t i;
	uint16_t pixelprogress;
	uint8_t c;

	pixelprogress = ((progress*(length*PROGRESSPIXELS_PER_CHAR))/maxprogress);
	for(i=0; i<length; i++)
	{
		if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)+5) > pixelprogress )
		{
			if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)) > pixelprogress )
			{
				c = 0;
			}
			else
			{
				c = pixelprogress % PROGRESSPIXELS_PER_CHAR;// this is a partial block
			}
		}
		else
		{
			c = 5;// this is a full block
		}
		lcd_dat(c);// write character to display
	}
}

// DISPLAY n-DIGIT INTEGER NUMER
void lcd_num_to_str(uint16_t value, uint8_t nDigit)
{
 switch(nDigit)
 {
  case 4: lcd_dat((value/1000)+'0');
  case 3: lcd_dat(((value/100)%10)+'0');
  case 2: lcd_dat(((value/10)%10)+'0');
  case 1: lcd_dat((value%10)+'0');
 }
}
