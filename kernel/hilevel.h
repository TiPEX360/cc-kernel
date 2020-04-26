/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// Include functionality relating to the kernel.

#include "lolevel.h"
#include     "int.h"

// Structs

#define MAX_PROCS 12

typedef int pid_t;

typedef enum {
    STATUS_INVALID,
    STATUS_CREATED,
    STATUS_TERMINATED,
    STATUS_READY,
    STATUS_EXECUTING,
    STATUS_WAITING
} status_t;


typedef struct {
    uint32_t cpsr, pc, gpr[13], sp, lr //note reverse order, but GPR[0] is still r0
} ctx_t;

typedef struct {
     pid_t pid; 
     status_t status; 
     uint32_t tos; 
     int priority;
     int initPriority;
     ctx_t ctx; 
} pcb_t;

#endif
