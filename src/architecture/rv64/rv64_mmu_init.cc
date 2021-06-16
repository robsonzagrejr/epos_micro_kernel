// EPOS RISC-V 64 MMU Mediator Initialization

#include <architecture/mmu.h>
#include <system.h>

__BEGIN_SYS

void RV32S_MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;

    free(System::info()->pmm.free1_base, pages(System::info()->pmm.free1_top - System::info()->pmm.free1_base));
}

__END_SYS

