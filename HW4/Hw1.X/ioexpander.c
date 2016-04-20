#include "ioexpander.h"

void initExpander()
{
    i2c_master_start();
    unsigned char devop = 0b01000000;
    i2c_master_send(devop);
    unsigned char reg = 0b00000000; //IODIR
    i2c_master_send(reg);
    unsigned char value = 0b11110000;
    i2c_master_send(value);
    i2c_master_stop();
}
//
char getExpander()
{
    i2c_master_start();
    unsigned char devop = 0b01000000;
    i2c_master_send(devop);
    unsigned char reg = 0x09;
    i2c_master_send(reg);
    i2c_master_restart();
    devop = 0b01000001;
    i2c_master_send(devop);
    unsigned char val = 0x0;
    val = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return val;
}
//
void setExpander(char pin, char level)
{
    i2c_master_start();
    unsigned char devop = 0b01000000;
    i2c_master_send(devop);
    unsigned char reg = 0b00001010;
    i2c_master_send(reg);
    unsigned char value = 0b00000000;
    value = value | (level << (int)pin);
    i2c_master_send(value);
    i2c_master_stop();
}