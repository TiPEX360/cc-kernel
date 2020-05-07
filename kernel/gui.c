#include "gui.h"
#include "font8x8_basic.h"
#include "pcb.h"
// #include "backgeound.h"

uint16_t fb[600][800];
// uint16_t bg[600][800] = background;


typedef struct {
    int x;
    int y;
    int marginLeft;
    int marginTop;
} cursor_t;

cursor_t cursor = {0, 0, 40, 40};

// void paintBackground() {
//     for(int y = 0; y < 600; y++) {
//         for (int x = 0; x < 800; x++) {
//             fb[y][x] = bg[y][x];
//         }
//     }
// }

void gui_putc(char x) {
    if(x == 10) {
        cursor.y++;
        cursor.x = 0;
    }
    else {
        if(cursor.x == 0) {
            char prefix[3];
            itoa(prefix, executing->pid);
            printConsole(prefix[0]);
            printConsole(prefix[1]);
            printConsole(prefix[2]);
        }
        printConsole(x);
    }
}

void printConsole(char x) {
        int posX = cursor.marginLeft + (cursor.x*8);
        int posY = cursor.marginTop + cursor.y*8;
        cursor.x = (cursor.x + 1) % 40;
        if(cursor.x == 0) cursor.y++;
        //if(posY > 40) clear();
        printChar(x, posY, posX, 0x7FFF);
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
    for(int y = 0; y < 320; y++) {
        for(int x = 0; x < 320; x++) {
            fb[y][x] = 0x0000;
        }
    }
}