/*
*  Unnamed semaphore implementation for quarantinOS
*/

typedef uint32_t sem_t;

int sem_wait(sem_t *);

int sem_destroy(sem_t *);

int sem_init(sem_t *);

int intsem_post(sem_t *);