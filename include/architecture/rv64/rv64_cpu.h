// EPOS RISC-V 64 CPU Mediator Declarations

#ifndef __rv64_h
#define __rv64_h

#include <architecture/cpu.h>

__BEGIN_SYS

class CPU: protected CPU_Common
{
    friend class Init_System;
    friend class Machine;

private:
    static const bool smp = Traits<System>::multicore;
    static const bool sup = Traits<System>::multitask;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using Reg = CPU_Common::Reg32;
    using Log_Addr = CPU_Common::Log_Addr<Reg>;
    using Phy_Addr = CPU_Common::Phy_Addr<Reg>;

    // Control and Status Register (CSR) for machine mode
    // Status Register (mstatus)
    typedef Reg32 Flags;
    enum {
        MIE             = 1 << 3,      // Machine Interrupts Enabled
        SIE             = 1 << 1,      // Supervisor Interrupts Enabled
        SPIE            = 1 << 5,      // Supervisor Previous Interrupts Enabled
        MPIE            = 1 << 7,      // Machine Previous Interrupts Enabled
        MPP             = 3 << 11,     // Machine Previous Privilege
        MPP_M           = 3 << 11,     // Machine Previous Privilege = Machine
        MPP_S           = 1 << 11,     // Machine Previous Privilege = Supervisor
        MPP_U           = 0 << 11,     // Machine Previous Privilege = User
        SPP             = 1 << 8,      // Supervisor Previous Privilege
        SPP_S           = 1 << 8,      // Supervisor Previous Privilege = Supervisor
        SPP_U           = 0 << 8,      // Supervisor Previous Privilege = User
        MPRV            = 1 << 17,     // Memory Priviledge
        TVM             = 1 << 20      // Trap Virtual Memory //not allow MMU
    };

    // Interrupt-Enable, Interrupt-Pending and Machine Cause Registers (mie, mip, and mcause when interrupt bit is set)
    enum {
        SSI             = 1 << 1,   // Supervisor Software Interrupt
        MSI             = 1 << 3,   // Machine Software Interrupt
        STI             = 1 << 5,   // Supervisor Timer Interrupt
        MTI             = 1 << 7,   // Machine Timer Interrupt
        SEI             = 1 << 9,   // Supervisor External Interrupt
        MEI             = 1 << 11   // Machine External Interrupt
    };

    // Exceptions (mcause with interrupt = 0)
    static const unsigned int EXCEPTIONS = 12;
    enum {
        EXC_IALIGN      = 0,    // Instruction address misaligned
        EXC_IFAULT      = 1,    // Instruction access fault
        EXC_IILLEGAL    = 2,    // Illegal instruction
        EXC_BREAK       = 3,    // Breakpoint
        EXC_DRALIGN     = 4,    // Load address misaligned
        EXC_DRFAULT     = 5,    // Load access fault
        EXC_DWALIGN     = 6,    // Store/AMO address misaligned
        EXC_DWFAULT     = 7,    // Store/AMO access fault
        EXC_ENVU        = 8,    // Environment call from U-mode
        EXC_ENVS        = 9,    // Environment call from S-mode
        EXC_ENVH        = 10,   // Environment call from H-mode
        EXC_ENVM        = 11    // Environment call from M-m
    };

