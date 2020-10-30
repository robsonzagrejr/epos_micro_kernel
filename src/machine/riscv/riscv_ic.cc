// EPOS RISC-V IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }
extern "C" { void _dispatch(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC8dispatchEv"))); }
extern "C" { void _undefined_instruction() __attribute__ ((alias("_ZN4EPOS1S2IC21undefined_instructionEj"))); }
extern "C" { void _software_interrupt() __attribute__ ((alias("_ZN4EPOS1S2IC18software_interruptEj"))); }
extern "C" { void _prefetch_abort() __attribute__ ((alias("_ZN4EPOS1S2IC14prefetch_abortEj"))); }
extern "C" { void _data_abort() __attribute__ ((alias("_ZN4EPOS1S2IC10data_abortEj"))); }
extern "C" { void _reserved() __attribute__ ((alias("_ZN4EPOS1S2IC8reservedEj"))); }
extern "C" { void _fiq() __attribute__ ((alias("_ZN4EPOS1S2IC3fiqEj"))); }
extern "C" { void _exception_handling() __attribute__ ((alias("_ZN4EPOS1S2IC18exception_handlingEv"))); }

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];

// Class methods
void IC::entry()
{
    // Handle interrupts in machine mode
    ASM("j _int_entry"); // IMPLEMENT
}

void IC::dispatch()
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    _int_vector[id](id);
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

void IC::undefined_instruction(Interrupt_Id i)
{
    db<IC>(ERR) << "Undefined instruction(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::software_interrupt(Interrupt_Id i)
{
    db<IC>(ERR) << "Software interrupt(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::prefetch_abort(Interrupt_Id i)
{
    db<IC>(ERR) << "Prefetch abort(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::data_abort(Interrupt_Id i)
{
    db<IC>(ERR) << "Data abort(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::reserved(Interrupt_Id i)
{
    db<IC>(ERR) << "Reserved(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::fiq(Interrupt_Id i)
{
    db<IC>(ERR) << "FIQ handler(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::exception_handling()
{
    db<IC>(ERR) << "Exception abort" << endl;
    // IMPLEMENT
    Machine::panic();
}
__END_SYS
