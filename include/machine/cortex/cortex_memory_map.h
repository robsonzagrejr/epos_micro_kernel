// EPOS ARM Cortex Common Memory Map

#ifndef __cortex_memory_map_h
#define __cortex_memory_map_h

#include <system/memory_map.h>

__BEGIN_SYS

struct Cortex_Memory_Map
{
    static const unsigned int NOT_USED = Traits<Machine>::NOT_USED;

    enum {
        // Physical Memory
        MIO_BASE        = Traits<Machine>::MIO_BASE,
        MIO_TOP         = Traits<Machine>::MIO_TOP,
        MEM_BASE        = Traits<Machine>::MEM_BASE,
        MEM_TOP         = Traits<Machine>::MEM_TOP,
        BOOT_STACK      = Traits<Machine>::BOOT_STACK,

        // Logical Address Space
        BOOT            = Traits<Machine>::BOOT,
        IMAGE           = Traits<Machine>::IMAGE,
        SETUP           = Traits<Machine>::SETUP,
        INIT            = Traits<Machine>::INIT,

        APP_LOW         = Traits<Machine>::APP_LOW,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO,
        SYS             = Traits<Machine>::SYS
    };
};

__END_SYS

#endif
