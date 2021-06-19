// EPOS Raspberry PI3 (ARM Cortex-A53) SETUP

// If emulating on QEMU, uboot has a vector table in ROM at 0x00000000.
// It relocates EPOS image (.bin), which has this vector table at the
// first address, to 0x00010000 and jumps to it.

#include <system/config.h>
#include <architecture/cpu.h>
#include <machine/cortex/engine/cortex_a53/bcm_mailbox.h>
#include <system/memory_map.h>

extern "C" { 
    void _vector_table() __attribute__ ((used, naked, section(".init"))); 

    void _entry();
    void _start();
}

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        ldr pc, reset_handler                                                   \t\n\
        ldr pc, undef_instruc                                                   \t\n\
        ldr pc, software_interrupt                                              \t\n\
        ldr pc, prefetch_abort                                                  \t\n\
        ldr pc, data_abort                                                      \t\n\
        nop                             // _reserved                            \t\n\
        ldr pc, int_entry               // IRQ                                  \t\n\
        ldr pc, fiq                                                             \t\n\
                                                                                \t\n\
reset_handler: .word _entry                                                     \t\n\
undef_instruc: .word _undefined_instruction                                     \t\n\
software_interrupt: .word _software_interrupt                                   \t\n\
prefetch_abort: .word _prefetch_abort                                           \t\n\
data_abort: .word _data_abort                                                   \t\n\
int_entry: .word _int_entry                                                     \t\n\
fiq: .word _fiq                                                                 \t\n\
        ");
}

using namespace EPOS::S;

void _entry() {

    if(!Traits<Machine>::SIMULATED) {
        CPU::Reg cpsr = CPU::cpsr();
        cpsr = cpsr & ~0x1F; // equivalent to a BIC (Bit Clear) operation
        cpsr = cpsr | 0x13; // these constants should be declared and explained somewehere (e.g., CPU.h)
        CPU::spsr_cxsf(cpsr);
        CPU::Reg address = CPU::lr();
        CPU::elr_hyp(address);
        CPU::msr12();
    }

    CPU::cpsrc(0xD2); // again, these values for cpsrc should be described at CPU.h
    CPU::sp(0x7ffc);

    CPU::cpsrc(0xD1);
    CPU::sp(0x3ffc);

    CPU::cpsrc(0xD3);
    CPU::sp(0x7000000);

    if (CPU::id() == 0) {
        //In the reset handler, we need to copy our interrupt vector table
        //to 0x0000 (it is currently at 0x8000)
        //the other way is to set vbar address via mrc p15, 0, r1, c12, c0, 0
        CPU::r0(EPOS::S::Traits<EPOS::S::Machine>::VECTOR_TABLE); // Store the source pointer on r0
        CPU::r1(0); // Store the destination pointer on r1

        // Here we copy the branch instructions
        // Load multiple values from the indexed address and auto-increments r0
        CPU::ldmia();
        // Store multiple values from the indexed address and auto-increments r1
        CPU::stmia();

        // To the branches get the correct addresses,
        //               we need to copy our whole vector table! So we run again.
        CPU::ldmia();
        CPU::stmia();
    } else {
        BCM_Mailbox * mbox = reinterpret_cast<BCM_Mailbox *>(Memory_Map::MBOX_CTRL_BASE);
        mbox->eoi(0);
        mbox->enable();
    }
    CPU::sp(Traits<Machine>::BOOT_STACK - (CPU::id() << 18));
    CPU::enable_fpu();
    _start();
}
