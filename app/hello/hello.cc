#include <utility/ostream.h>
// #include <syscall/stub_thread.h>
#include <syscall/stub_task.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Task id = " << Stub_Task::self()->id() << endl;
    cout << "Hello world!" << endl;


    return 0;
}
