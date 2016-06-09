#include "motor_control.h"
#include <xc.h>

void init_pwm()
{
    RPB15Rbits.RPB15R = 0b0101; // OC1
    RPB8Rbits.RPB8R = 0b0101; // OC2
    
    //Initialize timer to zero. 
    
    T2CONbits.TCKPS = 0b000;     //prescaler = 1
	PR2 = PERIOD; // (PR + 1) *N * 1/clock speed =  time period. 
	TMR2 = 0; // initialize count to 0
    
    OC1CONbits.OCM = 0b110; // PWM mode for OC1 with no fault
	OC1CONbits.OC32 = 0; //USe 16 bit timer
	OC1CONbits.OCTSEL = 0; // Use timer 2
	OC1RS = (PERIOD+1)/2; // set duty cycle to 50% of 4000 = 2000
	OC1R = (PERIOD+1)/2;
    
    OC2CONbits.OCM = 0b110; // PWM mode for OC2 with no fault
	OC2CONbits.OC32 = 0; //USe 16 bit timer
	OC2CONbits.OCTSEL = 0; // Use timer 2
	OC2RS = (PERIOD+1)/2; // set duty cycle to 50% of 4000 = 2000
	OC2R = (PERIOD+1)/2;
    
    T2CONbits.ON = 1; // turn on timer
	OC1CONbits.ON = 1; // turn on output 
    OC2CONbits.ON = 1;
    
}
void init_digital_io()
{
    TRISBbits.TRISB9 = 0; // make RB9 a digital output
    TRISBbits.TRISB13 = 0; // make RB13 a digital output.
    
    LATBbits.LATB9 = 1;
    LATBbits.LATB13 = 1;
    
}