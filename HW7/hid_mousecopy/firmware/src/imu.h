#ifndef _IMU_H
#define _IMU_H

#include "i2c.h"

void initIMU();
void readIMU(char reg, unsigned char *data, char length);
#endif