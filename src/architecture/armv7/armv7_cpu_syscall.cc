// EPOS ARMv7 System Call Function Implementation

#include <architecture/armv7/armv7_cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

void CPU::syscall(void * message)
{
    CPU::r1(reinterpret_cast<CPU::Reg>(message));
    CPU::svc();
}

__END_SYS
