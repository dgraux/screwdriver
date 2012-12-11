/*
	Simple VGA adapter demo2
This example demonstrate how to make advanced users handler
Used task switcher and here work 3 tasks:
1. Primitive logic analizer. Checked PortC.1 and show this state onto display
2. Primitive stop watch. Timer running on logical "0" at pin PortC.1.
   Count time value, encode it at decimal sec and show this value onto display
3. Primitive DC Voltmeter. Get the ADC value from ADC0, convert it to decimal Volts
   and show this value onto display
*/
#include <avr/io.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "symbol_tbl.h"

#define true 1
#define false 0
#define NOP asm("nop");

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
const char str1[] PROGMEM =   "  Simple VGA demo2";
const char str2[] PROGMEM =   "User hanlder example";
const char str3[] PROGMEM =   "PinC.1= ";
const char str4[] PROGMEM =   "Time is:     0sec";
const char str5[] PROGMEM =   "ADC0 channel: 0.00V";

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

//Show time word value onto display(DEC)
void _display_time (unsigned int w) {
    unsigned char tmp,j;
	unsigned int divider = 10000;
    signed char i;
	j = vga_symbols_per_row*5+9;
	
    for (i = 5; i > 0; i--)
    {

	  tmp = (w / divider) % 10;
	  divider /= 10;
     if (tmp > 0x09)
        tmp += 0x37;
      else
        tmp += 0x30;
	str_array[j++] = tmp;
    }
}

//Show ADC0 conversion value onto display(DEC)
void _display_adc (unsigned int w) {
    unsigned char tmp,j;
	unsigned int divider = 10000;
    signed char i;
	j = vga_symbols_per_row*10+14;
	
    for (i = 5; i > 2; i--)
    {

	  tmp = (w / divider) % 10;
	  divider /= 10;
     if (tmp > 0x09)
        tmp += 0x37;
      else
        tmp += 0x30;
   
		str_array[j++] = tmp;
		if (i==5) str_array[j++] = '.'; 
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

void adc_init(void)
{
	// Enable ADC
	ADCSRA = 6|1<<ADEN;  //ADC enable, start the single conversion ADCSRA |= 1<< ADSC;
						//ADC prescaler speed /64 FOSC,
						//wait the conversion something like: while(bit_is_set(ADCSRA, ADSC));
						
	ADMUX = 1<<ADLAR;//ADMUX = Voltage reference AREF,ADC left adjust result, channel ADC0;
	SFIOR = 1<<ADHSM;// High speed conversion mode
	/*
	Normal conversion time =< 25 ADC cycles => Speed conversion ~ 16MHz/(64*25)=10 kHz;
	*/
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
	strcpy_P(&str_array[vga_symbols_per_row*10],&str5[0]); 	
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
					static unsigned int framecount, time;
					//Simplest task switcher (linecount<45 not visible)
					switch ( linecount )
					{
					  //Task for make ADC0 convertion
					  case  0:  
						//Enable conversion only at the begin of video line count
						//to avoid flickering at the top display
						ADCSRA |= 1<<ADSC;//start the single conversion 
						while(bit_is_set(ADCSRA, ADSC));
					  break;
					  
					  //Task for refresh ADC0 convertion result
					  case  3:
						_display_adc(196*ADCH);
					  break;

					  //Task for show and count time value
					  case  9: 
						//Count frame for time count
						framecount++;
						//Time count
						if (framecount==60)
						{
							framecount = 0;
							//Running timer only when PinC.1 = "0";
							if (bit_is_clear(PINC,1))
							{
							//Increment every second
							_display_time(++time);
							}
							else
							{
								time = 0;
							}
						}
					  break;

					  //Default task Check and show state of PINC.1
					  default: 
						if (bit_is_set(PINC,1))
							str_array[7] = '1';
						else
							str_array[7] = '0';
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

  //ADC init
  adc_init();

  //Set power mode
  set_sleep_mode(SLEEP_MODE_IDLE); 	

  //Enable global interrupt
  asm ("sei");

  return;
}
    
	
    