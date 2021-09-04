// EPOS System Scaffold and System Component Implementation

#include <utility/ostream.h>
#include <utility/heap.h>
#include <machine.h>
#include <memory.h>
#include <system.h>

__BEGIN_SYS

// Global objects
// These objects might be reconstructed several times in multicore configurations, so their constructors must be stateless!
OStream kout;
OStream kerr;

// System class attributes
System_Info * System::_si = reinterpret_cast<System_Info *>(Memory_Map::SYS_INFO);
char System::_preheap[];
Segment * System::_heap_segment;
Heap * System::_heap;

__END_SYS
