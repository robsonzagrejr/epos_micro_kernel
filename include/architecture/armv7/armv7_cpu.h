// EPOS ARMv7 CPU Mediator Declarations

#ifndef __armv7_h
#define __armv7_h

#include <architecture/cpu.h>

__BEGIN_SYS

class ARMv7: protected CPU_Common
{
private:
    static const bool smp = Traits<System>::multicore;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using CPU_Common::Log_Addr;
    using CPU_Common::Phy_Addr;

protected:
    ARMv7() {};

public:
    // Register access
    static Reg32 sp() {
        Reg32 value;
        ASM("mov %0, sp" : "=r"(value) :);
        return value;
    }
    static void sp(const Reg32 & sp) {
        ASM("mov sp, %0" : : "r"(sp) : "sp");
        ASM("isb");
    }

    static Reg32 fr() {
        Reg32 value;
        ASM("mov %0, r0" : "=r"(value));
        return value;
    }
    static void fr(const Reg32 & fr) {
        ASM("mov r0, %0" : : "r"(fr) : "r0");
    }

    static Log_Addr ip() { // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset
        Reg32 value;
        ASM("mov %0, pc" : "=r"(value) :);
        return value;
    }

    static Reg32 pdp() { return 0; }
    static void pdp(const Reg32 & pdp) {}


    // Atomic operations
    template<typename T>
    static T tsl(volatile T & lock) {
        register T old;
        register T one = 1;
        ASM("1: ldrexb  %0, [%1]        \n"
            "   strexb  r3, %2, [%1]    \n"
            "   cmp     r3, #0          \n"
            "   bne     1b              \n" : "=&r"(old) : "r"(&lock), "r"(one) : "r3", "cc");
        return old;
    }

