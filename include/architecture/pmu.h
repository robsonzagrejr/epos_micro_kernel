// EPOS PMU Mediator Common Package

#ifndef __pmu_h
#define __pmu_h

#include <system/config.h>

__BEGIN_SYS

class PMU_Common
{
public:
    typedef unsigned int Channel;
    typedef unsigned int Event;
    typedef unsigned long long Count;

    enum Flags {
        NONE,
        INT
    };

protected:
    PMU_Common() {}
};

__END_SYS

#endif

#if defined(__PMU_H) && !defined(__pmu_common_only__)
#include __PMU_H
#endif
