// EPOS Zynq (ARM Cortex-A9) IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }
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


// Class methods
void IC::entry()
{
    ASM(".equ MODE_IRQ, 0x12                        \n"
        ".equ MODE_SVC, 0x13                        \n"
        ".equ IRQ_BIT,  0x80                        \n"
        ".equ FIQ_BIT,  0x40                        \n"
        // Go to SVC
        "msr cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT  \n"
        // Save current context (lr, sp and spsr are banked registers)
        "stmfd sp!, {r0-r3, r12, lr, pc}            \n"
        // Go to IRQ
        "msr cpsr_c, #MODE_IRQ | IRQ_BIT | FIQ_BIT  \n"
        // Return from IRQ address
        "sub r0, lr, #4                             \n"
        // Pass irq_spsr to SVC r1
        "mrs r1, spsr                               \n"
        // Go back to SVC
        "msr cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT  \n"
        // sp+24 is the position of the saved pc
        "add r2, sp, #24                            \n"
        // Save address to return from interrupt into the pc position to retore
        // context later on
        "str r0, [r2]                               \n"
        // Save IRQ-spsr
        "stmfd sp!, {r1}                            \n"
        //"bl %0                                      \n"
        "bl _dispatch                               \n"
        "ldmfd sp!, {r0}                            \n"
        // Restore IRQ's spsr value to SVC's spsr
        "msr spsr_cfxs, r0                          \n"
        // Restore context, the ^ in the end of the above instruction makes the
        // irq_spsr to be restored into svc_cpsr
        "ldmfd sp!, {r0-r3, r12, lr, pc}^           \n" : : "i"(dispatch));
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
