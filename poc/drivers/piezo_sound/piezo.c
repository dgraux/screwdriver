/**
 * @brief This file gives functions to manage sound with piezoelectrical speaker
 * @file piezo.c
 * @author Damien GRAUX
 * @date 16 december 2012
 * @note Read in an 8 bit port value (DIP switch ) and output
 * @note a musical note for each corresponding bit ie B0 -> middle C,
 * @note B1 -> C#, B2 -> D, etc...If more than one bit is set,
 * @note chose the lowest note frequency for output and keep
 * @note the synthesizer monophonic.
 */


// Useful librairies
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



/**
 * @brief Ports declarations
 */
#define BUZZER_PORT PORTA
/**
 * @brief Ports declarations
 */
#define BUZZER_DDR  DDRA
/**
 * @brief Ports declarations
 */
#define BUZZER_PIN  0


/**
 * @brief global variable used to play music
 */
int switch_input;

 
/**
 * @brief buzz plays a note while the switch_input is unchanged
 * @param[in] float period of the note with is bounded to be payed
 * @note Uses the @swith_input to know the moment to stop playing
 */
void buzz(float period)
{
    int new_input;
    float half_period;         
     
    new_input = PIND;                           //variable to test old switch_input variable
    half_period = period/2;                     // Compute a half cycle period
     
    BUZZER_DDR = (1 << BUZZER_PIN) | BUZZER_DDR;      // Set the port for the buzzer output
 
    while(new_input == switch_input)
    {
        _delay_ms(half_period);             // Wait a half cycle to toggle port pin
        BUZZER_PORT = (1 << BUZZER_PIN) | BUZZER_PORT;    // Set the port pin as a 1
        _delay_ms(half_period);             // Wait a half cycle to clear the port pin
        BUZZER_PORT = ~(1 << BUZZER_PIN) & BUZZER_PORT;   // Clear the port pin to 0
     
        new_input = PIND;
    }
 
    return;
}
 

/**
 * @brief  Provide the duration and period of buzzer signal in ms
 * @param[in] float duration of the note with is bounded to be payed
 * @param[in] float period of the note with is bounded to be payed
 */
void buzz_2(float duration, float period)
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


/**
 * @brief Implementation of a 'bip'
 */
void bip(void)
{
    int i;
 
    for (i=0; i<4; i=i+1)
    {
      // TODO -> remove this line if not really useful      
      PORTA = 0b00000010 ^ PORTA;     // Toggle the LED pin by XOR (whenever a bit is XORed with 1, it is toggled)
      
      buzz_2(75,0.5);                   // Output a waveform of 75ms at 2KHz (period of 2KHz is 1/2000=0.5 ms)
      _delay_ms(75);                  // Wait for 75 ms
    }
    return;
}


/**
 * @brief Play the sounds via thanks to the switch input
 * @note Uses the @swith_input to know the moment to stop playing
 */
void sound_player(void)
{  
    DDRD = 0x00;                //PORTD is all input for DIP switch
    DDRA = 0x01;                //PORTA pin 0 is an output for buzzer  
    PORTA = 0x00;               //Set the buzzer pin initially to 0
 
    int i,count,bit_check;
    float C,C2,D,D2,E,F,F2,G;  
 
    C = 261.63;         //initialize vairables to specifc frequency (in HZ)
    C2 = 277.18;
    D = 293.66;
    D2 = 311.13;
    E = 329.63;
    F = 349.23;
    F2 = 369.99;
    G = 392.00;
 
 
    while(1)
    {
        switch_input = PIND;        //Reads in the DIP switch values and stores in switch_input
 
LOOP:   switch(switch_input)        //determines which DIP switch is turned ON to play a
        {                           //specific note
            case 0x0:
                buzz((1/C)*100);    //Branches to function buzz with period of C ((1/261.63)*100 = 3.822 ms)
                break;
 
            case 0x1:
                buzz((1/C2)*100);
                break;
 
            case 0x2:
                buzz((1/D)*100);
                break;
 
            case 0x3:
                buzz((1/D2)*100);
                break;
 
            case 0x4:
                buzz((1/E)*100);
                break;
 
            case 0x5:
                buzz((1/F)*100);
                break;
 
            case 0x6:
                buzz((1/F2)*100);
                break;
 
            case 0x7:
                buzz((1/G)*100);
                break;
     
            default:                        //Default case is for if more than one switch is on
                                        //Check each bit, starting from the lowest bit, to determine
                count = 0;              //which is the lowest switch on.
                for(i=0x01;i<=0x80;i<<1)       //starts at bit 0 stop after bit 7, i shifts left 1 bit
                {
                    bit_check = (switch_input & i);     //Masks all bits except for bit i in PIND  
 
                    if(bit_check == i)          //checks if the bit is a 1 or a 0
                        break; 
                    else
                        count = count + 1;      //count used to determine what bit is lowest
                }
                 
                switch_input = pow(2,count);            //2^count gives the lowest switch
 
                goto LOOP;
        }
    }  
}


/**
 * @brief The main just launches the sound_player
 */
int main(){
  sound_player();
  return 1;
}
