// EPOS RISC-V 32 Program Starter

        .file "rv32_crt0.S"

        // Entry point called by SETUP for processors
        .section .text
        .align  4
        .global _start
        .type   _start, function
_start:
        // Temporary stack(s) for INIT are created and configure by SETUP

        // Only hart (core) 0 cleans cache
        mv      t0, tp
        bnez    t0, 2f

        // Set all bytes in the BSS section to zero.
        la      a0, __bss_start
        la      a1, _end
        bgeu    a0, a1, 2f
1:
        sw      zero, (a0)
        addi    a0, a0, 4
        bltu    a0, a1, 1b
2:
        // Call _init
        call      _init

        // Call main
        .align  4
        .globl  __epos_app_entry
        .type   __epos_app_entry, function
__epos_app_entry:
        call    main
stop:   addi    sp, sp, -4
        sw      a0, 0(sp)   // save main's return value to be used by exit()
        call    _fini
        lw      a0, 0(sp)
        call    _exit
