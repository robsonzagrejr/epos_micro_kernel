// EPOS IA32 CPU System Call Entry Implementation

#include <architecture/ia32/ia32_cpu.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

void CPU::syscalled()
{
    // We get here when an APP triggers INT_SYSCALL with the message address in CX
    // The CPU saves the user-level stack pointer in the stack and restores the system-level stack pointer also from the TSS
    // Stack contents at this point are always: ss, esp, eflags, cs, eip
    // CX holds the pointer to the message

    if(Traits<Build>::MODE == Traits<Build>::KERNEL) {
        // Do the system call by calling _exec with the message pointed by ecx
        ASM("        push    %ecx                # msg                       \n"
            "        call    _exec                                           \n"
            "        pop     %ecx                # clean up                  \n");

        // Return to user-level
        ASM("        iret                                                    \n");
    }
}

__END_SYS
