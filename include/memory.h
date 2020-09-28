// EPOS Memory Declarations

#ifndef __memory_h
#define __memory_h

#include <architecture.h>

__BEGIN_SYS

class Address_Space: private MMU::Directory
{

private:
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
    Log_Addr attach(Segment * seg, const Log_Addr & addr);
    void detach(Segment * seg);
    void detach(Segment * seg, const Log_Addr & addr);

    Phy_Addr physical(const Log_Addr & address);
};


class Segment: public MMU::Chunk
{
private:
    typedef MMU::Chunk Chunk;

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;

public:
    Segment(unsigned int bytes, const Color & color = Color::WHITE, const Flags & flags = Flags::APP);
    Segment(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags);
    ~Segment();

    unsigned int size() const;
    Phy_Addr phy_address() const;
    int resize(int amount);
};

__END_SYS

#endif
