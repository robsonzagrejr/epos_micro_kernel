// EPOS ARM Cortex IC Mediator Implementation

#include <architecture/cpu.h>
#include <machine/machine.h>
#include <machine/ic.h>
#include <machine/timer.h>
#include <machine/usb.h>
#include <machine/gpio.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }
extern "C" { void _dispatch(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC8dispatchEj"))); }
extern "C" { void _eoi(unsigned int) __attribute__ ((alias("_ZN4EPOS1S2IC3eoiEj"))); }
extern "C" { void _undefined_instruction() __attribute__ ((alias("_ZN4EPOS1S2IC21undefined_instructionEv"))); }
extern "C" { void _software_interrupt() __attribute__ ((alias("_ZN4EPOS1S2IC18software_interruptEv"))); }
extern "C" { void _prefetch_abort() __attribute__ ((alias("_ZN4EPOS1S2IC14prefetch_abortEv"))); }
extern "C" { void _data_abort() __attribute__ ((alias("_ZN4EPOS1S2IC10data_abortEv"))); }
extern "C" { void _reserved() __attribute__ ((alias("_ZN4EPOS1S2IC8reservedEv"))); }
extern "C" { void _fiq() __attribute__ ((alias("_ZN4EPOS1S2IC3fiqEv"))); }

extern "C" { void _go_user_mode() __attribute__ ((naked)); }
extern "C" { void __exit(); }
extern "C" { void _exit(int s); }

void _go_user_mode() {
    ASM("pop {r12}                     \n"
        "msr sp_usr, r12               \n"
        "pop {r12}                     \n"
        "msr lr_usr, r12               \n"
        "pop {r0}                      \n"
        "msr spsr_cfxs, r0             \n"
        "ldmfd sp!, {r0-r12, lr, pc}^  \n"
    );
}

__BEGIN_SYS

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];
// Class attributes
IC::Interrupt_Handler IC::_eoi_vector[INTS] = {
    0,
    Timer::eoi, // System Timer C1
    0,
    0, // System Timer C3 (User_Timer)--> should we add the EOI function here?
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Timer::eoi, // ARM TIMER INT
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    // For the sake of the simplicity of the code, a single interrupt id is used to issue IPIs.
    // In this way, we only map a single handler, agnostic of MBOX number and Core ID.
    // On the other hand, handlers take Core ID into consideration when performing EOIs.
    // As a single MBOX handler is used to address all IPIs, we clear all MBOX of a Core in this handler.
    mailbox_eoi, // MB0_CPU0
    0,//mailbox_eoi, // MB1_CPU0
    0,//mailbox_eoi, // MB2_CPU0
    0,//mailbox_eoi, // MB3_CPU0
    0,//mailbox_eoi, // MB0_CPU1
    0,//mailbox_eoi, // MB1_CPU1
    0,//mailbox_eoi, // MB2_CPU1
    0,//mailbox_eoi, // MB3_CPU1
    0,//mailbox_eoi, // MB0_CPU2
    0,//mailbox_eoi, // MB1_CPU2
    0,//mailbox_eoi, // MB2_CPU2
    0,//mailbox_eoi, // MB3_CPU2
    0,//mailbox_eoi, // MB0_CPU3
    0,//mailbox_eoi, // MB1_CPU3
    0,//mailbox_eoi, // MB2_CPU3
    0,//mailbox_eoi, // MB3_CPU3
    Timer::eoi, // CORE0 LOCAL TIMER INT = 117
    0
};

// Class methods
void IC::entry()
{
    ASM(".equ MODE_IRQ, 0x12                        \n"
        ".equ MODE_SVC, 0x13                        \n"
        ".equ IRQ_BIT,  0x80                        \n"
        ".equ FIQ_BIT,  0x40                        \n"
        // Go to SVC
        "msr cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT  \n"
        // Save current context (lr, sp and spsr are banked registers)
        "stmfd sp!, {r0-r3, r12, lr, pc}            \n"
        // Go to IRQ
        "msr cpsr_c, #MODE_IRQ | IRQ_BIT | FIQ_BIT  \n"
        // Return from IRQ address
        "sub r0, lr, #4                             \n"
        // Pass irq_spsr to SVC r1
        "mrs r1, spsr                               \n"
        // Go back to SVC
        "msr cpsr_c, #MODE_SVC | IRQ_BIT | FIQ_BIT  \n"
        // sp+24 is the position of the saved pc
        "add r2, sp, #24                            \n"
        // Save address to return from interrupt into the pc position to retore
        // context later on
        "str r0, [r2]                               \n"
        // Save IRQ-spsr
        "stmfd sp!, {r1}                            \n"
        //"bl %0                                      \n"
        "bl _dispatch                               \n"
        "ldmfd sp!, {r0}                            \n"
        // Restore IRQ's spsr value to SVC's spsr
        "msr spsr_cfxs, r0                          \n"
        // Restore context, the ^ in the end of the above instruction makes the
        // irq_spsr to be restored into svc_cpsr
        "ldmfd sp!, {r0-r3, r12, lr, pc}^           \n" : : "i"(dispatch));
}

void IC::dispatch(unsigned int i)
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ",handler=" << hex << reinterpret_cast<void *>(_int_vector[id]) << ")" << endl;

    assert(id < INTS);
    if(_eoi_vector[id])
        _eoi_vector[id](id);

    CPU::int_enable();

    _int_vector[id](id);
}

