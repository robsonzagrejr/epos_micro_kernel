// EPOS ARM Cortex Common Memory Map

#ifndef __cortex_memory_map_h
#define __cortex_memory_map_h

#include <system/memory_map.h>

__BEGIN_SYS

struct Cortex_Memory_Map
{
    // Physical Memory
    enum {
        MEM_BASE        = Traits<Machine>::MEM_BASE,
        MEM_TOP         = Traits<Machine>::MEM_TOP
    };

    // Logical Address Space
    enum {
        APP_LOW         = Traits<Machine>::APP_LOW,
        APP_CODE        = Traits<Machine>::APP_CODE,
        APP_DATA        = Traits<Machine>::APP_DATA,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO_BASE,

        SYS             = Traits<Machine>::SYS,
        SYS_INFO        = unsigned(-1),                                 // Usually not used during boot. Dynamically built during initialization.
        SYS_CODE        = Traits<Machine>::SYS_CODE,
        SYS_DATA        = Traits<Machine>::SYS_DATA,
        SYS_HEAP        = SYS_DATA,                                     // Not used for Cortex-M because multiheap can only be enabled with an MMU.
        SYS_STACK       = MEM_TOP + 1 - Traits<Machine>::STACK_SIZE     // This stack is used before main(). The stack pointer is initialized at crt0.S
    };
};

__END_SYS

#endif
