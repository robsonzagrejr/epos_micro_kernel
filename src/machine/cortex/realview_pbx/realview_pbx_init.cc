// EPOS Realview PBX (ARM Cortex-A9) Initialization

#include <machine/machine.h>

__BEGIN_SYS

// Useful bits in the CONTROL_REG0 register
enum {                                      // Description              Type    Value after reset
    RXRES                       = 1 << 0,   // Reset Rx data path       r/w     0
    TXRES                       = 1 << 1,   // Reset Tx data path       r/w     0
    RXEN                        = 1 << 2,   // Receive enable           r/w     0
    TXEN                        = 1 << 4    // Transmit enable          r/w     0
};

// Useful bits in the MODE_REG0 register
enum {                                      // Description              Type    Value after reset
    CHRL8                       = 0 << 1,   // Character Length 8 bits  r/w     0
    CHRL7                       = 2 << 1,   // Character Length 7 bits  r/w     0
    CHRL6                       = 3 << 1,   // Character Length 6 bits  r/w     0
    PAREVEN                     = 0 << 3,   // Even parity              r/w     0
    PARODD                      = 1 << 3,   // Odd parity               r/w     0
    PARNONE                     = 4 << 3,   // No parity                r/w     0
    NBSTOP2                     = 2 << 6,   // 2 stop bits              r/w     0
    NBSTOP1                     = 0 << 6,   // 1 stop bit               r/w     0
    CHMODENORM                  = 0 << 8,   // Normal mode              r/w     0
    CHMODELB                    = 2 << 8    // Loopback mode            r/w     0
};

// Useful bits in the INTRPT_EN_REG0, and INTRPT_DIS_REG0 registers
enum {                                      // Description              Type    Value after reset
    INTRPT_RTRIG                = 1 << 0,   // Receiver FIFO empty      wo      0
    INTRPT_TTRIG                = 1 << 10   // Transmitter FIFO trigger wo      0
};

// Useful bits in the CHANNEL_STS_REG0 register
enum {                                      // Description              Type    Value after reset
    STS_RTRIG                   = 1 << 0,   // Receiver FIFO trigger    ro      0
    STS_TFUL                    = 1 << 4    // Transmitter FIFO full    ro      0
};

// SLCR Registers offsets
enum {                                      // Description
    SLCR_LOCK                   = 0x004,    // Lock the SLCR
    SLCR_UNLOCK                 = 0x008,    // Unlock the SLCR
    UART_CLK_CTRL               = 0x154,    // UART Ref Clock Control
    FPGA0_CLK_CTRL              = 0x170,    // PL Clock 0 Output control
    PSS_RST_CTRL                = 0x200,    // PS Software Reset Control
    FPGA_RST_CTRL               = 0x240     // FPGA Software Reset Control
};

// Useful bits in SLCR_LOCK
enum {                                      // Description                  Type    Value after reset
    LOCK_KEY                    = 0x767B    // Lock key                     wo      0
};

// Useful bits in SLCR_UNLOCK
enum {                                      // Description                  Type    Value after reset
    UNLOCK_KEY                  = 0xDF0D    // Unlock key                   wo      0
};

// Useful bits in FPGAN_CLK_CTRL
enum {                                      // Description                  Type    Value after reset
    DIVISOR0                    = 1 << 8,   // First cascade divider        r/w     0x18
    DIVISOR1                    = 1 << 20   // Second cascade divider       r/w     0x1
};


// TTB definitions
enum {
    TTBCR_DOMAIN                = 0x55555555, // All access to client
    TTB_DESCRIPTOR              = 0x10c0A
    // According to ARMv7 Arch. Ref. Manual: (description beggining at pages 1326 to 1329)
    // REG[19] NS, 0b0 for secure address space -> no effect on  Physical Address Space. -> Page 1330
    // REG[18] = 0b0 (mapped on 1 MB), it is not a supersection -> page 1329
    // REG[17] = 0b0 the TTB is global -> page 1378
    // REG[16] = 0b1 means shareable memory -> page 1368
    // REG[15] = 0b0 means read/write memory (if 1, read-only) -> ->  page 1358
    // REG[14:12] = TEX - 0b000 means possibly shareable page -> 1367
    // REG[11:10] = 0b11 means read/write with full access ->  page 1358
    // REG[9] = 0b0, Implementation Defined. -> page 1329
    // Reg[8:5] = 0b0000, Domain - not supported (DEPRECATED) its written on MCR p15, 0, <rt>, c3, c0, 0 -
    // REG[4] = XN = 0b0, means code can be executed. 0b1 stands for not exacutable area (generates Page Fault) -> page 1359
    // REG[3] = C (cacheable) = 1 with the config of TEX it means no Write-Allocate -> page 1367
    // REG[2] = B (Bufferable) = 0 with the config of TEX and C, it means Write-Through -> page 1367
    // Reg[1] = 0b1 means it points a section (more or equal than a MEGABYTE) -> Page 1329
    // REG[0] = PXN = 0b0 means every core can run, else processor running at PL1 generates Permission Fault-> page 1359
};


