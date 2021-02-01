// EPOS RISC-V 64 Time-Stamp Counter Mediator Initialization


#include <architecture/tsc.h>
#include <machine/timer.h>

__BEGIN_SYS

void TSC::init()
{
    db<Init, TSC>(TRC) << "TSC::init()" << endl;


    if(CPU::id() == 0) {
        // Set timer to 0
        // reg(MTIME) = 0;
        // reg(MTIMEH) = 0;
    }
}

__END_SYS
