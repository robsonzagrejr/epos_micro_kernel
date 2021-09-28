#include <utility/ostream.h>
#include <syscall/stub_thread.h>
#include <syscall/stub_mutex.h>

using namespace EPOS;

OStream cout;

Stub_Mutex * m;

int func_a();
int func_b();

int main()
{
    cout << "Hello Syscall!" << endl;
    m->lock();
    Stub_Thread * a = new Stub_Thread(&func_a);
    Stub_Thread * b = new Stub_Thread(&func_b);
    m->unlock();
    a->join();
    b->join();
    cout << "By Syscall!" << endl;
    return 0;
}

int func_a(){
    m->lock();
    cout << "Funcao A" <<endl;
    m->unlock();
    cout << "By A" <<endl;
    return 0;
}

int func_b(){
    m->lock();
    cout << "Funcao B" <<endl;
    cout << "By B" <<endl;
    m->unlock();
    return 0;
}
