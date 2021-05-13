#include "Core8051.h"

int main(int argc, char* argv[])
{
    Core8051 testowy;
    testowy.LoadBinary("Test_CJNE.bin");
    std::cin.get();
    testowy.ProgramRun(0, true);
    testowy.Test();
    return 0;
}
