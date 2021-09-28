// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>
// #include <real-time.h>

__BEGIN_SYS

// The following Scheduling Criteria depend on Alarm, which is not available at scheduler.h
template <typename ... Tn>
FCFS::FCFS(int p, Tn & ... an): Priority((p == IDLE) ? IDLE : Alarm::elapsed()) {}

__END_SYS
