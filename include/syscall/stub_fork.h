// EPOS Component Declarations

#ifndef __stub_task_h
#define __stub_task_h

#include <architecture.h>
#include <syscall/message.h>
#include <memory.h>

__BEGIN_API

__USING_UTIL

static unsigned int fork() {
    _SYS::Message * msg = new  _SYS::Message(0,  _SYS::Message::ENTITY::FORK,  _SYS::Message::DO_FORK);
    msg->act();
    return msg->result();
};

__END_API

#endif
