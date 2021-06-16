// EPOS ARM Cortex Common Run-Time System Information

#ifndef __cortex_info_h
#define __cortex_info_h

#include <system/info.h>

__BEGIN_SYS

struct Cortex_System_Info: public System_Info_Common
{
public:
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
    Load_Map lm;
};

__END_SYS

#endif
