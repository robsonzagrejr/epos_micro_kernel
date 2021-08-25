// EPOS RISC-V 32 System Call Function Implementation

#include <architecture/rv32/rv32_cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

void CPU::syscall(void * message)
{
    CPU::a1(reinterpret_cast<CPU::Reg>(message));
    CPU::ecall();
}

__END_SYS
