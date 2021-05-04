#include "Core8051.h"

int main(int argc, char* argv[])
{
    Core8051 testowy;
    testowy.LoadHex("demo51.hex");
    std::cin.get();
    testowy.ProgramRun(0, true);
    return 0;
}
