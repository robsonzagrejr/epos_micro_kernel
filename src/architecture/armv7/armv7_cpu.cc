// EPOS ARMv7 CPU Mediator Implementation

#include <architecture/armv7/armv7_cpu.h>

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile
{
    // we are not making pushes and pops since we don't want to write things on stack!
    // Instead, we are using the address of the context object that called the function.
    ASM("       str     r12, [sp,#-68]          \n"
        "       mov     r12, pc                 \n");
if(thumb)
    ASM("       orr r12, #1                     \n");

    ASM("       str     r12, [%0, #-64]         \n"
        "       ldr     r12, [sp,#-68]          \n"
        "       str     lr, [%0, #-60]          \n"
        "       str     r12, [%0, #-56]         \n"
        "       str     r11, [%0, #-52]         \n"
        "       str     r10, [%0, #-48]         \n"
        "       str     r9, [%0, #-44]          \n"
        "       str     r8, [%0, #-40]          \n"
        "       str     r7, [%0, #-36]          \n"
        "       str     r6, [%0, #-32]          \n"
        "       str     r5, [%0, #-28]          \n"
        "       str     r4, [%0, #-24]          \n"
        "       str     r3, [%0, #-20]          \n"
        "       str     r2, [%0, #-16]          \n"
        "       str     r1, [%0, #-12]          \n"
        "       str     r0, [%0, #-8]           \n" : : "r"(this));
    mrs12();
    ASM("       str     r12, [%0, #-4]          \n"
        "       ldr     r12, [sp, #-68]         \n"
        "       add     %0, %0, #-68            \n"
        "       bx      lr                      \n" : : "r"(this)); // naked functions do not provide neither prologue nor epilogue, thus we need to force return
}

void CPU::Context::load() const volatile
{
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
    // Push the context into the stack and adjust "o" to match
    ASM("       sub     sp, #4                  \n"     // reserve room for PC
        "       push    {r12}                   \n"     // save r12 to use it as a temporary register
        "       adr     r12, .ret               \n");   // calculate return address

if(thumb)
    ASM("       orr r12, #1                     \n");   // adjust thumb

    ASM("       str     r12, [sp,#4]            \n"     // save calculated PC
        "       pop     {r12}                   \n"     // restore r12 used as temporary
        "       push    {r0-r12, lr}            \n");   // push all registers (LR first, r0 last)

if(Traits<FPU>::enabled && !Traits<FPU>::user_save)
    ASM("       vpush   {s0-s15}                \n"     // save FPU registers
        "       vpush   {s16-s31}               \n");

    mrs12();                                            // move flags to tmp register
    ASM("       push    {r12}                   \n");   // save flags
    ASM("       str     sp, [r0]                \n");   // update Context * volatile * o


    // Set the stack pointer to "n" and pop the context
    ASM("       mov     sp, r1                  \n"     // get Context * volatile n into SP
        "       isb                             \n");   // serialize the pipeline so SP gets updated before the pop

    ASM("       pop     {r12}                   \n");   // pop flags into the temporary register r12
    msr12();                                            // restore flags

if(Traits<FPU>::enabled && !Traits<FPU>::user_save)
    ASM("       vpop   {s16-s31}                \n"     // restore FPU registers
        "       vpop   {s0-s15}                 \n");

    ASM("       pop     {r0-r12, lr}            \n");   // pop all registers (r0 first, LR last)

if((Traits<Build>::MODEL == Traits<Build>::eMote3) || (Traits<Build>::MODEL == Traits<Build>::LM3S811))
    int_enable();

    ASM("       pop     {pc}                    \n"     // restore PC
        ".ret:  bx      lr                      \n");   // return
}

__END_SYS
