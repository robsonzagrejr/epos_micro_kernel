// EPOS RISC-V sifive SETUP

#include <system/config.h>

extern "C" { void _setup() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _setup()
{
    ASM("\t\n\
        j       .reset                                                          \t\n\
                                                                                \t\n\
.reset:                                                                         \t\n\
        # Disable interrupts                                                    \t\n\
        csrs    mstatus, 1 << 3                                                 \t\n\
                                                                                \t\n\
        # Disable paging                                                        \t\n\
        csrw    sptbr, zero                                                     \t\n\
                                                                                \t\n\
        # Put CLINT in direct mode (mtvec.mode = 0) and set mtvec to _int_entry \t\n\
        la      t0, _int_entry                                                  \t\n\
        andi    t0, t0, 0xfffffffe  # mtvec.mode = 0                            \t\n\
        csrw    mtvec, t0                                                       \t\n\
                                                                                \t\n\
        # Get the hart's id                                                     \t\n\
        csrr    a0, mhartid                                                     \t\n\
                                                                                \t\n\
        # Set a 16KB stack for each hart (#0 at __boot_stack__)                 \t\n\
        la      sp, __boot_stack__                                              \t\n\
        li      t0, 1                                                           \t\n\
        slli    t0, t0, 14                                                      \t\n\
        mul     t0, t0, a0                                                      \t\n\
        sub     sp, sp, t0                                                      \t\n\
                                                                                \t\n\
        # Non-bootstrapping harts wait for an IPI                               \t\n\
        bnez    a0, .secondary                                                  \t\n\
                                                                                \t\n\
        # Set mstatus to machine mode with interrupts disabled                  \t\n\
        # 0b11 << 11: Machine's previous protection mode is 3 (MPP=3)           \t\n\
        #    1 <<  7: Machine's previous interrupt-enable bit is 1 (MPIE=1)     \t\n\
        li      t0, (0b11 << 11) | (1 << 7)                                     \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # Set mepc to `_start` (will be used by mret)                           \t\n\
        la      t0, _start                                                      \t\n\
        csrw    mepc, t0                                                        \t\n\
                                                                                \t\n\
        # Go to _start and update mstatus accordingly by returning to mepc      \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
.secondary:                                                                     \t\n\
        # Set mstatus to machine mode with interrupts enabled                   \t\n\
        # 0b11 << 11: Machine's previous protection mode is 3 (MPP=3)           \t\n\
        #    1 <<  7: Machine's previous interrupt-enable bit is 1 (MPIE=1)     \t\n\
        #    1 <<  3: Machine's interrupt-enable bit is 1 (MIE=1)               \t\n\
        li      t0, (0b11 << 11) | (1 << 7) | (1 << 3)                          \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # Enable software interrupts so hart #0 can latter wake up this hart    \t\n\
        li      t0, (1 << 3) | (1 << 7) | (1 << 11)                             \t\n\
        csrw    mie, t0                                                         \t\n\
                                                                                \t\n\
        # Set mepc to `_wait` (will be used by mret)                            \t\n\
        la  t0, .wait                                                           \t\n\
        csrw mepc, t0                                                           \t\n\
                                                                                \t\n\
        # Go to _wait and update mstatus accordingly by returning to mepc       \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
.wait:                                                                          \t\n\
        wfi                                                                     \t\n\
        j _start                                                                \t\n\
        ");
}
