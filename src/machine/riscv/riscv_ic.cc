// EPOS RISC-V IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }
extern "C" { void _dispatch(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC8dispatchEv"))); }
extern "C" { void _undefined_instruction() __attribute__ ((alias("_ZN4EPOS1S2IC21undefined_instructionEj"))); }
extern "C" { void _software_interrupt() __attribute__ ((alias("_ZN4EPOS1S2IC18software_interruptEj"))); }
extern "C" { void _prefetch_abort() __attribute__ ((alias("_ZN4EPOS1S2IC14prefetch_abortEj"))); }
extern "C" { void _data_abort() __attribute__ ((alias("_ZN4EPOS1S2IC10data_abortEj"))); }
extern "C" { void _reserved() __attribute__ ((alias("_ZN4EPOS1S2IC8reservedEj"))); }
extern "C" { void _fiq() __attribute__ ((alias("_ZN4EPOS1S2IC3fiqEj"))); }
extern "C" { void _exception_handling() __attribute__ ((alias("_ZN4EPOS1S2IC18exception_handlingEv"))); }

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];

// Class methods
void IC::entry()
{
    // Handle interrupts in machine mode
    ASM("       .align 4                                            \n"
        // Save context
        "	addi     sp,      sp,   -128                        \n"     // 32 regs of 4 bytes each = 128 Bytes
        "	sw	 x1,   4(sp)                                \n"
        "	sw	 x2,   8(sp)                                \n"
        "	sw	 x3,  12(sp)                                \n"
        "	sw	 x4,  16(sp)                                \n"
        "	sw	 x5,  20(sp)                                \n"
        "	sw	 x6,  24(sp)                                \n"
        "	sw	 x7,  28(sp)                                \n"
        "	sw	 x8,  32(sp)                                \n"
        "	sw	 x9,  36(sp)                                \n"
        "	sw	x10,  40(sp)                                \n"
        "	sw	x11,  44(sp)                                \n"
        "	sw	x12,  48(sp)                                \n"
        "	sw	x13,  52(sp)                                \n"
        "	sw	x14,  56(sp)                                \n"
        "	sw	x15,  60(sp)                                \n"
        "	sw	x16,  64(sp)                                \n"
        "	sw	x17,  68(sp)                                \n"
        "	sw	x18,  72(sp)                                \n"
        "	sw	x19,  76(sp)                                \n"
        "	sw	x20,  80(sp)                                \n"
        "	sw	x21,  84(sp)                                \n"
        "	sw	x22,  88(sp)                                \n"
        "	sw	x23,  92(sp)                                \n"
        "	sw	x24,  96(sp)                                \n"
        "	sw	x25, 100(sp)                                \n"
        "	sw	x26, 104(sp)                                \n"
        "	sw	x27, 108(sp)                                \n"
        "	sw	x28, 112(sp)                                \n"
        "	sw	x29, 116(sp)                                \n"
        "	sw	x30, 120(sp)                                \n"
        "	sw	x31, 124(sp)                                \n"
        "	la       ra, restore                                \n" // Set LR to restore context before returning
        "	j       _dispatch                                   \n"
        // Restore context
        "restore:                                                   \n"
        "	lw	  x1,   4(sp)                               \n"
        "	lw	  x2,   8(sp)                               \n"
        "	lw	  x3,  12(sp)                               \n"
        "	lw	  x4,  16(sp)                               \n"
        "	lw	  x5,  20(sp)                               \n"
        "	lw	  x6,  24(sp)                               \n"
        "	lw	  x7,  28(sp)                               \n"
        "	lw	  x8,  32(sp)                               \n"
        "	lw	  x9,  36(sp)                               \n"
        "	lw	 x10,  40(sp)                               \n"
        "	lw	 x11,  44(sp)                               \n"
        "	lw	 x12,  48(sp)                               \n"
        "	lw	 x13,  52(sp)                               \n"
        "	lw	 x14,  56(sp)                               \n"
        "	lw	 x15,  60(sp)                               \n"
        "	lw	 x16,  64(sp)                               \n"
        "	lw	 x17,  68(sp)                               \n"
        "	lw	 x18,  72(sp)                               \n"
        "	lw	 x19,  76(sp)                               \n"
        "	lw	 x20,  80(sp)                               \n"
        "	lw	 x21,  84(sp)                               \n"
        "	lw	 x22,  88(sp)                               \n"
        "	lw	 x23,  92(sp)                               \n"
        "	lw	 x24,  96(sp)                               \n"
        "	lw	 x25, 100(sp)                               \n"
        "	lw	 x26, 104(sp)                               \n"
        "	lw	 x27, 108(sp)                               \n"
        "	lw	 x28, 112(sp)                               \n"
        "	lw	 x29, 116(sp)                               \n"
        "	lw	 x30, 120(sp)                               \n"
        "	lw	 x31, 124(sp)                               \n"
        "	addi      sp,     sp,    128                        \n"
        "	mret                                                \n" : : "i"(dispatch));
}

void IC::dispatch()
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    _int_vector[id](id);
}

void IC::int_not(Interrupt_Id i)
{
    db<IC>(WRN) << "IC::int_not(i=" << i << ")" << endl;
}

void IC::hard_fault(Interrupt_Id i)
{
    db<IC>(ERR) << "IC::hard_fault(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::undefined_instruction(Interrupt_Id i)
{
    db<IC>(ERR) << "Undefined instruction(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::software_interrupt(Interrupt_Id i)
{
    db<IC>(ERR) << "Software interrupt(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::prefetch_abort(Interrupt_Id i)
{
    db<IC>(ERR) << "Prefetch abort(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::data_abort(Interrupt_Id i)
{
    db<IC>(ERR) << "Data abort(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::reserved(Interrupt_Id i)
{
    db<IC>(ERR) << "Reserved(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::fiq(Interrupt_Id i)
{
    db<IC>(ERR) << "FIQ handler(i=" << i << ")" << endl;
    Machine::panic();
}

void IC::exception_handling()
{
    db<IC>(ERR) << "Exception abort" << endl;
    Interrupt_Id id = int_id();
    switch(id) {
        case 0: // unaligned Instruction
        case 1: // instruction access failure
            prefetch_abort(id);
            break;
        case 2: // illegal instruction
            undefined_instruction(id);
            break;
        case 3: // Break Point
            db<IC>(ERR) << "Software Break Point" << endl;
            software_interrupt(id);
            break;
        case 4: // unaligned load address
        case 5: // load access failure
        case 6: // unaligned store address
        case 7: // store access failure
            data_abort(id);
            break;
        case 8: // user-mode environment call
        case 9: // supervisor-mode environment call
        case 10: // reserved... not described
        case 11: // machine-mode environment call
            reserved(id);
            break;
        case 12: // Instruction Page Table failure
        case 13: // Load Page Table failure
        case 14: // reserved... not described
        case 15: // Store Page Table failure
            data_abort(id);
            break;
        default:
            int_not(id);
            break;
    }
    Machine::panic();
}
__END_SYS
