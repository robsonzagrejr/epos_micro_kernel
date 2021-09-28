#include <process.h>
#include <memory.h>
#include <system.h>
#include <utility/ostream.h>
#include <utility/elf.h>
#include <syscall/stub_thread.h>
#include <syscall/stub_task.h>
#include <syscall/stub_segment.h>
#include <syscall/stub_address_space.h>

using namespace EPOS;

OStream cout;

int main(int argc, char**argv)
{
    cout << "Hello Loader!" << endl;
    cout << "ARGC: "<< argc << "ARGV: "<< hex << argv << endl;

    Stub_Task * c_task = Stub_Task::self();
    cout << "TASK ID:  "<< c_task->id() << endl;

    int * extras = reinterpret_cast<int*>(argv);
    for (int app_size = *extras; app_size; extras += app_size/4, app_size = *reinterpret_cast<int*>(extras)) {
        ELF * ini_elf = reinterpret_cast<ELF *>(++extras);
        if (!ini_elf->valid()) {
            cout << "Skipping corrupted App" << endl;
            continue;
        }

        Elf32_Addr ini_entry = ini_elf->entry();
        int ini_segments = ini_elf->segments();
        Elf32_Addr ini_code = ini_elf->segment_address(0);
        int ini_code_size = ini_elf->segment_size(0);
        Elf32_Addr ini_data = 0xffffffff;
        int ini_data_size = 0;
        if(ini_elf->segments() > 1) {
            for(int i = 1; i < ini_elf->segments(); i++) {
                if(ini_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(ini_elf->segment_address(i) < ini_data) {
                    ini_data = ini_elf->segment_address(i);
                }
                ini_data_size += ini_elf->segment_size(i);
            }
        }

        // Add heap size to data segment
        ini_data_size = _SYS::MMU::align_page(ini_data_size);
        // ini_data_size += _SYS::MMU::align_page(Traits<Application>::STACK_SIZE);
        ini_data_size += _SYS::MMU::align_page(_SYS::Application::HEAP_SIZE);

        cout << "APP NEW: " << "\n"
             << "Entry: " << hex << ini_entry << "\n"
             << "Segments: " << ini_segments << "\n"
             << "Code: " << hex << ini_code << "\n"
             << "Code Size: " << ini_code_size << "\n"
             << "Data: " << hex << ini_data << "\n"
             << "Data Size: " << ini_data_size << endl;


        cout << "==============================" << endl;
        cout << "Copying code to segment " << endl;
        Stub_Segment * cs = new Stub_Segment(ini_code_size, _SYS::MMU::Flags::APP);
        _SYS::CPU::Log_Addr cs_addr = c_task->address_space()->attach(cs);
        ini_elf->load_segment(0, cs_addr);
        c_task->address_space()->detach(cs, cs_addr);
        cout << "Finished code to segment " << endl;

        cout << "Copying data to segment " << endl;
        Stub_Segment * ds = new Stub_Segment(ini_data_size, _SYS::MMU::Flags::APP);
        _SYS::CPU::Log_Addr ds_addr = c_task->address_space()->attach(ds);
        // ini_elf->load_segment(1, ds_addr);
        _SYS::CPU::Log_Addr aux_ds_addr =  ds_addr;
        for(int j = 1; j < ini_elf->segments(); j++){
            if(ini_elf->segment_size(j) > 0){
                ini_elf->load_segment(j, aux_ds_addr);
                aux_ds_addr += ini_elf->segment_size(j);
            }
        }
        c_task->address_space()->detach(ds, ds_addr);
        cout << "Finished data to segment " << endl;

        typedef int (Main)();
        Main * n_main = reinterpret_cast<Main *>(ini_entry);
        _SYS::CPU::Log_Addr h_ini_code = reinterpret_cast<void *>(ini_code);
        _SYS::CPU::Log_Addr h_ini_data = reinterpret_cast<void *>(ini_data);
        new Stub_Task(cs, ds, n_main, h_ini_code, h_ini_data);
        cout << "New task for APP created" << endl;
    }
    return 0;
}
