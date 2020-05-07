#include "pipephilo.h"

void pickup(int fork[2]) { //fd not assigned correctly
    int isFork = 0; 
    while(isFork == 0) {
        isFork = read(fork[0], "", 1);
    }
    write(STDOUT_FILENO, "Picking up fork\n", 16);
    write(STDGUI_FILENO, "Picking up fork\n", 16);
}

void release(int fork[2]) {
    write(fork[1], 0xFF, 1);
    write(STDOUT_FILENO, "Putting down fork\n", 18);
    write(STDGUI_FILENO, "Putting down fork\n", 18);
}

void eat() {
    int total = 0;
    int i,j;
    write(STDOUT_FILENO, "Eating\n", 7);
    write(STDGUI_FILENO, "Eating\n", 7);
    for(i = 1; i < 99999; i++) {
        for(j = 1; j < 9999; j++) {};
    }
}

void main_pipephilo() {
    
    int forks[16][2];
    for(int i = 0; i < 16; i++) {
        pipe(forks[i]);
        write(forks[i][1], "f", 1);
    }

    uint32_t isParent = 1;
    int left = 0; //Init left/right forks for this process
    int right = 1;

    for(int i = 1; i < 16 && isParent > 0; i++) { //CHILDREN NOT RECEIVING previous variables
        isParent = fork(); //Create a child process for each philosopher
        if(isParent == 0) {
            left = i; //Change left/right forks for child process
            right = (i+1)%16;
        }
    }

    pickup(left > right ? &forks[right] : &forks[left]);
    pickup(left > right ? &forks[left] : &forks[right]);
    eat();
    release(&forks[left]);
    release(&forks[right]);

    exit(EXIT_SUCCESS);
}