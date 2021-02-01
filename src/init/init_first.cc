// EPOS First Thread Initializer

#include <system.h>
#include <process.h>

__BEGIN_SYS

class Init_First
{
public:
    Init_First() {
        db<Init>(TRC) << "Init_First()" << endl;

        if(!Traits<System>::multithread) {
            CPU::int_enable();
            return;
        }

        db<Init>(INF) << "INIT ends here!" << endl;

        db<Init, Thread>(INF) << "Dispatching the first thread: " << Thread::running() << endl;

        // Interrupts have been disable at Thread::init() and will be reenabled by CPU::Context::load()
        // but we first reset the timer to avoid getting a time interrupt during load()
        Timer::reset();
        CPU::int_enable();
        Thread::running()->_context->load();
    }
};

// Global object "init_first" must be constructed last in the context of the
// OS, for it activates the first application thread (usually main())
Init_First init_first;

__END_SYS
