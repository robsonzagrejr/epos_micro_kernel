// EPOS ARMv8 MMU Mediator Declarations

#ifndef __armv8_mmu_h
#define __armv8_mmu_h

#include <architecture/mmu.h>
#include <system/memory_map.h>

__BEGIN_SYS

class ARMv8_MMU: public MMU_Common<10, 10, 12>
{
    friend class CPU;

private:
    typedef Grouping_List<Frame> List;

    static const bool colorful = Traits<MMU>::colorful;
    static const unsigned int COLORS = Traits<MMU>::COLORS;
    static const unsigned int MEM_BASE = Memory_Map::MEM_BASE;
    static const unsigned int APP_LOW = Memory_Map::APP_LOW;
    static const unsigned int PHY_MEM = Memory_Map::PHY_MEM;

public:
    // Page Flags
    class Page_Flags;

    // Page_Table
    class Page_Table;

    // Page Directory
    class Page_Directory;

    // Chunk (for Segment)
    class Chunk;

    // Directory (for Address_Space)
    class Directory;

    // DMA_Buffer
    class DMA_Buffer;

    // Class Translation performs manual logical to physical address translations for debugging purposes only
    class Translation;

public:
    ARMv8_MMU() {}

    static Phy_Addr alloc(unsigned int frames = 1, Color color = WHITE);
    static Phy_Addr calloc(unsigned int frames = 1, Color color = WHITE);
    static void free(Phy_Addr frame, int n = 1);
    static void white_free(Phy_Addr frame, int n);
    static unsigned int allocable(Color color = WHITE);

    static Page_Directory * volatile current();
    static Phy_Addr physical(Log_Addr addr);
    static PT_Entry phy2pte(Phy_Addr frame, Page_Flags flags);
    static Phy_Addr pte2phy(PT_Entry entry);
    static PD_Entry phy2pde(Phy_Addr frame);
    static Phy_Addr pde2phy(PD_Entry entry);

    static void flush_tlb();
    static void flush_tlb(Log_Addr addr);

    static Log_Addr phy2log(Phy_Addr phy) { return Log_Addr((MEM_BASE == PHY_MEM) ? phy : (MEM_BASE > PHY_MEM) ? phy - (MEM_BASE - PHY_MEM) : phy + (PHY_MEM - MEM_BASE)); }
    static Phy_Addr log2phy(Log_Addr log) { return Phy_Addr((MEM_BASE == PHY_MEM) ? log : (MEM_BASE > PHY_MEM) ? log + (MEM_BASE - PHY_MEM) : log - (PHY_MEM - MEM_BASE)); }

    static Color phy2color(Phy_Addr phy);

    static Color log2color(Log_Addr log);

private:
    static void init();

private:
    static List _free[colorful * COLORS + 1]; // +1 for WHITE
    static Page_Directory * _master;
};

class MMU: public IF<Traits<System>::multitask, ARMv8_MMU, No_MMU>::Result {};

__END_SYS

#endif
