// EPOS Realview PBX (ARM Cortex-A9) Memory Map

#ifndef __model_memory_map_h
#define __model_memory_map_h

#include <machine/cortex/cortex_memory_map.h>

__BEGIN_SYS

struct Memory_Map: public Cortex_Memory_Map
{
    enum {
        // Base addresses for memory-mapped control and I/O devices
        I2C_BASE                = 0x10002000, // Versatile I2C
        AAC_BASE                = 0x10004000, // PrimeCell PL041 Advanced Audio CODEC
        MMCI_BASE               = 0x10005000, // PrimeCell PL181 MultiMedia Card Interface
        KBD0_BASE               = 0x10006000, // PrimeCell PL050 PS2 Keyboard/Mouse Interface
        KBD1_BASE               = 0x10007000, // PrimeCell PL050 PS2 Keyboard/Mouse Interface
        UART0_BASE              = 0x10009000, // PrimeCell PL011 UART
        UART1_BASE              = 0x1000a000, // PrimeCell PL011 UART
        UART2_BASE              = 0x1000b000, // PrimeCell PL011 UART
        UART3_BASE              = 0x1000c000, // PrimeCell PL011 UART
        TIMER0_BASE             = 0x10011000, // ARM Dual-Timer Module SP804
        TIMER1_BASE             = 0x10012000, // ARM Dual-Timer Module SP804
        GPIOA_BASE              = 0x10013000, // PrimeCell PL061 GPIO
        GPIOB_BASE              = 0x10014000, // PrimeCell PL061 GPIO
        GPIOC_BASE              = 0x10015000, // PrimeCell PL061 GPIO
        RTC_BASE                = 0x10017000, // PrimeCell PL031 RTC
        LCD_BASE                = 0x10020000, // PrimeCell PL110 Color LCD Controller
        DMA_BASE                = 0x10030000, // PrimeCell PL080 DMA Controller

        PPS_BASE                = 0x1f000000, // A9 Private Peripheral Space
        SCU_BASE                = 0x1f000000, // MP Snoop Control Unit
        GIC_CPU_BASE            = 0x1f000100,
        GLOBAL_TIMER_BASE       = 0x1f000200,
        PRIVATE_TIMER_BASE      = 0x1f000600,
        GIC_DIST_BASE           = 0x1f001000,

        // Logical Address Space
        APP_CODE        = Traits<Machine>::VECTOR_TABLE,
        APP_DATA        = Traits<Machine>::VECTOR_TABLE,

        SYS_CODE                = Traits<Machine>::SYS_CODE,
        SYS_INFO                = NOT_USED,
        SYS_DATA                = Traits<Machine>::SYS_CODE,
        SYS_STACK               = NOT_USED,
        SYS_HEAP                = NOT_USED
    };
};

__END_SYS

#endif
