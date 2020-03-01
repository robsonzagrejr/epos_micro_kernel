// EPOS ARMv7 MMU Mediator Initialization

#include <architecture/mmu.h>

extern "C" void * __data_start;
extern "C" void * _edata;
extern "C" void * __bss_start;
extern "C" void * _end;

__BEGIN_SYS

void MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;

    db<Init, MMU>(INF) << "MMU::init::dat.b=" << &__data_start << ",dat.e=" << &_edata << ",bss.b=" << &__bss_start << ",bss.e=" << &_end << endl;

    // For machines that do not feature a real MMU, frame size = 1 byte
    // TODO: The stack left at the top of the memory for INIT is freed at Thread::init()
    free(&_end, pages(Memory_Map::SYS_STACK - reinterpret_cast<unsigned int>(&_end)));
}

__END_SYS

