// EPOS Segment Test Program
#include <time.h>
#include <process.h>
#include <machine.h>
#include <utility/fork.h>

using namespace EPOS;

int func_a();
int print_inf();

OStream cout;

int main()
{
    cout << "=====Test Task Fork "<< Task::self()->id()<<"=====" << endl;

    if (Task::self()->id() == 0) {
        fork(&func_a);
        fork(&func_a);
        cout << "Hello World! I'm Task: "<< Task::self()->id() << endl;
    }
    if (Task::self()->id() == 1) {
        cout << "Konnichiwa I'm Task: "<< Task::self()->id() << endl;
        func_a();
    }
    if (Task::self()->id() == 2) {
        cout << "Annyeong haseyo I'm Task: "<< Task::self()->id() << endl;
        func_a();
    }

    print_inf();
    cout << "Sayonara, bye! o/" << endl;
    return 0;
}

int print_inf() {
    Task * task = Task::self();
    Address_Space * as = task->address_space();
    cout << "===========================================" << "\n"
         << "=Address Space page directory: " << as->pd() << "=\n"
         << "=Code Logical Addr: " << static_cast<void *>(task->code()) << "=\n"
         << "=Code Physical Addr: " << static_cast<void *>(as->physical(task->code())) << "=\n"
         << "=Code Size: " << task->code_segment()->size() << " bytes long" << "=\n"
         << "=Data Logical Addr: " << static_cast<void *>(task->data()) << "=\n"
         << "=Data Physical Addr: " << static_cast<void *>(as->physical(task->data())) << "=\n"
         << "=Data Size: " << task->data_segment()->size() << " bytes long" << "=\n"
         << "===========================================" << endl;
    return 0;
}

int func_a() {
    cout << "Konnichiwa I'm Task: "<< Task::self()->id() << endl;
    int step = 1000000;
    for (int i = 0; i < 100000000; ++i) {
      if (!(i % step)) {
        ASM("_func_a_h:");
        cout << "Checkpoint! ("<<Task::self()->id() <<")("<<i/step<<")" << endl;
        // Alarm::delay(100000);
      }
    }
    cout << "Task " << Task::self()->id() << "says sayonara!" << endl;
    return 0;
}
