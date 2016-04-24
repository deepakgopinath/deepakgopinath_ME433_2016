#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c.h"
#include "imu.h"
#include <math.h>

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz. Crystal is 8Mhz. Therefore divide by 2. 
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV. 24*4 = 96
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz, 96/2 = 48. 
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0xDEED // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define CORE_TICKS 120000 // CORE_TIMER runs at 24Mhz. Therefore 24000000 ticks before rollover. 
//To get it flashing at 1000Hz (thats is the light will turn on a 1000 times) we want the flipping to happen after 12000 ticks
#define PERIOD 23999
#define ARRLEN 14
#define OUT_TEMP_L 0b00100000
#define OUTX_L_XL 0b00101000
#define OUTY_L_XL 0b00101010
#define OUTZ_L_XL 0b00101100
static unsigned char data[ARRLEN]; // 
static short dataS[ARRLEN/2];

void delay();
void initPWM();

int main() {
        
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
//    
//    // do your TRIS and LAT commands here
//   
    TRISAbits.TRISA4 = 0; // Make the Green Led which is RA4 to be an output. 
    TRISBbits.TRISB4 = 1; // Make the User button on RB4 an input. (by default this is already input.)
    LATAbits.LATA4 = 1; // Make RA4 high so that the green LED turns on.
    int i=0;
    for(i=0; i<ARRLEN; i++)
    {
        data[i] = 0x00;
    }
    for(i=0; i<ARRLEN/2; i++)
    {
        dataS[i] = 0x0000;
    }
    initI2C(); // initialize I2C
    initPWM();
    initIMU(); // initialize IMU registers. 
    
    __builtin_enable_interrupts();
    while(1) {
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = !LATAbits.LATA4; //Flip the LED. Could also use LATAINV to do the same.
        readIMU(OUT_TEMP_L, data, ARRLEN);
        for(i=0; i<ARRLEN/2; i++)
        {
            dataS[i] = (data[2*i+1] << 8 | data[2*i]);
        }
        OC3RS = (int)((((float)dataS[5] + 32767.0)/65535.0)*(PERIOD+1)); // data[6] is acceleration along z. the data is within -32767 to 32676. need to offset
        delay();
    }
    
}
void delay()
{
    while(_CP0_GET_COUNT() < CORE_TICKS){;}
}

void initPWM()
{
    //remap RPB as OC3
    RPB9Rbits.RPB9R = 0b0101; 
    
    // set up the TMR3 of OC3 and all the OC3 registers. 
    T3CONbits.TCKPS = 0b000;     //prescaler = 1
	PR3 = PERIOD; // period = 4000-1 for 20Khz
	TMR3 = 0; // initialize count to 0
    
    //OC3
    OC3CONbits.OCM = 0b110; // PWM mode for OC1 with no fault
	OC3CONbits.OC32 = 0; //USe 16 bit timer
	OC3CONbits.OCTSEL = 1; // Use timer 3
	OC3RS = (PERIOD+1)/3; // set duty cycle to 50% of 4000 = 2000
	OC3R = (PERIOD+1)/3;
    
    // turn timer and oc3 on
    T3CONbits.ON = 1; // turn on timer
	OC3CONbits.ON = 1; // turn on output 
    
}