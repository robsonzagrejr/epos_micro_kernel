// EPOS ARM Cortex IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>
#include <machine/timer.h>
#include <process.h>

extern "C" { void _int_entry() __attribute__ ((naked, nothrow, alias("_ZN4EPOS1S2IC5entryEv"))); }
extern "C" { void _dispatch(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC8dispatchEj"))); }
extern "C" { void _eoi(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC3eoiEj"))); }
extern "C" { void _undefined_instruction() __attribute__ ((alias("_ZN4EPOS1S2IC21undefined_instructionEv"))); }
extern "C" { void _software_interrupt() __attribute__ ((alias("_ZN4EPOS1S2IC18software_interruptEv"))); }
extern "C" { void _prefetch_abort() __attribute__ ((alias("_ZN4EPOS1S2IC14prefetch_abortEv"))); }
extern "C" { void _data_abort() __attribute__ ((alias("_ZN4EPOS1S2IC10data_abortEv"))); }
extern "C" { void _reserved() __attribute__ ((alias("_ZN4EPOS1S2IC8reservedEv"))); }
extern "C" { void _fiq() __attribute__ ((alias("_ZN4EPOS1S2IC3fiqEv"))); }

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];
// Class attributes
IC::Interrupt_Handler IC::_eoi_vector[INTS] = {
    0,
    Timer::eoi, // System Timer C1
    0,
    0, // System Timer C3 (User_Timer)--> should we add the EOI function here?
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Timer::eoi, // ARM TIMER INT
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    // For the sake of the simplicity of the code, a single interrupt id is used to issue IPIs.
    // In this way, we only map a single handler, agnostic of MBOX number and Core ID.
    // On the other hand, handlers take Core ID into consideration when performing EOIs.
    // As a single MBOX handler is used to address all IPIs, we clear all MBOX of a Core in this handler.
    mailbox_eoi, // MB0_CPU0
    0,//mailbox_eoi, // MB1_CPU0
    0,//mailbox_eoi, // MB2_CPU0
    0,//mailbox_eoi, // MB3_CPU0
    0,//mailbox_eoi, // MB0_CPU1
    0,//mailbox_eoi, // MB1_CPU1
    0,//mailbox_eoi, // MB2_CPU1
    0,//mailbox_eoi, // MB3_CPU1
    0,//mailbox_eoi, // MB0_CPU2
    0,//mailbox_eoi, // MB1_CPU2
    0,//mailbox_eoi, // MB2_CPU2
    0,//mailbox_eoi, // MB3_CPU2
    0,//mailbox_eoi, // MB0_CPU3
    0,//mailbox_eoi, // MB1_CPU3
    0,//mailbox_eoi, // MB2_CPU3
    0,//mailbox_eoi, // MB3_CPU3
    Timer::eoi, // CORE0 LOCAL TIMER INT = 117
    0
};

// Class methods
void IC::entry()
{
    // We assume A[T]PCS ARM ABI, so any function using registers r4 until r11 will save those upon beginning and restore them when exiting. 
    // An interrupt can happen in the middle of one such function, but if the ISR drives the PC through other functions that use the same registers, they will save and restore them. We therefore don't need to save r4-r11 here.
    ASM("       .equ    MODE_IRQ, 0x12                                  \n"
        "       .equ    MODE_SVC, 0x13                                  \n"
        "       .equ    IRQ_BIT,  0x80                                  \n"
        "       .equ    FIQ_BIT,  0x40                                  \n"
        "       msr     cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT           \n"     // go to SVC mode
        "       stmfd   sp!, {r0-r3, r12, lr, pc}                       \n"     // save current context (lr, sp and spsr are banked registers)
        "       msr     cpsr_c, #MODE_IRQ | IRQ_BIT | FIQ_BIT           \n"     // go to IRQ mode to recover LR
        "       sub     r0, lr, #4                                      \n"     // r0 = return address as entering IRQ more
        "       mrs     r1, spsr                                        \n"     // r1 = IRQ_spsr (to be visible at SVC)
        "       msr     cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT           \n"     // go back to SVC mode
        "       str     r0, [sp, #24]                                   \n"     // overwrite saved PC with r0 (saved return address)
        "       push    {r1}                                            \n"     // push IRQ_spsr
        "       bl      _dispatch                                       \n"
        "       pop     {r1}                                            \n"     // pop IRQ_spsr into SVC_spsr
        "       msr     spsr_cfxs, r1                                   \n"
        "       ldmfd   sp!, {r0-r3, r12, lr, pc}^                      \n");   // restore the context (including PC in ldmfd cause a mode change to the mode before the interrupt)
}

void IC::dispatch(unsigned int i)
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    assert(id < INTS);
    if(_eoi_vector[id])
        _eoi_vector[id](id);

    CPU::int_enable();

    _int_vector[id](id);
}

void IC::eoi(unsigned int id)
{
    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::eoi(i=" << id << ")" << endl;

    assert(id < INTS);
    if(_eoi_vector[id])
        _eoi_vector[id](id);
}

void IC::int_not(Interrupt_Id i)
{
    db<IC>(WRN) << "IC::int_not(i=" << i << ")" << endl;
}

void IC::hard_fault(Interrupt_Id i)
{
    db<IC>(ERR) << "IC::hard_fault(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::undefined_instruction()
{
    db<IC>(ERR) << "Undefined instruction" << endl;
    Machine::panic();
}

void IC::software_interrupt()
{
    db<IC>(ERR) << "Software interrupt" << endl;
    Machine::panic();
}

void IC::prefetch_abort()
{
    db<IC>(ERR) << "Prefetch abort" << endl;
    Machine::panic();
}

void IC::data_abort()
{
    db<IC>(ERR) << "Data abort" << endl;
    Machine::panic();
}

void IC::reserved()
{
    db<IC>(ERR) << "Reserved" << endl;
    Machine::panic();
}

void IC::fiq()
{
    db<IC>(ERR) << "FIQ handler" << endl;
    Machine::panic();
}

__END_SYS
