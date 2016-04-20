#include "dac.h"
#include "spi.h"


void setVoltage(char channel, char voltage)
{
    //Example voltage = 10110011
    //Get first 4 bits of voltage
    
//    CS = 0;
//    unsigned short data = 0;
//    data = voltage << 4; // 0000 1011 0011 0000
//    unsigned short config = 0;
//    if (channel == 0)
//    {
//        config = 0x3 << 12;
////        data = data | config; //0x3 is 0011 the MSBits for channel A. 
//        //0000 1011 0011 0000 | 0011 0000 0000 0000 = 0011 1011 0011 0000
//    } else
//    {
//        config = 0xB << 12;
////        data = data | (config); // 0xB is 1011 the MSBbits for channel B
//        //0000 1011 0011 0000 | 1011 0000 0000 0000 = 1011 1011 0011 0000
//    }
//    data = data | config;
//    spi_io((data & 0xFF00) >> 8); // 1011 1011 0011 0000 & 1111 1111 0000 0000 = 1011 1011 0000 0000 >> 8 = 1011 1011
//    spi_io(data & 0x00FF); // 1011 1011 0011 0000 & 0000 0000 1111 1111 = 0011 0000
//    CS = 1;
    CS = 0;
    unsigned char msbvolt = 0;
    msbvolt = (voltage & 0xF0); // 10110011 &11110000 = 10110000
    msbvolt = msbvolt >> 4; // 00001011
    
    unsigned char lsbvolt = 0;
    lsbvolt = (voltage & 0x0F); // 10110011 & 00001111 = 0000 0011
    lsbvolt = lsbvolt << 4; // 0011 0000
    
    unsigned char config = 0;
    unsigned char msbbyte = 0;
    if (channel == 0)
    {   
        config = 0x3; // 0000 0011
        config = config << 4; //0011 0000
        msbbyte = config | msbvolt; //0011 0000 | 0000 1011 = 0011 1011
        spi_io(msbbyte);
        spi_io(lsbvolt);
       // 0011(voltage)0000
    }else 
    {
        config = 0xB; // 0000 1011
        config = config << 4; // 1011 0000
        msbbyte = config | msbvolt; // 1011 0000 | 0000 1011 = 1011 1011
        spi_io(msbbyte);
        spi_io(lsbvolt);
        //1011(voltage)0000
    }
    CS = 1;
}