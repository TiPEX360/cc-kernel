/*
*  Unnamed semaphore implementation for quarantinOS
*/
#include <stdint.h>

typedef uint32_t sem_t;

void sem_wait(sem_t *sem);

void sem_destroy(sem_t *sem);

void sem_init(sem_t *sem, uint32_t value);

void sem_post(sem_t *sem);