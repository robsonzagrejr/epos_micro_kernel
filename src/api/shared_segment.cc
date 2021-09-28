// EPOS Component Declarations


#include <system.h>
#include <memory.h>

__BEGIN_SYS

Shared_Segment::SS_List Shared_Segment::_shared_segments;

Shared_Segment::Shared_Segment(int port, unsigned int bytes): Segment(bytes, MMU::Flags::APPD) {

    db<Segment>(TRC) << "Shared Memory Created" << endl;
    _port = port;
    SS_List::Element * e_shared_segment = new (SYSTEM) SS_List::Element(this);
    _shared_segments.insert(e_shared_segment);
}

Shared_Segment * Shared_Segment::using_port(int port) {
    SS_List::Element * e = _shared_segments.head();
    for(; e && (e->object()->get_port() != port); e = e->next());
    if (e) {
        db<Segment>(TRC) << "FOUND Port "<< port << " in " << e->object() << endl;
        return e->object();
    }
    return 0;
}

__END_SYS

