// EPOS RISC-V 64 CPU Mediator Implementation

#include <architecture/rv64/rv64_cpu.h>
#include <system.h>

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile
{
if(sup)
    ASM("       csrr     x3,  sstatus           \n");
else
    ASM("       csrr     x3,  mstatus           \n");

    ASM("       sw       x3, -120(sp)           \n"     // push st
        "       sw       x1, -116(sp)           \n"     // push ra as pc
        "       sw       x1, -112(sp)           \n"     // push ra
        "       sw       x5, -108(sp)           \n"     // push x5-x31
        "       sw       x6, -104(sp)           \n"
        "       sw       x7, -100(sp)           \n"
        "       sw       x8,  -96(sp)           \n"
        "       sw       x9,  -92(sp)           \n"
        "       sw      x10,  -88(sp)           \n"
        "       sw      x11,  -84(sp)           \n"
        "       sw      x12,  -80(sp)           \n"
        "       sw      x13,  -76(sp)           \n"
        "       sw      x14,  -72(sp)           \n"
        "       sw      x15,  -68(sp)           \n"
        "       sw      x16,  -64(sp)           \n"
        "       sw      x17,  -60(sp)           \n"
        "       sw      x18,  -56(sp)           \n"
        "       sw      x19,  -52(sp)           \n"
        "       sw      x20,  -48(sp)           \n"
        "       sw      x21,  -44(sp)           \n"
        "       sw      x22,  -40(sp)           \n"
        "       sw      x23,  -36(sp)           \n"
        "       sw      x24,  -32(sp)           \n"
        "       sw      x25,  -28(sp)           \n"
        "       sw      x26,  -24(sp)           \n"
        "       sw      x27,  -20(sp)           \n"
        "       sw      x28,  -16(sp)           \n"
        "       sw      x29,  -12(sp)           \n"
        "       sw      x30,   -8(sp)           \n"
        "       sw      x31,   -4(sp)           \n"
        "       addi     sp,      sp,  -120     \n"                     // complete the pushes above by adjusting the SP
        "       sw       sp,    0(%0)           \n" : : "r"(this));     // update the this pointer to match the context saved on the stack
}

// Context load does not verify if interrupts were previously enabled by the Context's constructor
// We are setting mstatus to MPP | MPIE, therefore, interrupts will be enabled only after mret
void CPU::Context::load() const volatile
{
    ASM("       mv       sp,      %0            \n"     // load the stack pointer with the this pointer
        "       addi     sp,      sp,   120     \n"     // adjust the stack pointer to match the subsequent series of pops
        "       lw       x1, -112(sp)           \n"     // pop ra
        "       lw       x5, -108(sp)           \n"     // pop x5-x31
        "       lw       x6, -104(sp)           \n"
        "       lw       x7, -100(sp)           \n"
        "       lw       x8,  -96(sp)           \n"
        "       lw       x9,  -92(sp)           \n"
        "       lw      x10,  -88(sp)           \n"
        "       lw      x11,  -84(sp)           \n"
        "       lw      x12,  -80(sp)           \n"
        "       lw      x13,  -76(sp)           \n"
        "       lw      x14,  -72(sp)           \n"
        "       lw      x15,  -68(sp)           \n"
        "       lw      x16,  -64(sp)           \n"
        "       lw      x17,  -60(sp)           \n"
        "       lw      x18,  -56(sp)           \n"
        "       lw      x19,  -52(sp)           \n"
        "       lw      x20,  -48(sp)           \n"
        "       lw      x21,  -44(sp)           \n"
        "       lw      x22,  -40(sp)           \n"
        "       lw      x23,  -36(sp)           \n"
        "       lw      x24,  -32(sp)           \n"
        "       lw      x25,  -28(sp)           \n"
        "       lw      x26,  -24(sp)           \n"
        "       lw      x27,  -20(sp)           \n"
        "       lw      x28,  -16(sp)           \n"
        "       lw      x29,  -12(sp)           \n"
        "       lw      x30,   -8(sp)           \n"
        "       lw      x31,   -4(sp)           \n" : : "r"(this));
if(sup)
    ASM("       lw       x3, -120(sp)           \n"     // pop st
        "       csrs    sstatus,   x3           \n"     // set sstatus for sret
        "       lw       x3, -116(sp)           \n"     // pop pc
        "       csrw    sepc,      x3           \n"     // move pc to sepc for sret
        "       sret                            \n");
else
    ASM("       lw       x3, -120(sp)           \n"     // pop st
        "       csrs    mstatus,   x3           \n"     // set mstatus for mret
        "       lw       x3, -116(sp)           \n"     // pop pc
        "       csrw    mepc,      x3           \n"     // move pc to mepc for mret
        "       mret                            \n");
}

