// EPOS Cortex Timer Mediator Implementation

#include <machine/machine.h>
#include <machine/timer.h>

__BEGIN_SYS

// Class attributes
Timer * Timer::_channels[CHANNELS];

//attribute only used for raspberry pi3
#ifdef __mmod_raspberry_pi3__
System_Timer_Engine::Count System_Timer_Engine::_count;
#endif

// Class methods
#ifdef __mmod_raspberry_pi3__

void Timer::int_handler(Interrupt_Id i)
{
    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[CPU::id()] <= 0)) {
        _channels[SCHEDULER]->_current[CPU::id()] = _channels[SCHEDULER]->_initial;
        if (CPU::id() == 0) {
            for (unsigned int cpu = 1; cpu < CPU::cores(); cpu++) {
                IC::ipi(cpu, IC::INT_RESCHEDULER);
            }
        }
        _channels[SCHEDULER]->_handler(i);
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (CPU::id() == 0))) && _channels[ALARM]) {
        _channels[ALARM]->_current[0] = _channels[ALARM]->_initial;
        _channels[ALARM]->_handler(i);
    }
}

#else

void Timer::int_handler(Interrupt_Id i)
{
    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[CPU::id()] <= 0)) {
        _channels[SCHEDULER]->_current[CPU::id()] = _channels[SCHEDULER]->_initial;
        _channels[SCHEDULER]->_handler(i);
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (CPU::id() == 0))) && _channels[ALARM]) {
        _channels[ALARM]->_current[0] = _channels[ALARM]->_initial;
        _channels[ALARM]->_handler(i);
    }
}

#endif

__END_SYS
