// EPOS ARM BCM2835 (Cortex-A53) IC Mediator Declarations

#ifndef __bcm2853_ic_h
#define __bcm2853_ic_h

#include <architecture/cpu.h>
#define __ic_common_only__
#include <machine/ic.h>
#undef __ic_common_only__

__BEGIN_SYS

class BCM_IC_Common: public IC_Common
{
protected:
    typedef CPU::Reg32 Reg32;

public:
    // IRQs
    static const unsigned int IRQS = Traits<IC>::IRQS;
    typedef Interrupt_Id IRQ;
    enum {
        // Interrupts not listed here should not be enabled as they will interfere with the GPU operation

        // IRQ1
        SYSTEM_TIMER_MATCH1             = 1,
        SYSTEM_TIMER_MATCH3             = 3,
        USB_CONTROLLER                  = 9,
        UART_AUX_INT                    = 29,

        // IRQ2
        I2C_SPI_SLV_INT                 = 43,
        PWA0                            = 45,
        PWA1                            = 46,
        SMI                             = 48,
        GPIO_INT0                       = 49,
        GPIO_INT1                       = 50,
        GPIO_INT2                       = 51,
        GPIO_INT3                       = 52,
        I2C_INT                         = 53,
        SPI_INT                         = 54,
        PCM_INT                         = 55,
        UART_INT0                       = 57,

        // BASIC (FIQ)
        ARM_TIMER_IRQ                   = 64,
        ARM_MAILBOX_IRQ                 = 65,
        ARM_DOORBELL_0_IRQ              = 66,
        ARM_DOORBELL_1_IRQ              = 67,
        GPU_0_HALTED_IRQ                = 68,
        GPU_1_HALTED_IRQ                = 69,
        ACCESS_ERROR_TYPE_1_IRQ         = 70,
        ACCESS_ERROR_TYPE_0_IRQ         = 71,

        // MAILBOX
        CORE0_MAILBOX0_IRQ              = 96,
        CORE0_MAILBOX1_IRQ              = 97,
        CORE0_MAILBOX2_IRQ              = 98,
        CORE0_MAILBOX3_IRQ              = 99,
        CORE1_MAILBOX0_IRQ              = 100,
        CORE1_MAILBOX1_IRQ              = 101,
        CORE1_MAILBOX2_IRQ              = 102,
        CORE1_MAILBOX3_IRQ              = 103,
        CORE2_MAILBOX0_IRQ              = 104,
        CORE2_MAILBOX1_IRQ              = 105,
        CORE2_MAILBOX2_IRQ              = 106,
        CORE2_MAILBOX3_IRQ              = 107,
        CORE3_MAILBOX0_IRQ              = 108,
        CORE3_MAILBOX1_IRQ              = 109,
        CORE3_MAILBOX2_IRQ              = 110,
        CORE3_MAILBOX3_IRQ              = 111
    };
};

class BCM_IRQ: public BCM_IC_Common
{
    // This is a hardware object.
    // Use with something like "new (Memory_Map::SPIx_BASE) BCM_IC".

public:
    // IC Registers (as offsets to IC_BASE)
    enum {                                   // Type    Value after reset
        IRQ_BASIC_PENDING           = 0x000, // r       0x000
        IRQ_PENDING_1               = 0x004, // r       0x000
        IRQ_PENDING_2               = 0x008, // r/w     0x000
        FIQ_CONTROL                 = 0x00c, // r/w     0x000
        ENABLE_IRQS_1               = 0x010, // r/w     0x000
        ENABLE_IRQS_2               = 0x014, // r/w     0x000
        ENABLE_BASIC_IRQS           = 0x018, // r/w     0x000
        DISABLE_IRQS_1              = 0x01c, // r/w     0x000
        DISABLE_IRQS_2              = 0x020, // r/w     0x000
        DISABLE_BASIC_IRQS          = 0x024  // r/w     0x000
    };

public:
    void enable() {
        irq(ENABLE_IRQS_1) = ~1;
    }

    void enable(int i) {
        irq(ENABLE_IRQS_1 + (i / 32) * 4) |= 1 << (i % 32);
    }

    void disable() {
        irq(ENABLE_IRQS_1) = 0;
    }

    void disable(int i) {
        irq(DISABLE_IRQS_1 + (i / 32) * 4) |= 1 << (i % 32);
    }

