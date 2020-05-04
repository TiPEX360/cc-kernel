#include "gui.h"
#include "font8x8_basic.h"

uint16_t fb[800][600];

char *font = font8x8_basic;

typedef struct {
    int x;
    int y;
    int marginLeft;
    int marginTop;
} cursor_t;

cursor_t cursor = {0, 0, 40, 40};

void gui_putc(char x) {
    int posX = cursor.marginLeft + (cursor.x*8);
    cursor.x = (cursor.x + 1) % 40;
    if(cursor.x == 0) cursor.y++;
    int posY = cursor.marginTop + cursor.y*8;
    if(posY > 40) clear();
    printChar(x, posX, posY, 0x7FFF);
}

void printChar(char x, int posX, int posY, uint16_t color) {
	unsigned char *glyph=font[x];
    char mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j++){
			if(glyph[j] & mask[i] > 0x00) fb[posX + i][posY + j] = glyph[j] & mask[i];
		}
	}
}

void clear() {
    for(int x = 0; x < 320; x++) {
        for(int y = 0; y < 320; y++) {
            fb[x][y] = 0x0000;
        }
    }
}