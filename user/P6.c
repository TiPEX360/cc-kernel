#include "P6.h"

void pickup(sem_t *fork) {
    sem_wait(fork);
    write(STDOUT_FILENO, "PICKUP", 6);
}

void release(sem_t *fork) {
    sem_post(fork);
    write(STDOUT_FILENO, "RELEASE", 7);
}

void eat() {
    int i,j;
    for(i = 1; i < 99999; i++) {
        write(STDOUT_FILENO, "E\n", 2);
    }
}

void main_P6() {
    static sem_t forks[5];
    for(int i = 0; i < 5; i++) {
        sem_init(&forks[i], 1);
    }

    uint32_t isParent = 1;
    int left = 0; //Init left/right forks for this process
    int right = 1;

    for(int i = 1; i < 5 && isParent > 0; i++) {
        isParent = fork(); //Create a child process for each philosopher
        if(isParent == 0) {
            left = i; //Change left/right forks for child process
            right = (i+1)%5;
        }
    }

    pickup(left > right ? &forks[right] : &forks[left]);
    pickup(left > right ? &forks[left] : &forks[right]);
    eat();
    release(&forks[left]);
    release(&forks[right]);

    exit(EXIT_SUCCESS);
}