// EPOS RISC-V 32 CPU Mediator Implementation

#include <architecture/rv32/rv32_cpu.h>
#include <system.h>

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile
{
    ASM("       la       x4,      pc            \n"
        "       sw       x4, -116(sp)           \n"     // push pc
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

    ASM("       addi     sp, sp, -116           \n"                     // complete the pushes above by adjusting the SP
        "       sw       sp, 0(%0)              \n" : : "r"(this));     // update the this pointer to match the context saved on the stack
}

void CPU::Context::load() const volatile
{
    ASM("       mv      sp, %0                  \n"                     // load the stack pointer with the this pointer
        "       addi    sp, sp, 116             \n" : : "r"(this));     // adjust the stack pointer to match the subsequent series of pops

    ASM("       lw       x4, -116(sp)           \n"     // pop pc
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
        "       lw      x31,   -4(sp)           \n"
        "       jalr     x0,     (x4)           \n");   // jump to pc stored in x4 (jalr with x0 is equivalent to jr)
}

void CPU::switch_context(Context ** o, Context * n)
{   
    // Push the context into the stack and update "o"
    ASM("       la       x4,    .ret            \n"     // get the return address in a temporary
        "       sw       x4, -116(sp)           \n"     // push the return address as pc
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
        "       addi     sp,      sp,   -116    \n"     // complete the pushes above by adjusting the SP
        "       sw       sp,    0(a0)           \n");   // update Context * volatile * o

    // Set the stack pointer to "n" and pop the context from the stack
    ASM("       mv       sp,      a1            \n"     // get Context * volatile n into SP
        "       addi     sp,      sp,    116    \n"     // adjust stack pointer as part of the subsequent pops
        "       lw       x4, -116(sp)           \n"     // pop pc to a temporary
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
        "       lw      x31,   -4(sp)           \n"
        "       jalr     x0,     (x4)           \n"     // return (for the thread entering the CPU)
        ".ret:  jalr     x0,     (x1)           \n");   // return (for the thread leaving the CPU)
}

__END_SYS
