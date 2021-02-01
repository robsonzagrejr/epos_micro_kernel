// EPOS RISC-V Memory Map

#ifndef __riscv_memory_map_h
#define __riscv_memory_map_h


#include <system/memory_map.h>


__BEGIN_SYS

struct Memory_Map
{
    // Physical Memory
    enum {
        TEST_BASE                   = 0x00100000, // SiFive test engine
        RTC_BASE                    = 0x00101000, // goldfish_rtc
        UART_BASE                   = 0x10000000, // 16550A NS UART
        CLINT_BASE                  = 0x02000000, // Sifive CLINT
        TIMER_BASE                  = 0x02004000, // CLINT timer
        PLIIC_CPU_BASE              = 0x0c000000  // SiFive PLIC
    };

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

        SYS             = Traits<Machine>::NOT_USED,
        SYS_INFO        = unsigned(-1),                 // Dynamically built during initialization.
        SYS_CODE        = Traits<Machine>::NOT_USED,
        SYS_DATA        = Traits<Machine>::NOT_USED,
        SYS_HEAP        = Traits<Machine>::NOT_USED,
        SYS_STACK       = Traits<Machine>::NOT_USED
    };

    // Logical Address Space
};

__END_SYS

#endif
