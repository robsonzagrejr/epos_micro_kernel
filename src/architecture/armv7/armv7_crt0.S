// EPOS ARMv7 Program Starter

        .file "armv7_crt0.S"

        // Entry point called by SETUP for microcontrollers
        .align  2
        .global _mcu_start
        .type   _mcu_start, function
_mcu_start:
        // Temporary Stack Pointer for INIT for Cortex-M; Cortex-A temporary stack(s) for INIT are created and configure by SETUP
        // Main's stack will be allocated by Thread::init()
        ldr     r0, =__boot_stack__
        mov     sp, r0

        // Entry point called by SETUP for processors
        .section .text
        .align  2
        .global _start
        .type   _start, function
_start:
        // Save argc and argv
        mov      r2, #3
        push    {r0-r1}
        // Clear the BSS
        eor     r0, r0
        ldr     r1, =__bss_start__
        ldr     r2, =__bss_end__
.L1:    str     r0, [r1]
        add     r1, #4
        cmp     r1, r2
        blt     .L1

        bl      _init

        // Call main
        .align  2
        .globl  __epos_app_entry
        .type   __epos_app_entry, function
__epos_app_entry:
//_user_mode:
        //msr cpsr_all, #16    // Change to user mode
        //mov r12, sp
        //mrs sp, sp_usr
        //msr sp_usr, r12

        // Load argc and argv
        pop     {r0-r1}
        bl      main
        push    {r0}         // save main's return value to be used by exit()
        bl      _fini
        bl      _exit
