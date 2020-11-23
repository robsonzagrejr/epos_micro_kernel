// EPOS System Initializer

#include <utility/random.h>
#include <machine.h>
#include <memory.h>
#include <system.h>
#include <process.h>

__BEGIN_SYS

class Init_System
{
private:
    static const unsigned int HEAP_SIZE = Traits<System>::HEAP_SIZE;

public:
    Init_System() {
        db<Init>(TRC) << "Init_System()" << endl;

        CPU::smp_barrier();

        // Only the bootstrap CPU runs INIT_SYSTEM fully
        if(CPU::id() == 0) {
            db<Init>(INF) << "Initializing the CPU: " << endl;
            CPU::init();
            db<Init>(INF) << "done!" << endl;

            db<Init>(INF) << "Initializing system's heap: " << endl;
            if(Traits<System>::multiheap) {
                Segment * tmp = reinterpret_cast<Segment *>(&System::_preheap[0]);
                System::_heap_segment = new (tmp) Segment(HEAP_SIZE, WHITE, Segment::Flags::SYS);
                System::_heap = new (&System::_preheap[sizeof(Segment)]) Heap(Address_Space(MMU::current()).attach(System::_heap_segment, Memory_Map::SYS_HEAP), System::_heap_segment->size());
            } else
                System::_heap = new (&System::_preheap[0]) Heap(MMU::alloc(MMU::pages(HEAP_SIZE)), HEAP_SIZE);
            db<Init>(INF) << "done!" << endl;

            db<Init>(INF) << "Initializing the machine: " << endl;
            Machine::init();
            db<Init>(INF) << "done!" << endl;

            CPU::smp_barrier(); // signalizes "machine ready" to other CPUs

        } else {

            CPU::smp_barrier(); // waits until the bootstrap CPU signalizes "machine ready"

            CPU::init();
            Timer::init();
        }

        db<Init>(INF) << "Initializing system abstractions: " << endl;
        System::init();
        db<Init>(INF) << "done!" << endl;

        if(CPU::id() == 0) {
            // Randomize the Random Numbers Generator's seed
            if(Traits<Random>::enabled) {
                db<Init>(INF) << "Randomizing the Random Numbers Generator's seed: " << endl;
                if(Traits<TSC>::enabled)
                    Random::seed(TSC::time_stamp());
                    
#ifdef __ipv4__
            // An ordinary IP network should produce decent entropy
            if(Traits<Ethernet>::enabled) {
                NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
                Random::seed(Random::random() ^ nic->statistics().rx_packets);
            }
#endif
#ifdef __ADC_H
            if(Traits<ADC>::enabled) {
                ADC adc;
                Random::seed(Random::random() ^ adc.read());
            }
#endif

                if(!Traits<TSC>::enabled)
                    db<Init>(WRN) << "Due to lack of entropy, Random is a pseudo random numbers generator!" << endl;
                db<Init>(INF) << "done!" << endl;
            }
        }

        // Initialization continues at init_first
    }
};

// Global object "init_system" must be constructed first.
Init_System init_system;

__END_SYS
