// EPOS IA32 CPU System Call Function Implementation

#include <architecture/ia32/ia32_cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

void CPU::syscall(void * msg)
{
    ASM("int %0" : : "i"(IC::INT_SYSCALL), "c"(msg));
}

__END_SYS
