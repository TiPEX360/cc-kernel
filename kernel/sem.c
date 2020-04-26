#include <sem.h>
extern void sem_inc(sem_t *sem);
extern void sem_dec(sem_t *sem);

void sem_init(sem_t *sem, uint32_t value) {
    *sem = value;
}

void sem_destroy(sem_t *sem) {
    *sem = 0;
}

void sem_wait(sem_t *sem) {
    sem_dec(sem);
}

void sem_post(sem_t *sem) {
    sem_inc(sem);
}