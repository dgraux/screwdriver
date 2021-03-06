//This example demonstrate how to make simplest users handler
//Here work 2 tasks:
//1. Checked PortC.1 and show this state onto display
//2. Count some time value, encode it at ��� and show this value onto display

#include <avr/io.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "symbol_tbl.h"

#define true 1
#define false 0
#define NOP asm("nop")

//Global definitions for VGA render
#define vga_field_line_count 525 //standart VGA quantity lines
#define vga_symbols_per_row 20   //symbols quantity per horizontal
#define vga_row_count 20         //symbols quantity per vertical
#define symbol_height 24		 //rendered symbol height

//Makro for video handle
//Video
#define video_off  DDRB=0x90
#define video_on  DDRB=0xB0
//HSYNC
#define hsync_off  sbi(PORTD,3)
#define hsync_on  cbi(PORTD,3)
//VSYNC
#define vsync_off  PORTD=0x04
#define vsync_on   PORTD=0x00

//Global variables
volatile unsigned char str_array[vga_symbols_per_row*vga_row_count+1]; //Chars array for display buffer
volatile unsigned int current_symbol; //Current symbol number at terminal

volatile unsigned char video_enable_flg;//flag for enable render
volatile unsigned char raw_render;//Current row at display buffer for render
volatile unsigned int linecount;// current line for render
volatile unsigned char y_line_render;	//Current Y-line at symbol for render 

//Store strings data at Program Space, to avoid RAM leakage
const char str1[] PROGMEM =   "  Simple VGA demo1";
const char str2[] PROGMEM =   "User hanlder example";
const char str3[] PROGMEM =   "PinC.1= ";
const char str4[] PROGMEM =   "Time is:";

static void avr_init(void);

//All VGA sincronize made here..
SIGNAL(SIG_OVERFLOW0)
{
 TCNT0 = 0xC3; //reload counter value
		//******Syncronization Handler********

			//Count number of lines
			if (++linecount == vga_field_line_count)
			{
			linecount = 0;
			
			//clear pointers for render display buffer
			raw_render = 0; 
			y_line_render = 0;
			}

			//Make Vsync length 2 VGA lines 
			if ((linecount == 10 )||(linecount == 11 ))
			{
				//Make here vertical syncronization & HSYNC syncro level on
				vsync_on;
			}
			else
			{
				//.. & HSYNC syncro level on
				vsync_off;
			}
		
		
			video_enable_flg = true;


			if (linecount < 45)
			{
				video_enable_flg = false;
					//Add to avoid flickering at top display
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					
			}
			else
			{
				 //Forming current string for rendering
				 if (++y_line_render == symbol_height)
				 {
				  raw_render++;
				  y_line_render = 0;
				 }
				 else
				 {
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
				 }
		
			}
		
			hsync_off; //HSYNC syncro level off
		
		//******Syncronization Handler********

}

//Show word value onto display
void display_byte (unsigned int byte) {

	signed char i;
	unsigned char tmp,j;
	j = vga_symbols_per_row*5+8;
	str_array[j++] = '0';
	str_array[j++] = 'x';
    for (i = 3; i >= 0; i--) {
      tmp = (byte >> (i * 4)) & 0x0F;
   
      if (tmp > 0x09)
        tmp += 0x37;
      else
        tmp += 0x30;
	str_array[j++] = tmp;
    }
}

void spi_init (void)
{
    //Set SPI PORT DDR bits
	sbi(DDRB, 7); //SCK
	cbi(DDRB, 6); //MISO
	sbi(DDRB, 5); //MOSI
	sbi(DDRB, 4); //SS
	SPSR = 1 << SPI2X;
	SPCR = (1 << SPE) | (1 << MSTR); //SPI enable as master ,FREQ = fclk/2
	//That's a great pity, that we can't work with SPI with FREQ = fclk,
	//because may be possible build terminal up 40 symbol per row!!!
}

static void uart_init(void)
{
	 UCSRB = 0x00; //disable while setting baud rate
	 UCSRA = 0x00;
	 UCSRC = 0x86;	//8 bits data & 2 stop bits
	//UART SPEED 19200 bs
	UBRRL = 0x33; //set baud rate lo
	UBRRH = 0x00; //set baud rate hi
	//UART SPEED 38400 bs
	//UBRRL = 0x19; //set baud rate lo
	//UBRRH = 0x00; //set baud rate hi
	//UART SPEED 76800 bs
	//UBRRL = 0x0C; //set baud rate lo
	//UBRRH = 0x00; //set baud rate hi
	//UART SPEED 115200 bs
	//UBRRL = 0x08; //set baud rate lo
	//UBRRH = 0x00; //set baud rate hi
	//UART RX Enable
	UCSRB = (1 << RXEN);
}



int main(void)
{
	unsigned char i;
	char * _ptr;
	const char * _ptr1;

	//Initialize display buffer with StartUp strings
	strcpy_P(&str_array[vga_symbols_per_row*18],&str1[0]); 	
	strcpy_P(&str_array[vga_symbols_per_row*19],&str2[0]); 	
	strcpy_P(&str_array[vga_symbols_per_row*0],&str3[0]); 	
	strcpy_P(&str_array[vga_symbols_per_row*5],&str4[0]); 	
    avr_init();
	for(;;)
    {

		//Wait compare interrupt signal from Timer1
		sleep_mode();
		
		
		//Check visible field
		if(video_enable_flg)
		{
		//OK, visible
		//Main render routine
				//Set pointer for render line (display bufffer)
				_ptr = &str_array[raw_render * vga_symbols_per_row];

				//Set pointer for render line (character generator)
				_ptr1 = &symbol[0][y_line_render >> 1];
	
				//Cycle for render line 
				i = vga_symbols_per_row;
				while(i--)
				{
					SPDR = PRG_RDB(_ptr1 + (* _ptr++)*symbol_height/2);
				    video_on;
					//That's a great pity can't shift data faster (8Mhz at FOSC=16Mhz)!!
					NOP;
					NOP;
				}
				//Delay for draw last symbol
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				video_off;

		}
		else
		{
		//Not visible
		//Can do something else..	
		//*****Users handlers example
					//Count frame
					static unsigned int framecount, time;
					framecount++;
					//Check and show state of PINC.1
					if (bit_is_set(PINC,1))
						str_array[7] = '1';
					else
						str_array[7] = '0';
					//Time count
					if (framecount==0x80)
					{
						framecount = 0;
						display_byte(time++);
					}
		//*****Users handlers example
		}
		
    }//for(;;)
    return(0);
}



static void avr_init(void)
{
  // Initialize device here.
  
  //Set pin jumper VGA/PAL
  cbi(DDRC,1);
  sbi(PORTC,1);
  
  // Initialize Sync for VGA
  TCCR0 = 0x00; //stop
  TCNT0 = 0xC3; //set count, One VGA line 31.77us
  TCCR0 = 1<<CS01; //start timer with prescaler select 1/8
  TIMSK = 1<<TOV0; //enable interrupt from Timer0 overflow

  //init ports
  //HSYNC
  sbi(PORTD,3);
  sbi(DDRD,3);
  //VSYNC
  sbi(PORTD,2);
  sbi(DDRD,2);

  //Enable SPI
  spi_init();

  //init uart
  //uart_init();

  //Set power mode
  set_sleep_mode(SLEEP_MODE_IDLE); 	

  //Enable global interrupt
  asm ("sei");

  return;
}
    
	
    