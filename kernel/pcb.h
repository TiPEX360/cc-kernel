#ifndef __PCB_H
#define __PCB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_PROCS 20

typedef int pid_t;


// Structs
typedef struct {
    uint32_t cpsr, pc, gpr[13], sp, lr //note reverse order, but GPR[0] is still r0
} ctx_t;

typedef enum {
    STATUS_INVALID,
    STATUS_CREATED,
    STATUS_TERMINATED,
    STATUS_READY,
    STATUS_EXECUTING,
    STATUS_WAITING
} status_t;

typedef struct {
     pid_t pid; 
     status_t status; 
     uint32_t tos; 
     int priority;
     int initPriority;
     ctx_t ctx; 
} pcb_t;


const int PageSize;

pcb_t procTab[MAX_PROCS]; 
pcb_t *executing;

#endif