// DSB causes completion of all cache maintenance operations appearing in program
// order before the DSB instruction.
inline static void dsb()
{
    ASM("dsb");
}

// An ISB instruction causes the effect of all branch predictor maintenance
// operations before the ISB instruction to be visible to all instructions
// after the ISB instruction.
inline static void isb()
{
    ASM("isb");
}

inline static void invalidate_caches()
{
    ASM("                  \t\n\
.invalidate_caches:                                                                                   \t\n\
        push    {r4-r12}                                                                              \t\n\
        // Based on code example given in section b2.2.4/11.2.4 of arm ddi 0406b                      \t\n\
        mov     r0, #0                                                                                \t\n\
        // ICIALLU - invalidate entire i cache, and flushes branch target cache                       \t\n\
        mcr     p15, 0, r0, c7, c5, 0                                                                 \t\n\
        mrc     p15, 1, r0, c0, c0, 1      // read CLIDR                                              \t\n\
        ands    r3, r0, #0x7000000                                                                    \t\n\
        mov     r3, r3, lsr #23            // cache level value (naturally aligned)                   \t\n\
        beq     .invalidate_caches_finished                                                           \t\n\
        mov     r10, #0                                                                               \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop1:                                                                             \t\n\
        add     r2, r10, r10, lsr #1       // work out 3xcachelevel                                   \t\n\
        mov     r1, r0, lsr r2             // bottom 3 bits are the cache type for this level         \t\n\
        and     r1, r1, #7                 // get those 3 bits alone                                  \t\n\
        cmp     r1, #2                                                                                \t\n\
        blt     .invalidate_caches_skip    // no cache or instruction at this level                   \t\n\
        mcr     p15, 2, r10, c0, c0, 0     // write the cache size selection register                 \t\n\
        isb                                // ISB to sync the change to the cachesizeid reg           \t\n\
        mrc     p15, 1, r1, c0, c0, 0      // reads current cache size id register                    \t\n\
        and     r2, r1, #0x7               // extract the line length field                           \t\n\
        add     r2, r2, #4                 // add 4 for the line length offset (log2 16 bytes)        \t\n\
        ldr     r4, =0x3ff                                                                            \t\n\
        ands    r4, r4, r1, lsr #3         // r4 is the max number on way size (right algn)           \t\n\
        clz     r5, r4                     // r5 is the bit position of the way size increment        \t\n\
        ldr     r7, =0x00007fff                                                                       \t\n\
        ands    r7, r7, r1, lsr #13        // r7 is max number of index size (right algn)             \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop2:                                                                             \t\n\
        mov     r9, r4                     // r9 working copy of the max way size (right algn)        \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop3:                                                                             \t\n\
        orr     r11, r10, r9, lsl r5       //factor in way and cache number into r11                  \t\n\
        orr     r11, r11, r7, lsl r2       // factor in the index number                              \t\n\
        mcr     p15, 0, r11, c7, c6, 2     // DCISW - invalidate by set/way                           \t\n\
        subs    r9, r9, #1                 // decrement the way number                                \t\n\
        bge     .invalidate_caches_loop3                                                              \t\n\
        subs    r7, r7, #1                 // decrement the index                                     \t\n\
        bge     .invalidate_caches_loop2                                                              \t\n\
                                                                                                      \t\n\
.invalidate_caches_skip:                                                                              \t\n\
        add     r10, r10, #2               // increment the cache number                              \t\n\
        cmp     r3, r10                                                                               \t\n\
        bgt     .invalidate_caches_loop1                                                              \t\n\
                                                                                                      \t\n\
.invalidate_caches_finished:                                                                          \t\n\
        pop     {r4-r12}                                                                              \t\n\
        ");
}

inline static void enable_branch_prediction() {
    ASM("   mov     r0, #0                                                                                  \t\n\
            mrc     p15, 0, r0, c1, c0, 0                   // Read SCTLR                                   \t\n\
            orr     r0, r0, #(1 << 11)                      // Set Global BP Enable bit (bit 11)            \t\n\
            mcr     p15, 0,r0, c1, c0, 0                    // Write SCTLR                                          ");
}

inline static void enable_dside_prefetch() {
    // The Data side memory system (D-side) handles load and store requests from the Load Store Unit (LSU).
    ASM("   mrc     p15, 0, r0, c1, c0, 1                   // Read ACTLR                                   \t\n\
            orr     r0, r0, #(0x1 << 2)                     // Enable D-Side prefetch                       \t\n\
            mcr     p15, 0, r0, c1, c0, 1                   // Write ACTLR                                          ");
}

