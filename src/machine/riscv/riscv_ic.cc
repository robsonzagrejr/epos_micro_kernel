// EPOS RISC-V IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>
#include <machine/timer.h>

extern "C" { void _int_entry() __attribute__ ((nothrow, alias("_ZN4EPOS1S2IC5entryEv"))); }

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];

// Class methods
void IC::entry()
{
    ASM("# Save context                                                 \n"
        "        addi        sp,     sp,   -132                         \n"
        "        sw          x1,   4(sp)                                \n"
        "        sw          x2,   8(sp)                                \n"
        "        sw          x3,  12(sp)                                \n"     // we don't save x4 (tp) because it can change across context switches and it is not ment be used by the compiler at application-level.
        "        sw          x5,  16(sp)                                \n"
        "        sw          x6,  20(sp)                                \n"
        "        sw          x7,  24(sp)                                \n"
        "        sw          x8,  28(sp)                                \n"
        "        sw          x9,  32(sp)                                \n"
        "        sw         x10,  36(sp)                                \n"
        "        sw         x11,  40(sp)                                \n"
        "        sw         x12,  44(sp)                                \n"
        "        sw         x13,  48(sp)                                \n"
        "        sw         x14,  52(sp)                                \n"
        "        sw         x15,  56(sp)                                \n"
        "        sw         x16,  60(sp)                                \n"
        "        sw         x17,  64(sp)                                \n"
        "        sw         x18,  68(sp)                                \n"
        "        sw         x19,  72(sp)                                \n"
        "        sw         x20,  76(sp)                                \n"
        "        sw         x21,  80(sp)                                \n"
        "        sw         x22,  84(sp)                                \n"
        "        sw         x23,  88(sp)                                \n"
        "        sw         x24,  92(sp)                                \n"
        "        sw         x25,  96(sp)                                \n"
        "        sw         x26, 100(sp)                                \n"
        "        sw         x27, 104(sp)                                \n"
        "        sw         x28, 108(sp)                                \n"
        "        sw         x29, 112(sp)                                \n"
        "        sw         x30, 116(sp)                                \n"
        "        sw         x31, 120(sp)                                \n");
if(sup)
    ASM("        csrr       x31, sstatus                                \n"
        "        sw         x31, 124(sp)                                \n"
        "        csrr       x31, sepc                                   \n"
        "        sw         x31, 128(sp)                                \n");
else
    ASM("        csrr       x31, mstatus                                \n"
        "        sw         x31, 124(sp)                                \n"
        "        csrr       x31, mepc                                   \n"
        "        sw         x31, 128(sp)                                \n");

    ASM("        la          ra, .restore                               \n" // set LR to restore context before returning
        "        j          %0                                          \n"
        "                                                               \n"
        "# Restore context                                              \n"
        ".restore:                                                      \n"
        "        lw          x1,   4(sp)                                \n"
        "        lw          x2,   8(sp)                                \n"
        "        lw          x3,  12(sp)                                \n"
        "        lw          x5,  16(sp)                                \n"
        "        lw          x6,  20(sp)                                \n"
        "        lw          x7,  24(sp)                                \n"
        "        lw          x8,  28(sp)                                \n"
        "        lw          x9,  32(sp)                                \n"
        "        lw         x10,  36(sp)                                \n"
        "        lw         x11,  40(sp)                                \n"
        "        lw         x12,  44(sp)                                \n"
        "        lw         x13,  48(sp)                                \n"
        "        lw         x14,  52(sp)                                \n"
        "        lw         x15,  56(sp)                                \n"
        "        lw         x16,  60(sp)                                \n"
        "        lw         x17,  64(sp)                                \n"
        "        lw         x18,  68(sp)                                \n"
        "        lw         x19,  72(sp)                                \n"
        "        lw         x20,  76(sp)                                \n"
        "        lw         x21,  80(sp)                                \n"
        "        lw         x22,  84(sp)                                \n"
        "        lw         x23,  88(sp)                                \n"
        "        lw         x24,  92(sp)                                \n"
        "        lw         x25,  96(sp)                                \n"
        "        lw         x26, 100(sp)                                \n"
        "        lw         x27, 104(sp)                                \n"
        "        lw         x28, 108(sp)                                \n"
        "        lw         x29, 112(sp)                                \n"
        "        lw         x30, 116(sp)                                \n" : : "i"(&dispatch));
if(sup)
    ASM("        lw         x31, 124(sp)                                \n"
        "        csrw   sstatus, x31                                    \n"
        "        lw         x31, 128(sp)                                \n"
        "        csrw      sepc, x31                                    \n");
else
    ASM("        lw         x31, 124(sp)                                \n"
        "        csrw   mstatus, x31                                    \n"
        "        lw         x31, 128(sp)                                \n"
        "        csrw      mepc, x31                                    \n");

    ASM("        lw         x31, 120(sp)                                \n"
        "        addi        sp, sp,    132                             \n");
if(sup)
    ASM("        sret                                                   \n");
else
    ASM("        mret                                                   \n");
}

