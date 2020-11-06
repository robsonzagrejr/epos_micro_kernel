// EPOS RISC-V sifive SETUP

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
    j _reset                                                                    \t\n\
    .align 4                                                                    \t\n\
vec:                                                                            \t\n\
    IRQ0:                                                                       \t\n\
        j _exception_handling                                                   \t\n\
    IRQ1:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ2:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ3:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ4:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ5:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ6:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ7:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ8:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ9:                                                                       \t\n\
        j _int_entry                                                            \t\n\
    IRQ10:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ11:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ12:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ13:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ14:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ15:                                                                      \t\n\
        j _int_entry                                                            \t\n\
    IRQ16:                                                                      \t\n\
        j _int_entry                                                            \t\n\
                                                                                \t\n\
_reset:                                                                         \t\n\
                                                                                \t\n\
        # SATP should be zero, but let's make sure. Each hart has its own.      \t\n\
        csrw    satp, zero                                                      \t\n\
                                                                                \t\n\
        # Non-bootstrapping harts wait for an IPI                               \t\n\
        csrr    t0, mhartid                                                     \t\n\
        bnez    t0, secondary                                                   \t\n\
                                                                                \t\n\
        # Control registers, set the stack, mstatus, mepc,                      \t\n\
        # and mtvec to return to the main function.                             \t\n\
        # 16kB * hart ID is subtracted from the boot stack to avoid overlapping \t\n\
        la      sp, __boot_stack__                                              \t\n\
        li      t0, 0x1                                                         \t\n\
        slli    t0, t0, 15                                                      \t\n\
        csrr    a0, mhartid                                                     \t\n\
        mul     t0, t0, a0                                                      \t\n\
        sub     sp, sp, t0                                                      \t\n\
                                                                                \t\n\
        # Setting `mstatus` register:                                           \t\n\
        # 0b11 << 11: Machine's previous protection mode is 3 (MPP=3).          \t\n\
        #       0 = USER                                                        \t\n\
        #       1 = SUPERVISOR                                                  \t\n\
        #       3 = MACHINE                                                     \t\n\
        # 1 << 7    : Machine's previous interrupt-enable bit is 1 (MPIE=1).    \t\n\
        # 1 << 3    : Machine's interrupt-enable bit is 1 (MIE=1).              \t\n\
        li      t0, (0b11 << 11) | (1 << 7) | (1 << 3)                          \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # seting paging disabled                                                \t\n\
        csrw sptbr, zero                                                        \t\n\
                                                                                \t\n\
        # Machine's exception program counter (MEPC) is set to `_start`.        \t\n\
        la      t1, _start                                                      \t\n\
        csrw    mepc, t1                                                        \t\n\
                                                                                \t\n\
        # Machine's trap vector base address is set to vec.                     \t\n\
        la      t2, vec                                                         \t\n\
        ori     t2, t2, 0x1  # OR 0x1 to active vector mode                     \t\n\
        csrw    mtvec, t2                                                       \t\n\
                                                                                \t\n\
        # Setting Machine's interrupt-enable bits (`mie` register):             \t\n\
        # 1 << 3 : Machine's M-mode software interrupt-enable bit is 1 (MSIE=1) \t\n\
        # 1 << 7 : Machine's timer interrupt-enable bit is 1 (MTIE=1).          \t\n\
        # 1 << 11: Machine's external interrupt-enable bit is 1 (MEIE=1).       \t\n\
        li      t3, (1 << 3) | (1 << 7) | (1 << 11)                             \t\n\
        csrw    mie, t3                                                         \t\n\
                                                                                \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
secondary:                                                                      \t\n\
        # IMPLEMENT : prepare to be awaken                                      \t\n\
                                                                                \t\n\
wait:                                                                           \t\n\
        wfi                                                                     \t\n\
        j _start                                                                \t\n\
        ");
}
