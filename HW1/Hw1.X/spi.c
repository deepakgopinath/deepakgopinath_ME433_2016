#include <proc/p32mx250f128b.h>
#include "spi.h"

void initSPI1()
{
    TRISBbits.TRISB7 = 0; // CS using RB7 digital output. 
    CS = 1;
    
     //Remappable pins for SDO1 -RPB13
    RPB13Rbits.RPB13R = 0b0011;
    //Remappable pins for SDI1 - RPB8 (not used)
    SDI1Rbits.SDI1R = 0b0100;
    
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x1;            // baud rate to 12 MHz [SPI4BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 1
    
}

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

char SPI1_IO(char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}