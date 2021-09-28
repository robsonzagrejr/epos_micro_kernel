// EPOS Memory Declarations

#ifndef __memory_h
#define __memory_h

#include <architecture.h>
#include <utility/list.h>

__BEGIN_SYS

class Address_Space: MMU::Directory
{
    friend class Init_System;   // for Address_Space(pd)
    friend class Thread;        // for Address_Space(pd)
    friend class Scratchpad;    // for Address_Space(pd)
    friend class Task;          // for activate()

//private:
public:
    using MMU::Directory::activate;

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

public:
    Address_Space();
    Address_Space(MMU::Page_Directory * pd);
    ~Address_Space();

    using MMU::Directory::pd;

    Log_Addr attach(Segment * seg);
    Log_Addr attach(Segment * seg, Log_Addr addr);
    void detach(Segment * seg);
    void detach(Segment * seg, Log_Addr addr);

    Phy_Addr physical(Log_Addr address);
};


class Segment: public MMU::Chunk
{
    friend class Thread;        // for Segment(pt)

private:
    typedef MMU::Chunk Chunk;

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;

public:
    Segment(unsigned int bytes, Flags flags = Flags::APP);
    Segment(Phy_Addr phy_addr, unsigned int bytes, Flags flags);
    ~Segment();

    unsigned int size() const;
    Phy_Addr phy_address() const;
    int resize(int amount);

private:
    Segment(Phy_Addr pt, unsigned int from, unsigned int to, Flags flags): Chunk(pt, from, to, flags) {
        db<Segment>(TRC) << "Segment(pt=" << pt << ",from=" << from << ",to=" << to << ",flags=" << flags << ") [Chunk::pt=" << Chunk::pt() << ",sz=" << Chunk::size() << "] => " << this << endl;
    }
};

/*
class Port_Shared_Segment
{
public:
    int _port;
    Shared_Segment * _shared_seg;

public:
    Port_Shared_Memory(int port, Shared_Segment * shared_seg): _port(port), _shared_seg(shared_seg) {}
    int get_port() { return _port; }
    int get_shared_seg() { return _shared_seg; }
};
*/


class Shared_Segment: public Segment
{
private:
    int _port;
    typedef List<Shared_Segment> SS_List;

public:
    static SS_List _shared_segments;

public:
    Shared_Segment(int port, unsigned int bytes);

    static Shared_Segment * using_port(int port);

    void set_port(int port) {_port = port;}
    int get_port() {return _port;}
};



__END_SYS

#endif
