#ifndef __traits_h
#define __traits_h

#include <system/config.h>

__BEGIN_SYS

// Build
template<> struct Traits<Build>: public Traits_Tokens
{
    // Basic configuration
    static const unsigned int MODE = LIBRARY;
    static const unsigned int ARCHITECTURE = ARMv7;
    static const unsigned int MACHINE = Cortex;
    static const unsigned int MODEL = LM3S811;
    static const unsigned int CPUS = 1;
    static const unsigned int NODES = 1; // (> 1 => NETWORKING)
    static const unsigned int EXPECTED_SIMULATION_TIME = 60; // s (0 => not simulated)

    // Default flags
    static const bool enabled = true;
    static const bool monitored = true;
    static const bool debugged = true;
    static const bool hysterically_debugged = false;

    // Default aspects
    typedef ALIST<> ASPECTS;
};


// Utilities
template<> struct Traits<Debug>: public Traits<Build>
{
    static const bool error   = true;
    static const bool warning = true;
    static const bool info    = false;
    static const bool trace   = false;
};

template<> struct Traits<Lists>: public Traits<Build>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Spin>: public Traits<Build>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Heaps>: public Traits<Build>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Observers>: public Traits<Build>
{
    // Some observed objects are created before initializing the Display
    // Enabling debug may cause trouble in some Machines
    static const bool debugged = false;
};


// System Parts (mostly to fine control debugging)
template<> struct Traits<Boot>: public Traits<Build>
{
};

template<> struct Traits<Setup>: public Traits<Build>
{
};

template<> struct Traits<Init>: public Traits<Build>
{
};

template<> struct Traits<Framework>: public Traits<Build>
{
};

template<> struct Traits<Aspect>: public Traits<Build>
{
    static const bool debugged = hysterically_debugged;
};


__END_SYS

// Mediators
#include __ARCHITECTURE_TRAITS_H
#include __MACHINE_TRAITS_H

__BEGIN_SYS


// API Components
template<> struct Traits<Application>: public Traits<Build>
{
    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = Traits<Machine>::HEAP_SIZE;
    static const unsigned int MAX_THREADS = Traits<Machine>::MAX_THREADS;
};

template<> struct Traits<System>: public Traits<Build>
{
    static const unsigned int mode = Traits<Build>::MODE;
    static const bool multithread = (Traits<Build>::CPUS > 1) || (Traits<Application>::MAX_THREADS > 1);
    static const bool multitask = (mode != Traits<Build>::LIBRARY);
    static const bool multicore = (Traits<Build>::CPUS > 1) && multithread;
    static const bool multiheap = multitask || Traits<Scratchpad>::enabled;

    static const unsigned long LIFE_SPAN = 1 * YEAR; // s
    static const unsigned int DUTY_CYCLE = 1000000; // ppm

    static const bool reboot = true;

    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = (Traits<Application>::MAX_THREADS + 1) * Traits<Application>::STACK_SIZE;
};

template<> struct Traits<Task>: public Traits<Build>
{
    static const bool enabled = Traits<System>::multitask;
};

template<> struct Traits<Thread>: public Traits<Build>
{
    static const bool enabled = Traits<System>::multithread;
    static const bool smp = Traits<System>::multicore;
    static const bool simulate_capacity = false;
    static const bool trace_idle = hysterically_debugged;

    static const unsigned int QUANTUM = 10000; // us
};

template<> struct Traits<Scheduler<Thread>>: public Traits<Build>
{
    static const bool debugged = Traits<Thread>::trace_idle || hysterically_debugged;
};

template<> struct Traits<Synchronizer>: public Traits<Build>
{
    static const bool enabled = Traits<System>::multithread;
};

template<> struct Traits<Alarm>: public Traits<Build>
{
    static const bool visible = hysterically_debugged;
};

template<> struct Traits<SmartData>: public Traits<Build>
{
    static const unsigned char PREDICTOR = NONE;
};

template<> struct Traits<Network>: public Traits<Build>
{
    typedef LIST<> NETWORKS;

    static const unsigned int RETRIES = 3;
    static const unsigned int TIMEOUT = 10; // s

    static const bool enabled = (Traits<Build>::NODES > 1) && (NETWORKS::Length > 0);
};

template<> struct Traits<ELP>: public Traits<Network>
{
    typedef Ethernet NIC_Family;
    static constexpr unsigned int NICS[] = {0}; // relative to NIC_Family (i.e. Traits<Ethernet>::DEVICES[NICS[i]]
    static const unsigned int UNITS = COUNTOF(NICS);

    static const bool enabled = Traits<Network>::enabled && (NETWORKS::Count<ELP>::Result > 0);
};

template<> struct Traits<TSTP>: public Traits<Network>
{
    typedef Ethernet NIC_Family;
    static constexpr unsigned int NICS[] = {0}; // relative to NIC_Family (i.e. Traits<Ethernet>::DEVICES[NICS[i]]
    static const unsigned int UNITS = COUNTOF(NICS);

    static const unsigned int KEY_SIZE = 16;
    static const unsigned int RADIO_RANGE = 8000; // approximated radio range in centimeters

    static const bool enabled = Traits<Network>::enabled && (NETWORKS::Count<TSTP>::Result > 0);
};

template<> struct Traits<IP>: public Traits<Network>
{
    typedef Ethernet NIC_Family;
    static constexpr unsigned int NICS[] = {0};  // relative to NIC_Family (i.e. Traits<Ethernet>::DEVICES[NICS[i]]
    static const unsigned int UNITS = COUNTOF(NICS);

    struct Default_Config {
        static const unsigned int  TYPE    = DHCP;
        static const unsigned long ADDRESS = 0;
        static const unsigned long NETMASK = 0;
        static const unsigned long GATEWAY = 0;
    };

    template<unsigned int UNIT>
    struct Config: public Default_Config {};

    static const unsigned int TTL  = 0x40; // Time-to-live

    static const bool enabled = Traits<Network>::enabled && (NETWORKS::Count<IP>::Result > 0);
};

template<> struct Traits<UDP>: public Traits<Network>
{
    static const bool checksum = true;
};

template<> struct Traits<TCP>: public Traits<Network>
{
    static const unsigned int WINDOW = 4096;
};

template<> struct Traits<DHCP>: public Traits<Network>
{
};

template<> struct Traits<Monitor>: public Traits<Build>
{
    static const bool enabled = monitored;

    static constexpr System_Event SYSTEM_EVENTS[]                 = {ELAPSED_TIME, DEADLINE_MISSES, CPU_EXECUTION_TIME, THREAD_EXECUTION_TIME, RUNNING_THREAD};
    static constexpr unsigned int SYSTEM_EVENTS_FREQUENCIES[]     = {           1,               1,                  1,                     1,              1}; // in Hz

    static constexpr PMU_Event PMU_EVENTS[]                       = {COMMITED_INSTRUCTIONS, BRANCHES, CACHE_MISSES};
    static constexpr unsigned int PMU_EVENTS_FREQUENCIES[]        = {                    1,        1,            1}; // in Hz

    static constexpr unsigned int TRANSDUCER_EVENTS[]             = {CPU_VOLTAGE, CPU_TEMPERATURE};
    static constexpr unsigned int TRANSDUCER_EVENTS_FREQUENCIES[] = {          1,           1}; // in Hz
};

__END_SYS

#endif
