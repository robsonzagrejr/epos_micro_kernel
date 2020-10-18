// EPOS RISC-V Mediator Declarations

#ifndef __riscv_machine_h
#define __riscv_machine_h

#include <architecture.h>
#include <machine/machine.h>
#include <machine/ic.h>
#include <system/info.h>
#include <system/memory_map.h>
#include <system.h>

__BEGIN_SYS

class Machine: private Machine_Common
{
    friend class Init_System;
    friend class First_Object;

private:
    static const bool smp = Traits<System>::multicore;

public:
    Machine() {}

    static void delay(const Microsecond & time) {
        // should this be implemented?
    }

    static void panic();

    static void reboot()
    {
        if (Traits<System>::reboot) {
            db<Machine>(WRN) << "Machine::reboot()" << endl;
            CPU::halt();
        } else {
            poweroff();
        }
    }
    static void poweroff()
    {
        db<Machine>(WRN) << "Machine::poweroff()" << endl;
        // implement
    }

    static void smp_barrier_init(unsigned int n_cpus) {
        db<Machine>(TRC) << "SMP::init()" << endl;
        // wake up secondary cores
    }

    static const UUID & uuid() { return System::info()->bm.uuid; }

private:
    static void pre_init(System_Info * si);
    static void init();
};

__END_SYS

#endif
