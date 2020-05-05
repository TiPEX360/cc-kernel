#ifndef __GUI_H
#define __GUI_H

#include "hilevel.h"

uint16_t fb[800][600];

void gui_putc(char x);

void printChar(char x, int posX, int posY, uint16_t color);

void clear();

#endif