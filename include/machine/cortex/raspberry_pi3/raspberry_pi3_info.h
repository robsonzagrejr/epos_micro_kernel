// EPOS Raspberry Pi3 (ARM Cortex-A53) Run-Time System Information

#ifndef __raspberry_pi3_info_h
#define __raspberry_pi3_info_h

#include <machine/cortex/cortex_info.h>

__BEGIN_SYS

struct System_Info
{
private:
    typedef unsigned int LAddr;
    typedef unsigned int PAddr;
    typedef unsigned int Size;

public:
    // The information we have at boot time (built by MKBI)
    // Modifications to this map requires adjustments at MKBI and SETUP
    struct Boot_Map
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

    // Physical Memory Map (built by SETUP)
    struct Physical_Memory_Map
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

    // Load Map (built by SETUP)
    struct Load_Map
    {
        bool  has_stp;
        bool  has_ini;
        bool  has_sys;
        bool  has_app;
        bool  has_ext;
        LAddr stp_entry;
        Size  stp_segments;
        LAddr stp_code;
        Size  stp_code_size;
        LAddr stp_data;
        Size  stp_data_size;
        LAddr ini_entry;
        Size  ini_segments;
        LAddr ini_code;
        Size  ini_code_size;
        LAddr ini_data;
        Size  ini_data_size;
        LAddr sys_entry;
        Size  sys_segments;
        LAddr sys_code;
        Size  sys_code_size;
        LAddr sys_data;
        Size  sys_data_size;
        LAddr sys_stack;
        Size  sys_stack_size;
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
    friend Debug & operator<<(Debug & db, const System_Info & si);

public:
    Boot_Map bm;
    Physical_Memory_Map pmm;
    Load_Map lm;
};

__END_SYS

#endif
