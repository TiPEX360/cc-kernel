#include "pipe.h"
#include "hilevel.h"

pipe_t pipeTab[MAX_PIPES];

int pipe(int pipefd[2]) {
    pipe_t *pipe = NULL;
    for(int i = 0; i < MAX_PIPES && pipe == NULL; i++) {
        if(pipeTab[i].fd[0] != -1) {
            pipeTab[i].fd[0] = 2*i + 3;
            pipeTab[i].fd[1] = 2*i + 4;
            pipeTab[i].head = &pipeTab[i].data[0];
            pipeTab[i].tail = &pipeTab[i].data[0];
            
            pipe = &pipeTab[i];
            pipefd[0] = pipe->fd[0];
            pipefd[1] = pipe->fd[1];
        }
    }
    if (pipe == NULL) return -1;
    return 0;
}

int close(int pipefd[2]) {
    pipe_t *pipe = NULL;
    for(int i = 0; i < MAX_PIPES && pipe == NULL; i++) {
        if(pipeTab[i].fd == pipefd) {
            pipeTab[i].fd[0] = -1;
            pipeTab[i].fd[1] = -1;
            pipeTab[i].head = NULL;
            pipeTab[i].tail = NULL;
            
            pipe = &pipeTab[i];
        }
    }
    if (pipe == NULL) return -1;
    return 0;
}