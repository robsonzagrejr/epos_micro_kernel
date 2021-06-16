// EPOS ARM Cortex-A9 Mediator Implementation

#include <machine/machine.h>
#include <machine/display.h>

__BEGIN_SYS

volatile unsigned int Realview_PBX::_cores;

void Realview_PBX::reboot()
{
    db<Machine>(WRN) << "Machine::reboot()" << endl;
//TODO: reboot!
}

__END_SYS
