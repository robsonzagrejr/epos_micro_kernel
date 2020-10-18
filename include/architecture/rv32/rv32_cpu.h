// EPOS RISC-V 32 CPU Mediator Declarations

#ifndef __riscv32_h
#define __riscv32_h

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
    using Reg = CPU_Common::Reg32;
    using Log_Addr = CPU_Common::Log_Addr<Reg>;
    using Phy_Addr = CPU_Common::Phy_Addr<Reg>;

    static const bool thumb = true;

    // CPU Flags
    typedef Reg32 Flags;
    enum {
        //implement
    };

    // CPU Context
    class Context
    {
    public:
        Context(const Log_Addr & entry, const Log_Addr & exit): reg_flags(FLAG_DEFAULTS), reg_ra(exit), reg_ip(entry) {}

        void save() volatile  __attribute__ ((naked));
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{REG NAME" << 0 //registers
               << "}" << dec;
            return db;
        }

    public:
        Reg32 reg_flags;
        Reg32 _general_use_reg0; //all registers that must be saved
        Reg32 _reg_ra; // return address register
        Reg32 _reg_ip; //instruction pointer
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
        //implement
        return 0;
    }

    static void sp(const Reg32 & sp) {
        //implement
    }

    static Reg32 fr() {
        //implement
        return 0;
    }

    static void fr(const Reg32 & fr) {
        //implement
    }

    static Log_Addr ip() {
        //implement
        return 0;
    }

    static Reg32 pdp() { return 0; }
    static void pdp(const Reg32 & pdp) {}


    // Atomic operations

    using CPU_Common::tsl;
    /*
    template<typename T>
    static T tsl(volatile T & lock) {
        //implement
    }
    */

    using CPU_Common::finc;
    /*
    template<typename T>
    static T finc(volatile T & value) {
        //implement
    }
    */

    using CPU_Common::fdec;
    /*
    template<typename T>
    static T fdec(volatile T & value) {
        //implement
    }
    */

    using CPU_Common::cas;
    /*
    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        //implement
    }
    */

    // Power modes
    static void halt() {
        //implement
    }

    //implement
    static Flags flags() {
        //implement
        return 0;
    }

    static void flags(const Flags & flags) {
        //implement
    }

    static unsigned int id() {
        //implement
        return 0;
    }

    static unsigned int cores() {
        return Traits<Build>::CPUS;
    }

    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void int_enable() { /* implement */ }
    static void int_disable() { /* implement */ }

    static bool int_enabled() { /* implement */ return 0; }
    static bool int_disabled() { /* implement */ return !int_enabled(); }

    static void csrr31() { /* implement - write ctrl and status register to x31 */ }
    static void csrw31() { /* implement - write x31 to ctrl and status register */ }

    static unsigned int int_id() { return 0; }

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(const Log_Addr & usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit);
        init_stack_helper(&ctx->_general_use_reg0, an ...);
        return ctx;
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr sp, void (* exit)(), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(0, exit);
        init_stack_helper(&ctx->_general_use_reg0, an ...);
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
