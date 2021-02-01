// EPOS RISC-V 64 Time-Stamp Counter Mediator Declarations

#ifndef __riscv64_tsc_h
#define __riscv64_tsc_h

#include <architecture/cpu.h>
#include <architecture/tsc.h>

#define __ic_common_only__
#include <machine/ic.h>
#undef __ic_common_only__

#include <architecture/cpu.h>
#include <architecture/tsc.h>

__BEGIN_SYS

class TSC: private TSC_Common
{
    friend class CPU;
    friend class IC;

private:
    static const unsigned int CLOCK = Traits<Machine>::TIMER_CLOCK;
    static const unsigned int ACCURACY = 40000; // ppb

    enum {
        TSC_BASE = 0x02000000
    };

    enum {              // Description
        MTIME = 0xbff8, // Counter
        MTIMEH = 0xbffc // Counter
    };

public:
    using TSC_Common::Time_Stamp;

    static const unsigned int FREQUENCY = CLOCK;

public:
    TSC() {}

    static Hertz frequency() { return CLOCK; }
    static PPB accuracy() { return ACCURACY; }

    static Time_Stamp time_stamp() {
        return reg(MTIME);
    }

private:
    static void init();

    static volatile CPU::Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile CPU::Reg32 *>(TSC_BASE)[o / sizeof(CPU::Reg32)]; }

};

__END_SYS

#endif
