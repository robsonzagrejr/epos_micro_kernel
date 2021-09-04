// EPOS ARM Cortex Common Run-Time System Information

#ifndef __cortex_info_h
#define __cortex_info_h

#include <system/info.h>

__BEGIN_SYS

struct Cortex_System_Info: public System_Info_Common
{
public:
    // Dummy Boot_Map for compatibility
    union Boot_Map
    {
        volatile unsigned int n_cpus;     // Number of CPUs in SMPs
        PAddr mem_base;                   // Memory base address
        PAddr mem_top;                    // Memory top address
        PAddr mio_base;                   // Memory-mapped I/O base address
        PAddr mio_top;                    // Memory-mapped I/O top address
        int node_id;                      // Local node id in SAN (-1 => RARP)
        int space_x;                      // Spatial coordinates of a node (-1 => mobile)
        int space_y;                      //
        int space_z;                      //
        unsigned char uuid[8];            // EPOS image Universally Unique Identifier
        Size img_size;                    // Boot image size (in bytes)
        Size setup_offset;                // Image offsets (-1 => not present)
        Size init_offset;
        Size system_offset;
        Size application_offset;
        Size extras_offset;
    };

    // Dummy Physical_Memory_Map for compatibility
    union Physical_Memory_Map
    {
        // Page tables
        PAddr sys_pd;           // System Page Directory
        PAddr sys_pt;           // System Page Table
        PAddr phy_mem_pts;      // Page tables to map the whole physical memory
        PAddr io_pts;           // Page tables to map the I/O address space
        PAddr app_code_pts;     // First Application code segment's Page Table
        PAddr app_data_pts;     // First Application data segment's Page Table
        PAddr app_extra_pts;    // First Application data segment's Page Table

        // Pointers and data structures
        PAddr sys_info;         // System Info
        PAddr sys_code;         // OS Code segment
        PAddr sys_data;         // OS Data segment
        PAddr sys_stack;        // OS Stack segment  (used only during init and for ukernels, with one stack per core)
        PAddr app_code;         // First Application code segment
        PAddr app_data;         // First Application data segment (including heap, stack, and extra)
        PAddr app_extra;        // APP EXTRA segment (copied from the boot image)
        PAddr usr_mem_base;     // User-visible memory base address
        PAddr usr_mem_top;      // User-visible memory top address
        PAddr free1_base;       // First free memory chunk base address
        PAddr free1_top;        // First free memory chunk top address
        PAddr free2_base;       // Second free memory chunk base address
        PAddr free2_top;        // Second free memory chunk top address
    };

    // Load Map (not used in this machine, but kept for architectural transparency)
    struct Load_Map
    {
        bool  has_ext;
        LAddr app_entry;
        Size  app_segments;
        LAddr app_code;
        Size  app_code_size;
        LAddr app_data;
        LAddr app_stack;
        LAddr app_heap;
        Size  app_data_size;
        LAddr app_extra;
        Size  app_extra_size;
    };

public:
    union {
        Boot_Map bm;
        Physical_Memory_Map pmm;
        Load_Map lm;
    };
};

__END_SYS

#endif
