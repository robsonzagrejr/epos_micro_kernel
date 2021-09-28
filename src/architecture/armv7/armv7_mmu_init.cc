// EPOS ARMv7 MMU Mediator Initialization

#include <architecture/mmu.h>

extern "C" char __data_start;
extern "C" char _edata;
extern "C" char __bss_start;
extern "C" char _end;

__BEGIN_SYS

void ARMv7_MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;

    // Limpar a memoria nao utilizada, no caso podemos utilizar os mesmos
    // valores Logicos pois fizemos um mapeamento com endereco fisico igual
    //MEM_BASE ate SYS
    for (unsigned int i=Memory_Map::MEM_BASE; i + sizeof(Page) < Memory_Map::SYS; i+=sizeof(Page)) {
        MMU::free(i);
    }

}

__END_SYS

