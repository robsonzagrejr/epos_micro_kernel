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

// TTB definitions
enum {
    TTBCR_DOMAIN                    = 0xffffffff, // All access to client
    TTB_MEMORY_DESCRIPTOR           = 0x90c0e,
    TTB_DEVICE_DESCRIPTOR           = 0x90c0a,
    TTB_PERIPHERAL_DESCRIPTOR       = 0x90c16

    // According to ARMv7 Arch. Ref. Manual: (description beggining at pages 1326 to 1329)
    // REG[19] NS, 0b0 for secure address space -> no effect on  Physical Address Space. -> Page 1330
    // REG[18] = 0b0 (mapped on 1 MB), it is not a supersection -> page 1329
    // REG[17] = 0b0 the TTB is global -> page 1378
    // REG[16] = 0b1 means shareable memory -> page 1368
    // REG[15] = 0b0 means read/write memory (if 1, read-only) -> ->  page 1358
    // REG[14:12] = TEX - 0b001 + C + B == outer inner write-back, write-allocate
    // REG[11:10] = 0b11 means read/write with full access ->  page 1358
    // REG[9] = 0b0, Implementation Defined. -> page 1329
    // Reg[8:5] = 0b0000, Domain - not supported (DEPRECATED) its written on mcr p15, 0, <rt>, c3, c0, 0 -
    // REG[4] = XN = 0b0, means code can be executed. 0b1 stands for not exacutable area (generates Page Fault) -> page 1359
    // REG[3] = C (cacheable) = 1 with the config of TEX it means no Write-Allocate -> page 1367
    // REG[2] = B (Bufferable) = 0 with the config of TEX and C, it means Write-Through -> page 1367
    // Reg[1] = 0b1 means it points a section (more or equal than a MEGABYTE) -> Page 1329
    // REG[0] = PXN = 0b0 means every core can run, else processor running at PL1 generates Permission Fault-> page 1359
};

static void branch_prediction_enable() {
    ASM("                                                                           \t\n\
        mrc     p15, 0, r0, c1, c0, 0                  // Read SCTLR                \t\n\
        orr     r0, r0, #(1 << 11)                     // Set the Z bit (bit 11)    \t\n\
        mcr     p15, 0,r0, c1, c0, 0                   // Write SCTLR               \t\n\
        ");
}

static void enable_dside_prefetch() {
    ASM("mrc p15, 0, r0, c1, c0, 1 \t\n\
         orr r0, r0, #(0x1 << 2)   \t\n\
         mcr p15, 0, r0, c1, c0, 1 \t\n\
        ");
}

static void invalidate_tlb() {
    ASM("mov r0, #0x0 \t\n mcr p15, 0, r0, c8, c7, 0"); // TLBIALL - Invalidate entire Unifed TLB
}

static void clear_branch_prediction_array() {
    ASM("mov r0, #0x0 \t\n mcr p15, 0, r0, c7, c5, 6"); // BPIALL - Invalidate entire branch predictor array
}

