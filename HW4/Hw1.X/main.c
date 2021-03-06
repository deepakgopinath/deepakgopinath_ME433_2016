#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "spi.h"
#include "dac.h"
#include "i2c.h"
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

#define CORE_TICKS 24000 // CORE_TIMER runs at 24Mhz. Therefore 24000000 ticks before rollover. 
//To get it flashing at 1000Hz (thats is the light will turn on a 1000 times) we want the flipping to happen after 12000 ticks
#define NUMSAMPS 1000
#define SINEFREQ 10
#define TRIFREQ 5

static volatile char triWave[NUMSAMPS];
static volatile char sineWave[NUMSAMPS];
static volatile char userButton;
void delay(void);
void makeTriangle();
void makeSine();
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
    
    initSPI1(); // initialize SPI1.
    initI2C(); // initialize I2C
    
    makeSine();
    makeTriangle();
    
    initExpander();

    
    __builtin_enable_interrupts();
    int index = 0;
    while(1) {
        _CP0_SET_COUNT(0);
        
        LATAbits.LATA4 = !LATAbits.LATA4; //Flip the LED. Could also use LATAINV to do the same.
        setVoltage(1, sineWave[index]); // Channel A - sine wave
        setVoltage(0, triWave[index]); // Channel B - triangle wave.
        index = index + 1;
        if(index == NUMSAMPS)
        {
            index = 0;
        }
        
        userButton = getExpander();
        if((userButton & 0x80) >> 7 == 0) // Button is pressed
        {
            setExpander(0, 1);
        }
        else
        {
            setExpander(0, 0);
        }
        delay();
    }
    
}

void delay()
{
    
    while(_CP0_GET_COUNT() < CORE_TICKS){;}
//    while(!PORTBbits.RB4) {
//        LATAbits.LATA4 = 0; // When the button is pressed turn the LED off. This is not necessary. But at times, it can remain on. 
//        ;   
//    }
}

void makeTriangle()
{
    int i=0;
    for(i=0; i< NUMSAMPS; i++)
    {
        triWave[i] = (char)(((i%(200))/100.0)*(128.0)); //5 cycles of traingle wave in a 1000 point array.
    }
}

void makeSine()
{
    int i=0;
    for(i=0; i<NUMSAMPS; i++)
    {
        sineWave[i] = (char)(127.0*sin(SINEFREQ*i*2*M_PI/NUMSAMPS)+128.0); //offset by 128 to bring output to 0-3.3V
    }
    
}