#include "Core8051.h"

int main(int argc, char* argv[])
{
    Core8051 testowy;
    testowy.LoadHex("Test_PUSH_POP.bin");
    std::cin.get();
    testowy.ProgramRun(0, true);
    testowy.Test();
    return 0;
}
