// EPOS ARMv7 CPU Mediator Implementation

#include <architecture/armv7/armv7_cpu.h>
#include <system.h>

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile
{
    ASM("       str     r12, [sp,#-68]          \n"
        "       mov     r12, pc                 \n");
    if(thumb)
        ASM("       orr r12, #1                     \n");

    ASM("       push    {r12}                   \n"
        "       ldr     r12, [sp,#-64]          \n"
        "       push    {r0-r12, lr}            \n");
    mrs12();
    ASM("       push    {r12}                   \n"
        "       sub     sp, #4                  \n"
        "       pop     {r12}                   \n"
        "       str     sp, [%0]                \n" : : "r"(this));
}

void CPU::Context::load() const volatile
{
    System::_heap->free(reinterpret_cast<void *>(Memory_Map::SYS_STACK), Traits<System>::STACK_SIZE);
    ASM("       mov     sp, %0                  \n"
        "       isb                             \n" // serialize the pipeline so that SP gets updated before the pop
        "       pop     {r12}                   \n" : : "r"(this));
    msr12();
    ASM("       pop     {r0-r12, lr}            \n"
        "       pop     {pc}                    \n");
}

// This function assumes A[T]PCS (i.e. "o" is in r0/a0 and "n" is in r1/a1)
void CPU::switch_context(Context ** o, Context * n)
{
    ASM("       sub     sp, #4                  \n"     // reserve room for PC
        "       push    {r12}                   \n"     // save tmp register
        "       adr     r12, .ret               \n");   // calculate return address
if(thumb)
    ASM("       orr r12, #1                     \n");   // adjust thumb
    ASM("       str     r12, [sp,#4]            \n"     // save calculate PC
        "       pop     {r12}                   \n"     // restore tmp register
        "       push    {r0-r12, lr}            \n");   // save all registers
    mrs12();                                            // move flags to tmp register
    ASM("       push    {r12}                   \n"     // save flags
        "       str     sp, [r0]                \n"     // update Context * volatile * o
        "       mov     sp, r1                  \n"     // get Context * volatile n into SP
        "       isb                             \n"     // serialize the pipeline so SP gets updated before the pop
        "       pop     {r12}                   \n");   // pop flags into tmp register
    msr12();                                            // restore flags
    ASM("       pop     {r0-r12, lr}            \n");   // restore all registers
    if((Traits<Build>::MODEL == Traits<Build>::eMote3) || (Traits<Build>::MODEL == Traits<Build>::LM3S811))
        int_enable();
    ASM("       pop     {pc}                    \n"     // restore PC
        ".ret:  bx      lr                      \n");   // return
}

__END_SYS
