/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

const int PageSize = (0x00006000 / MAX_PROCS) - ((0x00006000 / MAX_PROCS) % 8) ;

pcb_t procTab[MAX_PROCS]; 
pcb_t *executing = NULL;

void dispatch(ctx_t* ctx, pcb_t* prev, pcb_t* next) {
  char prev_pid = '?', next_pid = '?';

  if(NULL != prev) {
    memcpy(&prev->ctx, ctx, sizeof(ctx_t)); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if(NULL != next) {
    memcpy(ctx, &next->ctx, sizeof(ctx_t)); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc(UART0, '[', true);
    PL011_putc(UART0, prev_pid, true);
    PL011_putc(UART0, '-', true);
    PL011_putc(UART0, '>', true);
    PL011_putc(UART0, next_pid, true);
    PL011_putc(UART0, ']', true);

    executing = next;                           // update   executing process to P_{next}

  return;
}

extern void lolevel_handler_rst();
void schedule(ctx_t* ctx) { //TERMINATED PROC GETS SCHEDULED!
  //Reset priority for process which has just been executing
  if(executing->priority != executing->initPriority) executing->priority = executing->initPriority;

  pcb_t* next = executing;
  if(next->status == STATUS_TERMINATED) {
    next->priority = 0;
    next->initPriority = 0;
  }

  //Get next priority process
  for (int i = 0; i < MAX_PROCS; i++) {
    if(procTab[i].status == STATUS_READY && procTab[i].pid != executing->pid) {
      if(procTab[i].priority > next->priority) next = &procTab[i];
    }
  }
  if (next->status == STATUS_TERMINATED) lolevel_handler_rst();


  //Dispatch and update status
  pcb_t* prev = executing;
  dispatch(ctx, prev, next);
  if(prev->status == STATUS_EXECUTING) prev->status = STATUS_READY; //CHANGE HERE
  next->status = STATUS_EXECUTING;

  //Adjust priorities for next time
  for(int i = 0; i < MAX_PROCS; i++) {
    if(procTab[i].pid != executing->pid){
      procTab[i].priority++;
    }
  }
  
  return;
}

extern uint32_t* tos_usr;
extern void main_P1;
extern void main_P2;
extern void main_P3;
extern void main_P4;
extern void main_P5;
extern void main_P6;

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

  newProc(&main_P6, 3);

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

//Move to svc file?
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

void svc_handler_exec(ctx_t*ctx, uint32_t pc) {
  ctx->pc = pc; 
  ctx->sp = executing->tos;
  return;
}

void svc_handler_fork(ctx_t* ctx) {
  int pid = NULL;
  for(int i = 0; i < MAX_PROCS && pid == NULL; i++) {
    if(procTab[i].status == STATUS_INVALID || procTab[i].status == STATUS_TERMINATED) pid = i + 1;
  }

  if(pid == NULL) {
    ctx->gpr[0] = -1; 
    return;
  }

  memset(&procTab[pid - 1], 0, sizeof(pcb_t));
  procTab[pid - 1].pid = pid;
  procTab[pid - 1].status = STATUS_READY;
  procTab[pid - 1].tos = (uint32_t)(&tos_usr) - PageSize * (pid - 1); //Give it own stack?
  procTab[pid - 1].priority = 3;
  procTab[pid - 1].initPriority = 3;

  procTab[pid - 1].ctx.cpsr = 0x50;
  procTab[pid - 1].ctx.pc = ctx->pc;
  procTab[pid - 1].ctx.gpr[0] = 0; 
  procTab[pid - 1].ctx.sp = procTab[pid - 1].tos;
  procTab[pid - 1].ctx.lr = ctx->lr;

  for(int i = 1; i < 13; i++) procTab[pid - 1].ctx.gpr[i] = ctx->gpr[i];

  memcpy(procTab[pid - 1].tos, executing->tos, PageSize); 

  //Parent process return 1
  ctx->gpr[0] = pid; return; //Is it done?
};

void svc_handler_exit(ctx_t* ctx, status_t s) { // THIS NOT DONE PROPERLY PLS FIX
  executing->status = STATUS_TERMINATED;
  schedule(ctx);
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {

  uint32_t *args = ctx->gpr;

  switch(id) {
    case 0x00:
      //Yield
      schedule(ctx);
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