    template<typename T>
    static T finc(volatile T & value) {
        register T old;
        if(sizeof(T) == sizeof(Reg8))
            ASM("1: ldrexb  %0, [%1]        \n"
                "   add     %0, #1          \n"
                "   strexb  r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        else if(sizeof(T) == sizeof(Reg16))
            ASM("1: ldrexh  %0, [%1]        \n"
                "   add     %0, #1          \n"
                "   strexh  r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        else
            ASM("1: ldrex   %0, [%1]        \n"
                "   add     %0, #1          \n"
                "   strex   r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        return old - 1;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        register T old;
        if(sizeof(T) == sizeof(Reg8))
            ASM("1: ldrexb  %0, [%1]        \n"
                "   sub     %0, #1          \n"
                "   strexb  r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        else if(sizeof(T) == sizeof(Reg16))
            ASM("1: ldrexh  %0, [%1]        \n"
                "   sub     %0, #1          \n"
                "   strexh  r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        else
            ASM("1: ldrex   %0, [%1]        \n"
                "   sub     %0, #1          \n"
                "   strex   r3, %0, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n" : "=&r"(old) : "r"(&value) : "r3", "cc");
        return old + 1;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        register T old;
        if(sizeof(T) == sizeof(Reg8))
            ASM("1: ldrexb  %0, [%1]        \n"
                "   cmp     %0, %2          \n"
                "   bne     2f              \n"
                "   strexb  r3, %3, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n"
                "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "r3", "cc");
        else if(sizeof(T) == sizeof(Reg16))
            ASM("1: ldrexh  %0, [%1]        \n"
                "   cmp     %0, %2          \n"
                "   bne     2f              \n"
                "   strexh  r3, %3, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n"
                "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "r3", "cc");
        else
            ASM("1: ldrex   %0, [%1]        \n"
                "   cmp     %0, %2          \n"
                "   bne     2f              \n"
                "   strex   r3, %3, [%1]    \n"
                "   cmp     r3, #0          \n"
                "   bne     1b              \n"
                "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "r3", "cc");
        return old;
    }


    // Power modes
    static void halt() { ASM("wfi"); }
};

class ARMv7_M: public ARMv7
{
public:
    static const bool thumb = true;

    // CPU Flags
    typedef Reg32 Flags;
    enum {
        FLAG_THUMB      = 1 << 24,      // Thumb state
        FLAG_Q          = 1 << 27,      // DSP Overflow
        FLAG_V          = 1 << 28,      // Overflow
        FLAG_C          = 1 << 29,      // Carry
        FLAG_Z          = 1 << 30,      // Zero
        FLAG_N          = 1 << 31,      // Negative
        FLAG_DEFAULTS   = FLAG_THUMB
    };

    // Exceptions
    typedef Reg32 Exception_Id;
    enum {                      // Priority
        EXC_RESET       = 1,    // -3 (highest)
        EXC_NMI         = 2,    // -2
        EXC_HARD        = 3,    // -1
        EXC_MPU         = 4,    // programmable
        EXC_BUS         = 5,    // programmable
        EXC_USAGE       = 6,    // programmable
        EXC_SVCALL      = 11,   // programmable
        EXC_DEBUG       = 12,   // programmable
        EXC_PENDSV      = 14,   // programmable
        EXC_SYSTICK     = 15    // programmable
    };

protected:
    ARMv7_M() {};

public:
    static Flags flags() {
        register Reg32 value;
        ASM("mrs %0, xpsr" : "=r"(value) :);
        return value;
    }
    static void flags(const Flags & flags) {
        ASM("msr xpsr_nzcvq, %0" : : "r"(flags) : "cc");
    }

    static unsigned int id() { return 0; }

    static unsigned int cores() { return 1; }

    static void int_enable() { ASM("cpsie i"); }
    static void int_disable() { ASM("cpsid i"); }

    static void smp_barrier(unsigned long cores = cores()) { assert(cores == 1); }

    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() {
        bool disabled;
        ASM("mrs %0, primask" : "=r"(disabled));
        return disabled;
    }

    static void mrs12() { ASM("mrs r12, xpsr" : : : "r12"); }
    static void msr12() { ASM("msr xpsr_nzcvq, r12" : : : "cc"); }
};

class ARMv7_A: public ARMv7
{
public:
    static const bool thumb = false;

    // CPU Flags
    typedef Reg32 Flags;
    enum {
        FLAG_M          = 0x1f << 0,       // Processor Mode
        FLAG_T          = 1    << 5,       // Thumb state
        FLAG_F          = 1    << 6,       // FIQ disable
        FLAG_I          = 1    << 7,       // IRQ disable
        FLAG_A          = 1    << 8,       // Imprecise Abort disable
        FLAG_E          = 1    << 9,       // Endianess (0 ->> little, 1 -> big)
        FLAG_GE         = 0xf  << 16,      // SIMD Greater than or Equal (4 bits)
        FLAG_J          = 1    << 24,      // Jazelle state
        FLAG_Q          = 1    << 27,      // Underflow and/or DSP saturation
        FLAG_V          = 1    << 28,      // Overflow
        FLAG_C          = 1    << 29,      // Carry
        FLAG_Z          = 1    << 30,      // Zero
        FLAG_N          = 1    << 31,      // Negative

        // FLAG_M values
        FLAG_USER       = 0x10,      // User mode
        FLAG_FIQ        = 0x11,      // FIQ mode
        FLAG_IRQ        = 0x12,      // IRQ mode
        FLAG_SVC        = 0x13,      // SVC mode
        FLAG_ABORT      = 0x17,      // Abort mode
        FLAG_UNDEFINED  = 0x1b,      // Undefined mode
        FLAG_SYSTEM     = 0x1f,      // System mode

        FLAG_DEFAULTS   = FLAG_SVC
    };

    // Exceptions
    typedef Reg32 Exception_Id;
    enum {
        EXC_START                   = 1,
        EXC_UNDEFINED_INSTRUCTION   = 2,
        EXC_SWI                     = 3,
        EXC_PREFETCH_ABORT          = 4,
        EXC_DATA_ABORT              = 5,
        EXC_RESERVED                = 6,
        EXC_IRQ                     = 7,
        EXC_FIQ                     = 8
    };

protected:
    ARMv7_A() {};

public:
    static Flags flags() {
        register Reg32 value;
        ASM("mrs %0, cpsr_all" : "=r"(value) :);
        return value;
    }
    static void flags(const Flags & flags) {
        ASM("msr cpsr_all, %0" : : "r"(flags) : "cc");
    }

    static unsigned int id() {
        int id;
        ASM("mrc p15, 0, %0, c0, c0, 5" : "=r"(id) : : );
        return id & 0x3;
    }

    static unsigned int cores() {
        int n;
        ASM("mrc p15, 4, %0, c15, c0, 0 \t\n\
             ldr %0, [%0, #0x004]" : "=r"(n) : : );
        return (n & 0x3) + 1;
    }

    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void int_enable() { flags(flags() & ~0xc0); }
    static void int_disable() { flags(flags() | 0xc0); }

    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() { return flags() & 0xc0; }

    static void mrs12() { ASM("mrs r12, cpsr_all" : : : "r12"); }
    static void msr12() { ASM("msr cpsr_all, r12" : : : "cc"); }

    static unsigned int int_id() { return 0; }
};

#ifndef __armv8_h

class CPU: public SWITCH<Traits<Build>::MODEL, CASE<Traits<Build>::eMote3, ARMv7_M, CASE<Traits<Build>::LM3S811, ARMv7_M, CASE<DEFAULT, ARMv7_A>>>>::Result
{
    friend class Init_System;

private:
    typedef SWITCH<Traits<Build>::MODEL, CASE<Traits<Build>::eMote3, ARMv7_M, CASE<Traits<Build>::LM3S811, ARMv7_M, CASE<DEFAULT, ARMv7_A>>>>::Result Base;

public:
    // CPU Native Data Types
    using Base::Reg8;
    using Base::Reg16;
    using Base::Reg32;
    using Base::Reg64;
    using Base::Log_Addr;
    using Base::Phy_Addr;

    // CPU Context
    class Context
    {
    public:
        Context(const Log_Addr & entry, const Log_Addr & exit): _flags(FLAG_DEFAULTS), _lr(exit | (thumb ? 1 : 0)), _pc(entry | (thumb ? 1 : 0)) {}
//        _r0(0), _r1(1), _r2(2), _r3(3), _r4(4), _r5(5), _r6(6), _r7(7), _r8(8), _r9(9), _r10(10), _r11(11), _r12(12),

        void save() volatile  __attribute__ ((naked));
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{r0="  << c._r0
               << ",r1="  << c._r1
               << ",r2="  << c._r2
               << ",r3="  << c._r3
               << ",r4="  << c._r4
               << ",r5="  << c._r5
               << ",r6="  << c._r6
               << ",r7="  << c._r7
               << ",r8="  << c._r8
               << ",r9="  << c._r9
               << ",r10=" << c._r10
               << ",r11=" << c._r11
               << ",r12=" << c._r12
               << ",sp="  << &c
               << ",lr="  << c._lr
               << ",pc="  << c._pc
               << ",psr=" << c._flags
               << "}" << dec;
            return db;
        }

    public:
        Reg32 _flags;
        Reg32 _r0;
        Reg32 _r1;
        Reg32 _r2;
        Reg32 _r3;
        Reg32 _r4;
        Reg32 _r5;
        Reg32 _r6;
        Reg32 _r7;
        Reg32 _r8;
        Reg32 _r9;
        Reg32 _r10;
        Reg32 _r11;
        Reg32 _r12;
        Reg32 _lr;
        Reg32 _pc;
    };

    // I/O ports
    typedef Reg16 IO_Irq;

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)();

public:
    CPU() {}

    static Hertz clock() { return _cpu_clock; }
    static Hertz bus_clock() { return _bus_clock; }

    using Base::flags;
    using Base::id;
    using Base::cores;

    using Base::int_enable;
    using Base::int_disable;
    using Base::int_enabled;
    using Base::int_disabled;

    using Base::sp;
    using Base::fr;
    using Base::ip;
    using Base::pdp;

    using Base::tsl;
    using Base::finc;
    using Base::fdec;
    using Base::cas;

    using Base::halt;

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(const Log_Addr & usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit);
        init_stack_helper(&ctx->_r0, an ...);
        return ctx;
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr sp, void (* exit)(), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(0, exit);
        init_stack_helper(&ctx->_r0, an ...);
        return sp;
    }

    static int syscall(void * message);
    static void syscalled();

    using Base::htole64;
    using Base::htole32;
    using Base::htole16;
    using Base::letoh64;
    using Base::letoh32;
    using Base::letoh16;

    using Base::htobe64;
    using Base::htobe32;
    using Base::htobe16;
    using Base::betoh64;
    using Base::betoh32;
    using Base::betoh16;

    using Base::htonl;
    using Base::htons;
    using Base::ntohl;
    using Base::ntohs;

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg64 htole64(CPU::Reg64 v) { return CPU::htole64(v); }
inline CPU::Reg32 htole32(CPU::Reg32 v) { return CPU::htole32(v); }
inline CPU::Reg16 htole16(CPU::Reg16 v) { return CPU::htole16(v); }
inline CPU::Reg64 letoh64(CPU::Reg64 v) { return CPU::letoh64(v); }
inline CPU::Reg32 letoh32(CPU::Reg32 v) { return CPU::letoh32(v); }
inline CPU::Reg16 letoh16(CPU::Reg16 v) { return CPU::letoh16(v); }

inline CPU::Reg64 htobe64(CPU::Reg64 v) { return CPU::htobe64(v); }
inline CPU::Reg32 htobe32(CPU::Reg32 v) { return CPU::htobe32(v); }
inline CPU::Reg16 htobe16(CPU::Reg16 v) { return CPU::htobe16(v); }
inline CPU::Reg64 betoh64(CPU::Reg64 v) { return CPU::betoh64(v); }
inline CPU::Reg32 betoh32(CPU::Reg32 v) { return CPU::betoh32(v); }
inline CPU::Reg16 betoh16(CPU::Reg16 v) { return CPU::betoh16(v); }

inline CPU::Reg32 htonl(CPU::Reg32 v) { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v) { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v) { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v) { return CPU::ntohs(v); }

#endif

__END_SYS

#endif
