// EPOS LM3S811 (ARM Cortex-M3) Timer Mediator Declarations

#ifndef __lm3s811_timer_h
#define __lm3s811_timer_h

#define __ic_common_only__
#include <machine/ic.h>
#undef __ic_common_only__
#include <machine/cortex/engine/cortex_m3/systick.h>
#include <machine/cortex/engine/cortex_m3/gptm.h>
#include <system/memory_map.h>
#include <utility/convert.h>

__BEGIN_SYS

class System_Timer_Engine: public Timer_Common
{
private:
    typedef IC_Common::Interrupt_Id Interrupt_Id;

public:
    typedef SysTick::Count Count;

public:
    System_Timer_Engine() { new(systick()) SysTick; }

    Count count() const { return systick()->count(); }

    void enable() const { systick()->enable(); }
    void disable() const { systick()->disable(); }

    Hertz clock() const { return systick()->clock(); }

protected:
    static void eoi(Interrupt_Id id) { systick()->eoi(id); };

    static void init(const Hertz & frequency) {
        systick()->config(systick()->clock() / frequency, true, true);
        systick()->enable();
    }

private:
    static SysTick * systick() { return reinterpret_cast<SysTick *>(Memory_Map::SCB_BASE); }
};


class User_Timer_Engine: public Timer_Common
{
private:
    static const unsigned int UNITS = Traits<Timer>::UNITS;

    typedef IC_Common::Interrupt_Id Interrupt_Id;

public:
    typedef GPTM::Count Count;

public:
    User_Timer_Engine(unsigned int unit, const Microsecond & time, bool interrupt, bool periodic) {
        assert(unit < UNITS);
        power(FULL);
        _gptm = new (gptm(unit)) GPTM;
        _gptm->config(time, interrupt, periodic);
    }
    ~User_Timer_Engine() {
        power(OFF);
    }

    Count count() const { return _gptm->count(); }

    void enable() const { _gptm->enable(); }
    void disable() const { _gptm->disable(); }

    Hertz clock() const { return _gptm->clock(); }

    void power(const Power_Mode & mode) {}

 protected:
    static void eoi(Interrupt_Id id) { int2gptm(id)->eoi(id); };

private:
    // TODO: incorporate in eoi and move to .cc
    static GPTM * int2gptm(Interrupt_Id id) {
        int i;
        switch(id) {
        case IC::INT_USER_TIMER0: i = 0; break;
        case IC::INT_USER_TIMER1: i = 1; break;
        case IC::INT_USER_TIMER2: i = 2; break;
        default: i = 3;
        }
        return gptm(i);
    }

    static GPTM * gptm(unsigned int unit) { return reinterpret_cast<GPTM *>(Memory_Map::TIMER0_BASE + 0x1000 * unit); }

private:
    GPTM * _gptm;
};

__END_SYS

#endif
