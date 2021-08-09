// EPOS Raspberry Pi3 (ARM Cortex-A53) Memory Map

#ifndef __raspberry_pi3_memory_map_h
#define __raspberry_pi3_memory_map_h

#include <machine/cortex/cortex_memory_map.h>

__BEGIN_SYS

struct Memory_Map: public Cortex_Memory_Map
{
    enum {
        // Base addresses for memory-mapped control and I/O devices
        MBOX_COM_BASE           = 0x3ef00000, // RAM memory for device-os communication (must be mapped as device by the MMU)
        MBOX_CTRL_BASE          = 0x40000000, // BCM MailBox
        PPS_BASE                = 0x3f000000, // Private Peripheral Space
        TIMER0_BASE             = Traits<Machine>::SIMULATED ? MBOX_CTRL_BASE : 0x3f003000, // System Timer (free running)
        DMA0_BASE               = 0x3f007000,
        IC_BASE                 = 0x3f00b200,
        TIMER1_BASE             = 0x3f00b400, // ARM Timer (frequency relative to processor frequency)
        MBOX_BASE               = 0x3f00b800,
        // ARM_MBOX0               = 0x3f00b880, // should we create this? the ioctrl (MBOX 0) is mapped over this address
        PM_BASE                 = 0x3f100000, // Power Manager
        RAND_BASE               = 0x3f104000,
        GPIO_BASE               = 0x3f200000,
        UART_BASE               = 0x3f201000, // PrimeCell PL011 UART
        SD0_BASE                = 0x3f202000, // Custom sdhci controller
        AUX_BASE                = 0x3f215000, // mini UART + 2 x SPI master
        SD1_BASE                = 0x3f300000, // Arasan sdhci controller
        DMA1_BASE               = 0x3fe05000,

        // Logical Address Space -- Need to be verified
        BOOT            = Traits<System>::multitask ? Traits<Machine>::BOOT : NOT_USED,
        IMAGE           = Traits<Machine>::IMAGE,
        SETUP           = Traits<System>::multitask ? Traits<Machine>::SETUP : NOT_USED,
        INIT            = Traits<System>::multitask ? Traits<Machine>::INIT : NOT_USED,

        APP_LOW         = Traits<System>::multitask ? Traits<Machine>::APP_LOW : Traits<Machine>::VECTOR_TABLE,
        APP_CODE        = APP_LOW,
        APP_DATA        = Traits<System>::multitask ? APP_LOW + 4 * 1024 * 1024 : APP_LOW,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO,

        SYS             = Traits<Machine>::SYS,
        SYS_CODE        = Traits<System>::multitask ? SYS + 0x00000000 : NOT_USED,
        SYS_DATA        = Traits<System>::multitask ? SYS + 0x00000000 : NOT_USED,
        SYS_INFO        = Traits<System>::multitask ? SYS + 0x00100000 : NOT_USED,
        SYS_PT          = Traits<System>::multitask ? SYS + 0x00101000 : NOT_USED, // 12KB = 256 + 256 + 256 pt entries to map from SYS to SYS_HEAP 
        SYS_PD          = Traits<System>::multitask ? SYS + 0x00104000 : NOT_USED, // 16KB mem == 4k PD entries
        SYS_STACK       = Traits<System>::multitask ? SYS + 0x00108000 : NOT_USED, // 16KB mem == STACK_SIZE
        SYS_HEAP        = NOT_USED
    };
};

__END_SYS

#endif