void IC::eoi(unsigned int id)
{
    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::eoi(i=" << id << ")" << endl;

    assert(id < INTS);
    if(_eoi_vector[id])
        _eoi_vector[id](id);
}

void IC::int_not(Interrupt_Id i)
{
    db<IC>(WRN) << "IC::int_not(i=" << i << ")" << endl;
}

void IC::hard_fault(Interrupt_Id i)
{
    ASM(
        // Go to SVC to kill thread
        "msr cpsr_c, #0x13                          \n"
    );
    db<IC>(ERR) << "IC::hard_fault(i=" << i << ")" << endl;
    //Machine::panic();
    _exit(-1);
}

void IC::undefined_instruction()
{
    ASM(
        // Go to SVC to kill thread
        "msr cpsr_c, #0x13                          \n"
    );
    db<IC>(ERR) << "Undefined instruction" << endl;
    //Machine::panic();
    _exit(-1);
}

void IC::software_interrupt()
{
    //Salvar Contexto IC
    ASM(
        "stmfd sp!, {r0-r3, lr}  \n"
        "mrs r1, spsr            \n"
        "push {r1}               \n"
    );
    CPU::syscalled();
    ASM(
        "pop {r1}                \n"
        "msr spsr_cfxs, r1       \n"
        "ldmfd sp!, {r0-r3, pc}^ \n"
    );
}

void IC::prefetch_abort()
{
    ASM(
        // Go to SVC to execute __exit or to kill thread
        "msr cpsr_c, #0x13                          \n"
        // Get the addr of prefetch function
        "mrs r1, lr_abt                             \n"
        "sub r1, r1, #0x4                           \n"
        "ldr r2, =__exit                            \n"
        "cmp r1, r2                                 \n"
        "bne _prefetch_abort_error_exit             \n"
        // Continues to thread __exit
        "bx r1                                      \n"
        "_prefetch_abort_error_exit:                \n"
    );
    _exit(-1);
}

void IC::data_abort()
{
    ASM(
        // Go to SVC to kill thread
        "msr cpsr_c, #0x13                          \n"
    );
    db<IC>(ERR) << "Data abort" << endl;
    _exit(-1);
}

void IC::reserved()
{
    ASM(
        // Go to SVC to kill thread
        "msr cpsr_c, #0x13                          \n"
    );
    db<IC>(ERR) << "Reserved" << endl;
    //Machine::panic();
    _exit(-1);
}

void IC::fiq()
{
    ASM(
        // Go to SVC to kill thread
        "msr cpsr_c, #0x13                          \n"
    );
    db<IC>(ERR) << "FIQ handler" << endl;
    //Machine::panic();
    _exit(-1);
}

__END_SYS
