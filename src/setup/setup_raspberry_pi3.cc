// EPOS Raspberry PI3 (ARM Cortex-A53) SETUP

// If emulating on QEMU, uboot has a vector table in ROM at 0x00000000.
// It relocates EPOS image (.bin), which has this vector table at the
// first address, to 0x00010000 and jumps to it.

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        ldr pc, reset_handler                                                   \t\n\
        ldr pc, undef_instruc                                                   \t\n\
        ldr pc, software_interrupt                                              \t\n\
        ldr pc, prefetch_abort                                                  \t\n\
        ldr pc, data_abort                                                      \t\n\
        nop                             // _reserved                            \t\n\
        ldr pc, int_entry               // IRQ                                  \t\n\
        ldr pc, fiq                                                             \t\n\
                                                                                \t\n\
reset_handler: .word _reset                                                     \t\n\
undef_instruc: .word _undefined_instruction                                     \t\n\
software_interrupt: .word _software_interrupt                                   \t\n\
prefetch_abort: .word _prefetch_abort                                           \t\n\
data_abort: .word _data_abort                                                   \t\n\
int_entry: .word _int_entry                                                     \t\n\
fiq: .word _fiq                                                                 \t\n\
_reset:                                                                         \t\n\
        mrs     r0,cpsr                                                         \t\n\
        bic     r0,r0,#0x1F                                                     \t\n\
        orr     r0,r0,#0x13                                                     \t\n\
        msr     spsr_cxsf,R0                                                    \t\n\
        add     r0,pc,#4                                                        \t\n\
        msr     ELR_hyp,r0                                                      \t\n\
        eret                                                                    \t\n\
                                                                                \t\n\
        mov     r0,#0xD2                                                        \t\n\
        msr     cpsr_c,r0                                                       \t\n\
        mov     sp,#0x7ffc                                                      \t\n\
                                                                                \t\n\
        mov     r0,#0xD1                                                        \t\n\
        msr     cpsr_c,r0                                                       \t\n\
        mov     sp,#0x3ffc                                                      \t\n\
                                                                                \t\n\
        mov     r0,#0xD3                                                        \t\n\
        msr     cpsr_c,r0                                                       \t\n\
        mov     sp,#0x7000000                                                   \t\n\
                                                                                \t\n\
        mrc     p15, 0, r2, c0, c0, 5  // Read Multiprocessor Affinity Register \t\n\
        ands    r2, r2, #0x3           // Mask off, leaving the CPU ID field    \t\n\
        bne     secondary_core                                                  \t\n\
                                                                                \t\n\
        //In the reset handler, we need to copy our interrupt vector table      \t\n\
        //to 0x0000, its currently at 0x8000                                    \t\n\
        //the other way is to  use vbar address: mrc p15, 0, r1, c12, c0, 0     \t\n\
                                                                                \t\n\
        mov     r0,#0x8000    // Store the source pointer                       \t\n\
        mov     r1,#0x0000    // Store the destination pointer.                 \t\n\
                                                                                \t\n\
        // Here we copy the branch instructions                                 \t\n\
        // Load multiple values from indexed address.       ; Auto-increment r0 \t\n\
        ldmia   r0!,{r2,r3,r4,r5,r6,r7,r8,r9}                                   \t\n\
        // Store multiple values from the indexed address.  ; Auto-increment r1 \t\n\
        stmia   r1!,{r2,r3,r4,r5,r6,r7,r8,r9}                                   \t\n\
                                                                                \t\n\
        // To the branches get the correct addresses,                           \t\n\
        //               we also need to copy our vector table!                 \t\n\
        // Load from 4*n of regs (8) as R0 is now incremented.                  \t\n\
        ldmia   r0!,{r2,r3,r4,r5,r6,r7,r8,r9}                                   \t\n\
        // Store this extra set of data.                                        \t\n\
        stmia   r1!,{r2,r3,r4,r5,r6,r7,r8,r9}                                   \t\n\
                                                                                \t\n\
        b       all                                                             \t\n\
secondary_core:                                                                 \t\n\
                                                                                \t\n\
        ldr     r0, =0x400000CC             //ARM local mailbox3 clr0           \t\n\
        mov     r1, #0x10                                                       \t\n\
        mul     r1, r2, r1                  //r2 is CPU id                      \t\n\
        mov     r3, #0                                                          \t\n\
        str     r3, [r0, r1]                                                    \t\n\
        ldr     r0, =0x40000050             //arm local mailbox int control0    \t\n\
        mov     r1, #4                                                          \t\n\
        mul     r1, r2, r1                  //r2 is CPU id                      \t\n\
        mov     r3, #0xF                                                        \t\n\
        str     r3, [r0, r1]                                                    \t\n\
all:                                                                            \t\n\
        mrc     p15, 0, r2, c0, c0, 5   // Multiprocessor Affinity Register     \t\n\
        ands    r2, r2, #0x03                                                   \t\n\
        mov     r2, r2, LSL #18         // this sets 256KB boot_stack per core  \t\n\
        ldr     r1, =__boot_stack__                                             \t\n\
        sub     r1, r1, r2                                                      \t\n\
        mov     sp, r1                                                          \t\n\
                                                                                \t\n\
        // mov R0, #0                                                           \t\n\
        // MSR SPSR, R0                                                         \t\n\
                                                                                \t\n\
        @ enable the FPU                                                        \t\n\
        //-mfloat-abi=hard       on compiling flags                             \t\n\
        //check context switch when working with more than one thread           \t\n\
        mrc     p15, 0, r0, c1, c0, 2                                           \t\n\
        orr     r0, r0, #0x300000            /* single precision */             \t\n\
        orr     r0, r0, #0xC00000            /* double precision */             \t\n\
        mcr     p15, 0, r0, c1, c0, 2                                           \t\n\
        mov     r0, #0x40000000                                                 \t\n\
        fmxr    fpexc,r0                                                        \t\n\
                                                                                \t\n\
        b       _start                                                          \t\n\
    ");
}