    // Context
    class Context
    {
    public:
        // Contexts are loaded with mret, which gets pc from mepc and updates some bits of mstatus, that's why _st is initialized with MPIE and MPP
        Context(const Log_Addr & entry, const Log_Addr & exit): _st(sup ? (SPIE | SPP_S) : (MPIE | MPP_M)), _pc(entry), _x1(exit) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                                                                        _x5 =  5;  _x6 =  6;  _x7 =  7;  _x8 =  8;  _x9 =  9;
                _x10 = 10; _x11 = 11; _x12 = 12; _x13 = 13; _x14 = 14; _x15 = 15; _x16 = 16; _x17 = 17; _x18 = 18; _x19 = 19;
                _x20 = 20; _x21 = 21; _x22 = 22; _x23 = 23; _x24 = 24; _x25 = 25; _x26 = 26; _x27 = 27; _x28 = 28; _x29 = 29;
                _x30 = 30; _x31 = 31;
            }
        }

        void save() volatile  __attribute__ ((naked));
        void load() const volatile __attribute__ ((naked));

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{sp="   << &c
               << ",st="   << c._st
               << ",pc="   << c._pc
               << ",lr="   << c._x1
               << ",x5="   << c._x5
               << ",x6="   << c._x6
               << ",x7="   << c._x7
               << ",x8="   << c._x8
               << ",x9="   << c._x9
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
               << "}" << dec;
            return db;
        }

    public:
        Reg32  _st; // mstatus
        Reg32  _pc; // pc
    //  Reg32  _x0; // zero
        Reg32  _x1; // ra, ABI Link Register
    //  Reg32  _x2; // sp, ABI Stack Pointer, saved as this
    //  Reg32  _x3; // gp, ABI Global Pointer, managed by the linker
    //  Reg32  _x4; // tp, ABI Thread Pointer, used in EPOS as a system-level temporary
        Reg32  _x5; // t0
        Reg32  _x6; // t1
        Reg32  _x7; // t2
        Reg32  _x8; // s0
        Reg32  _x9; // s1
        Reg32 _x10; // a0
        Reg32 _x11; // a1
        Reg32 _x12; // a2
        Reg32 _x13; // a3
        Reg32 _x14; // a4
        Reg32 _x15; // a5
        Reg32 _x16; // a6
        Reg32 _x17; // a7
        Reg32 _x18; // s2
        Reg32 _x19; // s3
        Reg32 _x20; // s4
        Reg32 _x21; // s5
        Reg32 _x22; // s6
        Reg32 _x23; // s7
        Reg32 _x24; // s8
        Reg32 _x25; // s9
        Reg32 _x26; // s10
        Reg32 _x27; // s11
        Reg32 _x28; // t3
        Reg32 _x29; // t4
        Reg32 _x30; // t5
        Reg32 _x31; // t6
    };

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)();

