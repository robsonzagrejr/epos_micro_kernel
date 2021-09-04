// EPOS ARMv7 CPU Mediator Declarations

#ifndef __armv7_h
#define __armv7_h

#include <architecture/cpu.h>

__BEGIN_SYS

class ARMv7: protected CPU_Common
{
    friend class Init_System; // for CPU::init()

protected:
    static const bool multicore = Traits<System>::multicore;
    static const bool multitask = Traits<System>::multitask;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using CPU_Common::Reg;
    using CPU_Common::Log_Addr;
    using CPU_Common::Phy_Addr;

    class Context
    {
    public:
        Context() {}
        Context(Log_Addr usp, Log_Addr ulr, Reg flags, Log_Addr  lr, Log_Addr pc): _usp(usp), _ulr(ulr), _flags(flags), _lr(lr), _pc(pc) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                _r0 = 0; _r1 = 1; _r2 = 2; _r3 = 3; _r4 = 4; _r5 = 5; _r6 = 6; _r7 = 7; _r8 = 8; _r9 = 9; _r10 = 10; _r11 = 11; _r12 = 12;
            }
        }

    public:
        Reg _usp;     // usp (only used in multitasking)
        Reg _ulr;     // ulr (only used in multitasking)
        Reg _flags;
        Reg _r0;
        Reg _r1;
        Reg _r2;
        Reg _r3;
        Reg _r4;
        Reg _r5;
        Reg _r6;
        Reg _r7;
        Reg _r8;
        Reg _r9;
        Reg _r10;
        Reg _r11;
        Reg _r12;
        Reg _lr;
        Reg _pc;
    };

    // Interrupt Service Routines
    typedef void (* ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (* FSR)();

protected:
    ARMv7() {};

public:
    static Log_Addr pc() { Reg r; ASM("mov %0, pc" : "=r"(r) :); return r; } // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset

    static Log_Addr sp() { Reg r; ASM("mov %0, sp" : "=r"(r) :); return r; }
    static void sp(Log_Addr sp) { ASM("mov sp, %0" : : "r"(Reg(sp))); ASM("isb"); }

    static Reg fr() { Reg r; ASM("mov %0, r0" : "=r"(r)); return r; }
    static void fr(Reg r) {  ASM("mov r0, %0" : : "r"(r) : "r0"); }

    static Log_Addr ra() { Reg r; ASM("mov %0, lr" : "=r"(r) :); return r; } // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset

    static void halt() { ASM("wfi"); }

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

    // ARMv7 specifics
    static Reg r0() { Reg r; ASM("mov %0, r0" : "=r"(r) : : ); return r; }
    static void r0(Reg r) { ASM("mov r0, %0" : : "r"(r): ); }

    static Reg r1() { Reg r; ASM("mov %0, r1" : "=r"(r) : : ); return r; }
    static void r1(Reg r) { ASM("mov r1, %0" : : "r"(r): ); }

    static Reg sctlr() { Reg r; ASM("mrc p15, 0, %0, c1, c0, 0" : "=r"(r)); return r; }
    static void sctlr(Reg r) {  ASM("mcr p15, 0, %0, c1, c0, 0" : : "r"(r) : "r0"); }

    static Reg actlr() { Reg r; ASM("mrc p15, 0, %0, c1, c0, 1" : "=r"(r)); return r; }
    static void actlr(Reg r) {  ASM("mcr p15, 0, %0, c1, c0, 1" : : "r"(r) : "r0"); }

    static void dsb() { ASM("dsb"); }
    static void isb() { ASM("isb"); }

    static void svc() { ASM("svc 0x0"); }
};

class ARMv7_M: public ARMv7
{
public:
    static const bool thumb = true;

    // CPU Flags
    typedef Reg Flags;
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
    typedef Reg Exception_Id;
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

    // CPU Context
    class Context: public ARMv7::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): ARMv7::Context(usp, exit | thumb, FLAG_DEFAULTS, exit | thumb, entry | thumb) {}
    };

protected:
    ARMv7_M() {};

public:
    static unsigned int id() { return 0; }
    static unsigned int cores() { return 1; }

    static void int_enable()  { ASM("cpsie i"); }
    static void int_disable() { ASM("cpsid i"); }
    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() {
        bool disabled;
        ASM("mrs %0, primask" : "=r"(disabled));
        return disabled;
    }

    static void smp_barrier(unsigned long cores = cores()) { assert(cores == 1); }

    static Reg pd() { return 0; }       // no MMU
    static void pd(Reg r) {}            // no MMU

    static void flush_tlb() {}          // no MMU
    static void flush_tlb(Reg r) {}     // no MMU

    // ARMv7-M specifics
    static Flags flags() { Reg r; ASM("mrs %0, xpsr"       : "=r"(r) :); return r; }
    static void flags(Flags r) {  ASM("msr xpsr_nzcvq, %0" : : "r"(r) : "cc"); }

    static void psr_to_r12() { ASM("mrs r12, xpsr" : : : "r12"); }
    static void r12_to_psr() {  ASM("msr xpsr_nzcvq, r12" : : : "cc"); }
};

