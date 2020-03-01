// EPOS Machine Mediator Common Package

#ifndef __machine_common_h
#define __machine_common_h

#include <system/config.h>

__BEGIN_SYS

class Machine_Common
{
public:
    template<typename Family, int unit = 0>
    struct Initializer
    {
        typedef typename Traits<Family>::DEVICES::template Get<unit>::Result DEV;

        static void init() {
            if(Traits<DEV>::enabled)
                DEV::init(unit);

            Initializer<Family, unit + 1>::init();
        };
    };

    template<typename Family>
    struct Initializer<Family, Traits<Family>::DEVICES::Length>
    {
        static void init() {};
    };

protected:
    Machine_Common() {}
};

__END_SYS

#ifdef __MACH_H
#include __MACH_H
#endif

#endif
