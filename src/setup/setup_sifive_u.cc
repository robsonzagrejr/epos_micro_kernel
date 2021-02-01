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
        csrr    t0, mcause                                                      \t\n\
        bnez    t0, _exception_handling                                         \t\n\
                                                                                \t\n\
        # SATP should be zero, but let's make sure. Each HART has its own       \t\n\
        # SATP register.                                                        \t\n\
        csrw    satp, zero                                                      \t\n\
                                                                                \t\n\
        # Any hardware threads (hart) that are not bootstrapping                \t\n\
        # need to wait for an IPI                                               \t\n\
        csrr    t0, mhartid                                                     \t\n\
        bnez    t0, 3f                                                          \t\n\
                                                                                \t\n\
        # Disable linker instruction relaxation for the `la` instruction below. \t\n\
        # This disallows the assembler from assuming that `gp` is already initialized \t\n\
        # This causes the value stored in `gp` to be calculated from `pc`.      \t\n\
        # The job of the global pointer is to give the linker the ability to address \t\n\
        # memory relative to GP instead of as an absolute address.              \t\n\
.option push                                                                    \t\n\
.option norelax                                                                 \t\n\
    la      gp, end                                                             \t\n\
.option pop                                                                     \t\n\
                                                                                \t\n\
        # Set all bytes in the BSS section to zero.                             \t\n\
        #la      a0, __bss_start__                                              \t\n\
        #la      a1, __bss_end__                                                \t\n\
        #bgeu    a0, a1, 2f                                                     \t\n\
1:                                                                              \t\n\
        #sw      zero, (a0)                                                     \t\n\
        #addi    a0, a0, 8                                                      \t\n\
        #bltu    a0, a1, 1b                                                     \t\n\
        #li t0, 0x10000000                                                      \t\n\
        #li t1, 0x3                                                             \t\n\
        #sw t1, 3(t0)                                                           \t\n\
        #li t2, 0x1                                                             \t\n\
        #sw t2, 2(t0)                                                           \t\n\
        #sw t2, 1(t0)                                                           \t\n\
        #li t3, 0xd                                                             \t\n\
        #slli t2, t2, 0x7                                                       \t\n\
        #or t2, t1, t2                                                          \t\n\
        #sw t2, 3(t0)                                                           \t\n\
        #sw t3, 0(t0)                                                           \t\n\
        #li t3, 0x0                                                             \t\n\
        #sw t3, 1(t0)                                                           \t\n\
        #sw t1, 3(t0)                                                           \t\n\
        #lui t0, 0x10000                                                        \t\n\
        #li t1, 0x0                                                             \t\n\
        #addi t1, t1, 72                                                        \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 101                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 108                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 108                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 111                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
                                                                                \t\n\
        #write ,                                                                \t\n\
        #li t1, 44                                                              \t\n\
        #sw t1, 0(t0)                                                           \t\n\
                                                                                \t\n\
        # write ' '                                                             \t\n\
        #li t1, 32                                                              \t\n\
        #sw t1, 0(t0)                                                           \t\n\
                                                                                \t\n\
        #write World                                                            \t\n\
        #li t1, 87                                                              \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 111                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 114                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 108                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
        #li t1, 100                                                             \t\n\
        #sw t1, 0(t0)                                                           \t\n\
                                                                                \t\n\
        #write !                                                                \t\n\
        #li t1, 33                                                              \t\n\
        #sw t1, 0(t0)                                                           \t\n\
                                                                                \t\n\
        #write EOL                                                              \t\n\
        #li t1, 10                                                              \t\n\
        #sw t1, 0(t0)                                                           \t\n\
2:                                                                              \t\n\
        # Control registers, set the stack, mstatus, mepc,                      \t\n\
        # and mtvec to return to the main function.                             \t\n\
        # li        t5, 0xffff;                                                 \t\n\
        # csrw  medeleg, t5                                                     \t\n\
        # csrw  mideleg, t5                                                     \t\n\
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
        # Machine's trap vector base address is set to `asm_trap_vector`.       \t\n\
        #la      t2, _vector_table                                              \t\n\
        la      t2, vec                                                         \t\n\
        ori     t2, t2, 0x1                                                     \t\n\
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
3:                                                                              \t\n\
                                                                                \t\n\
        la      sp, __boot_stack__                                              \t\n\
        li      t0, 0x1                                                         \t\n\
        slli    t0, t0, 15                                                      \t\n\
        csrr    a0, mhartid                                                     \t\n\
        mul     t0, t0, a0                                                      \t\n\
        sub     sp, sp, t0                                                      \t\n\
                                                                                \t\n\
        # The parked harts will be put into machine mode with interrupts enabled \t\n\
        li      t0, 0b11 << 11 | (1 << 7) | (1 << 13)                           \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # Allow for MSIP (Software interrupt). We will write the MSIP from hart #0 to \t\n\
        # awaken these parked harts.                                            \t\n\
        li      t3, (1 << 3)                                                    \t\n\
        csrw    mie, t3                                                         \t\n\
                                                                                \t\n\
        # Machine's exception program counter (MEPC) is set to the initialization \t\n\
        # code and waiting loop.                                                \t\n\
        la  t1, wait                                                            \t\n\
        csrw mepc, t1                                                           \t\n\
                                                                                \t\n\
        # Machine's trap vector base address is set to `m_trap_vector`, for     \t\n\
        # 'machine' trap vector. The Rust initialization routines will give each \t\n\
        # hart its own trap frame. We can use the same trap function and distinguish \t\n\
        # between each hart by looking at the trap frame.                       \t\n\
        # la      t2, asm_trap_vector                                           \t\n\
        #la      t2, _vector_table                                              \t\n\
        la      t2, vec                                                         \t\n\
        ori     t2, t2, 0x1                                                     \t\n\
        csrw    mtvec, t2                                                       \t\n\
                                                                                \t\n\
        # Whenever our hart is done initializing, we want it to return to the waiting \t\n\
        # loop, which is just below mret.                                       \t\n\
        # We use mret here so that the mstatus register is properly updated.    \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
wait:                                                                           \t\n\
                                                                                \t\n\
        wfi                                                                     \t\n\
        j _start                                                                \t\n\
        ");
}