void IC::dispatch()
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    if(sup) {
        if(id == INT_RESCHEDULER)
            CPU::sipc(CPU::SSI);

        if(id == INT_SYS_TIMER)
            CPU::siec(CPU::STI);
    } else {
        // IPIs must be acknowledged before calling the ISR, because in RISC-V, MIP set bits will keep on triggering interrupts until they are cleared
        if(id == INT_RESCHEDULER)
            IC::ipi_eoi(id);

        // MIP.MTI is a direct logic on (MTIME == MTIMECMP) and reseting the Timer clears it
        if(id == INT_SYS_TIMER)
            Timer::reset();
    }

    _int_vector[id](id);
}

void IC::int_not(Interrupt_Id id)
{
    db<IC>(WRN) << "IC::int_not(i=" << id << ")";
    if(Traits<Build>::hysterically_debugged)
        db<IC>(ERR) << endl;
    else
        db<IC>(WRN) << endl;
}

void IC::exception(Interrupt_Id id)
{
    CPU::Reg status = sup ? CPU::sstatus() : CPU::mstatus();
    CPU::Reg cause = sup? CPU::scause() : CPU::mcause();
    CPU::Reg hartid = CPU::id();
    CPU::Reg epc = sup ? CPU::sepc() : CPU::mepc();
    CPU::Reg tval = sup ? CPU::stval() : CPU::mtval();
    CPU::Reg sp = CPU::sp();

    db<IC,System>(WRN) << "IC::Exception(" << id << ") => {" << hex << "status=" << status << ",cause=" << cause << ",hartid=" << hartid << ",epc=" << epc << ",tval=" << tval << ",sp=" << sp <<  "}" << dec;

    switch(id) {
        case 0: // unaligned Instruction
        case 1: // instruction access failure
            db<IC, System>(WRN) << " => prefetch abort";
            break;
        case 2: // illegal instruction
            db<IC, System>(WRN) << " => illegal instruction";
            break;
        case 3: // Break Point
            db<IC, System>(WRN) << " => break point";
            break;
        case 4: // unaligned load address
        case 5: // load access failure
        case 6: // unaligned store address
        case 7: // store access failure
            db<IC, System>(WRN) << " => data error (unaligned)";
            break;
        case 8: // user-mode environment call
        case 9: // supervisor-mode environment call
        case 10: // reserved... not described
        case 11: // machine-mode environment call
            db<IC, System>(WRN) << " => reserved";
            break;
        case 12: // Instruction Page Table failure
        case 13: // Load Page Table failure
        case 14: // reserved... not described
        case 15: // Store Page Table failure
            db<IC, System>(WRN) << " => data abort";
            break;
        default:
            int_not(id);
            break;
    }

    if(Traits<Build>::hysterically_debugged)
        db<IC, System>(ERR) << endl;
    else
        db<IC, System>(WRN) << endl;
}

__END_SYS
