// EPOS RISC-V 64 CPU Mediator Declarations

#ifndef __riscv64_h
#define __riscv64_h

#include <architecture/cpu.h>

__BEGIN_SYS

class CPU: protected CPU_Common
{
    friend class Init_System;
    friend class Machine;

private:
    static const bool smp = Traits<System>::multicore;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using Reg = CPU_Common::Reg64;
    using Log_Addr = CPU_Common::Log_Addr<Reg>;
    using Phy_Addr = CPU_Common::Phy_Addr<Reg>;

    static const bool thumb = true;

    // CPU Flags
    typedef Reg32 Flags;
    enum {
        FLAG_MIE        = 1 << 3,      // Machine Interrupts Enabled
        FLAG_SIE        = 1 << 1,      // Supervisor Interrupts Enabled
        FLAG_SPIE       = 1 << 5,      // Supervisor Previous Interrupts Enabled
        FLAG_MPIE       = 1 << 7,      // Machine Previous Interrupts Enabled
        FLAG_MPP        = 3 << 11,     // Machine Previous Privilege 
        FLAG_SPP        = 3 << 12,     // Supervisor Previous Privilege
        FLAG_MPRV       = 1 << 17,     // Memory Priviledge
        FLAG_TVM        = 1 << 20,      // Trap Virtual Memory //not allow MMU
        FLAG_DEFAULTS   = (FLAG_MIE | FLAG_SPP | FLAG_MPIE | FLAG_SPIE | FLAG_MPP | FLAG_SIE)
    };

    // CPU Context
    class Context
    {
    public:
        Context(const Log_Addr & entry, const Log_Addr & exit): _mstatus(FLAG_DEFAULTS), _x1(exit), _pc(entry) {}

        void save() volatile  __attribute__ ((naked));
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               // << "{x0="  << c._x0
               // << ",x1="  << c._x1
               // << ",x2="  << c._x2
               // << ",x3="  << c._x3
               // << ",x4="  << c._x4
               << "{x5="  << c._x5
               << ",x6="  << c._x6
               << ",x7="  << c._x7
               << ",x8="  << c._x8
               << ",x9="  << c._x9
               << ",x10="  << c._x10
               << ",x11="  << c._x11
               << ",x12="  << c._x12
               << ",x13="  << c._x13
               << ",x14="  << c._x14
               << ",x15="  << c._x15
               << ",x16="  << c._x16
               << ",x17="  << c._x17
               << ",x18="  << c._x18
               << ",x19="  << c._x19
               << ",x20="  << c._x20
               << ",x21="  << c._x21
               << ",x22="  << c._x22
               << ",x23="  << c._x23
               << ",x24="  << c._x24
               << ",x25="  << c._x25
               << ",x26="  << c._x26
               << ",x27="  << c._x27
               << ",x28="  << c._x28
               << ",x29="  << c._x29
               << ",x30="  << c._x30
               << ",x31="  << c._x31
               << ",sp="  << &c
               << ",ra="  << c._x1
               << ",pc="  << c._pc
               << ",mstatus=" << c._mstatus
               << "}" << dec;
            return db;
        }

    public:
        Reg32 _mstatus;
        // Reg32 _x0;
        Reg32 _x1; //ra
        // Reg32 _x2; //sp
        // Reg32 _x3;
        // Reg32 _x4;
        Reg32 _x5; //t0
        Reg32 _x6; //t1
        Reg32 _x7; //t2
        Reg32 _x8; //s0
        Reg32 _x9; //s1
        Reg32 _x10; //a0
        Reg32 _x11; //a1
        Reg32 _x12; //a2
        Reg32 _x13; //a3
        Reg32 _x14; //a4
        Reg32 _x15; //a5
        Reg32 _x16; //a6
        Reg32 _x17; //a7
        Reg32 _x18; //s2
        Reg32 _x19; //s3
        Reg32 _x20; //s4
        Reg32 _x21; //s5
        Reg32 _x22; //s6
        Reg32 _x23; //s7
        Reg32 _x24; //s8
        Reg32 _x25; //s9
        Reg32 _x26; //s10
        Reg32 _x27; //s11
        Reg32 _x28; //t3
        Reg32 _x29; //t4
        Reg32 _x30; //t5
        Reg32 _x31; //t6
        Reg32 _pc;
    };

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)();

