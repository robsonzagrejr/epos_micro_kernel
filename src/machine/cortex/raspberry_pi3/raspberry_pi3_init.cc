// EPOS Raspberry Pi3 (Cortex-A53) Initialization

#include <system/config.h>
#include <machine.h>
#include <architecture/cpu.h>

__BEGIN_SYS

// usefull bits for secondary cores boot
enum {
    FLAG_SET_REG                    = 0x40000000, //base for BCM CTRL, where you write the addresses of the boot
    CORE1_BASE_OFFSET               = 0x9c,
    CORE2_BASE_OFFSET               = 0xac,
    CORE3_BASE_OFFSET               = 0xbc,
    CORE_OFFSET_VARIATION           = 0x10
};

void Raspberry_Pi3::pre_init()
{
    // SMP initialization
    if(CPU::id() == 0) {
        // Primary core

        // This replaces the code commented below (that runned on previous versions),
        // it is not moved to smp init as on Realview_PBX because there is no IPI to wakeup the secondary cores
        // instead, the Raspberry Pi secondary cores are waken by a Send Event (runned after the dsb instruction)
        if (Traits<Build>::CPUS >= 2) {
            ASM("str %0, [%1, #0x9c]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);
            if (Traits<Build>::CPUS >= 3) {
                ASM("str %0, [%1, #0xac]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);
                if (Traits<Build>::CPUS == 4) {
                    ASM("str %0, [%1, #0xbc]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);
                }
            }
        }
        // this is only a flat segment register that allows mapping the start point for the secondary cores
        // static const unsigned int FLAG_SET_REG = 0x40000000;
        // set flag register to the address secondary CPUS are meant to start executing
        // ASM("str %0, [%1, #0x9c]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);
        // ASM("str %0, [%1, #0xac]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);
        // ASM("str %0, [%1, #0xbc]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);

        // secondary cores reset
        if (Traits<Build>::CPUS > 1)
            ASM ("SEV");
    }
}

__END_SYS
