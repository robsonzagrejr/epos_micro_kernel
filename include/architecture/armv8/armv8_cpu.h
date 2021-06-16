// EPOS ARMv8 CPU Mediator Declarations

#ifndef __armv8_h
#define __armv8_h

#define __cpu_common_only__
#include <architecture/cpu.h>
#include <architecture/armv7/armv7_cpu.h>
#undef __cpu_common_only__

__BEGIN_SYS

class ARMv8_A: public ARMv7_A
{
protected:
    ARMv8_A() {};

public:
    static unsigned int cores() {
        // Cortex A53 cannot execute "mrc p15, 4, r0, c15, c0, 0".
        // The amount of cores booted is equal to the Traits value (defined at Raspberry_Pi3::pre_init::pre_init for instance, up to four)
        return Traits<Build>::CPUS; 
    }

    // We need to redefine because we also redefined cores()
    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }
};

class CPU: private ARMv8_A
{
    friend class Init_System;

private:
    typedef ARMv8_A Base;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using Reg = CPU_Common::Reg32;
    using Log_Addr = CPU_Common::Log_Addr<Reg>;
    using Phy_Addr = CPU_Common::Phy_Addr<Reg>;

    // CPU Context
    class Context
    {
    public:
        Context(){}
        Context(Log_Addr  entry, Log_Addr exit, Log_Addr usp): _flags(FLAG_DEFAULTS), _lr(exit | (thumb ? 1 : 0)), _pc(entry | (thumb ? 1 : 0)) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                _r0 = 0; _r1 = 1; _r2 = 2; _r3 = 3; _r4 = 4; _r5 = 5; _r6 = 6; _r7 = 7; _r8 = 8; _r9 = 9; _r10 = 10; _r11 = 11; _r12 = 12;
            }
        }

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

    using Base::pc;
    using Base::flags;
    using Base::sp;
    using Base::fr;
    using Base::pdp;

    using Base::id;
    using Base::cores;

    static Hertz clock() { return _cpu_clock; }
    static void clock(const Hertz & frequency); // defined along with each machine's IOCtrl
    static Hertz max_clock();
    static Hertz min_clock();

    static Hertz bus_clock() { return _bus_clock; }

    using Base::int_enable;
    using Base::int_disable;
    using Base::int_enabled;
    using Base::int_disabled;

    using Base::halt;

    using Base::tsl;
    using Base::finc;
    using Base::fdec;
    using Base::cas;

    using Base::smp_barrier;

    static void fpu_save() {
        if(Traits<Build>::MODEL == Traits<Build>::Raspberry_Pi3)
            ASM("       vpush    {s0-s15}               \n"
                "       vpush    {s16-s31}              \n");
    }

    static void fpu_restore() {
        if(Traits<Build>::MODEL == Traits<Build>::Raspberry_Pi3)
            ASM("       vpop    {s0-s15}                \n"
                "       vpop    {s16-s31}               \n");
    }

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(Log_Addr usp, Log_Addr ksp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        ksp -= sizeof(Context);
        Context * ctx = new(ksp) Context(entry, exit, usp);
        init_stack_helper(&ctx->_r0, an ...);
        return ctx;
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr usp, void (* exit)(), Tn ... an) {
        usp -= sizeof(Context);
        Context * ctx = new(usp) Context(0, exit, 0);
        init_stack_helper(&ctx->_r0, an ...);
        return usp;
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
