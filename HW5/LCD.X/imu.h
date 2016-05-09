#ifndef _IMU_H
#define _IMU_H

void initIMU();
void readIMU(char reg, unsigned char *data, char length);
#endif