inline static void invalidate_tlb() {
    ASM("   mov     r0, #0x0                                                                                \t\n\
            mcr     p15, 0, r0, c8, c7, 0                   // TLBIALL - Invalidate entire Unified TLB              ");
}

inline static void clear_branch_prediction_array() {
    ASM("   mov     r0, #0x0                                                                                \t\n\
            mcr     p15, 0, r0, c7, c5, 6                   // BPIALL - Invalidate entire branch predictor array    ");
}

inline static void set_domain_access() {
    ASM("mcr p15, 0, %0, c3, c0, 0" : : "p"(TTBCR_DOMAIN) :);
}

inline static void enable_maintenance_broadcast() {
    // Enable the broadcasting of cache & TLB maintenance operations
    // When enabled AND in SMP, broadcast all "inner sharable"
    // cache and TLM maintenance operations to other SMP cores
    ASM("   mrc     p15, 0, r0, c1, c0, 1                   // Read ACTLR                                   \t\n\
            orr     r0, r0, #(0x01)                         // Set the FW bit (bit 0)                       \t\n\
            mcr     p15, 0, r0, c1, c0, 1                   // Write ACTLR                                          ");
}

inline static void join_smp() {
    ASM("   mrc     p15, 0, r0, c1, c0, 1                   // Read ACTLR                                   \t\n\
            orr     r0, r0, #(0x01 << 6)                    // Set SMP bit                                  \t\n\
            mcr     p15, 0, r0, c1, c0, 1                   // Write ACTLR                                         ");
}

inline static void page_tables_setup() {
    CPU::Reg32 aux = 0x0;
    for (int curr_page = 4095; curr_page > 0; curr_page--) {
        aux = TTB_DESCRIPTOR | (curr_page << 20);
        reinterpret_cast<volatile CPU::Reg32 *>(Traits<Machine>::PAGE_TABLES)[curr_page] = aux;
    }
    reinterpret_cast<volatile CPU::Reg32 *>(Traits<Machine>::PAGE_TABLES)[0] = TTB_DESCRIPTOR;
}

inline static void enable_mmu() {
    // TTB0 size is 16 kb, there is no TTB1 and no TTBCR
    // ARMv7 Architecture Reference Manual, pages 1330
    ASM ("MOV r0, #0x0 \t\n MCR p15, 0, r0, c2, c0, 2"); // Write Translation Table Base Control Register.
    ASM ("MCR p15, 0, %0, c2, c0, 0" : : "p"(Traits<Machine>::PAGE_TABLES) :); // Write Translation Table Base Register 0.

    // Enable MMU
    //-------------
    //0     - M, set to enable MMU
    // Leaving the caches disabled until after scatter loading.
    ASM ("                                                                      \t\n\
        MRC     p15, 0, r0, c1, c0, 0       // Read current control reg         \t\n\
        ORR     r0, r0, #(0x1 << 2)         // The C bit (data cache).          \t\n\
        ORR     r0, r0, #(0x1 << 12)        // The I bit (instruction cache).   \t\n\
        ORR     r0, r0, #0x01               // Set M bit                        \t\n\
        MCR     p15, 0, r0, c1, c0, 0       // Write reg back                   \t\n\
        ");
}

inline static void clear_bss() {
    CPU::Reg32 bss_start, bss_end;
    ASM("LDR %0, =__bss_start__" : "=r"(bss_start) :);
    ASM("LDR %0, =__bss_end__" : "=r"(bss_end) :);
    CPU::Reg32 limit = (bss_end - bss_start)/4;
    for(CPU::Reg32 i = 0; i < limit; i++) {
        reinterpret_cast<volatile CPU::Reg32 *>(bss_start)[i] = 0x0;
    }
}

void Realview_PBX::pre_init()
{
    // Relocated the vector table
    ASM("MCR p15, 0, %0, c12, c0, 0" : : "p"(Traits<Machine>::VECTOR_TABLE) :);

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

    // MMU now enable - Virtual address system now active

    // Branch Prediction init
    enable_branch_prediction();

    // SMP initialization
    if(CPU::id() == 0) {
        //primary core
        scu()->enable();
        scu()->secure_invalidate();
        join_smp();
        enable_maintenance_broadcast();
        scu()->enable_cache_coherence();

        // gic enable is now on Machine::pre_init()

        // FIXME:: is the following part of memory map or is there really a flat segment register?
        // this is only a flat segment register that allows mapping the start point for the secondary cores
        static const unsigned int FLAG_SET_REG                = 0x10000030;
        // set flag register to the address secondary CPUS are meant to start executing
        ASM("str %0, [%1]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);

        // secondary cores reset is now on Machine::pre_init()

        clear_bss();
    } else {
        //clear_interrupt();
        scu()->secure_invalidate();
        join_smp();
        enable_maintenance_broadcast();
        scu()->enable_cache_coherence();
    }
}

__END_SYS
