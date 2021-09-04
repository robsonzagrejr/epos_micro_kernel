// EPOS Cortex-A Interrupt Controller Initialization

#include <architecture/cpu.h>
#include <machine/ic.h>
#include <system/memory_map.h>

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

    CPU::FSR * vt = reinterpret_cast<CPU::FSR *>(Memory_Map::VECTOR_TABLE + 32);
    // vt[0] = _reset is defined by SETUP
    vt[1] = _undefined_instruction;
    vt[2] = _software_interrupt;
    vt[3] = _prefetch_abort;
    vt[4] = _data_abort;
    vt[5] = _int_entry;
    vt[6] = _fiq;

    // Set all interrupt handlers to int_not()
    for(Interrupt_Id i = 0; i < INTS; i++)
        _int_vector[i] = int_not;
}

__END_SYS
