#ifndef _IOEXPANDER_H
#define _IOEXPANDER_H

#include "i2c.h"

void initExpander();
void setExpander(char pin, char level);
char getExpander();
#endif