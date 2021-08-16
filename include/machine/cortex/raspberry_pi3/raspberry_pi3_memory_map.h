// EPOS Raspberry Pi3 (ARM Cortex-A53) Memory Map

#ifndef __raspberry_pi3_memory_map_h
#define __raspberry_pi3_memory_map_h

#include <machine/cortex/cortex_memory_map.h>

__BEGIN_SYS

static const unsigned int MIO_OFFSET = Traits<System>::multitask ? (Traits<Machine>::IO - Traits<Machine>::MIO_BASE) : 0;
static constexpr unsigned int adjust_mio(unsigned int addr) { return addr + MIO_OFFSET; }
static constexpr unsigned int no_mio(unsigned int addr) { return addr - MIO_OFFSET; }

struct Memory_Map: public Cortex_Memory_Map
{
    enum : unsigned int {
        // Base addresses for memory-mapped control and I/O devices
        MBOX_COM_BASE   = adjust_mio(0x3ef00000), // RAM memory for device-os communication (must be mapped as device by the MMU)
        MBOX_CTRL_BASE  = adjust_mio(0x40000000), // BCM MailBox
        PPS_BASE        = adjust_mio(0x3f000000), // Private Peripheral Space
        TSC_BASE        = adjust_mio(0x3f003000),
        TIMER0_BASE     = Traits<Machine>::SIMULATED ? MBOX_CTRL_BASE : adjust_mio(0x3f003000), // System Timer (free running)
        DMA0_BASE       = adjust_mio(0x3f007000),
        IC_BASE         = adjust_mio(0x3f00b200),
        TIMER1_BASE     = adjust_mio(0x3f00b400), // ARM Timer (frequency relative to processor frequency)
        MBOX_BASE       = adjust_mio(0x3f00b800),
        ARM_MBOX0       = adjust_mio(0x3f00b880), // IOCtrl (MBOX 0) is also mapped on this address
        PM_BASE         = adjust_mio(0x3f100000), // Power Manager
        RAND_BASE       = adjust_mio(0x3f104000),
        GPIO_BASE       = adjust_mio(0x3f200000),
        UART_BASE       = adjust_mio(0x3f201000), // PrimeCell PL011 UART
        SD0_BASE        = adjust_mio(0x3f202000), // Custom sdhci controller
        AUX_BASE        = adjust_mio(0x3f215000), // mini UART + 2 x SPI master
        SD1_BASE        = adjust_mio(0x3f300000), // Arasan sdhci controller
        DMA1_BASE       = adjust_mio(0x3fe05000),

        // Logical Address Space -- Need to be verified
        APP_LOW         = Traits<Machine>::APP_LOW,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        APP_CODE        = Traits<Machine>::APP_CODE,
        APP_DATA        = Traits<Machine>::APP_DATA,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO,
        SYS             = Traits<Machine>::SYS,
        SYS_CODE        = Traits<System>::multitask ? SYS + 0x00000000 : NOT_USED,
        SYS_DATA        = Traits<System>::multitask ? SYS + 0x00000000 : NOT_USED,
        SYS_INFO        = Traits<System>::multitask ? SYS + 0x00100000 : NOT_USED,
        SYS_PT          = Traits<System>::multitask ? SYS + 0x00101000 : NOT_USED, // 4KB = 256 + 256 pt entries to map from SYS to SYS_STACK
        SYS_PD          = Traits<System>::multitask ? SYS + 0x00103000 : NOT_USED, // 16KB mem == 4k PD entries
        SYS_STACK       = Traits<System>::multitask ? SYS + 0x00107000 : NOT_USED, // 16KB mem == STACK_SIZE
        SYS_HEAP        = Traits<System>::multitask ? SYS + 0x00200000 : NOT_USED
    };
};

__END_SYS

#endif
