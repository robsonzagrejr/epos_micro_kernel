// EPOS Zynq (ARM Cortex-A9) SETUP

// If emulating on QEMU, uboot has a vector table in ROM at 0x00000000.
// It relocates EPOS image (.bin), which has this vector table at the
// first address, to 0x00010000 and jumps to it.

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        b       _reset                                                          \t\n\
        b       _undefined_instruction                                          \t\n\
        b       _software_interrupt                                             \t\n\
        b       _prefetch_abort                                                 \t\n\
        b       _data_abort                                                     \t\n\
        nop                             // _reserved                            \t\n\
        b       _int_entry              // IRQ                                  \t\n\
        b       _fiq                                                            \t\n\
                                                                                \t\n\
_reset:                                                                         \t\n\
        mrc     p15, 0, r2, c0, c0, 5   // Multiprocessor Affinity Register     \t\n\
        ands    r2, r2, #0x03                                                   \t\n\
        mov     r2, r2, LSL #14                                                 \t\n\
        ldr     r1, =__boot_stack__                                             \t\n\
        sub     r1, r1, r2                                                      \t\n\
        mov     sp, r1                                                          \t\n\
                                                                                \t\n\
        b       _start                                                          \t\n\
        ");
}