static void set_domain_access() {
    ASM("mcr p15, 0, %0, c3, c0, 0" : : "p"(TTBCR_DOMAIN) :);
}
/*
static void enable_maintenance_broadcast() {
    // Enable the broadcasting of cache & TLB maintenance operations
    // When enabled AND in SMP, broadcast all "inner sharable"
    // cache and TLM maintenance operations to other SMP cores
    ASM("mrc     p15, 0, r0, c1, c0, 1   // Read ACTLR.             \t\n\
         orr     r0, r0, #(0x01)         // Set the FW bit (bit 0). \t\n\
         mcr     p15, 0, r0, c1, c0, 1   // Write ACTLR."
    );
}

static void join_smp() {
    ASM("                                                                       \t\n\
        mrc     p15, 0, r0, c1, c0, 1   // Read ACTLR                           \t\n\
        orr     r0, r0, #(0x01 << 6)    // Set SMP bit                          \t\n\
        mcr     p15, 0, r0, c1, c0, 1   // Write ACTLR                          \t\n\
        ");
}
*/
static void page_tables_setup() {
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

static void enable_mmu() {
    // TTB0 size is 16 kb, there is no TTB1 and no TTBCR
    // ARMv7 Architecture Reference Manual, pages 1330
    ASM ("mov r0, #0x0 \t\n mcr p15, 0, r0, c2, c0, 2"); // Write Translation Table Base Control Register.
    ASM ("mcr p15, 0, %0, c2, c0, 0" : : "p"(Traits<Machine>::PAGE_TABLES) :); // Write Translation Table Base Register 0.

    // Enable MMU
    //-------------
    //0     - M, set to enable MMU
    // Leaving the caches disabled until after scatter loading.
    ASM ("                                                                      \t\n\
        mrc     p15, 0, r0, c1, c0, 1       // Read ACTLR                       \t\n\
        orr     r0, r0, #(0x01 << 6)        // Set SMP bit                      \t\n\
        mcr     p15, 0, r0, c1, c0, 1       // Write ACTLR                      \t\n\
        mrc     p15, 0, r0, c1, c0, 0       // Read current control reg         \t\n\
        orr     r0, r0, #(0x1 << 2)         // The C bit (data cache).          \t\n\
        bic     r0, r0, #(0x1 << 29)        // Set AFE to 0 disable Access Flag.\t\n\
        orr     r0, r0, #(0x1 << 12)        // The I bit (instruction cache).   \t\n\
        orr     r0, r0, #0x01               // Set M bit                        \t\n\
        mcr     p15, 0, r0, c1, c0, 0       // Write reg back                   \t\n\
        ");
}

static void clear_bss() {
    CPU::Reg32 bss_start, bss_end;
    ASM("ldr %0, =__bss_start__" : "=r"(bss_start) :);
    ASM("ldr %0, =__bss_end__" : "=r"(bss_end) :);
    CPU::Reg32 limit = (bss_end - bss_start)/4;
    for(CPU::Reg32 i = 0; i < limit; i++) {
        reinterpret_cast<volatile CPU::Reg32 *>(bss_start)[i] = 0x0;
    }
}

// DSB causes completion of all cache maintenance operations appearing in program
// order before the DSB instruction.
void dsb()
{
    ASM("dsb");
}



// An ISB instruction causes the effect of all branch predictor maintenance
// operations before the ISB instruction to be visible to all instructions
// after the ISB instruction.
void isb()
{
    ASM("isb");
}

void invalidate_caches()
{
    ASM("                  \t\n\
    // Disable L1 Caches.                                                               \t\n\
    mrc     p15, 0, r1, c1, c0, 0 // Read SCTLR.                                        \t\n\
    bic     r1, r1, #(0x1 << 2) // Disable D Cache.                                     \t\n\
    mcr     p15, 0, r1, c1, c0, 0 // Write SCTLR.                                       \t\n\
                                                                                        \t\n\
    // Invalidate Data cache to create general-purpose code. Calculate there            \t\n\
    // cache size first and loop through each set + way.                                \t\n\
    mov     r0, #0x0 // r0 = 0x0 for L1 dcache 0x2 for L2 dcache.                       \t\n\
    mcr     p15, 2, r0, c0, c0, 0 // CSSELR Cache Size Selection Register.              \t\n\
    mrc     p15, 1, r4, c0, c0, 0 // CCSIDR read Cache Size.                            \t\n\
    and     r1, r4, #0x7                                                                \t\n\
    add     r1, r1, #0x4 // r1 = Cache Line Size.                                       \t\n\
    ldr     r3, =0x7fff                                                                 \t\n\
    and     r2, r3, r4, lsr #13 // r2 = Cache Set Number – 1.                           \t\n\
    ldr     r3, =0x3ff                                                                  \t\n\
    and     r3, r3, r4, lsr #3 // r3 = Cache Associativity Number – 1.                  \t\n\
    clz     r4, r3 // r4 = way position in CISW instruction.                            \t\n\
    mov     r5, #0 // r5 = way loop counter.                                            \t\n\
way_loop:                                                                               \t\n\
    mov     r6, #0 // r6 = set loop counter.                                            \t\n\
set_loop:                                                                               \t\n\
    orr     r7, r0, r5, lsl r4 // Set way.                                              \t\n\
    orr     r7, r7, r6, lsl r1 // Set set.                                              \t\n\
    mcr     p15, 0, r7, c7, c6, 2 // DCCISW r7.                                         \t\n\
    add     r6, r6, #1 // Increment set counter.                                        \t\n\
    cmp     r6, r2 // Last set reached yet?                                             \t\n\
    ble     set_loop // If not, iterate set_loop,                                       \t\n\
    add     r5, r5, #1 // else, next way.                                               \t\n\
    cmp     r5, r3 // Last way reached yet?                                             \t\n\
    ble     way_loop // if not, iterate way_loop.                                       \t\n\
    // mov r2, #0                                                                       \t\n\
    // mcr p15, 0, r2, c7, c7, 0                                                        \t\n\
    ");
}



void Raspberry_Pi3::pre_init()
{
    // Relocated the vector table
    // ASM("mcr p15, 0, %0, c12, c0, 0" : : "p"(Traits<Machine>::VECTOR_TABLE) :);
    ASM ("mov r0, #0 \t\n msr spsr, r0 \t\n cpsie aif");
    // MMU init
    invalidate_caches();
    clear_branch_prediction_array();
    invalidate_tlb();
    enable_dside_prefetch();
    set_domain_access();
    dsb();
    isb();

    // Initialize PageTable.

    // Create a basic L1 page table in RAM, with 1MB sections containing a flat
    // (VA=PA) mapping, all pages Full Access, Strongly Ordered.

    // It would be faster to create this in a read-only section in an assembly file.

    if(CPU::id() == 0)
        page_tables_setup();

    // Activate the MMU
    enable_mmu();
    dsb();
    isb();

    // MMU now enabled - Virtual address system now active

    // Branch Prediction init
    branch_prediction_enable();

    // SMP initialization
    if(CPU::id() == 0) {
        //primary core

        // this replaces the code commented below (that runned on previous versions),
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

        dsb();
        // secondary cores reset
        if (Traits<Build>::CPUS > 1)
            ASM ("SEV");

        clear_bss();
    }
}

//void Raspberry_Pi3::init()
//{
//}
//
__END_SYS
