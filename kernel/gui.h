#ifndef __GUI_H
#define __GUI_H

#include "hilevel.h"

uint16_t fb[600][800];

void gui_putc(char x);

void paintSprite(int sizeX, int sizeY, int posX, int posY, uint16_t *buffer);

void printChar(char x, int posX, int posY, uint16_t color);

void clear();

#endif