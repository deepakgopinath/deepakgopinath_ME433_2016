#include "imu.h"

unsigned char devop = 0b1101011;
void init_accel();
void init_gyro();
void init_seq_read();

void initIMU()
{
    init_accel();
    init_gyro();
    init_seq_read();
}

void init_accel()
{
    //initialize the CTRL1_XL
    i2c_master_start();
    i2c_master_send((devop << 1) | 0x00); // construct addr + write bit
    unsigned char reg = 0b00010000; // CTRL1_XL
    i2c_master_send(reg);
    unsigned char d = 0b10000000; //1000(1.66)00(2g)00(400Hz)
    i2c_master_send(d);
    i2c_master_stop();
}

void init_gyro()
{
    i2c_master_start();
    i2c_master_send((devop << 1) | 0x00); // addr plus 0 for write
    unsigned char reg = 0b00010001; //CTRL2_G
    i2c_master_send(reg);
    unsigned char d = 0b10000000; //1000(1.66)00(245dps)0
    i2c_master_send(d);
    i2c_master_stop();
}
void init_seq_read()
{
    i2c_master_start();
    i2c_master_send((devop << 1) | 0x00);
    unsigned char reg = 0b00010010; // CTRL3_C
    i2c_master_send(reg);
    unsigned char d = 0b00000100; //Set IF_INC bit
    i2c_master_send(d);
    i2c_master_stop();
}

void  readIMU(char reg, unsigned char *data, char length) // this function will be called from main. With starting register 
// data array to be filled out with data and the number of registers to be read. 
{
    i2c_read_multiple(devop, reg, data, length);
}