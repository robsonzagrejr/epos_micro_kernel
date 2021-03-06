// EPOS Component Declarations

#ifndef __stub_task_h
#define __stub_task_h

#include <architecture.h>
#include <syscall/message.h>
#include <syscall/stub_address_space.h>
#include <syscall/stub_segment.h>
#include <memory.h>
#include <process.h>

__BEGIN_API

__USING_UTIL

class Stub_Task
{
private:
    int _id;
    static Stub_Task * volatile _current;
    typedef _SYS::Message Message;
    typedef _SYS::Address_Space Address_Space;
    typedef _SYS::Segment Segment;
    typedef _SYS::CPU CPU;

public:
    Stub_Task(){}
   
    template<typename ... Tn>
    Stub_Task(Stub_Segment * stub_cs, Stub_Segment * stub_ds, int (* entry)(Tn ...), const CPU::Log_Addr & code, const CPU::Log_Addr & data, Tn ... an){
        Segment * cs = reinterpret_cast<Segment *>(stub_cs->get_id());
        Segment * ds = reinterpret_cast<Segment *>(stub_ds->get_id());

        Message * msg = new Message(0, Message::ENTITY::TASK, Message::TASK_CREATE, cs, ds, entry, code, data);
        msg->act();
        _id = msg->result();
    }

    Stub_Address_Space * address_space() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_ADDRESS_SPACE);
        msg->act();
        int a = msg->result();
        Stub_Address_Space * as = new Stub_Address_Space();
        as->set_id(a);
        return reinterpret_cast<Stub_Address_Space *>(as);
    }

    Stub_Segment * code_segment() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_CODE_SEGMENT);
        msg->act();
        int s = msg->result();
        Stub_Segment * sc = new Stub_Segment();
        sc->set_id(s);
        return reinterpret_cast<Stub_Segment *>(sc);
    }

    Stub_Segment * data_segment() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_DATA_SEGMENT);
        msg->act();
        int s = msg->result();
        Stub_Segment * sd = new Stub_Segment();
        sd->set_id(s);
        return reinterpret_cast<Stub_Segment *>(sd);
    }

    Address_Space::Log_Addr code() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_CODE);
        msg->act();
        int l = msg->result();
        //return reinterpret_cast<Address_Space::Log_Addr>(l);
        return l;
    }

    Address_Space::Log_Addr data() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_DATA);
        msg->act();
        int l = msg->result();
        //return reinterpret_cast<Address_Space::Log_Addr>(l);
        return l;
    }

    /*Stub_Thread * main() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_MAIN);
        msg->act();
        int m = msg->result();
        Stub_Thread * st = new Stub_Thread();
        st->set_id(m);
        return reinterpret_cast<Stub_Thread *>(st);
    }*/

    int id() {
        Message * msg = new Message(_id, Message::ENTITY::TASK, Message::TASK_ID);
        msg->act();
        int r = msg->result();
        return reinterpret_cast<int>(r);
    }

    static Stub_Task * volatile self() {
        Message * msg = new Message(0, Message::ENTITY::TASK, Message::TASK_SELF);
        msg->act();
        int current_id = msg->result();
        _current->_id = current_id;
        return _current;
    }

    void set_id(int id) {_id = id;}

};

__END_API

#endif
