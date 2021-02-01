// EPOS Raspberry Pi3 (Cortex-A53) Metainfo and Configuration

#ifndef __raspberry_pi3_traits_h
#define __raspberry_pi3_traits_h

#include <system/config.h>

__BEGIN_SYS

class Machine_Common;
template <> struct Traits<Machine_Common>: public Traits<Build>
{
    static const bool debugged = Traits<Build>::debugged;
};

template <> struct Traits<Machine>: public Traits<Machine_Common>
{
    static const bool cpus_use_local_timer      = false;

    static const unsigned int NOT_USED          = 0xffffffff;
    static const unsigned int CPUS              = Traits<Build>::CPUS;

    // Boot Image
    static const unsigned int BOOT_LENGTH_MIN   = NOT_USED;
    static const unsigned int BOOT_LENGTH_MAX   = NOT_USED;

    // Physical Memory
    static const unsigned int MEM_BASE          = 0x00000000;
    static const unsigned int VECTOR_TABLE      = 0x00008000; // Defined by uboot@QEMU
    static const unsigned int PAGE_TABLES       = 0x3eef0000; // 1006 MB
    static const unsigned int MEM_TOP           = 0x3eeeffff; // 1 GB
    static const unsigned int BOOT_STACK        = 0x3eeefffc; // MEM_TOP - sizeof(int) - 1M for boot stacks
    // Logical Memory Map
    static const unsigned int BOOT              = NOT_USED;
    static const unsigned int SETUP             = NOT_USED;
    static const unsigned int INIT              = NOT_USED;

    static const unsigned int APP_LOW           = 0x00000000;
    static const unsigned int APP_CODE          = 0x00008000;
    static const unsigned int APP_DATA          = 0x00008000;
    static const unsigned int APP_HIGH          = 0x3eeeffff;

    static const unsigned int PHY_MEM           = 0x40000000; // 2 GB
    static const unsigned int IO_BASE           = 0x40000000; // 4 GB - 256 MB
    static const unsigned int IO_TOP            = 0x400000ff; // 4 GB - 12 MB

    static const unsigned int SYS               = IO_TOP;     // 4 GB - 12 MB
    static const unsigned int SYS_CODE          = 0xff700000;
    static const unsigned int SYS_DATA          = 0xff740000;

    // Default Sizes and Quantities
    static const unsigned int STACK_SIZE        = 16 * 1024;
    static const unsigned int HEAP_SIZE         = 16 * 1024 * 1024;
    static const unsigned int MAX_THREADS       = 16;

    // PLL clocks
    static const unsigned int ARM_PLL_CLOCK     = 1333333333;
    static const unsigned int IO_PLL_CLOCK      = 1000000000;
    static const unsigned int DDR_PLL_CLOCK     = 1066666666;
};

template <> struct Traits<IC>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    static const unsigned int IRQS = 96;
    static const unsigned int INTS = 128;
};

template<> struct Traits<Timer>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    static const unsigned int UNITS = 1;

    // Meaningful values for the timer frequency range from 100 to
    // 10000 Hz. The choice must respect the scheduler time-slice, i. e.,
    // it must be higher than the scheduler invocation frequency.
    static const int FREQUENCY = 1000; // Hz
};

template<> struct Traits<UART>: public Traits<Machine_Common>
{
    static const unsigned int UNITS = 1; // only mini UART, since PL011 is Bluetooth

    // CLOCK_DIVISOR is hard coded in ps7_init.tcl
    static const unsigned int CLOCK_DIVISOR = 4;
    static const unsigned int CLOCK = Traits<Machine>::IO_PLL_CLOCK/CLOCK_DIVISOR;

    static const unsigned int DEF_UNIT = 0;
    static const unsigned int DEF_BAUD_RATE = 115200;
    static const unsigned int DEF_DATA_BITS = 8;
    static const unsigned int DEF_PARITY = 0; // none
    static const unsigned int DEF_STOP_BITS = 1;
};

template<> struct Traits<GPIO>: public Traits<Machine_Common>
{
    static const unsigned int UNITS = 0;
    static const bool supports_power_up = false;
};

template<> struct Traits<Serial_Display>: public Traits<Machine_Common>
{
    static const bool enabled = true;
    static const int ENGINE = UART;
    static const int UNIT = 0;
    static const int COLUMNS = 80;
    static const int LINES = 24;
    static const int TAB_SIZE = 8;
};

template<> struct Traits<Serial_Keyboard>: public Traits<Machine_Common>
{
    static const bool enabled = true;
};

template<> struct Traits<Scratchpad>: public Traits<Machine_Common>
{
    static const bool enabled = false;
};

template<> struct Traits<Ethernet>: public Traits<Machine_Common>
{
    // NICS that don't have a network in Traits<Network>::NETWORKS will not be enabled
    typedef LIST<Ethernet_NIC> DEVICES;
    static const unsigned int UNITS = DEVICES::Length;

    static const bool enabled = (Traits<Build>::NODES > 1) && (UNITS > 0);
    static const bool promiscuous = false;
};

__END_SYS

#endif