class ARMv7_A: public ARMv7
{
public:
    static const bool thumb = false;

    // CPU Flags
    typedef Reg Flags;
    enum {
        FLAG_M          = 0x1f << 0,       // Processor Mode (5 bits)
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
    typedef Reg Exception_Id;
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

    enum {
        CLI_DOMAIN = 0x55555555, // 0b01 - Client, all memory domains check for memory access permission
        MNG_DOMAIN = 0xFFFFFFFF  // 0b11 - Manager, memory access permissions are not checked
    };

    // SCTLR bits
    enum {
        MMU_ENABLE  = 1 << 0,  // MMU enable
        DCACHE      = 1 << 2,  // Data cache enable
        BRANCH_PRED = 1 << 11, // Z bit, branch prediction enable
        ICACHE      = 1 << 12, // Instruction cache enable
        AFE         = 1 << 29  // Access Flag enable
    };

    // ACTLR bits
    enum {
        DCACHE_PREFE = 1 << 2, // DCache prefetch Enabled
        SMP          = 1 << 6 // SMP bit
    };

    // CPU Context
    class Context: public ARMv7::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): ARMv7::Context(usp, exit | thumb, multitask ? (usp ? FLAG_USER : FLAG_DEFAULTS) : FLAG_DEFAULTS, exit | thumb, entry | thumb) {}
    };

protected:
    ARMv7_A() {};

public:
    static Flags flags() { return cpsr(); }
    static void flags(Flags flags) { cpsr(flags); }

    static unsigned int id() {
        Reg id;
        ASM("mrc p15, 0, %0, c0, c0, 5" : "=r"(id) : : );
        return id & 0x3;
    }

    static unsigned int cores() {
        if(Traits<Build>::MODEL == Traits<Build>::Raspberry_Pi3) {
            return Traits<Build>::CPUS;
        } else {
            Reg n;
            ASM("mrc p15, 4, %0, c15, c0, 0 \t\n\
                 ldr %0, [%0, #0x004]" : "=r"(n) : : );
            return (n & 0x3) + 1;
        }
    }

    static void int_enable() {  flags(flags() & ~(FLAG_F | FLAG_I)); }
    static void int_disable() { flags(flags() | (FLAG_F | FLAG_I)); }

    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() { return flags() & (FLAG_F | FLAG_I); }

    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void fpu_save() {    ASM("vpush {s0-s15} \n vpush {s16-s31}"); }
    static void fpu_restore() { ASM("vpop  {s0-s15} \n vpop  {s16-s31}"); }

    // ARMv7-A specifics
    static void psr_to_r12() { ASM("mrs r12, cpsr" : : : "r12"); }
    static void r12_to_psr() { ASM("msr cpsr, r12" : : : "cc"); }

    static Reg cpsr() { Reg r; ASM("mrs %0, cpsr" : "=r"(r) : : ); return r; }
    static void cpsr(Reg r) { ASM("msr cpsr, %0" : : "r"(r) : "cc"); }

    static Reg cpsrc() { Reg r; ASM("mrs %0, cpsr_c" : "=r"(r) : : ); return r; }
    static void cpsrc(Reg r) { ASM("msr cpsr_c, %0" : : "r"(r): ); }

    static Reg elr_hyp() { Reg r; ASM("mrs %0, ELR_hyp" : "=r"(r) : : ); return r; }
    static void elr_hyp(Reg r) { ASM("msr ELR_hyp, %0" : : "r"(r): ); }

    static void ldmia() { ASM("ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}" : : : ); }
    static void stmia() { ASM("stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}" : : : ); }

    // CP15 operations
    static Reg ttbr0() { Reg r; ASM ("mrc p15, 0, %0, c2, c0, 0" : "=r"(r) : :); return r; }
    static void ttbr0(Reg r) {  ASM ("mcr p15, 0, %0, c2, c0, 0" : : "p"(r) :); }

    static Reg ttbcr() { Reg r; ASM ("mrc p15, 0, %0, c2, c0, 2" : "=r"(r) : :); return r; }
    static void ttbcr(Reg r) {  ASM ("mcr p15, 0, %0, c2, c0, 2" : : "p"(r) :); }

    static Reg dacr() { Reg r; ASM ("mrc p15, 0, %0, c3, c0, 0" : "=r"(r) : :); return r; }
    static void dacr(Reg r) {  ASM ("mcr p15, 0, %0, c3, c0, 0" : : "p"(r) :); }

    static Reg pd() { return ttbr0(); }
    static void pd(Reg r) {  ttbr0(r); }

    static void flush_tlb() {      ASM("mcr p15, 0, %0, c8, c7, 0" : : "r" (0)); } // TLBIALL - invalidate entire unifed TLB
    static void flush_tlb(Reg r) { ASM("mcr p15, 0, %0, c8, c7, 0" : : "r" (r)); }

    static void flush_branch_predictors() { ASM("mcr p15, 0, %0, c7, c5, 6" : : "r" (0)); }

    static void flush_caches() {
        ASM("// Disable L1 Caches                                                                       \t\n\
             mrc     p15, 0, r1, c1, c0, 0      // read SCTLR                                           \t\n\
             bic     r1, r1, #(0x1 << 2)        // disable D Cache                                      \t\n\
             mcr     p15, 0, r1, c1, c0, 0      // write SCTLR                                          \t\n\
                                                                                                        \t\n\
             // Invalidate Data cache, calculating the cache size and looping through each set and way  \t\n\
             mov     r0, #0x0                   // r0 = 0x0 for L1 dcache 0x2 for L2 dcache             \t\n\
             mcr     p15, 2, r0, c0, c0, 0      // CSSELR cache size selection Register                 \t\n\
             mrc     p15, 1, r4, c0, c0, 0      // CCSIDR read cache size                               \t\n\
             and     r1, r4, #0x7                                                                       \t\n\
             add     r1, r1, #0x4               // r1 = cache line size                                 \t\n\
             ldr     r3, =0x7fff                                                                        \t\n\
             and     r2, r3, r4, lsr #13        // r2 = cache set number - 1                            \t\n\
             ldr     r3, =0x3ff                                                                         \t\n\
             and     r3, r3, r4, lsr #3         // r3 = cache associativity number - 1                  \t\n\
             clz     r4, r3                     // r4 = way position in CISW instruction                \t\n\
             mov     r5, #0                     // r5 = way loop counter                                \t\n\
         way_loop:                                                                                      \t\n\
             mov     r6, #0                     // r6 = set loop counter                                \t\n\
         set_loop:                                                                                      \t\n\
             orr     r7, r0, r5, lsl r4         // set way                                              \t\n\
             orr     r7, r7, r6, lsl r1         // set set                                              \t\n\
             mcr     p15, 0, r7, c7, c6, 2      // DCCISW r7                                            \t\n\
             add     r6, r6, #1                 // increment set counter                                \t\n\
             cmp     r6, r2                     // last set reached?                                    \t\n\
             ble     set_loop                   // if not, iterate set_loop                             \t\n\
             add     r5, r5, #1                 // else, next way                                       \t\n\
             cmp     r5, r3                     // last way reached?                                    \t\n\
             ble     way_loop                   // if not, iterate way_loop                                  ");
    }

    static void enable_fpu() {
        // This code assumes a compilation with mfloat-abi=hard and does not care for context switches
        ASM("mrc     p15, 0, r0, c1, c0, 2                                              \t\n\
             orr     r0, r0, #0x300000           // single precision                    \t\n\
             orr     r0, r0, #0xc00000           // double precision                    \t\n\
             mcr     p15, 0, r0, c1, c0, 2                                              \t\n\
             mov     r0, #0x40000000                                                    \t\n\
             fmxr    fpexc,r0                                                                ");
    }

};

