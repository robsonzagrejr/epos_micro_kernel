// EPOS Timer Mediator Common Package

#ifndef __timer_h
#define __timer_h

#include <machine/ic.h>

__BEGIN_SYS

class Timer_Common
{
public:
    typedef int Tick;
    typedef IC_Common::Interrupt_Handler Handler;

protected:
    Timer_Common() {}

public:
    Hertz frequency();
    void frequency(const Hertz & f);

    Tick read();
    Percent reset();

    void enable();
    void disable();

    void handler(const Handler & handler);
};

__END_SYS

#endif

#if defined(__TIMER_H) && !defined(__timer_common_only__)
#include __TIMER_H
#endif
