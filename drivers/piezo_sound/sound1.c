#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"
 
#define BUZZER_PORT PORTA
#define BUZZER_DDR  DDRA
#define BUZZER_PIN  0
 
// Provide the duration and period of buzzer signal in ms
void BUZZ(float duration, float period)
{
    long int i,cycles;
    float half_period;  // Initialize variables
 
    cycles=duration/period; // Compute the number of cycles to loop toggling the pin
    half_period = period/2; // Compute a half cycle period

    //The Data Direction Register (DDR) tells each corresponding pin whether it's in or out.
    //That line of code is configuring a pin to be output. 
    BUZZER_DDR = (1 << BUZZER_PIN) | BUZZER_DDR;  // Set the port for the buzzer output
 
    for (i=0;i<cycles;i++)   // Toggle the speaker the appropriate number of cycles
    {
        _delay_ms(half_period);                         // Wait a half cycle to toggle port pin
        BUZZER_PORT = (1 << BUZZER_PIN) | BUZZER_PORT;    // Set the port pin
        _delay_ms(half_period);                         // Wait a half cycle to clear the port pin
        BUZZER_PORT = ~(1 << BUZZER_PIN) & BUZZER_PORT;   // Clear the port pin
    }
 
    return;     // Return to the main program
}
 
void Alert(void)
{
    int i;
 
    for (i=0; i<4; i=i+1)
    {
        BUZZ(75,0.5);                   // Output a waveform of 75ms at 2KHz (period of 2KHz is 1/2000=0.5 ms)
        PORTA = 0b00000010 ^ PORTA;     // Toggle the LED pin by XOR (whenever a bit is XORed with 1, it is toggled)
        _delay_ms(75);                  // Wait for 75 ms
    }
    return;
}
 
int main(void)
{
    LCD_INIT();
 
    DDRA = DDRA | 0b00000010;
    PORTA = PORTA & 0b00000000;
 
    while(1)
    {
        LCD_COMMAND(LCD_CLEAR_HOME);    // Clear LCD screen, send cursor to start
        _delay_ms(500);                 // Wait for 500 ms
        LCD_STRING("Junior Design");    // Write "Juior Design" to LCD
        _delay_ms(500);                 // Wait for 500 ms
 
        LCD_ADDR(0x40);                 // Send cursor to address 0x40 (second row)
        LCD_STRING("ATMEL Lecture");    // Write "ATMEL Lecture" to LCD
        _delay_ms(500);                 // Wait for 500 ms
        Alert();                        // Go to subroutine Alert()
    }  
}