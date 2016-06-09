
#ifndef _SPI_H    /* Guard against multiple inclusion */
#define _SPI_H

#include <xc.h>
#define CS LATBbits.LATB7      // chip select pin

void initSPI1();
char SPI1_IO(char write);
#endif