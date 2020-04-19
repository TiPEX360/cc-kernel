/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

/* Each of the following is a low-level interrupt handler: each one is
 * tasked with handling a different interrupt type, and acts as a sort
 * of wrapper around a high-level, C-based handler.
 */

.global lolevel_handler_rst
.global lolevel_handler_irq
.global lolevel_handler_svc

lolevel_handler_rst: bl    int_init                @ initialise interrupt vector table

                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_irq            @ initialise IRQ mode stack
                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_svc            @ initialise SVC mode stack
                     
                     add   sp, #68                 @ allocate room for ctx param
                     mov   r0, sp                  @ set arg0 to ctx(point to element)
                     bl    hilevel_handler_rst     @ invoke high-level C function
                                                   @ ctx is no longer blank after dispatch
                     
                     ldmia sp!, { r0, lr }         @ load pc and lr from ctx
                     msr   spsr, r0                @ context pc into SPSR
                     ldmia sp, {r0-r12, sp, lr}^  @ context registers into USR registers 
                     add   sp, sp, #60
@normally would clear stack here, but tried in one line with !
                     movs pc, lr                   @ run context

lolevel_handler_irq: sub   lr, lr, #4              @ correct return address
                    @  stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     sub   sp, sp, #60             @ make room for USR registers
                     stmia sp, {r0-r12, sp, lr}^   @ save USR registers
                     mrs   r0, spsr                @ move USR CPSR(SPSR)
                     stmdb sp!, {r0, lr}           @ save USR PC(LR) and current CPSR

                     mov   r0, sp                  @ set arg0 as context
                     bl    hilevel_handler_irq     @ invoke high-level C function

                     ldmia sp!, {r0, lr}           @ recover USR CPSR and LR
                     msr   spsr, r0                @ set SPSR to USR CPSR
                     ldmia sp, {r0-r12, sp, lr}^   @ restore USR context
                     add   sp, sp, #60              @ clear context from stack

                    @  ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_svc: sub   lr, lr, #0              @ correct return address
                     stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     bl    hilevel_handler_svc     @ invoke high-level C function

                     ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt 
