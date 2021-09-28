#include <utility/ostream.h>
#include <syscall/stub_fork.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Hello world!" << endl;
    int r;
    r = fork();
    cout << "HAHAHA FUNCIONOU : " << r << endl;
    cout << "HOHOHO FUNCIONOU : " << r << endl;
    return 0;
}
