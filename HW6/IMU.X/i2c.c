#include <proc/p32mx250f128b.h>
#include "i2c.h"

void initI2C()
{
    ANSELBbits.ANSB2 = 0; // turn off SDA2 pin's analog functionality
    ANSELBbits.ANSB3 = 0; // turn off SCL2 pin's analog functionality
    i2c_master_setup();
}

void i2c_master_setup(void) {
  I2C2BRG = 53;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 // look up PGD for your PIC32
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}

//mainly used for reading data from imu. 

void i2c_read_multiple(char address, char reg, unsigned char * data, char length)
{
    i2c_master_start();
    //First write
    unsigned char devopW = (address << 1) | 0x00; // append 0 at the end of the address for the write
    unsigned char devopR = (address << 1) | 0x01;   
    i2c_master_send(devopW);
    i2c_master_send(reg); // register to start reading from 
    i2c_master_restart();
    i2c_master_send(devopR);
    int i=0;
    for(i=0; i<(int)length-1; i++)
    {
        data[i] = i2c_master_recv();
        i2c_master_ack(0); // continue reading length - 1 bytes
    }
    data[(int)length-1] = i2c_master_recv(); // read the last byte
    i2c_master_ack(1); // send the last acknowledgment with 1
    i2c_master_stop();
}