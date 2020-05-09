#include "semphilo.h"

static void pickup(sem_t *fork) {
    sem_wait(fork);
    write(STDOUT_FILENO, "Picking up fork\n", 16);
    write(STDGUI_FILENO, "Picking up fork\n", 16);
}

static void release(sem_t *fork) {
    sem_post(fork);
    write(STDOUT_FILENO, "Putting down fork\n", 18);
    write(STDGUI_FILENO, "Putting down fork\n", 18);
}

static void eat() {
    int total = 0;
    int i,j;
    write(STDOUT_FILENO, "Eating\n", 7);
    write(STDGUI_FILENO, "Eating\n", 7);
    for(i = 1; i < 99999; i++) {
        for(j = 1; j < 9999; j++) {};
    }
}

void main_semphilo() {
    static sem_t forks[16];
    for(int i = 0; i < 16; i++) {
        sem_init(&forks[i], 1);
    }

    uint32_t isParent = 1;
    int left = 0; //Init left/right forks for this process
    int right = 1;

    for(int i = 1; i < 16 && isParent > 0; i++) {
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