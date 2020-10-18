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
    // typedef IC_Engine Engine;
    typedef CPU::Reg32 Reg32;

    static const unsigned int INTS = Traits<IC>::INTS;
public:
    using IC_Common::Interrupt_Id;
    using IC_Common::Interrupt_Handler;

    // MIE interrupt flags
    enum {
        // implement
    };

    // MASKS
    enum {
        // implement
    };

    // Interrupt IDS
    enum {
        INT_SYS_TIMER   = XX,
        INT_USER_TIMER0 = XX, // it could be 5, if we adopt supervisor execution mode
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
        INT_RESCHEDULER = XX
    };

    // address
    enum {
        // implement
    };

    // clint offsets
    enum {
        // implement
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
        // enable all interrupts
    }
    static void enable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::enable(int=" << i << ")" << endl;
        assert(i < INTS);
        // enable interrupt with id = i
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        // disable all interrupts
    }
    static void disable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::disable(int=" << i << ")" << endl;
        assert(i < INTS);
        // disable interrupt with id = i
    }

    static Interrupt_Id int_id() {
        // return interrupt id
        return 0;
    }

    int irq2int(int i) { return i; }

    int int2irq(int i) { return i; }

    static void ipi(unsigned int cpu, Interrupt_Id i) {
        db<IC>(TRC) << "IC::ipi(cpu=" << cpu << ",int=" << i << ")" << endl;
        assert(i < INTS);
        // SEND IPI
    }

    void undefined_instruction();
    void software_interrupt();
    void prefetch_abort();
    void data_abort();
    void reserved();
    void fiq();
    void exception_handling();

private:
    static void dispatch(unsigned int i);

    // Logical handlers
    static void int_not(Interrupt_Id i);
    static void hard_fault(Interrupt_Id i);

    // Physical handler
    static void entry();

    static void init();

private:
    static Interrupt_Handler _int_vector[INTS];
};

__END_SYS

#endif
