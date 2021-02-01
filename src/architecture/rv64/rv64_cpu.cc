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
    /*
    * pc, x1-x31, mstatus
    */
    ASM("       sw      x31, -124(sp)           \n"
        "       la      x31, pc                 \n");

    ASM("       sw      x31, -4(sp)             \n"
        "       lw      x31, -124(sp)           \n"
        "       lw      x5, -8(sp)              \n"
        "       sw      x6, -12(sp)             \n"
        "       sw      x7, -16(sp)             \n"
        "       sw      x8, -20(sp)             \n"
        "       sw      x9, -24(sp)             \n"
        "       sw      x10, -28(sp)            \n"
        "       sw      x11, -32(sp)            \n"
        "       sw      x12, -36(sp)            \n"
        "       sw      x13, -40(sp)            \n"
        "       sw      x14, -44(sp)            \n"
        "       sw      x15, -48(sp)            \n"
        "       sw      x16, -52(sp)            \n"
        "       sw      x17, -56(sp)            \n"
        "       sw      x18, -60(sp)            \n"
        "       sw      x19, -64(sp)            \n"
        "       sw      x20, -68(sp)            \n"
        "       sw      x21, -72(sp)            \n"
        "       sw      x22, -76(sp)            \n"
        "       sw      x23, -80(sp)            \n"
        "       sw      x24, -84(sp)            \n"
        "       sw      x25, -88(sp)            \n"
        "       sw      x26, -92(sp)            \n"
        "       sw      x27, -96(sp)            \n"
        "       sw      x28, -100(sp)           \n"
        "       sw      x29, -104(sp)           \n"
        "       sw      x30, -108(sp)           \n"
        "       sw      x31, -112(sp)           \n"
        "       sw      x1, -116(sp)            \n");
    csrr31();

    ASM("       sw      x31, -120(sp)           \n");

    ASM("       lw      x31, -124(sp)           \n"
        "       addi    sp, sp, -120            \n"
        "       sw      sp, 0(%0)               \n" : : "r"(this));
}

void CPU::Context::load() const volatile
{
    System::_heap->free(reinterpret_cast<void *>(Memory_Map::SYS_STACK), Traits<System>::STACK_SIZE);
    ASM("       mv      sp, %0                  \n"
        "       addi    sp, sp, 120             \n"
        "       lw      x31, -120(sp)           \n" : : "r"(this));
    csrw31();
    ASM("       lw      x5, -8(sp)              \n"
        "       lw      x6, -12(sp)             \n"
        "       lw      x7, -16(sp)             \n"
        "       lw      x8, -20(sp)             \n"
        "       lw      x9, -24(sp)             \n"
        "       lw      x10, -28(sp)            \n"
        "       lw      x11, -32(sp)            \n"
        "       lw      x12, -36(sp)            \n"
        "       lw      x13, -40(sp)            \n"
        "       lw      x14, -44(sp)            \n"
        "       lw      x15, -48(sp)            \n"
        "       lw      x16, -52(sp)            \n"
        "       lw      x17, -56(sp)            \n"
        "       lw      x18, -60(sp)            \n"
        "       lw      x19, -64(sp)            \n"
        "       lw      x20, -68(sp)            \n"
        "       lw      x21, -72(sp)            \n"
        "       lw      x22, -76(sp)            \n"
        "       lw      x23, -80(sp)            \n"
        "       lw      x24, -84(sp)            \n"
        "       lw      x25, -88(sp)            \n"
        "       lw      x26, -92(sp)            \n"
        "       lw      x27, -96(sp)            \n"
        "       lw      x28, -100(sp)           \n"
        "       lw      x29, -104(sp)           \n"
        "       lw      x30, -108(sp)           \n"
        "       lw      x31, -112(sp)           \n"
        "       lw      x1, -116(sp)            \n"
        "       lw      x31, -4(sp)             \n"
        "       jalr    x0, (x31)               \n");
}

void CPU::switch_context(Context ** o, Context * n)
{
    ASM("c_switch:                              \n"
        "       j   c_switch                    \n");
    // ASM("       sub     sp, #4                  \n"     // reserve room for PC
    //     "       push    {r12}                   \n"     // save tmp register
    //     "       adr     r12, .ret               \n");   // calculate return address
    // ASM("       str     r12, [sp,#4]            \n"     // save calculate PC
    //     "       pop     {r12}                   \n"     // restore tmp register
    //     "       push    {r0-r12, lr}            \n");   // save all registers
    // mrs12();                                            // move flags to tmp register
    // ASM("       push    {r12}                   \n"     // save flags
    //     "       str     sp, [r0]                \n"     // update Context * volatile * o
    //     "       mov     sp, r1                  \n"     // get Context * volatile n into SP
    //     "       isb                             \n"     // serialize the pipeline so SP gets updated before the pop
    //     "       pop     {r12}                   \n");   // pop flags into tmp register
    // msr12();                                            // restore flags
    // ASM("       pop     {r0-r12, lr}            \n");   // restore all registers
    // ASM("       pop     {pc}                    \n"     // restore PC
    //     ".ret:  bx      lr                      \n");   // return
}

__END_SYS
