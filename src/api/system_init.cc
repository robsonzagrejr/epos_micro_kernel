// EPOS System Initialization

#include <system.h>
#include <time.h>
#include <process.h>

__BEGIN_SYS

void System::init()
{
    // These abstractions are initialized only once by the bootstrap CPU
    if(CPU::id() == 0) {
        if(Traits<Alarm>::enabled)
            Alarm::init();
    }

    // These abstractions are initialized by all CPUs
    if(Traits<Thread>::enabled)
        Thread::init();
}

__END_SYS
