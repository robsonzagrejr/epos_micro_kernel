// EPOS RISC-V IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];

// Class methods
void IC::entry()
{
    // Handle interrupts in machine mode
    ASM("        .align 4                                               \n"
        "                                                               \n"
        "# Save context                                                 \n"
        "        addi        sp,     sp,   -140                         \n"          // 32 regs of 4 bytes each = 128 Bytes
        "        sw          x1,   4(sp)                                \n"
        "        sw          x2,   8(sp)                                \n"
        "        sw          x3,  12(sp)                                \n"
        "        sw          x4,  16(sp)                                \n"
        "        sw          x5,  20(sp)                                \n"
        "        sw          x6,  24(sp)                                \n"
        "        sw          x7,  28(sp)                                \n"
        "        sw          x8,  32(sp)                                \n"
        "        sw          x9,  36(sp)                                \n"
        "        sw         x10,  40(sp)                                \n"
        "        sw         x11,  44(sp)                                \n"
        "        sw         x12,  48(sp)                                \n"
        "        sw         x13,  52(sp)                                \n"
        "        sw         x14,  56(sp)                                \n"
        "        sw         x15,  60(sp)                                \n"
        "        sw         x16,  64(sp)                                \n"
        "        sw         x17,  68(sp)                                \n"
        "        sw         x18,  72(sp)                                \n"
        "        sw         x19,  76(sp)                                \n"
        "        sw         x20,  80(sp)                                \n"
        "        sw         x21,  84(sp)                                \n"
        "        sw         x22,  88(sp)                                \n"
        "        sw         x23,  92(sp)                                \n"
        "        sw         x24,  96(sp)                                \n"
        "        sw         x25, 100(sp)                                \n"
        "        sw         x26, 104(sp)                                \n"
        "        sw         x27, 108(sp)                                \n"
        "        sw         x28, 112(sp)                                \n"
        "        sw         x29, 116(sp)                                \n"
        "        sw         x30, 120(sp)                                \n"
        "        sw         x31, 124(sp)                                \n"
        "        csrr       x31, mie                                    \n"
        "        sw         x31, 128(sp)                                \n"
        "        csrr       x31, mstatus                                \n"
        "        sw         x31, 132(sp)                                \n"
        "        csrr       x31, mepc                                   \n"
        "        sw         x31, 136(sp)                                \n"
        "        la          ra, .restore                               \n" // Set LR to restore context before returning
        "        j          %0                                          \n"
        "                                                               \n"
        "# Restore context                                              \n"
        ".restore:                                                      \n"
        "        lw          x1,   4(sp)                                \n"
        "        lw          x2,   8(sp)                                \n"
        "        lw          x3,  12(sp)                                \n"
        "        lw          x4,  16(sp)                                \n"
        "        lw          x5,  20(sp)                                \n"
        "        lw          x6,  24(sp)                                \n"
        "        lw          x7,  28(sp)                                \n"
        "        lw          x8,  32(sp)                                \n"
        "        lw          x9,  36(sp)                                \n"
        "        lw         x10,  40(sp)                                \n"
        "        lw         x11,  44(sp)                                \n"
        "        lw         x12,  48(sp)                                \n"
        "        lw         x13,  52(sp)                                \n"
        "        lw         x14,  56(sp)                                \n"
        "        lw         x15,  60(sp)                                \n"
        "        lw         x16,  64(sp)                                \n"
        "        lw         x17,  68(sp)                                \n"
        "        lw         x18,  72(sp)                                \n"
        "        lw         x19,  76(sp)                                \n"
        "        lw         x20,  80(sp)                                \n"
        "        lw         x21,  84(sp)                                \n"
        "        lw         x22,  88(sp)                                \n"
        "        lw         x23,  92(sp)                                \n"
        "        lw         x24,  96(sp)                                \n"
        "        lw         x25, 100(sp)                                \n"
        "        lw         x26, 104(sp)                                \n"
        "        lw         x27, 108(sp)                                \n"
        "        lw         x28, 112(sp)                                \n"
        "        lw         x29, 116(sp)                                \n"
        "        lw         x30, 120(sp)                                \n"
        "        lw         x31, 124(sp)                                \n"
        "        lw         x31, 128(sp)                                \n"
        "        csrs       mie, x31                                    \n"
        "        lw         x31, 132(sp)                                \n"
        "        csrs   mstatus, x31                                    \n"
        "        lw         x31, 136(sp)                                \n"
        "        csrw      mepc, x31                                    \n"
        "        addi        sp, sp,    140                             \n"
        "        mret                                                   \n" : : "i"(&dispatch));
}

void IC::dispatch()
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    if(id == INT_RESCHEDULER)
        IC::ipi_eoi(id);

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
    CPU::Reg mstatus = CPU::mstatus();
    CPU::Reg mcause = CPU::mcause();
    CPU::Reg mhartid = CPU::id();
    CPU::Reg mepc;
    ASM("csrr %0, mepc" : "=r"(mepc) : :);
    CPU::Reg sepc;
    ASM("csrr %0, sepc" : "=r"(sepc) : :);
    CPU::Reg mtval;
    ASM("csrr %0, mtval" : "=r"(mtval) : :);

    db<IC>(WRN) << "IC::Exception(" << id << ") => {" << hex << "mstatus=" << mstatus << ",mcause=" << mcause << ",mhartid=" << mhartid << ",mepc=" << hex << mepc << ",sepc=" << sepc << ",mtval=" << mtval << "}" << dec;

    switch(id) {
        case 0: // unaligned Instruction
        case 1: // instruction access failure
            db<IC>(WRN) << " => prefetch abort";
            break;
        case 2: // illegal instruction
            db<IC>(WRN) << " => illegal instruction";
            break;
        case 3: // Break Point
            db<IC>(WRN) << " => break point";
            break;
        case 4: // unaligned load address
        case 5: // load access failure
        case 6: // unaligned store address
        case 7: // store access failure
            db<IC>(WRN) << " => unaligned data";
            break;
        case 8: // user-mode environment call
        case 9: // supervisor-mode environment call
        case 10: // reserved... not described
        case 11: // machine-mode environment call
            db<IC>(WRN) << " => reserved";
            break;
        case 12: // Instruction Page Table failure
        case 13: // Load Page Table failure
        case 14: // reserved... not described
        case 15: // Store Page Table failure
            db<IC>(WRN) << " => data abort";
            break;
        default:
            int_not(id);
            break;
    }

    if(Traits<Build>::hysterically_debugged)
        db<IC>(ERR) << endl;
    else
        db<IC>(WRN) << endl;
}

__END_SYS
