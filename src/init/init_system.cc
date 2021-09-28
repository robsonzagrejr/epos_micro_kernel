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
    static const unsigned int HEAP_SIZE = Traits<System>::multitask ? Traits<Machine>::HEAP_SIZE : Traits<System>::HEAP_SIZE;

public:
    Init_System() {
        db<Init>(TRC) << "Init_System()" << endl;

        db<Init>(INF) << "Initializing the CPU: " << endl;
        CPU::init();
        db<Init>(INF) << "done!" << endl;

        db<Init>(INF) << "Initializing system's heap: " << endl;
        if(Traits<System>::multiheap) {
            System::_heap_segment = new (&System::_preheap[0]) Segment(HEAP_SIZE, Segment::Flags::SYS);
            if(Memory_Map::SYS_HEAP == Traits<Machine>::NOT_USED)
                System::_heap = new (&System::_preheap[sizeof(Segment)]) Heap(Address_Space(MMU::current()).attach(System::_heap_segment), System::_heap_segment->size());
            else
                System::_heap = new (&System::_preheap[sizeof(Segment)]) Heap(Address_Space(MMU::current()).attach(System::_heap_segment, Memory_Map::SYS_HEAP), System::_heap_segment->size());
        } else
            System::_heap = new (&System::_preheap[0]) Heap(MMU::alloc(MMU::pages(HEAP_SIZE)), HEAP_SIZE);
        db<Init>(INF) << "done!" << endl;

        db<Init>(INF) << "Initializing shared's heap: " << endl;

        if(Traits<System>::shared) {
            // db<Init>(INF) << "Segment done..." << endl;
            // db<Init>(INF) << "Segment size:" << SharedMemory::_heap_segment->size() << endl;
            // db<Init>(INF) << "SharedMemory address:" << hex << Traits<Machine>::SHARED_MEM << endl;
            SharedMemory::_heap_segment = new (&SharedMemory::_preheap[0]) Segment(SharedMemory::SHARED_HEAP_SIZE, Segment::Flags::SYS);
            SharedMemory::_heap = new (&SharedMemory::_preheap[sizeof(Segment)]) Heap((void *) Traits<Machine>::SHARED_MEM, SharedMemory::_heap_segment->size());
            // db<Init>(INF) << "Preheap 0:" << hex <<  SharedMemory::_preheap[sizeof(Segment)] << endl;
            // db<Init>(INF) << "Preheap 0:" << hex <<  SharedMemory::_heap << endl;

        }

        db<Init>(INF) << "done!" << endl;


        db<Init>(INF) << "Initializing the machine: " << endl;
        Machine::init();
        db<Init>(INF) << "done!" << endl;

        db<Init>(INF) << "Initializing system abstractions: " << endl;
        System::init();
        db<Init>(INF) << "done!" << endl;

        // Randomize the Random Numbers Generator's seed
        if(Traits<Random>::enabled) {
            db<Init>(INF) << "Randomizing the Random Numbers Generator's seed: ";
            if(Traits<TSC>::enabled)
                Random::seed(TSC::time_stamp());

            if(!Traits<TSC>::enabled)
                db<Init>(WRN) << "Due to lack of entropy, Random is a pseudo random numbers generator!" << endl;
            db<Init>(INF) << "done!" << endl;
        }

        // Initialization continues at init_end
    }
};

// Global object "init_system" must be constructed first.
Init_System init_system;

__END_SYS
