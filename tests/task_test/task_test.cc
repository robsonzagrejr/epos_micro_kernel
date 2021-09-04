// EPOS Task Test Program

#include <time.h>
#include <process.h>

using namespace EPOS;

const int iterations = 10;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int main()
{
    cout << "Task test" << endl;

    m = Thread::self();

    Task * task0 = Task::self();
    Address_Space * as0 = task0->address_space();
    cout << "My address space's page directory is located at " << as0->pd() << endl;

    Segment * cs0 = task0->code_segment();
    CPU::Log_Addr code0 = task0->code();
    cout << "My code segment is located at " << static_cast<void *>(code0) << " and it is " << cs0->size() << " bytes long" << endl;

    Segment * ds0 = task0->data_segment();
    CPU::Log_Addr data0 = task0->data();
    cout << "My data segment is located at " << static_cast<void *>(data0) << " and it is " << ds0->size() << " bytes long" << endl;

    cout << "I'll now fork a new task:" << endl;

    cout << "Creating the new code segment ... ";
    Segment * cs1 = new Segment(cs0->size());
    cout << "done!" << endl;
    CPU::Log_Addr code1 = as0->attach(cs1);
    cout << "The new code segment is attached to my address space at " << code1 << endl;
    cout << "Creating the new data segment ... ";
    Segment * ds1 = new Segment(ds0->size());
    cout << "done!" << endl;
    CPU::Log_Addr data1 = as0->attach(ds1);
    cout << "The new data segment is attached to my address space at " << data1 << endl;
    cout << "Coping the code segment ... ";
    memcpy(code1, code0, cs0->size());
    cout << "done!" << endl;
    cout << "Coping the data segment ... ";
    memcpy(data1, data0, ds0->size());
    cout << "done!" << endl;
    cout << "Detaching segments from my address space ... ";
    as0->detach(cs1);
    as0->detach(ds1);
    cout << "done!" << endl;

    cout << "Creating the new task that will run func B ... ";
    Task * task1 = new Task(cs1, ds1, code0, data0, &func_b);
    b = task1->main();
    cout << "done!" << endl;

    cout << "Creating a thread to run func A ... ";
    a = new Thread(&func_a);
    cout << " done!" << endl;

    cout << "I'll now suspend my self to see the other threads running." << endl;
    m->suspend();

    Alarm::delay(3000000);

    cout << "Both threads must be done by now and must have suspended themselves. I'll now wait for 1 second and then wake them up so they can exit ...";
    a->resume();

    Thread::yield();
    b->resume();

    cout << " done!" << endl;

    cout << "I'll now wait for the threads to finish ... " << endl;
    int status_a = a->join();
    int status_b = b->join();
    cout << " done!" << endl;
    cout << "Thread A exited with status " << status_a << " and thread B exited with status " << status_b << "." << endl;

    cout << "I'll now delete everything I have created ... ";
    delete task1;
    delete cs1;
    delete ds1;
    delete a;

    cout << " done!" << endl;


    cout << "\nI'll now repeat the test with Task(task) constructor." << endl;

    cout << "Creating the new task that will run func B ... ";
    task1 = new Task(task0, &func_b);
    b = task1->main();
    cout << "done!" << endl;

    cout << "Creating a thread to run func A ... ";
    a = new Thread(&func_a);
    cout << " done!" << endl;

    cout << "I'll now suspend my self to see the other threads running." << endl;
    m->suspend();

    Alarm::delay(3000000);

    cout << "Both threads must be done by now and must have suspended themselves. I'll now wait for 1 second and then wake them up so they can exit ...";
    a->resume();

    Thread::yield();
    b->resume();

    cout << " done!" << endl;

    cout << "I'll now wait for the threads to finish ... " << endl;
    status_a = a->join();
    status_b = b->join();
    cout << " done!" << endl;
    cout << "Thread A exited with status " << status_a << " and thread B exited with status " << status_b << "." << endl;

    cout << "I'll now delete everything I have created ... " << endl;
    delete task1;
    delete a;

    cout << "I'm also done, bye!" << endl;

    return 0;
}


int func_a(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "a";
        cout << endl;
        Thread::yield();
    }

    Thread::self()->suspend();

    return 'A';
}

int func_b(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "b";
        cout << endl;
        Thread::yield();
    }

    m->resume();

    Thread::self()->suspend();

    return 'B';
}
