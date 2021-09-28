// EPOS Component Declarations

#ifndef __stub_shared_memory_h
#define __stub_shared_memory_h

#include <architecture.h>
#include <syscall/message.h>
#include <utility/list.h>
#include <memory.h>

__BEGIN_API

__USING_UTIL


class Stub_Shared_Segment
{
private:
    int _id;
    typedef _SYS::Message Message;
    typedef _SYS::MMU MMU;
    typedef _SYS::Segment Segment;


public:
    Stub_Shared_Segment(){}
    void set_id(int id) {_id = id;}
    int get_id() {return _id;}

    Stub_Shared_Segment(int port, unsigned int bytes) {
        Message * msg = new Message(0, Message::ENTITY::SHARED_SEGMENT, Message::SHARED_SEGMENT_CREATE, port, bytes);
        msg->act();
        _id = msg->result();
    }        

    int get_port() {
        Message * msg = new Message(_id, Message::ENTITY::SHARED_SEGMENT, Message::SHARED_SEGMENT_PORT);
        msg->act();
        return msg->result();
    }
    /*
    static void add_to_list(List::Element * e){
        _shared_ports.insert(e);
    }

    static int using_port(int port) {
        int id = -1;
        List::Element * e = _shared_ports.head();
        for(; e && (e->object()->get_port() != port); e = e->next());
        if (e) {
            id = e->object()->get_stub_shared_mem_id();
        }
        return id;
    }

    //Stub_Shared_Memory(int port, unsigned int bytes);
    Stub_Shared_Memory(int port, unsigned int bytes){
        int id = using_port(port);
        if (id == -1 ) {
            Message * msg = new Message(0, Message::ENTITY::SHARED_MEMORY, Message::SHARED_MEMORY_CREATE, bytes, MMU::Flags::APPD);
            msg->act();
            id = msg->result();
            List::Element * e = new List::Element(new Port_Shared_Memory(port, id));
            add_to_list(e);
        }
        set_id(id);
        set_port(port);
    }
    */
};

__END_API

#endif
