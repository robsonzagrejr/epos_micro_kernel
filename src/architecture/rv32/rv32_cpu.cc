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
    ASM("c_save:                                \n"
        "       j   c_save                      \n");
    // implement
}

void CPU::Context::load() const volatile
{
    ASM("c_load:                                \n"
        "       j   c_load                      \n");
    // implement
}

void CPU::switch_context(Context ** o, Context * n)
{
    ASM("c_switch:                              \n"
        "       j   c_switch                    \n");
    // implement
}

__END_SYS
