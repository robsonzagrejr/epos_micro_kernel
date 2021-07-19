// EPOS ARMv8 MMU Mediator Initialization

#include <architecture/mmu.h>

extern "C" char __data_start;
extern "C" char _edata;
extern "C" char __bss_start;
extern "C" char _end;

__BEGIN_SYS

void ARMv8_MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;
}

__END_SYS

