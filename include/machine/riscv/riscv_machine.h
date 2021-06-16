// EPOS RISC-V Mediator Declarations

#ifndef __riscv_machine_h
#define __riscv_machine_h

#include <architecture.h>
#include <machine/machine.h>
#include <machine/ic.h>
#include <machine/display.h>
#include <system/info.h>
#include <system/memory_map.h>
#include <system.h>

__BEGIN_SYS

class Machine: private Machine_Common
{
    friend class Setup;
    friend class Init_Begin;
    friend class Init_System;

private:
    static const bool smp = Traits<System>::multicore;

public:
    Machine() {}

    static void delay(const Microsecond & time) {
        assert(Traits<TSC>::enabled);
        TSC::Time_Stamp end = TSC::time_stamp() + time * (TSC::frequency() / 1000000);
        while(end > TSC::time_stamp());
    }

    static void panic();

    static void reboot()
    {
        if(Traits<System>::reboot) {
            db<Machine>(WRN) << "Machine::reboot()" << endl;
            CPU::Reg * reset = reinterpret_cast<CPU::Reg *>(Memory_Map::TEST_BASE);
            reset[0] = 0x5555;

            while(true);
        } else {
            poweroff();
        }
    }

    static void poweroff()
    {
        db<Machine>(WRN) << "Machine::poweroff()" << endl;
        CPU::Reg * reset = reinterpret_cast<CPU::Reg *>(Memory_Map::TEST_BASE);
        reset[0] = 0x5555;

        while(true);
    }

    static const UUID & uuid() { return System::info()->bm.uuid; }

private:
    static void smp_barrier_init(unsigned int n_cpus);
    static void pre_init(System_Info * si);
    static void init();
};

__END_SYS

#endif
