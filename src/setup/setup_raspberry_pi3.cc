// EPOS Raspberry Pi3 (Cortex-A53) SETUP

#include <system/config.h>
#include <architecture.h>
#include <machine.h>
#include <machine/cortex/engine/cortex_a53/bcm_mailbox.h> // for mailbox eoi in multicore
#include <utility/elf.h>
#include <utility/string.h>

extern "C" {
    void _start();
    void _init();
    void _int_entry();

    // SETUP entry point is _vector_table() and resides in the .init section (not in .text), so it will be linked first and will be the first function after the ELF header in the image.
    void _vector_table() __attribute__ ((used, naked, section(".init")));
    void _reset() __attribute__ ((naked)); // so it can be safely reached from the vector table
    void _setup(); // just to creat a Setup object

    // LD eliminates this variable while performing garbage collection, so --undefined=__boot_time_system_info must be present while linking
    char __boot_time_system_info[sizeof(EPOS::S::System_Info)] = "System_Info placeholder. Actual System_Info will be added by mkbi!";
}

__BEGIN_SYS

extern OStream kout, kerr;

class Setup
{
private:
    // Physical memory map
    static const unsigned int MIO_TOP   = Memory_Map::MIO_TOP;
    static const unsigned int MEM_BASE  = Memory_Map::MEM_BASE;
    static const unsigned int MEM_TOP   = Memory_Map::MEM_TOP;
    static const unsigned int IMAGE     = Memory_Map::IMAGE;
    static const unsigned int SETUP     = Memory_Map::SETUP;

    // Logical memory map
    static const unsigned int APP_LOW   = Memory_Map::APP_LOW;
    static const unsigned int PHY_MEM   = Memory_Map::PHY_MEM;
    static const unsigned int IO        = Memory_Map::IO;
    static const unsigned int SYS       = Memory_Map::SYS;
    static const unsigned int SYS_INFO  = Memory_Map::SYS_INFO;
    static const unsigned int SYS_PT    = Memory_Map::SYS_PT;
    static const unsigned int SYS_PD    = Memory_Map::SYS_PD;
    static const unsigned int SYS_CODE  = Memory_Map::SYS_CODE;
    static const unsigned int SYS_DATA  = Memory_Map::SYS_DATA;
    static const unsigned int SYS_STACK = Memory_Map::SYS_STACK;
    static const unsigned int APP_CODE  = Memory_Map::APP_CODE;
    static const unsigned int APP_DATA  = Memory_Map::APP_DATA;
    static const unsigned int APPC_PT   = Memory_Map::APPC_PT;
    static const unsigned int APPD_PT   = Memory_Map::APPD_PT;

    // Architecture Imports
    typedef CPU::Reg32 Reg32;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    typedef MMU::Page Page;
    typedef MMU::Page_Flags Flags;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;
    typedef MMU::PD_Entry PD_Entry;

    // System_Info Imports
    typedef System_Info::Boot_Map BM;
    typedef System_Info::Physical_Memory_Map PMM;
    typedef System_Info::Load_Map LM;

private:
    // TTBR0 Page Table Entry Descriptor for Sections configuration --> One level translation for Flat Mapping
    enum {
        TTB_MEMORY_DESCRIPTOR           = 0x90c0e,
        TTB_DEVICE_DESCRIPTOR           = 0x90c0a,
        TTB_PERIPHERAL_DESCRIPTOR       = 0x90c16
    };

public:
    Setup();

private:
    void flat_map_page_tables_setup();
    void build_lm();
    void build_pmm();

    void say_hi();

    void configure_page_table_descriptors(PT_Entry * pts, Phy_Addr base, unsigned int size, unsigned int n_pts, Flags flag, bool print = false);
    void setup_sys_pt();
    void setup_app_pt();
    void setup_sys_pd();
    void enable_paging();

    void load_parts();
    void call_next();

    void panic() { Machine::panic(); }

private:
    char * bi;
    System_Info * si;

    static volatile bool paging_ready;
};

volatile bool Setup::paging_ready = false;
Setup::Setup()
{
    CPU::int_disable(); // interrupts will be reenabled at init_end
    Display::init();
    if(Traits<System>::multitask) {
        bi = reinterpret_cast<char *>(SETUP);
        si = reinterpret_cast<System_Info *>(&__boot_time_system_info);

        db<Setup>(TRC) << "Setup(bi=" << reinterpret_cast<void *>(bi) << ",sp=" << reinterpret_cast<void *>(CPU::sp()) << ")" << endl;
        db<Setup>(INF) << "Setup:si=" << *si << endl;

        if(si->bm.n_cpus > Traits<Machine>::CPUS)
            si->bm.n_cpus = Traits<Machine>::CPUS;


        if(CPU::id() == 0) { // Boot strap CPU (BSP)
            // Build the memory model
            build_lm();
            //build_pmm();

            // Print basic facts about this EPOS instance
            say_hi();

            // Load EPOS parts (e.g. INIT, SYSTEM, APPLICATION)
            load_parts();

            // Configure the memory model defined above
            setup_sys_pt();
            //setup_app_pt();
            setup_sys_pd();

            // Enable paging
            enable_paging();

            // Signalize other CPUs that paging is up
            paging_ready = true;

        } else { // Additional CPUs (APs)

            // Wait for the Boot CPU to setup page tables
            while(!paging_ready);

            enable_paging();
        }
    } else {
        if (CPU::id() == 0) {
            db<Setup>(TRC) << "Setup for Library Mode" << endl;
            flat_map_page_tables_setup();
            enable_paging();

            paging_ready = true;
        } else {
            while(!paging_ready);
            enable_paging();
        }
    }
    // Paging is now enabled and the MMU is translating logical address to physical ones

    // SETUP ends here, transfer control to next stage (INIT or APP)
    if(Traits<System>::multitask)
        call_next();
    else
        _start();
    // SETUP is now part of the free memory and this point should never be
    // reached, but, just in case ... :-)
    panic();
}