#ifndef __armv8_h

class CPU: public SWITCH<Traits<Build>::MODEL, CASE<Traits<Build>::eMote3, ARMv7_M, CASE<Traits<Build>::LM3S811, ARMv7_M, CASE<DEFAULT, ARMv7_A>>>>::Result
{
    friend class Init_System;

private:
    typedef SWITCH<Traits<Build>::MODEL, CASE<Traits<Build>::eMote3, ARMv7_M, CASE<Traits<Build>::LM3S811, ARMv7_M, CASE<DEFAULT, ARMv7_A>>>>::Result Base;

public:
    // CPU Native Data Types
    using ARMv7::Reg8;
    using ARMv7::Reg16;
    using ARMv7::Reg32;
    using ARMv7::Reg64;
    using ARMv7::Reg;
    using ARMv7::Log_Addr;
    using ARMv7::Phy_Addr;

    // CPU Context
    class Context: public Base::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): Base::Context(entry, exit, usp) {}

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
               << ",usp=" << c._usp
               << ",ulr=" << c._ulr
               << "}" << dec;
            return db;
        }
    };

public:
    CPU() {}

    using ARMv7::pc;
    using ARMv7::ra;
    using ARMv7::sp;
    using ARMv7::fr;

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

    using ARMv7::halt;

    using Base::fpu_save;
    using Base::fpu_restore;

    using ARMv7::tsl;
    using ARMv7::finc;
    using ARMv7::fdec;
    using ARMv7::cas;

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(Log_Addr usp, Log_Addr ksp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        ksp -= sizeof(Context);
        Context * ctx = new(ksp) Context(entry, exit, usp); // init_stack is called with usp = 0 for kernel threads
        init_stack_helper(&ctx->_r0, an ...);
        if(multitask) {

	// handle trampoline context here

        }
        return ctx;
    }

    // In ARMv7, the main thread of each task gets parameters over registers, not the stack, and they are initialized by init_stack.
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr usp, void (* exit)(), Tn ... an) { return usp; }

    static void syscall(void * message);
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

    static void context_load_helper();

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

inline CPU::Reg32 htonl(CPU::Reg32 v)   { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v)   { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v)   { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v)   { return CPU::ntohs(v); }

#endif

__END_SYS

#endif