    Interrupt_Id int_id() {
        Reg32 pending = irq(IRQ_BASIC_PENDING);
        if(pending) {
            if(pending & 1)     return ARM_TIMER_IRQ;
        }

        pending = irq(IRQ_PENDING_1);
        if(pending) {
            if(pending & 2)     return SYSTEM_TIMER_MATCH1; // handler needs to reset counter compare register
            if(pending & 4)     return SYSTEM_TIMER_MATCH3; // handler needs to reset counter compare register
            return SYSTEM_TIMER_MATCH1;
        }

        pending = irq(IRQ_PENDING_2);
        if(pending) {
            return pending;
        }

        return LAST_INT; // Unsupported interrupt
    }

private:
    volatile Reg32 & irq(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};


class BCM_Mailbox: public BCM_IC_Common
{
    // This is a hardware object.
    // Use with something like "new (Memory_Map::SPIx_BASE) BCM_IC".

public:
    // Registers offsets from BASE (i.e. this)
    enum {
        GPU_INT_ROUTING         = 0x00c,
        PMU_INT_ROUTING_SET     = 0x010,
        PMU_INT_ROUTING_CLR     = 0x014,
        LOCAL_INT_ROUTING       = 0x024, // Timer interrupt
        LOCAL_TIMER_CS          = 0x034, // Control/Status
        LOCAL_TIMER_WFLAGS      = 0x038, //

        CORE0_TIMER_INT_CTRL    = 0x040,
        CORE1_TIMER_INT_CTRL    = 0x044,
        CORE2_TIMER_INT_CTRL    = 0x048,
        CORE3_TIMER_INT_CTRL    = 0x04c,

        CORE0_MBOX_INT_CTRL     = 0x050,
        CORE1_MBOX_INT_CTRL     = 0x054,
        CORE2_MBOX_INT_CTRL     = 0x058,
        CORE3_MBOX_INT_CTRL     = 0x05c,

        CORE0_IRQ_SRC           = 0x060,
        CORE1_IRQ_SRC           = 0x064,
        CORE2_IRQ_SRC           = 0x068,
        CORE3_IRQ_SRC           = 0x06c,

        MBOX_WS                 = 0x080, // Each CPU has 4 Mailboxes WRITE-SET   registers of 4 Bytes
        MBOX_WC                 = 0x0c0  // Each CPU has 4 Mailboxes WRITE-CLEAR registers of 4 Bytes
    };

    // COREx_IRQ_SRC useful bits
    enum {
        SRC_CNTPS               = 1 << 0,
        SRC_CNTPNS              = 1 << 1,
        SRC_CNTHP               = 1 << 2,
        SRC_CNTV                = 1 << 3,
        SRC_MBOX1               = 1 << 4,
        SRC_MBOX2               = 1 << 5,
        SRC_MBOX3               = 1 << 6,
        SRC_MBOX4               = 1 << 7
    };

public:
    void enable() {
        mailbox(CORE0_MBOX_INT_CTRL) = ~1;
        mailbox(CORE1_MBOX_INT_CTRL) = ~1;
        mailbox(CORE2_MBOX_INT_CTRL) = ~1;
        mailbox(CORE3_MBOX_INT_CTRL) = ~1;
    }

    void enable(int i) {
        mailbox(CORE0_MBOX_INT_CTRL + (i % 32)) |= 1 << i % 4;
    }

    void disable() {
        mailbox(CORE0_MBOX_INT_CTRL) = 0;
        mailbox(CORE1_MBOX_INT_CTRL) = 0;
        mailbox(CORE2_MBOX_INT_CTRL) = 0;
        mailbox(CORE3_MBOX_INT_CTRL) = 0;
    }

    void disable(int i) {
        mailbox(CORE0_MBOX_INT_CTRL + (i % 32)) &= ~(1 << i % 4);
    }

    Interrupt_Id int_id() {
        unsigned int cpu = CPU::id();
        Reg32 src = mailbox(CORE0_IRQ_SRC + 4 * cpu);
        // Does not matter the CPU from where the IPI came from
        // 0x10 = CPU 0 | 0x20 = CPU 1 | 0x40 = CPU 2 | 0x80 = CPU 3
        if (src & 0x10 || src & 0x20 || src & 0x40 || src & 0x80)
            return CORE0_MAILBOX0_IRQ;
        else
            return LAST_INT;
    }

    void ipi(unsigned int cpu, Interrupt_Id id) {
        mailbox(MBOX_WS + 16 * cpu) = 1 << 31;
    }

    void eoi(Interrupt_Id int_id) {
        unsigned int cpu_base = CPU::id() * 16;
        //Clear all the interrups, as all IPIs use the same ID now
        mailbox(MBOX_WC + cpu_base + 0)  = 1 << 31; // ACK From CPU0
        mailbox(MBOX_WC + cpu_base + 4)  = 1 << 31; // ACK From CPU1
        mailbox(MBOX_WC + cpu_base + 8)  = 1 << 31; // ACK From CPU2
        mailbox(MBOX_WC + cpu_base + 12) = 1 << 31; // ACK From CPU3
        ASM("dsb \t\n isb");
    }

    void init(void) {
        // Enable All IPIs
        mailbox(CORE0_MBOX_INT_CTRL) = 0xf;
        mailbox(CORE1_MBOX_INT_CTRL) = 0xf;
        mailbox(CORE2_MBOX_INT_CTRL) = 0xf;
        mailbox(CORE3_MBOX_INT_CTRL) = 0xf;
    }

private:
    volatile Reg32 & mailbox(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};

__END_SYS

#endif
