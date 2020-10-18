// EPOS RISC-V UART Mediator Declarations

#ifndef __riscv_uart_h
#define __riscv_uart_h

#include <architecture/cpu.h>
#include <machine/uart.h>
#include <system/memory_map.h>

__BEGIN_SYS

class UART: private UART_Common
{
private:

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg32 Reg32;

    static const unsigned int UNIT = Traits<UART>::DEF_UNIT;
    static const unsigned int BAUD_RATE = Traits<UART>::DEF_BAUD_RATE;
    static const unsigned int DATA_BITS = Traits<UART>::DEF_DATA_BITS;
    static const unsigned int PARITY = Traits<UART>::DEF_PARITY;
    static const unsigned int STOP_BITS = Traits<UART>::DEF_STOP_BITS;

    // typedef UART_Engine Engine;

    // uart offsets
    enum {
        // implement
    };

    // uart useful bits
    enum {
        // implement
    };

    enum {
        UART_BASE       = Memory_Map::UART_BASE
    };

public:
    using UART_Common::NONE;
    using UART_Common::EVEN;
    using UART_Common::ODD;

public:
    UART(unsigned int unit = UNIT, unsigned int baud_rate = BAUD_RATE, unsigned int data_bits = DATA_BITS, unsigned int parity = PARITY, unsigned int stop_bits = STOP_BITS) 
    {
        config(baud_rate, data_bits, parity, stop_bits);
    }

    void config(unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits)
    {
        // implement
    }

    void config(unsigned int * baud_rate, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {}

    Reg8 rxd() { 
        // implement
        return Reg8(0);
    }
    void txd(Reg8 c) { 
        // implement
    }

    bool rxd_ok() { 
        // implement
        return true;
    }

    bool txd_ok() { 
        // implement
        return true;
    }

    bool rxd_full() { return false; } // implement
    
    bool txd_empty() { 
        // implement
        return true; 
    }

    bool busy() {
        return false;
    }

    void enable() {}
    void disable() {}

    void reset() {}

    void loopback(bool flag) {}


    char get() { while(!rxd_ok()); return rxd(); }
    void put(char c) { while(!txd_ok()); txd(c); }

    void flush() { while(!txd_empty()); }
    bool ready_to_get() { return rxd_ok(); }
    bool ready_to_put() { return txd_ok(); }

    void int_enable(bool receive = true, bool transmit = true, bool line = true, bool modem = true) {}
    void int_disable(bool receive = true, bool transmit = true, bool line = true, bool modem = true) {}

    void power(const Power_Mode & mode) {}

private:
    static void init() {}
};

__END_SYS

#endif
