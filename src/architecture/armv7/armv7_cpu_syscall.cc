// EPOS ARMV7 CPU System Call Entry Implementation

#include <architecture/armv7/armv7_cpu.h>

__BEGIN_SYS

void CPU::syscall(void * msg)
{
    ASM(
        //Salvando contexto
        // "push {lr}  \n"
        "push {r0}  \n"
        //Chamando sycall
        "mov r0, %0 \n"
        //"mov r12, sp \n"  // salvar tmp sp_usr
        //"mrs sp, sp_usr \n" // carregar sp_svc
        //"msr sp_usr, r12 \n"// salvar sp_usr

        "SVC 0x0    \n"

        //"mov r12, sp \n"  //salvar tmp sp_svc
        //"mrs sp, sp_usr \n" //carregar sp_usr
        //"msr sp_usr, r12 \n" //salver_sp_svc

        //Retornando contexto
        "pop {r0}   \n"
        // "pop {lr}   \n"
        "" :: "r"(msg)
    );
}

__END_SYS
