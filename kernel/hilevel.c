/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "scheduler.h"

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


void schedule(ctx_t* ctx) {
  //Adjust current process priority
  if(executing->priority != executing->initPriority) executing->priority = executing->initPriority;

  //Get next priority process
  pcb_t* next = executing;

  for (int i = 0; i < MAX_PROCS; i++) {
    if(procTab[i].status == STATUS_READY && procTab[i].pid != executing->pid) {
      if(procTab[i].priority > next->priority) next = &procTab[i];
      else procTab[i].priority++; //Adjust skipped process priority 
    }
  }

  //Dispatch and update status
  pcb_t* prev = executing;
  dispatch(ctx, prev, next);
  prev->status = STATUS_READY;
  next->status = STATUS_EXECUTING;

  return;
}

extern uint32_t* tos_usr;
extern void main_P1;
extern void main_P2;
extern void main_P3;
extern void main_P4;

void loadProcs() {
  // memset(&procTab[0], 0, sizeof(pcb_t)); // initialise 0-th PCB = P_1
  // procTab[0].pid = 1;
  // procTab[0].status = STATUS_READY;
  // procTab[0].tos = (uint32_t)(&tos_usr) - PageSize * 0;
  // procTab[0].ctx.cpsr = 0x50;
  // procTab[0].ctx.pc = (uint32_t)(&main_P3);
  // procTab[0].ctx.sp = procTab[0].tos;
  // procTab[0].priority = 6;
  // procTab[0].initPriority = 6;

  // memset(&procTab[1], 0, sizeof(pcb_t)); // initialise 1-st PCB = P_2
  // procTab[1].pid = 2;
  // procTab[1].status = STATUS_READY;
  // procTab[1].tos = (uint32_t)(&tos_usr) - PageSize * 1;
  // procTab[1].ctx.cpsr = 0x50;
  // procTab[1].ctx.pc = (uint32_t)(&main_P4);
  // procTab[1].ctx.sp = procTab[1].tos;
  // procTab[1].priority = 6;
  // procTab[1].initPriority = 6;


  svc_exec_handler(&main_P3);
  svc_exec_handler(&main_P4);

  return;
}

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
  PL011_putc(UART0, 'R', true);


  //Set up pcb vector
  for( int i = 0; i < MAX_PROCS; i++ ) {
    procTab[ i ].status = STATUS_INVALID;
  }

  svc_exec_handler(&main_P3, 3);
  svc_exec_handler(&main_P4, 3);

  //TODO: call init() function here to load starting programs etc

  dispatch(ctx, NULL, &procTab[0]);

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
int svc_exec_handler(uint32_t pc, int priority) {
  int pid = NULL;
  for(int i = 0; i < MAX_PROCS && pid == NULL; i++) {
    if(procTab[i].status == STATUS_INVALID || procTab[i].status == STATUS_TERMNATED) pid = i + 1;
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

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {


  uint32_t *args = ctx->gpr;

  switch(id) {
    case 0x03:
      //Fork
      PL011_putc(UART0, 'F', true);
      break;
    case 0x04:
      //Exit
      PL011_putc(UART0, 'E', true);
      break;
    case 0x05:
      svc_exec_handler((uint32_t)&args[0], 3);
      break;
    default:

      break;
  }

  return;
}
