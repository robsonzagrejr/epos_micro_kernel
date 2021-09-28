// EPOS ARMV7 CPU System Call Entry Implementation

#include <architecture/armv7/armv7_cpu.h>

extern "C" { void _sysexec(); }

__BEGIN_SYS

void CPU::syscalled()
{
    ASM("push {lr}  \n"
        "push {r0}  \n"
        "bl _sysexec   \n"
        "pop {r0}  \n"
        "pop {lr}  \n"
    );
}

__END_SYS
