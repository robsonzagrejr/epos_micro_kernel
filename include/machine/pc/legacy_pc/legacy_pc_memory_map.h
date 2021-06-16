// EPOS PC Memory Map

#ifndef __pc_memory_map_h
#define __pc_memory_map_h

#include <system/memory_map.h>

__BEGIN_SYS

struct Memory_Map
{
    enum {
        NOT_USED        = Traits<Machine>::NOT_USED,

        // Physical Memory
        MEM_BASE        = Traits<Machine>::MEM_BASE,
        MEM_TOP         = Traits<Machine>::MEM_TOP,
        MIO_BASE        = Traits<Machine>::MIO_BASE,
        MIO_TOP         = Traits<Machine>::MIO_TOP,
        BOOT_STACK      = Traits<Machine>::BOOT_STACK,

        // Logical Address Space
        BOOT            = Traits<Machine>::BOOT,
        IMAGE           = Traits<Machine>::IMAGE,
        SETUP           = Traits<Machine>::SETUP,
        INIT            = Traits<Machine>::INIT,

        APP_LOW         = Traits<Machine>::APP_LOW,
        APP_CODE        = APP_LOW,
        APP_DATA        = APP_LOW + 4 * 1024 * 1024,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO,
        APIC            = IO,
        IO_APIC         = APIC    +  4 * 1024,
        VGA             = IO_APIC +  4 * 1024,
        PCI             = VGA + 32 * 1024, // VGA text mode

        SYS             = Traits<Machine>::SYS,
        IDT             = SYS + 0x00000000,
        GDT             = SYS + 0x00001000,
        SYS_PT          = SYS + 0x00002000,
        SYS_PD          = SYS + 0x00003000,
        SYS_INFO        = SYS + 0x00004000,
        TSS0            = SYS + 0x00005000,
        SYS_CODE        = SYS + 0x00100000,
        SYS_DATA        = SYS + 0x00200000,
        SYS_STACK       = SYS + 0x00300000,
        SYS_HEAP        = SYS + 0x00400000
    };
};

__END_SYS

#endif
