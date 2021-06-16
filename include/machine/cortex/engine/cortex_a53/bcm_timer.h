// EPOS ARM Cortex-A53 BCM Timer Mediator Declarations

#ifndef __bcm_timer_h
#define __bcm_timer_h

#include <architecture/cpu.h>
#define __timer_common_only__
#include <machine/timer.h>
#undef __timer_common_only__

__BEGIN_SYS

class BCM_Timer : public Timer_Common
{
    // This is a hardware object
    // Use with something like "new (Memory_Map::TIMER0_BASE) BCM_Timer"

private:
    typedef CPU::Reg32 Reg32;
    typedef IC_Common::Interrupt_Id Interrupt_Id;

public:
    typedef CPU::Reg64 Count;

    static const unsigned int CLOCK = 1000000;

    // Registers offsets from BASE (i.e. this)
    enum {                                      // Description
        STCS    = 0x00,     // Control/Status
        STCLO   = 0x04,     // Low COUNTER
        STCHI   = 0x08,     // High Counter
        STC0    = 0x0c,     // Compare 0 - Used by GPU
        STC1    = 0x10,     // Compare 1 - Value used to generate interrupt 1
        STC2    = 0X14,     // Compare 2 - Used by GPU
        STC3    = 0X18      // Compare 3 - Value used to generate interrupt 3
        // Interrupts mapped to "Enable IRQ 1" - c1 and c3 == irq1 and irq3
    };

public:
    void config(unsigned int unit, const Count & count) {
        assert(unit < 2);
        if(unit == 0) {
            timer(STCS) |= 1 << 1;
            timer(STC1) = count + timer(STCLO);
        } else {
            timer(STCS) |= 1 << 3;
            timer(STC3) = count + timer(STCLO);
        }
    }

    Count count() {
        return static_cast<Count>(timer(STCLO));
    }

    //check for an implementation for this method
    void enable() {}

    //check for an implementation for this method
    void disable() {}

    //check for an implementation for this method
    void eoi() {}

    //check for an implementation for this method
    void set(const Count & count) {}

    Hertz clock() { return CLOCK; }

private:
    volatile Reg32 & timer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};

class BCM_Local_Timer : public Timer_Common
{
    // This is a hardware object
    // Use with something like "new (Memory_Map::TIMER0_BASE) BCM_Timer"

private:
    typedef CPU::Reg32 Reg32;
    typedef IC_Common::Interrupt_Id Interrupt_Id;

public:
    typedef CPU::Reg64 Count;

    static const unsigned int CLOCK = 2*19200000; // 2*19.2 MHz using crystal clock

    // Registers offsets from BASE (i.e. this)
    enum {                                      // Description
        CTCR    = 0x0,      // Core Timer Control Register
        CTPE    = 0x8,      // Core Timer Pre-Scaler
        LTIR    = 0x24,     // Local Timer Interrupt Routing
        LTCS    = 0x34,     // Control/Status
        LTIRQC  = 0x38,     // IRQ clear & reload (write-only)
        LTC0IC  = 0x40,     // Core 0 Interrupt Control
        LTC1IC  = 0x44,     // Core 1 Interrupt Control
        LTC2IC  = 0x48,     // Core 2 Interrupt Control
        LTC3IC  = 0x4C,     // Core 3 Interrupt Control
    };

    // LTIRQC flags
    enum {
        INTERRUPT_F = 1 << 31, // clear interrupt flag
        RELOAD_F    = 1 << 30  // reload counter without generating an interruption
    };

    // LTCS bits
    enum {
        READ_INTERRUPT_FLAG = 31,
        INTERRUPT_ENABLE    = 29,
        TIMER_ENABLE        = 28,
        RELOAD_VALUE        = 0
    };

    // LTC[0-3]IC
    enum {
        VIRQ            = 3,    // Virtual physical
        HPIRQ           = 2,    // Hypervisor IRQ Control
        NSIRQ           = 1,    // Non-Secure mode IRQ Control
        SIRQ            = 0     // Secure mode IRQ Control
        // bits 4-7 are FIQ bits and must be set to 0
    };

    // CTCR bits
    enum {
        SOURCE          = 8,
        INCREMENT       = 9,
    };

public:
    void config(unsigned int unit, const Count & count) {
        timer(LTIR) = 0; // Locat Timer Interrupt goes to core 0 as IRQ
        timer(LTCS) = 1 << INTERRUPT_ENABLE | 1 << TIMER_ENABLE |
                        count; // Enable timer and interrupts, load count to reload value
        timer(LTC0IC) = 1 << NSIRQ;//0xF; // Enable timer in every mode
        timer(LTIRQC) = INTERRUPT_F | RELOAD_F; // clear flags and do a reload
    }

    Count count() {
        return static_cast<Count>(0);
    }

    void enable() {
        timer(LTCS) |= 1 << INTERRUPT_ENABLE | 1 << TIMER_ENABLE;
    }

    void disable() {
        timer(LTCS) &= ~(1 << INTERRUPT_ENABLE | 1 << TIMER_ENABLE);
    }

    void eoi() {
        timer(LTIRQC) = INTERRUPT_F | RELOAD_F; // clear flags and do a reload
    }

    void set(const Count & count) {
        timer(LTCS) |= count;
    }

    Hertz clock() { return CLOCK; }

private:
    volatile Reg32 & timer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};

__END_SYS

#endif