public:
    CPU() {};

public:
    // Register access
    static Reg32 sp() {
        Reg32 value;
        ASM("mv %0, sp" : "=r"(value) :);
        return value;
    }
    static void sp(const Reg32 & sp) {
        ASM("mv sp, %0" : : "r"(sp) :);
    }

    static Reg32 fr() {
        Reg32 value;
        ASM("mv %0, x10" : "=r"(value)); // x10 is a0
        return value;
    }
    static void fr(const Reg32 & fr) {
        ASM("mv x10, %0" : : "r"(fr) :); // x10 is a0
    }

    static Log_Addr ip() {
        Reg32 value;
        ASM("mv %0, pc" : "=r"(value) :);
        return value;
    }

    static Reg32 pdp() { return 0; }
    static void pdp(const Reg32 & pdp) {}


    // Atomic operations

    // using CPU_Common::tsl;
    template<typename T>
    static T tsl(volatile T & lock) {
        register T old;
        register T one = 1;
        ASM("1: lr.w    %0, (%1)        \n"
            "   sc.w    t3, %2, (%1)    \n"
            "   bnez    t3, 1b              \n" : "=&r"(old) : "r"(&lock), "r"(one) : "t3", "cc");
        return old;
    }

    // using CPU_Common::finc;
    template<typename T>
    static T finc(volatile T & value) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   addi    %0, %0, 1       \n"
            "   sc.w    t3, %0, (%1)    \n"
            "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc");
        return old - 1;
    }

    // using CPU_Common::fdec;
    template<typename T>
    static T fdec(volatile T & value) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   addi    %0, %0, -1      \n"
            "   sc.w    t3, %0, (%1)    \n"
            "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc");
        return old + 1;
    }

    // using CPU_Common::cas;
    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   bne     %0, %2, 2f      \n"
            "   sc.w    t3, %3, (%1)    \n"
            "   bnez    t3, 1b          \n"
            "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "t3", "cc");
        return old;
    }

    static void halt() { ASM("wfi"); }

    using CPU_Common::clock;
    using CPU_Common::min_clock;
    using CPU_Common::max_clock;

    static Flags flags() {
        Reg32 value;
        ASM("csrr %0, mstatus" : "=r"(value) :);
        return value;
    }
    static void flags(const Flags & flags) {
        ASM("csrw mstatus, %0" : : "r"(flags) : "cc");
    }

    static unsigned int id() {
        // return 0;
        int id;
        ASM("csrr %0, mhartid" : "=r"(id) : : );
        return id & 0x3;
    }

    static unsigned int cores() {
        return Traits<Build>::CPUS;
    }

    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void int_enable() { flags(flags() | (FLAG_MIE | FLAG_SIE)); }
    static void int_disable() { flags(flags() & ~(FLAG_MIE | FLAG_SIE)); }

    static bool int_enabled() { return flags() & (FLAG_MIE | FLAG_SIE); }
    static bool int_disabled() { return !int_enabled(); }

    static void csrr31() { ASM("csrr x31, mstatus" : : : "x31"); }
    static void csrw31() { ASM("csrw mstatus, x31" : : : "cc"); }

    static unsigned int int_id() { return 0; }

    static void fpu_save();
    static void fpu_restore();
    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(const Log_Addr & usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit);
        init_stack_helper(&ctx->_x10, an ...); // x10 is a0
        return ctx;
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr sp, void (* exit)(), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(0, exit);
        init_stack_helper(&ctx->_x10, an ...); // x10 is a0
        return sp;
    }

    static int syscall(void * message);
    static void syscalled();

    using CPU_Common::htole64;
    using CPU_Common::htole32;
    using CPU_Common::htole16;
    using CPU_Common::letoh64;
    using CPU_Common::letoh32;
    using CPU_Common::letoh16;

    using CPU_Common::htobe64;
    using CPU_Common::htobe32;
    using CPU_Common::htobe16;
    using CPU_Common::betoh64;
    using CPU_Common::betoh32;
    using CPU_Common::betoh16;

    using CPU_Common::htonl;
    using CPU_Common::htons;
    using CPU_Common::ntohl;
    using CPU_Common::ntohs;

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

__END_SYS

#endif
