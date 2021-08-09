// EPOS Cortex-A Interrupt Controller Initialization

#include <architecture/cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

extern "C" { void _int_entry();
             void _undefined_instruction();
             void _software_interrupt();
             void _prefetch_abort();
             void _data_abort();
             void _reserved();
             void _fiq(); 
}

// Class methods
void IC::init()
{
    db<Init, IC>(TRC) << "IC::init()" << endl;

    CPU::int_disable(); // will be reenabled at Thread::init() by Context::load()
    Engine::init();

    disable(); // will be enabled on demand as handlers are registered

    // Set all interrupt handlers to int_not()
    for(Interrupt_Id i = 0; i < INTS; i++)
        _int_vector[i] = int_not;

    // As we are using virtual memory for system code, we need to update Vector table handler addresses
    if (Traits<System>::multitask) { 
        CPU::Reg32 * handler_entry = reinterpret_cast<CPU::Reg32 *>(0x20);

        handler_entry[1] = reinterpret_cast<CPU::Reg32>(_undefined_instruction);
        handler_entry[2] = reinterpret_cast<CPU::Reg32>(_software_interrupt);
        handler_entry[3] = reinterpret_cast<CPU::Reg32>(_prefetch_abort);
        handler_entry[4] = reinterpret_cast<CPU::Reg32>(_data_abort);
        handler_entry[5] = reinterpret_cast<CPU::Reg32>(_int_entry);
        handler_entry[6] = reinterpret_cast<CPU::Reg32>(_fiq);
    }
}

__END_SYS
