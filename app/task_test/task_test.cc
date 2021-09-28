// EPOS Task Test Program

#include <time.h>
#include <process.h>
#include <machine.h>
#include <system.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Task Addres Space and Segment test" << endl;
    System_Info * si = System::info();
    cout << "App location in System Info \n" 
         << " App Code Segment :" << CPU::Phy_Addr(si->pmm.app_code) << "\n"
         << " App Data Segment :" << CPU::Phy_Addr(si->pmm.app_data)
         << endl;

    Task * task0 = Task::self();
    Address_Space * as0 = task0->address_space();
    cout << "\nMy address space's page directory is located at " << as0->pd() << endl;

    Segment * cs0 = task0->code_segment();
    CPU::Log_Addr code0 = task0->code();
    CPU::Phy_Addr code_phy = as0->physical(code0);
    cout << "\nMy code segment is : " << "\n"
         << "Logical Addr: " << static_cast<void *>(code0) << "\n"
         << "Physical Addr: " << static_cast<void *>(code_phy) << "\n"
         << "Size: " << cs0->size() << " bytes long" << endl;

    // Assert if Logical Memory is Valid in Code Segment
    cout << "\nAssert if logic code is valid" << endl;
    assert(code0 == Memory_Map::APP_CODE);

    Segment * ds0 = task0->data_segment();
    CPU::Log_Addr data0 = task0->data();
    CPU::Phy_Addr data_phy = as0->physical(data0);
    cout << "\nMy data segment is : " << "\n"
         << "Logical Addr: " << static_cast<void *>(data0) << "\n"
         << "Physical Addr: " << static_cast<void *>(data_phy) << "\n"
         << "Size: " << ds0->size() << " bytes long" << endl;
    
    // Assert if Logical Memory is Valid in Data Segment
    cout << "\nAssert if logic data is valid" << endl;
    assert(data0 == Memory_Map::APP_DATA);

    // Assert if the Physicall memory data is located with correct space from code
    cout << "Assert if phy data has size needed" << endl;
    assert((data_phy + ds0->size()) == code_phy);

    cout << "I'm done, bye!" << endl;
    return 0;
}

