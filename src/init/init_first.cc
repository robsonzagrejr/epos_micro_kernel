// EPOS First Thread Initializer

#include <system.h>
#include <process.h>
#include <machine.h>

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

        // Thread::self() and Task::self() can be safely called after the construction of MAIN
        // even if no reschedule() was called (running is set by the Scheduler at each insert())
        // It will return MAIN for CPU0 and IDLE for the others
        Thread * first = Thread::self();

        db<Init, Thread>(INF) << "Dispatching the first thread: " << first << endl;

        // This barrier is particularly important, since afterwards the temporary stacks
        // and data structures established by SETUP and announced as "free memory" will indeed be
        // available to user threads.
        CPU::smp_barrier();
        first->_context->load();
    }
};

// Global object "init_first" must be constructed last in the context of the
// OS, for it activates the first application thread (usually main())
Init_First init_first;

__END_SYS
