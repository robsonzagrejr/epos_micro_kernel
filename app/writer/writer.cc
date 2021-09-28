#include <process.h>
#include <memory.h>
#include <utility/ostream.h>
#include <syscall/stub_shared_segment.h>
#include <syscall/stub_task.h>
#include <syscall/stub_segment.h>

using namespace EPOS;

OStream cout;

int main()
{
    int port = 13;
    unsigned int size = 1024;
    Stub_Task * c_task = Stub_Task::self();
    Stub_Shared_Segment * shared_seg = new Stub_Shared_Segment(port, size);
    

    cout << "Shared_Memory " << "\n"
         << "id: "   << shared_seg->get_id()   << "\n"
         << "port: " << shared_seg->get_port() << endl;

    Stub_Segment * seg_shared_seg = reinterpret_cast<Stub_Segment *>(shared_seg);
    _SYS::CPU::Log_Addr shared_seg_addr = c_task->address_space()->attach(seg_shared_seg);

    long unsigned int * shared_aux = shared_seg_addr;
    char * shared_msg = reinterpret_cast<char *>(shared_aux);
    char msg[60] = "Essa é uma mensagem do writer para o reader :)";
    for (int i = 0; i < 60; i++) {
        shared_msg[i] = msg[i];
    }
    cout << "Writer escreveu em " << hex << &shared_msg << " mensagem: '"; 
    for (int i = 0; i < 60; i++) {
        cout << shared_msg[i];
    }
    cout << "'" << endl;
    return 0;
}
