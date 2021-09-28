#include <utility/ostream.h>
#include <syscall/stub_thread.h>
#include <syscall/stub_mutex.h>

using namespace EPOS;

OStream cout;

Stub_Mutex m;

int func_a();
int func_b();

int main()
{
    cout << "Hello Syscall!" << endl;
    m.lock();
    Stub_Thread * a = new Stub_Thread(&func_a);
    Stub_Thread * b = new Stub_Thread(&func_b);
    m.unlock();
    a->join();
    for (int i = 0; i < 10000000; ++i) {
        if (!(i % 100000)) {
            cout << "Apos A " << i / 100000 << endl;
        }
    }
    b->join();
    cout << "Bye Syscall!" << endl;
    return 0;
}

int func_a(){
    m.lock();
    cout << "Funcao A" <<endl;
    m.unlock();
    for (int i = 0; i < 10000000; ++i) {
        if (!(i % 100000)) {
            cout << "FA " << i / 100000 << endl;
        }
    }
    cout << "Bye A" <<endl;
    return 0;
}

int func_b(){
    m.lock();
    cout << "Funcao B" <<endl;
    m.unlock();
    for (int i = 0; i < 10000000; ++i) {
        if (!(i % 100000)) {
            cout << "FB " << i / 100000 << endl;
        }
    }
    cout << "Bye B" <<endl;
    return 0;
}
