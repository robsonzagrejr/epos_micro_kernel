// EPOS ARMv7 MMU Mediator Initialization

#include <architecture/mmu.h>

extern "C" void * __data_start;
extern "C" void * _edata;
extern "C" void * __bss_start;
extern "C" void * _end;

__BEGIN_SYS

void ARMv7_MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;
}

__END_SYS

