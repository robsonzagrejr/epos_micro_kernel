// EPOS Thread Component Declarations

#ifndef __process_h
#define __process_h

#include <architecture.h>
#include <machine.h>
#include <utility/queue.h>
#include <utility/handler.h>
#include <memory.h>
#include <scheduler.h>

extern "C" { void __exit(); }

__BEGIN_SYS

class Thread
{
    friend class Init_End;              // context->load()
    friend class Init_System;           // for init() on CPU != 0
    friend class Scheduler<Thread>;     // for link()
    friend class Synchronizer_Common;   // for lock() and sleep()
    friend class Alarm;                 // for lock()
    friend class System;                // for init()
    friend class IC;                    // for link() for priority ceiling

protected:
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const bool multitask = Traits<System>::multitask;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = multitask ? Traits<System>::STACK_SIZE : Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING,
        READY,
        SUSPENDED,
        WAITING,
        FINISHING
    };

    // Thread Scheduling Criterion
    typedef Traits<Thread>::Criterion Criterion;
    enum {
        HIGH    = Criterion::HIGH,
        NORMAL  = Criterion::NORMAL,
        LOW     = Criterion::LOW,
        MAIN    = Criterion::MAIN,
        IDLE    = Criterion::IDLE
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;

    // Thread Configuration
    // t = 0 => Task::self()
    struct Configuration {
        Configuration(const State & s = READY, const Criterion & c = NORMAL, Task * t = 0, unsigned int ss = STACK_SIZE)
        : state(s), criterion(c), task(t), stack_size(ss) {}

        State state;
        Criterion criterion;
        Task * task;
        unsigned int stack_size;
    };


public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Tn>
    Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }
    const volatile Criterion::Statistics & statistics() { return criterion().statistics(); }

    const volatile Criterion & priority() const { return _link.rank(); }
    void priority(const Criterion & p);

    Task * task() const { return _task; }

    int join();
    void pass();
    void suspend();
    void resume();

    static Thread * volatile self() { return running(); }
    static void yield();
    static void exit(int status = 0);

protected:
    void constructor_prologue(unsigned int stack_size);
    void constructor_epilogue(Log_Addr entry, unsigned int stack_size);

    Criterion & criterion() { return const_cast<Criterion &>(_link.rank()); }
    Queue::Element * link() { return &_link; }

    static Thread * volatile running() { return _scheduler.chosen(); }

    static void lock() { CPU::int_disable(); }
    static void unlock() { CPU::int_enable(); }
    static bool locked() { return CPU::int_disabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void time_slicer(IC::Interrupt_Id interrupt);

    static void dispatch(Thread * prev, Thread * next, bool charge = true);

    static int idle();

private:
    static void init();

protected:
    Task * _task;

    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
};


// Task (only used in multitasking configurations)
class Task
{
    friend class Thread;        // for insert()

private:
    static const bool multitask = Traits<System>::multitask;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Context Context;
    typedef Thread::Queue Queue;

protected:
    // This constructor is only used by Thread::init()
    template<typename ... Tn>
    Task(Address_Space * as, Segment * cs, Segment * ds, Log_Addr code, Log_Addr data, int (* entry)(Tn ...), Tn ... an)
    : _as(as), _cs(cs), _ds(ds), _code(code), _data(data), _entry(entry) {
        db<Task, Init>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",code=" << _code << ",data=" << _data << ",entry=" << _entry << ") => " << this << endl;

        _current = this;
        activate();
        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN, this), entry, an ...);
    }

public:
    template<typename ... Tn>
    Task(Segment * cs, Segment * ds, Log_Addr code, Log_Addr data, int (* entry)(Tn ...), Tn ... an)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _code(_as->attach(_cs, code)), _data(_as->attach(_ds, data)), _entry(entry) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::MAIN, this), entry, an ...);
    }
    
    template<typename ... Tn>
    Task(const Thread::Configuration & conf, Segment * cs, Segment * ds, Log_Addr code, Log_Addr data, int (* entry)(Tn ...), Tn ... an)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _code(_as->attach(_cs, code)), _data(_as->attach(_ds, data)), _entry(entry) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _main = new (SYSTEM) Thread(Thread::Configuration(conf.state, conf.criterion, this), entry, an ...);
    }
    
    template<typename ... Tn>
    Task(Task * task = _current, int (* entry)(Tn ...) = 0, Tn ... an) { // fork-like constructor
        // Allocate resources
        _as = new (SYSTEM) Address_Space;
        _cs = new (SYSTEM) Segment(task->code_segment()->size(), Segment::Flags::APP);
        _ds = new (SYSTEM) Segment(task->data_segment()->size(), Segment::Flags::APP);
        _entry = entry ? entry : static_cast<int (*)(Tn ...)>(task->entry());

        // Copy segments
        Log_Addr src_code, src_data;
        if(task == _current) {
            src_code = task->code();
            src_data = task->data();
        } else {
            src_code = _current->address_space()->attach(task->code_segment());
            src_data = _current->address_space()->attach(task->data_segment());
        }
        Log_Addr dst_code = _current->address_space()->attach(_cs);
        Log_Addr dst_data = _current->address_space()->attach(_ds);
        memcpy(dst_code, src_code, task->code_segment()->size());
        memcpy(dst_data, src_data, task->data_segment()->size());
        _current->address_space()->detach(_cs);
        _current->address_space()->detach(_ds);
        if(task != _current) {
            _current->address_space()->detach(task->code_segment());
            _current->address_space()->detach(task->data_segment());
        }

        // Map segments
        _code = _as->attach(_cs, task->code());
        _data = _as->attach(_ds, task->data());

        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        // Create the task's main thread
        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::MAIN, this), static_cast<int (*)(Tn ...)>(_entry), an ...);
    }

    ~Task();

    Address_Space * address_space() const { return _as; }

    Segment * code_segment() const { return _cs; }
    Segment * data_segment() const { return _ds; }

    Log_Addr code() const { return _code; }
    Log_Addr data() const { return _data; }
    Log_Addr entry() const { return _entry; }

    Thread * main() const { return _main; }

    static Task * volatile self() { return current(); }

private:
    void activate() const { _current = const_cast<Task *>(this); _as->activate(); }

    void insert(Thread * t) { _threads.insert(new (SYSTEM) Queue::Element(t)); }
    void remove(Thread * t) { Queue::Element * el = _threads.remove(t); if(el) delete el; }

    static Task * volatile current() { return _current; }
    static void current(Task * t) { _current = t; }

    static void init();

private:
    Address_Space * _as;
    Segment * _cs;
    Segment * _ds;
    Log_Addr _code;
    Log_Addr _data;
    Log_Addr _entry;
    Thread * _main;
    Queue _threads;

    static Task * volatile _current;
};


// A Java-like Active Object
class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() {}

    void operator()() { _handler->resume(); }

private:
    Thread * _handler;
};


// Thread inline methods that depend on Task
template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
: _task(Task::self()), _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
{
    constructor_prologue(STACK_SIZE);
    _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

template<typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
: _task(conf.task ? conf.task : Task::self()), _state(conf.state), _waiting(0), _joining(0), _link(this, conf.criterion)
{
    constructor_prologue(conf.stack_size);
    _context = CPU::init_stack(0, _stack + conf.stack_size, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

__END_SYS

#endif
