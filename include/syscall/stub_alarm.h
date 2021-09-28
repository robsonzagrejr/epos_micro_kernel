// EPOS Component Declarations

#ifndef __stub_alarm_h
#define __stub_alarm_h

#include <architecture.h>
#include <syscall/message.h>
#include <time.h>

__BEGIN_API

__USING_UTIL

class Stub_Alarm
{
private:
    int id;
    typedef _SYS::Message Message;
    typedef _SYS::Microsecond Microsecond;
    typedef _SYS::Hertz Hertz;

public:
    template<typename ... Tn>
    Stub_Alarm(const Microsecond & time, Handler * handler, unsigned int times = 1, Tn ... an){
        Message * msg = new Message(0, Message::ENTITY::ALARM, Message::ALARM_CREATE, time, handler, times);
        msg->act();
        id = msg->result();
    }

    /*
    const Microsecond & period() {
        Message *msg = new Message(id, Message::ENTITY::ALARM, Message::ALARM_PERIOD);
        msg->act();
        return msg->result();
    }
    */


    void period(const Microsecond & p) {
        Message *msg = new Message(id, Message::ENTITY::ALARM, Message::ALARM_PERIOD1, p);
        msg->act();
    }

    void reset() {
        Message *msg = new Message(id, Message::ENTITY::ALARM, Message::ALARM_RESET);
        msg->act();
    }

    static Hertz frequency() {
        Message *msg = new Message(0, Message::ENTITY::ALARM, Message::ALARM_FREQUENCY);
        msg->act();
        return (msg->result());
    }

    static void delay(const Microsecond & time) {
        Message *msg = new Message(0, Message::ENTITY::ALARM, Message::ALARM_DELAY, time);
        msg->act();
    }

};

__END_API

#endif
