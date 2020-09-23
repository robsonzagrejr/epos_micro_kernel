// EPOS PC Mediator Initialization

#include <machine.h>

__BEGIN_SYS

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    if(Traits<IC>::enabled)
        IC::init();

    if(Traits<Timer>::enabled)
        Timer::init();

    if(Traits<PCI>::enabled)
        PCI::init();

    if(Traits<Scratchpad>::enabled)
        Scratchpad::init();

#ifdef __KEYBOARD_H
    if(Traits<Keyboard>::enabled)
        Keyboard::init();
#endif

#ifdef __FPGA_H
    if(Traits<FPGA>::enabled)
        FPGA::init();
#endif
}

__END_SYS
