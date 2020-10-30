// EPOS RISC-V Mediator Declarations

#ifndef __riscv_ic_h
#define __riscv_ic_h

#include <architecture/cpu.h>
#include <machine/ic.h>
#include <system/memory_map.h>

__BEGIN_SYS

class IC: private IC_Common
{
    friend class Machine;

private:
    typedef CPU::Reg32 Reg32;

    static const unsigned int INTS = Traits<IC>::INTS;
public:
    using IC_Common::Interrupt_Id;
    using IC_Common::Interrupt_Handler;

    // MIE interrupt IDs
    enum {
        // interrupt IDs
    };

    enum {
        INT_SYS_TIMER   = 0, // IMPLEMENT: FIX this value
        INT_USER_TIMER0 = 0, // IMPLEMENT: FIX this value
        INT_USER_TIMER1 = 0,
        INT_USER_TIMER2 = 0,
        INT_USER_TIMER3 = 0,
        INT_GPIOA       = 0,
        INT_GPIOB       = 0,
        INT_GPIOC       = 0,
        INT_GPIOD       = 0,
        INT_NIC0_RX     = 0,
        INT_NIC0_TX     = 0,
        INT_NIC0_ERR    = 0,
        INT_NIC0_TIMER  = 0,
        INT_USB0        = 0,
        INT_FIRST_HARD  = 0,
        INT_LAST_HARD   = 0,
        INT_RESCHEDULER = 0 // IMPLEMENT: FIX this value
    };

    // clint offsets
    enum {
        // CORE WAKEUP OFFSET
    };

public:
    IC() {}

    static Interrupt_Handler int_vector(Interrupt_Id i) {
        assert(i < INTS);
        return _int_vector[i];
    }

    static void int_vector(Interrupt_Id i, const Interrupt_Handler & h) {
        db<IC>(TRC) << "IC::int_vector(int=" << i << ",h=" << reinterpret_cast<void *>(h) <<")" << endl;
        assert(i < INTS);
        _int_vector[i] = h;
    }

    static void enable() {
        db<IC>(TRC) << "IC::enable()" << endl;
        // IMPLEMENT
    }
    static void enable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::enable(int=" << i << ")" << endl;
        assert(i < INTS);
        // IMPLEMENT
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        // IMPLEMENT
    }
    static void disable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::disable(int=" << i << ")" << endl;
        assert(i < INTS);
        // IMPLEMENT
    }

    static Interrupt_Id int_id() {
        // IMPLEMENT
        return 0;
    }

    int irq2int(int i) { return i; }

    int int2irq(int i) { return i; }

    static void ipi(unsigned int cpu, Interrupt_Id i) {
        db<IC>(TRC) << "IC::ipi(cpu=" << cpu << ",int=" << i << ")" << endl;
        assert(i < INTS);
        // IMPLEMENT
    }

    static void ipi_eoi(Interrupt_Id i) {
        // IMPLEMENT
    }


private:
    static void dispatch();

    // Logical handlers
    static void int_not(Interrupt_Id i);
    static void hard_fault(Interrupt_Id i);
    static void undefined_instruction(Interrupt_Id i);
    static void software_interrupt(Interrupt_Id i);
    static void prefetch_abort(Interrupt_Id i);
    static void data_abort(Interrupt_Id i);
    static void reserved(Interrupt_Id i);
    static void fiq(Interrupt_Id i);

    // Physical handler
    static void entry();
    static void exception_handling();  // this is a global exception handler sensitive to mcause

    static void init();

    static volatile CPU::Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile CPU::Reg32 *>(Memory_Map::CLINT_BASE)[o / sizeof(CPU::Reg32)]; }

private:
    static Interrupt_Handler _int_vector[INTS];
};

__END_SYS

#endif
