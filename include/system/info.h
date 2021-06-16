// EPOS Run-Time System Information

#ifndef __info_h
#define __info_h

#include <system/config.h>


__BEGIN_SYS

struct System_Info_Common
{
protected:
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

public:
    Boot_Map bm;
};

__END_SYS

#include __HEADER_MMOD(info)

#endif