public:
    CPU() {};

    static Reg32 flags() { return sup ? sstatus() : mstatus(); }
    static void flags(const Flags st) { sup ? sstatus(st) : mstatus(st); }

    static Reg32 sp()      { Reg32 r; ASM("mv %0, sp" :  "=r"(r) :); return r; }
    static void sp(const Reg32 & r) { ASM("mv sp, %0" : : "r"(r) :); }

    static Reg32 fr() { Reg32 r;      ASM("mv %0, a0" :  "=r"(r)); return r; }
    static void fr(const Reg32 & r) { ASM("mv a0, %0" : : "r"(r) :); }

    static Log_Addr ip() { Reg32 r; ASM("auipc %0, 0" : "=r"(r) :); return r; }

    static Reg32 pdp() { return sup ? (satp() << 12) : 0; }
    static void pdp(Reg32 pdp) { if(sup) satp((1 << 31) | (pdp >> 12)); }

    static unsigned int id() { return sup ? tp() : mhartid(); }

    static unsigned int cores() { return Traits<Build>::CPUS; }

    using CPU_Common::clock;
    using CPU_Common::min_clock;
    using CPU_Common::max_clock;

    static void int_enable() { sup ? sstatuss(SIE) : mstatuss(MIE); }
    static void int_disable() { sup ? sstatusc(SIE) : mstatusc(MIE); }
    static bool int_enabled() { return sup ? (sstatus() & SIE) : (mstatus() & MIE) ; }
    static bool int_disabled() { return !int_enabled(); }

    static void halt() { ASM("wfi"); }

    static void fpu_save();
    static void fpu_restore();
    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    static int syscall(void * message);
    static void syscalled();

    template<typename T>
    static T tsl(volatile T & lock) {
        register T old;
        register T one = 1;
        ASM("1: lr.w    %0, (%1)        \n"
            "   sc.w    t3, %2, (%1)    \n"
            "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&lock), "r"(one) : "t3", "cc", "memory");
        return old;
    }

    template<typename T>
    static T finc(volatile T & value) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   addi    %0, %0, 1       \n"
            "   sc.w    t3, %0, (%1)    \n"
            "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        return old - 1;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   addi    %0, %0, -1      \n"
            "   sc.w    t3, %0, (%1)    \n"
            "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        return old + 1;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        register T old;
        ASM("1: lr.w    %0, (%1)        \n"
            "   bne     %0, %2, 2f      \n"
            "   sc.w    t3, %3, (%1)    \n"
            "   bnez    t3, 1b          \n"
            "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "t3", "cc", "memory");
        return old;
    }

    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

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

public:
    // RISC-V 32 specifics
    static Reg tp() { Reg r; ASM("mv %0, tp" : "=r"(r) :); return r; }
    static void tp(Reg r) { ASM("mv tp, %0" : : "r"(r) :); }

    // Machine mode
    static Reg mhartid() { Reg r; ASM("csrr %0, mhartid" : "=r"(r) : : "memory", "cc"); return r & 0x3; }

    static void mscratch(Reg r)   { ASM("csrw mscratch, %0" : : "r"(r) : "cc"); }
    static Reg  mscratch() { Reg r; ASM("csrr %0, mscratch" : "=r"(r) : : ); return r; }

    static void mstatus(Reg r)   { ASM("csrw mstatus, %0" : : "r"(r) : "cc"); }
    static void mstatusc(Reg r)  { ASM("csrc mstatus, %0" : : "r"(r) : "cc"); }
    static void mstatuss(Reg r)  { ASM("csrs mstatus, %0" : : "r"(r) : "cc"); }
    static Reg  mstatus() { Reg r; ASM("csrr %0, mstatus" : "=r"(r) : : ); return r; }

    static void mie(Reg r)   { ASM("csrw mie, %0" : : "r"(r) : "cc"); }
    static void miec(Reg r)  { ASM("csrc mie, %0" : : "r"(r) : "cc"); }
    static void mies(Reg r)  { ASM("csrs mie, %0" : : "r"(r) : "cc"); }
    static Reg  mie() { Reg r; ASM("csrr %0, mie" : "=r"(r) : : ); return r; }

    static void mip(Reg r)   { ASM("csrw mip, %0" : : "r"(r) : "cc"); }
    static void mipc(Reg r)  { ASM("csrc mip, %0" : : "r"(r) : "cc"); }
    static void mips(Reg r)  { ASM("csrs mip, %0" : : "r"(r) : "cc"); }
    static Reg  mip() { Reg r; ASM("csrr %0, mip" : "=r"(r) : : ); return r; }

    static Reg mcause() { Reg r; ASM("csrr %0, mcause" : "=r"(r) : : ); return r; }
    static Reg mtval()  { Reg r; ASM("csrr %0, mtval" : "=r"(r) : : ); return r; }

    static void mepc(Reg r)   { ASM("csrw mepc, %0" : : "r"(r) : "cc"); }
    static Reg  mepc() { Reg r; ASM("csrr %0, mepc" : "=r"(r) : : ); return r; }

    static void mret() { ASM("mret"); }

    static void mideleg(Reg value) { ASM("csrw mideleg, %0" : : "r"(value) : "cc"); }
    static void medeleg(Reg value) { ASM("csrw medeleg, %0" : : "r"(value) : "cc"); }

    // Supervisor mode
    static void sstatus(Reg r)   { ASM("csrw sstatus, %0" : : "r"(r) : "cc"); }
    static void sstatusc(Reg r)  { ASM("csrc sstatus, %0" : : "r"(r) : "cc"); }
    static void sstatuss(Reg r)  { ASM("csrs sstatus, %0" : : "r"(r) : "cc"); }
    static Reg  sstatus() { Reg r; ASM("csrr %0, sstatus" : "=r"(r) : : ); return r; }

    static void sie(Reg r)   { ASM("csrw sie, %0" : : "r"(r) : "cc"); }
    static void siec(Reg r)  { ASM("csrc sie, %0" : : "r"(r) : "cc"); }
    static void sies(Reg r)  { ASM("csrs sie, %0" : : "r"(r) : "cc"); }
    static Reg  sie() { Reg r; ASM("csrr %0, sie" : "=r"(r) : : ); return r; }

    static void sip(Reg r)   { ASM("csrw sip, %0" : : "r"(r) : "cc"); }
    static void sipc(Reg r)  { ASM("csrc sip, %0" : : "r"(r) : "cc"); }
    static void sips(Reg r)  { ASM("csrs sip, %0" : : "r"(r) : "cc"); }
    static Reg  sip() { Reg r; ASM("csrr %0, sip" : "=r"(r) : : ); return r; }

    static Reg scause() { Reg r; ASM("csrr %0, scause" : "=r"(r) : : ); return r; }
    static Reg stval()  { Reg r; ASM("csrr %0, stval" : "=r"(r) : : ); return r; }

    static void sepc(Reg r)   { ASM("csrw sepc, %0" : : "r"(r) : "cc"); }
    static Reg  sepc() { Reg r; ASM("csrr %0, sepc" : "=r"(r) : : ); return r; }

    static void sret() { ASM("sret"); }

    static void satp(Reg32 r) { ASM("csrw satp, %0" : : "r"(r) : "cc"); }
    static Reg  satp() { Reg r; ASM("csrr %0, satp" : "=r"(r) : : ); return r; }

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