void Setup::flat_map_page_tables_setup()
{
    CPU::Reg32 aux = 0x0;
    for (int curr_page = 1006; curr_page >= 0; curr_page--) {
        aux = TTB_MEMORY_DESCRIPTOR | (curr_page << 20);
        reinterpret_cast<volatile CPU::Reg32 *>(Traits<Machine>::PAGE_TABLES)[curr_page] = aux;
    }
    aux = TTB_DEVICE_DESCRIPTOR | (1007 << 20);
    reinterpret_cast<volatile CPU::Reg32 *>(Traits<Machine>::PAGE_TABLES)[1007] = aux;
    for (int curr_page = 4095; curr_page > 1007; curr_page--) {
        aux = TTB_PERIPHERAL_DESCRIPTOR | (curr_page << 20);
        reinterpret_cast<volatile CPU::Reg32 *>(Traits<Machine>::PAGE_TABLES)[curr_page] = aux;
    }
}

void Setup::build_lm()
{
    db<Setup>(TRC) << "Setup::build_lm()" << endl;

    // Get boot image structure
    si->lm.has_stp = (si->bm.setup_offset != -1u);
    si->lm.has_ini = (si->bm.init_offset != -1u);
    si->lm.has_sys = (si->bm.system_offset != -1u);
    si->lm.has_app = (si->bm.application_offset != -1u);
    si->lm.has_ext = (si->bm.extras_offset != -1u);

    /*
    // Nosso SETUP ja esta situado no lugar correto, decidimos nao realocar
    // Check SETUP integrity and get the size of its segments
    si->lm.stp_entry = 0;
    si->lm.stp_segments = 0;
    si->lm.stp_code = ~0U;
    si->lm.stp_code_size = 0;
    si->lm.stp_data = ~0U;
    si->lm.stp_data_size = 0;
    if(si->lm.has_stp) {
        ELF * stp_elf = reinterpret_cast<ELF *>(&bi[si->bm.setup_offset]);
        if(!stp_elf->valid()) {
            db<Setup>(ERR) << "SETUP ELF image is corrupted!" << endl;
            panic();
        }

        si->lm.stp_entry = stp_elf->entry();
        si->lm.stp_segments = stp_elf->segments();
        si->lm.stp_code = stp_elf->segment_address(0);
        si->lm.stp_code_size = stp_elf->segment_size(0);
        if(stp_elf->segments() > 1) {
            for(int i = 1; i < stp_elf->segments(); i++) {
                if(stp_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(stp_elf->segment_address(i) < si->lm.stp_data)
                    si->lm.stp_data = stp_elf->segment_address(i);
                si->lm.stp_data_size += stp_elf->segment_size(i);
            }
        }
    }
    */

    // Check INIT integrity and get the size of its segments
    si->lm.ini_entry = 0;
    si->lm.ini_segments = 0;
    si->lm.ini_code = ~0U;
    si->lm.ini_code_size = 0;
    si->lm.ini_data = ~0U;
    si->lm.ini_data_size = 0;
    if(si->lm.has_ini) {
        ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
        if(!ini_elf->valid()) {
            db<Setup>(ERR) << "INIT ELF image is corrupted!" << endl;
            panic();
        }

        //si->lm.ini_entry = ini_elf->entry();
        //Por algum motivo a funcao de cima retornava o endereco 0x0, logo
        //forcamos para o endereco mapeado logicamente o qual corresponde nesse
        //caso para o fisico
        si->lm.ini_entry = 0x00800000;
        ASM("_kakaka:");
        si->lm.ini_segments = ini_elf->segments();
        si->lm.ini_code = ini_elf->segment_address(0);
        si->lm.ini_code_size = ini_elf->segment_size(0);
        if(ini_elf->segments() > 1) {
            for(int i = 1; i < ini_elf->segments(); i++) {
                if(ini_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(ini_elf->segment_address(i) < si->lm.ini_data)
                    si->lm.ini_data = ini_elf->segment_address(i);
                si->lm.ini_data_size += ini_elf->segment_size(i);
            }
        }
        ASM("_kikiki:");
    }

    // Check SYSTEM integrity and get the size of its segments
    si->lm.sys_entry = 0;
    si->lm.sys_segments = 0;
    si->lm.sys_code = ~0U;
    si->lm.sys_code_size = 0;
    si->lm.sys_data = ~0U;
    si->lm.sys_data_size = 0;
    si->lm.sys_stack = SYS_STACK;
    si->lm.sys_stack_size = Traits<System>::STACK_SIZE * si->bm.n_cpus;
    if(si->lm.has_sys) {
        ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
        if(!sys_elf->valid()) {
            db<Setup>(ERR) << "OS ELF image is corrupted!" << endl;
            panic();
        }

        si->lm.sys_entry = sys_elf->entry();
        si->lm.sys_segments = sys_elf->segments();
        si->lm.sys_code = sys_elf->segment_address(0);
        si->lm.sys_code_size = sys_elf->segment_size(0);
        if(sys_elf->segments() > 1) {
            for(int i = 1; i < sys_elf->segments(); i++) {
                if(sys_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(sys_elf->segment_address(i) < si->lm.sys_code)
                    si->lm.sys_code = sys_elf->segment_address(i);
                si->lm.sys_code_size += sys_elf->segment_size(i);
            }
            db<Setup>(INF) << "SYS Segments: " << sys_elf->segments()<< endl;
            db<Setup>(INF) << "SYS Segments[0]: " << reinterpret_cast<void *>(sys_elf->segment_address(0)) << ", size="<< sys_elf->segment_size(0) << endl;
            db<Setup>(INF) << "SYS Segments[1]: " << reinterpret_cast<void *>(sys_elf->segment_address(1)) << ", size="<< sys_elf->segment_size(1) << endl;
        }

        // CODE and DATA Segments are concatenated, only code seg is available...
        if(si->lm.sys_code != SYS_CODE) {
            db<Setup>(ERR) << "OS code segment address (" << reinterpret_cast<void *>(si->lm.sys_code) << ") does not match the machine's memory map (" << reinterpret_cast<void *>(SYS_CODE) << ")!" << endl;
            panic();
        }

        if(si->lm.sys_code + si->lm.sys_code_size > si->lm.sys_stack) {
            db<Setup>(ERR) << "OS code segment is too large!" << endl;
            db<Setup>(ERR) << "OS code end:" << reinterpret_cast<void *>(si->lm.sys_code + si->lm.sys_code_size) << endl;
            db<Setup>(ERR) << "OS stack:" << si->lm.sys_stack << endl;
            panic();
        }


        /*
        if(MMU::page_tables(MMU::pages(si->lm.sys_stack + si->lm.sys_stack_size - (si->lm.sys_code + si->lm.sys_code_size))) > 1) {
            db<Setup>(ERR) << "OS stack segment is too large!" << endl;
            panic();
        }
        */

    }

    // Check APPLICATION integrity and get the size of its segments
    si->lm.app_entry = 0;
    si->lm.app_segments = 0;
    si->lm.app_code = ~0U;
    si->lm.app_code_size = 0;
    si->lm.app_data = ~0U;
    si->lm.app_data_size = 0;
    if(si->lm.has_app) {
        ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
        if(!app_elf->valid()) {
            db<Setup>(ERR) << "APP ELF image is corrupted!" << endl;
            panic();
        }
        si->lm.app_entry = app_elf->entry();
        ASM("_kokoko:");
        si->lm.app_segments = app_elf->segments();
        si->lm.app_code = app_elf->segment_address(0);
        si->lm.app_code_size = app_elf->segment_size(0);
        if(app_elf->segments() > 1) {
            for(int i = 1; i < app_elf->segments(); i++) {
                if(app_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(app_elf->segment_address(i) < si->lm.app_data)
                    si->lm.app_data = app_elf->segment_address(i);
                si->lm.app_data_size += app_elf->segment_size(i);
            }
        }
        if(si->lm.app_data == ~0U) {
            db<Setup>(WRN) << "APP ELF image has no data segment!" << endl;
            si->lm.app_data = MMU::align_page(APP_DATA);
        }
        if(Traits<System>::multiheap) { // Application heap in data segment
            si->lm.app_data_size = MMU::align_page(si->lm.app_data_size);
            si->lm.app_stack = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_data_size += MMU::align_page(Traits<Application>::STACK_SIZE);
            si->lm.app_heap = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_data_size += MMU::align_page(Traits<Application>::HEAP_SIZE);
        }
        if(si->lm.has_ext) { // Check for EXTRA data in the boot image
            si->lm.app_extra = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_extra_size = si->bm.img_size - si->bm.extras_offset;
            if(Traits<System>::multiheap)
                si->lm.app_extra_size = MMU::align_page(si->lm.app_extra_size);
            si->lm.app_data_size += si->lm.app_extra_size;
        }
    }
}

void Setup::build_pmm()
{
    db<Setup>(TRC) << "Setup::build_pmm() => top_page=" << MMU::pages(si->bm.mem_top) << endl;

    // Allocate (reserve) memory for all entities we have to setup.
    // We'll start at the highest address to make possible a memory model
    // on which the application's logical and physical address spaces match.
    Phy_Addr top_page = MMU::pages(si->bm.mem_top);

    // System Page Directory (4 x sizeof(Page)) -- up to 4k PD entries of 32 bits, moreover we need a page aligned to 16 KB
    top_page -= 4;
    si->pmm.sys_pd = top_page * sizeof(Page);


    // Page tables to map the System address space
    top_page -= 4;
    si->pmm.sys_pt = top_page * sizeof(Page);

    // Page tables to map the whole physical memory
    // = NP/NPTE_PT * sizeof(Page)
    //   NP = size of physical memory in pages
    //   NPTE_PT = number of page table entries per page table, e.g., 256 in this config (12, 8, 12)
    top_page -= MMU::page_tables(MMU::pages(si->bm.mem_top - si->bm.mem_base));
    si->pmm.phy_mem_pts = top_page * sizeof(Page);

    // Page tables to map the IO address space
    // = NP/NPTE_PT * sizeof(Page)
    // NP = size of I/O address space in pages
    // NPTE_PT = number of page table entries per page table
    top_page -= MMU::page_tables(MMU::pages(si->bm.mio_top - si->bm.mio_base));
    si->pmm.io_pts = top_page * sizeof(Page);

    // Page tables to map the first APPLICATION code segment
    top_page -= MMU::page_tables(MMU::pages(si->lm.app_code_size));
    si->pmm.app_code_pts = top_page * sizeof(Page);

    // Page tables to map the first APPLICATION data segment (which contains heap, stack and extra)
    top_page -= MMU::page_tables(MMU::pages(si->lm.app_data_size));
    si->pmm.app_data_pts = top_page * sizeof(Page);

    // System Info (1 x sizeof(Page))
    if (SYS_INFO != Traits<Machine>::NOT_USED)
        top_page -= 1;
    si->pmm.sys_info = top_page * sizeof(Page);

    // SYSTEM code segment -- For this test, everything will be in physical memory
    top_page -= MMU::pages(si->lm.sys_code_size);
    si->pmm.sys_code = top_page * sizeof(Page);

    // SYSTEM data segment
    top_page -= MMU::pages(si->lm.sys_data_size);
    si->pmm.sys_data = top_page * sizeof(Page);

    // The memory allocated so far will "disappear" from the system as we set mem_top as follows:
    si->pmm.usr_mem_base = si->bm.mem_base;
    si->pmm.usr_mem_top = top_page * sizeof(Page);

    // APPLICATION code segment
    top_page -= MMU::pages(si->lm.app_code_size);
    si->pmm.app_code = top_page * sizeof(Page);

    // APPLICATION data segment (contains stack, heap and extra)
    top_page -= MMU::pages(si->lm.app_data_size);
    si->pmm.app_data = top_page * sizeof(Page);

    // SYSTEM stack segment -- We use boot stack right after sys_pt
    top_page -= MMU::pages(si->lm.sys_stack_size);
    si->pmm.sys_stack = top_page * sizeof(Page);

    // Free chunks (passed to MMU::init)
    si->pmm.free1_base = si->bm.mem_base; // vector table should not be deleted!
    si->pmm.free1_top = top_page * sizeof(Page); // we will free the stack here
    db<Setup>(TRC) << "Top page = " << top_page << endl;

    // Test if we didn't overlap SETUP and the boot image
    if(si->pmm.usr_mem_top <= si->lm.stp_code + si->lm.stp_code_size + si->lm.stp_data_size) {
        db<Setup>(ERR) << "SETUP would have been overwritten!" << endl;
        panic();
    }
}

void Setup::say_hi()
{
    db<Setup>(TRC) << "Setup::say_hi()" << endl;
    db<Setup>(INF) << "System_Info=" << *si << endl;

    kout << endl;

    if(!si->lm.has_app) {
        db<Setup>(ERR) << "No APPLICATION in boot image, you don't need EPOS!" << endl;
        panic();
    }
    if(!si->lm.has_sys)
        db<Setup>(INF) << "No SYSTEM in boot image, assuming EPOS is a library!" << endl;

    kout << "Setting up this machine as follows: " << endl;
    kout << "  Processor:    " << Traits<Machine>::CPUS << " x Cortex A53 at " << Traits<CPU>::CLOCK / 1000000 << " MHz (BUS clock = " << Traits<CPU>::CLOCK / 1000000 << " MHz)" << endl;
    kout << "  Memory:       " << (si->bm.mem_top - si->bm.mem_base) / 1024 << " KB [" << (void *)si->bm.mem_base << ":" << (void *)si->bm.mem_top << "]" << endl;
    kout << "  User memory:  " << (si->pmm.usr_mem_top - si->pmm.usr_mem_base) / 1024 << " KB [" << (void *)si->pmm.usr_mem_base << ":" << (void *)si->pmm.usr_mem_top << "]" << endl;
    kout << "  I/O space:    " << (si->bm.mio_top - si->bm.mio_base) / 1024 << " KB [" << (void *)si->bm.mio_base << ":" << (void *)si->bm.mio_top << "]" << endl;
    kout << "  Node Id:      ";
    if(si->bm.node_id != -1)
        kout << si->bm.node_id << " (" << Traits<Build>::NODES << ")" << endl;
    else
        kout << "will get from the network!" << endl;
    kout << "  Position:     ";
    if(si->bm.space_x != -1)
        kout << "(" << si->bm.space_x << "," << si->bm.space_y << "," << si->bm.space_z << ")" << endl;
    else
        kout << "will get from the network!" << endl;
    if(si->lm.has_stp)
        kout << "  Setup:        " << si->lm.stp_code_size + si->lm.stp_data_size << " bytes" << endl;
    if(si->lm.has_ini)
        kout << "  Init:         " << si->lm.ini_code_size + si->lm.ini_data_size << " bytes" << endl;
    if(si->lm.has_sys)
        kout << "  OS code:      " << si->lm.sys_code_size << " bytes" << "\tdata: " << si->lm.sys_data_size << " bytes" << "    stack: " << si->lm.sys_stack_size << " bytes" << endl;
    if(si->lm.has_app)
        kout << "  APP code:     " << si->lm.app_code_size << " bytes" << "\tdata: " << si->lm.app_data_size << " bytes" << endl;
    if(si->lm.has_ext)
        kout << "  Extras:       " << si->lm.app_extra_size << " bytes" << endl;

    kout << endl;
}

void Setup::configure_page_table_descriptors(PT_Entry * pts, Phy_Addr base, unsigned int size, unsigned int n_pts, Flags flag, bool print) {
    unsigned int last_addr = 0;
    // n_pts equal to the number of PDs necessary to map the requested PTEs (given by size) from the memory base
    // Each PTE maps one Page (4k),
    // Each Page can have 256 PTEs
    // Thus, for each PD, map 256 pte until all requested ptes are mapped
    for(unsigned int curr_page = 0; curr_page < n_pts; curr_page++) {
        for(unsigned int i = last_addr; (i < (curr_page+1) * MMU::PT_ENTRIES && i < size); i++) {
            pts[curr_page*sizeof(Page)/sizeof(PT_Entry) + i - last_addr] = MMU::phy2pte((base + i * sizeof(Page)), flag);
            if (Traits<Setup>::hysterically_debugged && print)
                db<Setup>(INF) << "pts[" << curr_page*sizeof(Page)/sizeof(PT_Entry) + i - last_addr << "]=" << pts[curr_page*sizeof(Page)/sizeof(PT_Entry) + i - last_addr] << ",addr="<< &pts[i] << endl;
        }
        last_addr += MMU::PT_ENTRIES;
    }
}

void Setup::setup_sys_pt()
{
    db<Setup>(TRC) << "Setup::setup_sys_pt(pmm="
                   << "{si="      << (void *)si->pmm.sys_info
                   << ",pt="      << (void *)si->pmm.sys_pt
                   << ",pd="      << (void *)si->pmm.sys_pd
                   << ",sysc={b=" << (void *)si->pmm.sys_code << ",s=" << MMU::pages(si->lm.sys_code_size) << "}"
                   << ",sysd={b=" << (void *)si->pmm.sys_data << ",s=" << MMU::pages(si->lm.sys_data_size) << "}"
                   << ",syss={b=" << (void *)si->pmm.sys_stack << ",s=" << MMU::pages(si->lm.sys_stack_size) << "}"
                   << "})" << endl;

    // Get the physical address for the SYSTEM Page Table
    PT_Entry * sys_pt = reinterpret_cast<PT_Entry *>(SYS_PT);

    // Clear the System Page Table
    //memset(sys_pt, 0, 2*sizeof(Page));

    // System Info
    //sys_pt[MMU::page(SYS_INFO)] = MMU::phy2pte(SYS_INFO, Flags::SYS);

    // Set an entry to this page table, so the system can access it later -- ??? pages for entries
    //sys_pt[MMU::page(SYS_PT)] = MMU::phy2pte(SYS_PT, Flags::SYS);

    // System Page Directory -- ??? Pages for directory
    //sys_pt[MMU::page(SYS_PD)] = MMU::phy2pte(SYS_PD, Flags::SYS);

    // Mapeando toda a memoria para os Page Entrys
    unsigned int mem_size = MMU::page(MMU::align_page(MIO_TOP) - MEM_BASE);
    //configure_page_table_descriptors(sys_pt, SYS_CODE, si->lm.sys_code_size, n_pts, Flags::SYS, false);
    for (unsigned int i=0; i < mem_size; i++) {
        sys_pt[i] = MMU::phy2pte(i * sizeof(Page), Flags::SYS);
    }
    // SYSTEM code

    // SYSTEM data
    //sys_pt = reinterpret_cast<PT_Entry *>(sys_pt - n_pts * sizeof(MMU::Page));
    //n_pts = MMU::page_tables(MMU::pages(si->lm.sys_data_size));
    //configure_page_table_descriptors(sys_pt, SYS_DATA, si->lm.sys_data_size, n_pts, Flags::SYS, false);
    //db<Setup>(TRC) << "SYS_PT_DATA=" << *reinterpret_cast<Page_Table *>(sys_pt) << endl;

    // SYSTEM stack (used only during init and for the ukernel model)
    //sys_pt = reinterpret_cast<PT_Entry *>(sys_pt - n_pts * sizeof(MMU::Page));
    //n_pts = MMU::page_tables(MMU::pages(si->lm.sys_stack_size));
    //configure_page_table_descriptors(sys_pt, SYS_STACK, si->lm.sys_stack_size, n_pts, Flags::SYS, false);
    ASM("_setup_sys_pt_end:");

    //db<Setup>(TRC) << "SYS_PT_STACK=" << *reinterpret_cast<Page_Table *>(sys_pt) << endl;
}

void Setup::setup_app_pt()
{
    db<Setup>(TRC) << "Setup::setup_app_pt(appc={b=" << (void *)si->pmm.app_code << ",s=" << MMU::pages(si->lm.app_code_size) << "}"
                   << ",appd={b=" << (void *)si->pmm.app_data << ",s=" << MMU::pages(si->lm.app_data_size) << "}"
                   << ",appe={b=" << (void *)si->pmm.app_extra << ",s=" << MMU::pages(si->lm.app_extra_size) << "}"
                   << "})" << endl;

    // Get the physical address for the first APPLICATION Page Tables
    PT_Entry * app_code_pt = reinterpret_cast<PT_Entry *>(APPC_PT);
    PT_Entry * app_data_pt = reinterpret_cast<PT_Entry *>(APPD_PT);

    // Clear the first APPLICATION Page Tables
    memset(app_code_pt, 0, MMU::page_tables(MMU::pages(si->lm.app_code_size)) * sizeof(Page));
    memset(app_data_pt, 0, MMU::page_tables(MMU::pages(si->lm.app_data_size)) * sizeof(Page));

    // APPLICATION code
    unsigned int n_pts = MMU::page_tables(MMU::pages(si->lm.app_code_size));
    configure_page_table_descriptors(app_code_pt, APP_CODE, si->lm.app_code_size, n_pts, Flags::APP, false);

    // APPLICATION data (contains stack, heap and extra)
    n_pts = MMU::page_tables(MMU::pages(si->lm.app_data_size));
    configure_page_table_descriptors(app_data_pt, APP_DATA, si->lm.app_data_size, n_pts, Flags::APP, false);
    // configure_page_table_descriptors(???);

    //db<Setup>(INF) << "APPC_PT=" << *reinterpret_cast<Page_Table *>(app_code_pt) << endl;
    //db<Setup>(INF) << "APPD_PT=" << *reinterpret_cast<Page_Table *>(app_data_pt) << endl;
}

void Setup::setup_sys_pd()
{
    db<Setup>(TRC) << "setup_sys_pd(bm="
                   << "{memb="  << (void *)si->bm.mem_base
                   << ",memt="  << (void *)si->bm.mem_top
                   << ",miob="  << (void *)si->bm.mio_base
                   << ",miot="  << (void *)si->bm.mio_top
                   << "{si="    << (void *)si->pmm.sys_info
                   << ",spt="   << (void *)si->pmm.sys_pt
                   << ",spd="   << (void *)si->pmm.sys_pd
                   << ",mem="   << (void *)si->pmm.phy_mem_pts
                   << ",io="    << (void *)si->pmm.io_pts
                   << ",umemb=" << (void *)si->pmm.usr_mem_base
                   << ",umemt=" << (void *)si->pmm.usr_mem_top
                   << ",sysc="  << (void *)si->pmm.sys_code
                   << ",sysd="  << (void *)si->pmm.sys_data
                   << ",syss="  << (void *)si->pmm.sys_stack
                   << ",apct="  << (void *)si->pmm.app_code_pts
                   << ",apdt="  << (void *)si->pmm.app_data_pts
                   << ",fr1b="  << (void *)si->pmm.free1_base
                   << ",fr1t="  << (void *)si->pmm.free1_top
                   << ",fr2b="  << (void *)si->pmm.free2_base
                   << ",fr2t="  << (void *)si->pmm.free2_top
                   << "})" << endl;

    // Get the physical address for the System Page Directory
    PT_Entry * sys_pd = reinterpret_cast<PT_Entry *>(SYS_PD);

    // Clear the System Page Directory
    //memset(sys_pd, 0, sizeof(Page));

    // Calculate the number of page tables needed to map the physical memory
    unsigned int mem_size = MMU::pages(MMU::align_page(MIO_TOP) - MEM_BASE);
    unsigned int n_pts = MMU::page_tables(mem_size);
    db<Setup>(TRC) << "SIZE OF PAGE: " << hex << sizeof(Page) << endl;
    db<Setup>(TRC) << "SIZE OF PAGETABLE: " << hex << sizeof(Page_Table) << endl;
    db<Setup>(TRC) << "MEM SIZE: " << hex << mem_size << endl;
    db<Setup>(TRC) << "N_PTS: "    << hex << n_pts << endl;


    // Map the whole physical memory into the page tables pointed by phy_mem_pts
    //PT_Entry * pts = reinterpret_cast<PT_Entry *>(SYS_PT);

    // configure_page_table_descriptors(???);
    //configure_page_table_descriptors(pts, 0, mem_size, n_pts, Flags::SYS, false);

    // Attach the portion of the physical memory used by Setup at SETUP ??
    //assert((MMU::directory(MMU::align_directory(SETUP)) + n_pts) < (MMU::PD_ENTRIES - 4)); // check if it would overwrite the OS

    // Mapeando todas as entradas para os diretorios
    for(unsigned int i = 0; i < n_pts; i++)
        sys_pd[i] = MMU::phy2pde(SYS_PT +  i*sizeof(Page_Table));


    // Attach all physical memory starting at MEM_BASE ??
    //assert((MMU::directory(MMU::align_directory(MEM_BASE)) + n_pts) < (MMU::PD_ENTRIES - 4)); // check if it would overwrite the OS
    //for(unsigned int i = MMU::directory(MMU::align_directory(MEM_BASE)), j = 0; i < MMU::directory(MMU::align_directory(MEM_BASE)) + n_pts; i++, j++)
    //    sys_pd[i] = MMU::phy2pde((SYS_PD + j * sizeof(Page)));


    // Calculate the number of page tables needed to map the IO address space
    //unsigned int io_size = MMU::pages(si->bm.mio_top - si->bm.mio_base);
    //n_pts = MMU::page_tables(io_size);

    // Map IO address space into the page tables pointed by io_pts
    //pts = reinterpret_cast<PT_Entry *>(si->pmm.io_pts);
    // configure_page_table_descriptors(???);
    //configure_page_table_descriptors(pts, si->bm.mio_base, io_size, n_pts, Flags::IO, false);

    // Attach devices' memory at Memory_Map::IO
    //assert((MMU::directory(MMU::align_directory(IO)) + n_pts) < (MMU::PD_ENTRIES - 3)); // check if it would overwrite the OS
    //for(unsigned int i = MMU::directory(MMU::align_directory(IO)), j = 0; i < MMU::directory(MMU::align_directory(IO)) + n_pts; i++, j++)
        //sys_pd[i] = MMU::phy2pde((si->pmm.io_pts + j * sizeof(Page)));

    // Attach the OS (i.e. sys_pt)
    //sys_pd[MMU::directory(SYS)] = MMU::phy2pde(SYS_PT);

    // Attach the first APPLICATION CODE (i.e. app_code_pt)
    //sys_pd[MMU::directory(APP_CODE)] = MMU::phy2pde(APPC_PT);

    // Attach the first APPLICATION DATA (i.e. app_data_pt, containing heap, stack and extra)
    //sys_pd[MMU::directory(APP_DATA)] = MMU::phy2pde(APPD_PT);

    db<Setup>(TRC) << "Finishi setup_pd" << endl;
}

void Setup::enable_paging()
{
    db<Setup>(TRC) << "Setup::enable_paging()" << endl;
    if(Traits<Setup>::hysterically_debugged) {
        db<Setup>(INF) << "pc=" << CPU::pc() << endl;
        db<Setup>(INF) << "sp=" << reinterpret_cast<void *>(CPU::sp()) << endl;
    }

    CPU::dacr((Traits<System>::multitask) ? CPU::CLI_DOMAIN : CPU::MNG_DOMAIN);

    CPU::dsb();
    CPU::isb();

    // Clear TTBCR for the system to use ttbr0 instead of 1
    CPU::ttbcr(0);
    // Set ttbr0 with base address
    CPU::pdp((Traits<System>::multitask) ? SYS_PD : Traits<Machine>::PAGE_TABLES);

    // Enable MMU through SCTLR and ACTLR
    ASM("_enable_mmu_before_smp:");
    CPU::actlr(CPU::actlr() | CPU::SMP); // Set SMP bit
    CPU::sctlr((CPU::sctlr() | CPU::DCACHE | CPU::ICACHE | CPU::MMU_ENABLE) & ~(CPU::AFE));
    ASM("_enable_mmu_after_smp:");

    CPU::dsb();
    CPU::isb();
    // MMU now enabled - Virtual address system now active

    // Branch Prediction Enable
    CPU::sctlr(CPU::sctlr() | (1 << 11)); // Z bit

    // Flush TLB to ensure we've got the right memory organization
    MMU::flush_tlb();

    if(Traits<Setup>::hysterically_debugged) {
        db<Setup>(INF) << "pc=" << CPU::pc() << endl;
        db<Setup>(INF) << "sp=" << reinterpret_cast<void *>(CPU::sp()) << endl;
    }
}

void Setup::load_parts()
{
    db<Setup>(TRC) << "Setup::load_parts()" << endl;

    // Relocate System_Info
    if(sizeof(System_Info) > sizeof(Page))
        db<Setup>(WRN) << "System_Info is bigger than a page (" << sizeof(System_Info) << ")!" << endl;

    if(Traits<Setup>::hysterically_debugged)
        db<Setup>(INF) << "Setup:SYS_INFO: " << MMU::Translation(SYS_INFO) << endl;
    memcpy(reinterpret_cast<void *>(SYS_INFO), si, sizeof(System_Info));
    si = reinterpret_cast<System_Info *>(SYS_INFO);

    // Load INIT
    if(si->lm.has_ini) {
        db<Setup>(TRC) << "Setup::load_init()" << endl;
        ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:ini_elf: " << MMU::Translation(ini_elf) << endl;
            db<Setup>(INF) << "Setup:ini_elf[0]: " << MMU::Translation(ini_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:ini_elf[0].size: " << ini_elf->segment_size(0) << endl;
        }
        if(ini_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "INIT code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 1; i < ini_elf->segments(); i++)
            if(ini_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "INIT data segment was corrupted during SETUP!" << endl;
                panic();
            }
    }

    // Load SYSTEM
    if(si->lm.has_sys) {
        db<Setup>(TRC) << "Setup::load_os()" << endl;
        ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:sys_elf: " << MMU::Translation(sys_elf) << endl;
            db<Setup>(INF) << "Setup:sys_elf[0]: " << MMU::Translation(sys_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:sys_elf[0].size: " << sys_elf->segment_size(0) << endl;
        }
        if(sys_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "OS code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 1; i < sys_elf->segments(); i++) {
            if(sys_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "OS data segment was corrupted during SETUP!" << endl;
                panic();
            }
        }
    }

    // Load APP
    if(si->lm.has_app) {
        db<Setup>(TRC) << "Setup::load_app()" << endl;
        ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:app_elf: " << (void*)app_elf << endl;
            db<Setup>(INF) << "Setup:app_elf: " << MMU::Translation(app_elf) << endl;
            db<Setup>(INF) << "Setup:app_elf[0]: " << MMU::Translation(app_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:app_elf[0].size: " << app_elf->segment_size(0) << endl;
        }
        if(app_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "Application code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 1; i < app_elf->segments(); i++) {
            if(app_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "Application data segment was corrupted during SETUP!" << endl;
                panic();
            }
        }
    }

    // Load EXTRA
    if(si->lm.has_ext) {
        db<Setup>(TRC) << "Setup::load_extra()" << endl;
        if(Traits<Setup>::hysterically_debugged)
            db<Setup>(INF) << "Setup:APP_EXTRA:" << MMU::Translation(si->lm.app_extra) << endl;
        memcpy(reinterpret_cast<void *>(si->lm.app_extra), &bi[si->bm.extras_offset], si->lm.app_extra_size);
    }
}

void Setup::call_next()
{
    int cpu_id = CPU::id();

    // Check for next stage and obtain the entry point
    register Log_Addr ip;
    if(si->lm.has_ini) {
        if(CPU::id() == 0) {
            db<Setup>(TRC) << "Executing system's global constructors ..." << endl;
            reinterpret_cast<void (*)()>((void *)si->lm.sys_entry)();
        }
        ip = si->lm.ini_entry;
    } else if(si->lm.has_sys)
        ip = si->lm.sys_entry;
    else
        ip = si->lm.app_entry;

    // Arrange a stack for each CPU to support stage transition
    register Log_Addr sp = SYS_STACK + Traits<Machine>::STACK_SIZE * (cpu_id + 1);

    db<Setup>(TRC) << "Setup::call_next(ip=" << ip << ",sp=" << sp << ") => ";
    if(si->lm.has_ini)
        db<Setup>(TRC) << "INIT" << endl;
    else if(si->lm.has_sys)
        db<Setup>(TRC) << "SYSTEM" << endl;
    else
        db<Setup>(TRC) << "APPLICATION" << endl;

    db<Setup>(INF) << "SETUP ends here!" << endl;

    // Set SP and call next stage
    CPU::sp(sp);

    db<Setup>(INF) << "Calling next stage" << endl;
    static_cast<void (*)()>(ip)();

    if(CPU::id() == 0) { // Boot strap CPU (BSP)
        // This will only happen when INIT was called and Thread was disabled
        // Note we don't have the original stack here anymore!
        ASM("_call_app:");
        db<Setup>(INF) << "Calling app entry" << endl;
        reinterpret_cast<void (*)()>(si->lm.app_entry)();
    }
}

__END_SYS

using namespace EPOS::S;

// Interrupt Vector Table
void _vector_table()
{
    // We use and indirection table for the ldr instructions because the offset can be to far from the PC to be encoded
    ASM("               ldr pc, reset                                           \t\n\
                        ldr pc, ui                                              \t\n\
                        ldr pc, si                                              \t\n\
                        ldr pc, pa                                              \t\n\
                        ldr pc, da                                              \t\n\
                        nop             // _reserved                            \t\n\
                        ldr pc, irq                                             \t\n\
                        ldr pc, fiq                                             \t\n\
                                                                                \t\n\
        reset:          .word _reset                                            \t\n\
        ui:             .word _undefined_instruction                            \t\n\
        si:             .word _software_interrupt                               \t\n\
        pa:             .word _prefetch_abort                                   \t\n\
        da:             .word _data_abort                                       \t\n\
        irq:            .word _int_entry                                        \t\n\
        fiq:            .word _fiq                                              ");
}

void _reset()
{
    // QEMU get us here in SVC mode with interrupt disabled, but the real Raspberry Pi3 starts in hypervisor mode, so we must switch to SVC mode
    if(!Traits<Machine>::SIMULATED) {
        CPU::Reg cpsr = CPU::cpsr();
        cpsr = cpsr & ~CPU::FLAG_M;     // clear mode bits
        cpsr = cpsr | CPU::FLAG_SVC;    // set supervisor flag
        CPU::spsr_cxsf(cpsr);           // enter supervisor mode
        CPU::Reg address = CPU::lr();
        CPU::elr_hyp(address);
        CPU::msr12();
    }

    // Configure a temporary stack for IRQ mode
    CPU::cpsrc(CPU::FLAG_I | CPU::FLAG_F | CPU::FLAG_IRQ); // enter IRQ mode with interrupts disabled
    CPU::sp(0x7ffc);

    // Configure a temporary stack for FIQ mode
    CPU::cpsrc(CPU::FLAG_I | CPU::FLAG_F | CPU::FLAG_FIQ); // enter FIQ mode with interrupts disabled
    CPU::sp(0x3ffc);

    // Configure a stack for SVC mode, which will be used until the first Thread is created
    CPU::cpsrc(CPU::FLAG_I | CPU::FLAG_F | CPU::FLAG_SVC); // enter SVC mode with interrupts disabled
    CPU::sp(Traits<Machine>::BOOT_STACK + Traits<Machine>::STACK_SIZE * CPU::id());

    if(CPU::id() == 0) {
        // After a reset, we copy the vector table to 0x0000 to get a cleaner memory map (it is originally at 0x8000)
        // An alternative would be to set vbar address via mrc p15, 0, r1, c12, c0, 0
        CPU::r0(EPOS::S::Traits<EPOS::S::Machine>::VECTOR_TABLE); // load r0 with the source pointer
        CPU::r1(0); // load r1 with the destination pointer

        // Copy the first 32 bytes
        CPU::ldmia(); // load multiple registers from the memory pointed by r0 and auto-increment it accordingly
        CPU::stmia(); // store multiple registers to the memory pointed by r1 and auto-increment it accordingly

        // Repeat to copy the subsequent 32 bytes
        CPU::ldmia();
        CPU::stmia();
    } else {
        BCM_Mailbox * mbox = reinterpret_cast<BCM_Mailbox *>(Memory_Map::MBOX_CTRL_BASE);
        mbox->eoi(0);
        mbox->enable();
    }

    //_setup();
    ASM("bl _setup");
}

void _setup()
{
    CPU::int_disable(); // interrupts will be reenabled at init_end

    CPU::enable_fpu();
    CPU::invalidate_caches();
    CPU::invalidate_all_branch_predictors();
    CPU::invalidate_tlb();
    CPU::actlr(CPU::actlr() | CPU::DCACHE_PREFE); // enable Dside prefetch

    Setup setup;
}