void CPU::switch_context(Context ** o, Context * n)
{   
    // Push the context into the stack and update "o"
    ASM("       sw       x1, -116(sp)           \n"     // push the return address as pc
        "       sw       x1, -112(sp)           \n"     // push ra
        "       sw       x5, -108(sp)           \n"     // push x5-x31
        "       sw       x6, -104(sp)           \n"
        "       sw       x7, -100(sp)           \n"
        "       sw       x8,  -96(sp)           \n"
        "       sw       x9,  -92(sp)           \n"
        "       sw      x10,  -88(sp)           \n"
        "       sw      x11,  -84(sp)           \n"
        "       sw      x12,  -80(sp)           \n"
        "       sw      x13,  -76(sp)           \n"
        "       sw      x14,  -72(sp)           \n"
        "       sw      x15,  -68(sp)           \n"
        "       sw      x16,  -64(sp)           \n"
        "       sw      x17,  -60(sp)           \n"
        "       sw      x18,  -56(sp)           \n"
        "       sw      x19,  -52(sp)           \n"
        "       sw      x20,  -48(sp)           \n"
        "       sw      x21,  -44(sp)           \n"
        "       sw      x22,  -40(sp)           \n"
        "       sw      x23,  -36(sp)           \n"
        "       sw      x24,  -32(sp)           \n"
        "       sw      x25,  -28(sp)           \n"
        "       sw      x26,  -24(sp)           \n"
        "       sw      x27,  -20(sp)           \n"
        "       sw      x28,  -16(sp)           \n"
        "       sw      x29,  -12(sp)           \n"
        "       sw      x30,   -8(sp)           \n"
        "       sw      x31,   -4(sp)           \n");
if(sup)
    ASM("       csrr    x31,  sstatus           \n");   // get sstatus
else
    ASM("       csrr    x31,  mstatus           \n");   // get mstatus

    ASM("       sw      x31, -120(sp)           \n"     // push st
        "       addi     sp,      sp,   -120    \n"     // complete the pushes above by adjusting the SP
        "       sw       sp,    0(a0)           \n");   // update Context * volatile * o

    // Set the stack pointer to "n" and pop the context from the stack
    ASM("       mv       sp,      a1            \n"     // get Context * volatile n into SP
        "       addi     sp,      sp,    120    \n"     // adjust stack pointer as part of the subsequent pops
        "       lw      x31, -116(sp)           \n");   // pop pc to a temporary
if(sup)
    ASM("       csrw   sepc,     x31            \n");
else
    ASM("       csrw   mepc,     x31            \n");

    ASM("       lw       x1, -112(sp)           \n"     // pop ra
        "       lw       x5, -108(sp)           \n"     // pop x5-x31
        "       lw       x6, -104(sp)           \n"
        "       lw       x7, -100(sp)           \n"
        "       lw       x8,  -96(sp)           \n"
        "       lw       x9,  -92(sp)           \n"
        "       lw      x10,  -88(sp)           \n"
        "       lw      x11,  -84(sp)           \n"
        "       lw      x12,  -80(sp)           \n"
        "       lw      x13,  -76(sp)           \n"
        "       lw      x14,  -72(sp)           \n"
        "       lw      x15,  -68(sp)           \n"
        "       lw      x16,  -64(sp)           \n"
        "       lw      x17,  -60(sp)           \n"
        "       lw      x18,  -56(sp)           \n"
        "       lw      x19,  -52(sp)           \n"
        "       lw      x20,  -48(sp)           \n"
        "       lw      x21,  -44(sp)           \n"
        "       lw      x22,  -40(sp)           \n"
        "       lw      x23,  -36(sp)           \n"
        "       lw      x24,  -32(sp)           \n"
        "       lw      x25,  -28(sp)           \n"
        "       lw      x26,  -24(sp)           \n"
        "       lw      x27,  -20(sp)           \n"
        "       lw      x28,  -16(sp)           \n"
        "       lw      x29,  -12(sp)           \n"
        "       lw      x31, -120(sp)           \n");   // pop st
if(sup)
    ASM("       li      x30,   1 << 8           \n"     // set sstatus.MPP = supervisor through x30
        "       or      x31, x31, x30           \n"     // machine mode on MPP is needed to avoid errors on mret (when dealing with machine mode)
        "       csrw    sstatus, x31            \n"
        "       lw      x30,   -8(sp)           \n"
        "       lw      x31,   -4(sp)           \n"
        "       sret                            \n");
else
    ASM("       li      x30,  3 << 11           \n"     // set sstatus.MPP = machine through x30
        "       or      x31, x31, x30           \n"     // machine mode on MPP is needed to avoid errors on mret
        "       csrw    mstatus,  x31           \n"
        "       lw      x30,   -8(sp)           \n"
        "       lw      x31,   -4(sp)           \n"
        "       mret                            \n");
}

__END_SYS

