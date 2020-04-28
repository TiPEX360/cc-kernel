/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "pcb.h"

extern uint32_t* tos_usr;
extern void main_P1;
extern void main_P2;
extern void main_P3;
extern void main_P4;
extern void main_P5;
extern void main_philosophers;
extern void main_console;

void hilevel_handler_rst(ctx_t* ctx) {

  //Set up timers
  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  //Set up pcb vector
  for( int i = 0; i < MAX_PROCS; i++ ) {
    procTab[ i ].status = STATUS_INVALID;
  }

  // newProc(&main_P3, 3);
  // newProc(&main_P4, 3);
  newProc(&main_philosophers, 3);
  newProc(&main_console, 3);

  //TODO: call init() function here to load starting programs etc

  dispatch(ctx, NULL, &procTab[0]); //Replace with schedule?

  int_enable_irq();

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {

  uint32_t id = GICC0->IAR;

  if(id == GIC_SOURCE_TIMER0) {
    schedule(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {

  uint32_t *args = ctx->gpr;

  switch(id) {
    case 0x00:
      //Yield
      schedule(ctx);
      break;
    case 0x01 : 
      {
        int   fd = ( int   )( args[ 0 ] );  
        char*  x = ( char* )( args[ 1 ] );  
        int    n = ( int   )( args[ 2 ] ); 
        for( int i = 0; i < n; i++ ) {
          PL011_putc( UART0, *x++, true );
        }
        args[ 0 ] = n;
      }
      break;
    case 0x03:
      //Fork
      svc_handler_fork(ctx);
      break;
    case 0x04:
      //Exit
      svc_handler_exit(ctx, args[0]);
      break;
    case 0x05:
      svc_handler_exec(ctx, args[0]);
      break;
    case 0x08:
      //block process
      executing->status = STATUS_WAITING;
      schedule(ctx);
      break;
    case 0x09:
      //unblock all
      for(int i = 0; i < MAX_PROCS; i++) {
        if(procTab[i].status == STATUS_WAITING) procTab[i].status = STATUS_READY;
      }
      break;
    default:
      break;
  }

  return;
}

int newProc(uint32_t pc, int priority) {
  int pid = NULL;
  for(int i = 0; i < MAX_PROCS && pid == NULL; i++) {
    if(procTab[i].status == STATUS_INVALID || procTab[i].status == STATUS_TERMINATED) pid = i + 1;
  }

  if(pid == NULL) return 1;

  memset(&procTab[pid - 1], 0, sizeof(pcb_t));
  procTab[pid - 1].pid = pid;
  procTab[pid - 1].status = STATUS_READY;
  procTab[pid - 1].tos = (uint32_t)(&tos_usr) - PageSize * (pid - 1);
  procTab[pid - 1].ctx.cpsr = 0x50;
  procTab[pid - 1].ctx.pc = pc;
  procTab[pid - 1].ctx.sp = procTab[pid - 1].tos;
  procTab[pid - 1].priority = priority;
  procTab[pid - 1].initPriority = priority;

  return 0;
}
