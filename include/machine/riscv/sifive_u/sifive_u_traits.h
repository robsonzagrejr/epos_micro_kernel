// EPOS RISC-V Sifive Metainfo and Configuration

#ifndef __riscv_sifive_traits_h
#define __riscv_sifive_traits_h

#include <system/config.h>

__BEGIN_SYS

class Machine_Common;
template<> struct Traits<Machine_Common>: public Traits<Build> {};

template <> struct Traits<Machine>: public Traits<Machine_Common>
{
    static const unsigned int NOT_USED          = 0xffffffff;
    static const unsigned int CPUS              = Traits<Build>::CPUS;

    // Boot Image
    static const unsigned int BOOT_LENGTH_MIN   = NOT_USED;
    static const unsigned int BOOT_LENGTH_MAX   = NOT_USED;

    // Physical Memory
    static const unsigned int MEM_BASE          = XXXXXXXXXX;
    static const unsigned int VECTOR_TABLE      = XXXXXXXXXX;
    static const unsigned int PAGE_TABLES       = XXXXXXXXXX;
    static const unsigned int MEM_TOP           = XXXXXXXXXX;
    static const unsigned int BOOT_STACK        = XXXXXXXXXX;

    // Logical Memory Map
    static const unsigned int BOOT              = NOT_USED;
    static const unsigned int SETUP             = NOT_USED;
    static const unsigned int INIT              = NOT_USED;

    static const unsigned int APP_LOW           = XXXXXXXXXX;
    static const unsigned int APP_CODE          = XXXXXXXXXX;
    static const unsigned int APP_DATA          = XXXXXXXXXX;
    static const unsigned int APP_HIGH          = XXXXXXXXXX;

    static const unsigned int PHY_MEM           = XXXXXXXXXX;
    static const unsigned int IO_BASE           = XXXXXXXXXX;
    static const unsigned int IO_TOP            = XXXXXXXXXX;

    static const unsigned int SYS               = IO_TOP;
    static const unsigned int SYS_CODE          = XXXXXXXXXX;
    static const unsigned int SYS_DATA          = XXXXXXXXXX;

    // Default Sizes and Quantities
    static const unsigned int STACK_SIZE        = 16 * 1024;
    static const unsigned int HEAP_SIZE         = 16 * 1024 * 1024;
    static const unsigned int MAX_THREADS       = 16;

    // PLL clocks
    static const unsigned int IO_PLL_CLOCK      = XXXXXXXXXX;
    static const unsigned int TIMER_CLOCK       = XXXXXXXXXX;
};

template <> struct Traits<IC>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    static const unsigned int IRQS = XXX;
    static const unsigned int INTS = XXx;
};

template <> struct Traits<Timer>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    static const unsigned int UNITS = 1;

    // Meaningful values for the timer frequency range from 100 to 10000 Hz. The
    // choice must respect the scheduler time-slice, i. e., it must be higher
    // than the scheduler invocation frequency.
    static const int FREQUENCY = 1000; // Hz
};

template <> struct Traits<UART>: public Traits<Machine_Common>
{
    static const unsigned int UNITS = 2;

    static const unsigned int CLOCK_DIVISOR = XXX;
    static const unsigned int CLOCK = Traits<Machine>::IO_PLL_CLOCK/CLOCK_DIVISOR;

    static const unsigned int DEF_UNIT = 1;
    static const unsigned int DEF_BAUD_RATE = 115200;
    static const unsigned int DEF_DATA_BITS = 8;
    static const unsigned int DEF_PARITY = 0; // none
    static const unsigned int DEF_STOP_BITS = 1;
};

template<> struct Traits<Serial_Display>: public Traits<Machine_Common>
{
    static const bool enabled = (Traits<Build>::EXPECTED_SIMULATION_TIME != 0);
    static const int ENGINE = UART;
    static const int UNIT = 1;
    static const int COLUMNS = 80;
    static const int LINES = 24;
    static const int TAB_SIZE = 8;
};

__END_SYS

#endif
