#ifndef __PIPE_H
#define __PIPE_H

#define MAX_PIPES 40

typedef struct {
    int fd[2];
    char data[16];
    int head;
    int tail;
} pipe_t;

pipe_t pipeTab[MAX_PIPES];

int pipe(int pipefd[2]);

int close(int pipefd[2]);

#endif