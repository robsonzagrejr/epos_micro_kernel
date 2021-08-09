// EPOS ARMv7 MMU Mediator Initialization

#include <architecture/mmu.h>
#include <system.h>

extern "C" char __data_start;
extern "C" char _edata;
extern "C" char __bss_start;
extern "C" char _end;

__BEGIN_SYS

void ARMv7_MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << System::info() << hex << ", base=" << System::info()->pmm.free1_base << ", top=" << System::info()->pmm.free1_top << endl;
    if (Traits<System>::multitask)
        free(System::info()->pmm.free1_base, pages(System::info()->pmm.free1_top - System::info()->pmm.free1_base));
    else
        free(&_end, pages(Memory_Map::MEM_TOP + 1 - 1036 * sizeof(Page) - reinterpret_cast<unsigned int>(&_end))); // on a regular 1:1 MMU mapping with two levels translation MMU, 1036 pages are used to map the physical memory and device memory
}

__END_SYS

