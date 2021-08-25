// EPOS Initializer Starting Point

#include <system.h>
#include <machine.h>
#include <utility/string.h>

extern "C" char __bss_start;    // defined by GCC
extern "C" char _end;           // defined by GCC

__BEGIN_SYS

// This class purpose is simply to define a well-known starting point for the initialization of the system.
// It must be linked last so init_begin becomes the first constructor in the global's constructor list.
class Init_Begin
{
public:
    Init_Begin() {
        // Init is not linked with CRT0, so we must handle BSS here for kernels
	if(CPU::id() == 0)
            memset(reinterpret_cast<void *>(__bss_start), 0, _end - __bss_start);

        Machine::pre_init(System::info());
    }
};

Init_Begin init_begin;

__END_SYS
