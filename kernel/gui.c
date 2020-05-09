#include "gui.h"
#include "font8x8_basic.h"
#include "pcb.h"
#include "background.h"

uint16_t fb[600][800];

typedef struct {
    int col;
    int line;
    int marginLeft;
    int marginTop;
    int spacing;
} cursor_t;

cursor_t cursor = {0, 0, 83, 130, 3};

uint16_t rgbtobgr(uint16_t x) {
    uint16_t new = 0x0000;
    new |= (x & 0x001F) << 10;
    new |= (x & 0x03E0);
    new |= (x & 0x7C00) >> 10;
    return new;
}

void paintSprite(int sizeX, int sizeY, int posX, int posY, uint16_t *buffer) {
    uint16_t pixel;
    int i = 0;
    for(int y = 0; y < sizeY; y++) {
        for (int x = 0; x < sizeX; x++) {
            //pixel = ((uint16_t)buffer[x*2] << 8) | (uint16_t)buffer[x*2+1];
            // pixel = ((0xFFFF & buffer[i*2]) << 0x8) | buffer[i*2+1];
            //pixel = (uint16_t)buffer[x*2];
            pixel = buffer[i];
            fb[y + posY][x + posX] = rgbtobgr(pixel);
            i++;
        }
    }
}

void printConsole(char x) {
        if(cursor.line > 35) clear();
        int posX = cursor.marginLeft + (cursor.col*8);
        int posY = cursor.marginTop + cursor.line*8 + cursor.spacing;
        cursor.col = (cursor.col + 1) % 40;
        if(cursor.col == 0) cursor.line++;
        printChar(x, posY, posX, 0x656F);
}

void gui_putc(char x) {
    if(x == 10) {
        cursor.line++;
        cursor.col = 0;
    }
    else {
        if(cursor.col == 0) {
            char prefix[3];
            itoa(prefix, executing->pid);
            printConsole(prefix[0]);
            printConsole(prefix[1]);
            printConsole(prefix[2]);
        }
        printConsole(x);
    }
}


void printChar(char x, int posY, int posX, uint16_t color) {
	unsigned char *glyph=font8x8_basic[x];
    char mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

	for(int xy = 0; xy < 8; xy++){
		for(int xx = 0; xx < 8; xx++){
			if(glyph[xy] >> xx & 1) fb[posY + xy][posX + xx] = color;
		}
	}
}

void clear() {
    cursor.col = 0;
    cursor.line = 0;
    paintSprite(800, 600, 0, 0, background);
}