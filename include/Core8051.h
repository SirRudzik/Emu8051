#pragma once
#include <iostream>
#include <string>
#include <functional>

union MemoryCell
{
    std::uint8_t byte;
    std::int8_t signed_byte;
    struct
    {
        std::uint8_t n0 : 4;
        std::uint8_t n1 : 4;
    } nibble;
    struct
    {
        std::int8_t b0 : 1;
        std::int8_t b1 : 1;
        std::int8_t b2 : 1;
        std::int8_t b3 : 1;
        std::int8_t b4 : 1;
        std::int8_t b5 : 1;
        std::int8_t b6 : 1;
        std::int8_t b7 : 1;
    } bit;
};


class Core8051
{
    public:
        Core8051();
        virtual ~Core8051();
        LoadHex(std::string file);

    private:
        static std::function<void(Core8051* pt)> InstructionDecode[0x100];
        MemoryCell code[0x10000];
        MemoryCell ram[0x100];
        MemoryCell externalRam[0x10000];
        union
        {
        std::uint16_t word;
        struct
        {
            MemoryCell lowByte;
            MemoryCell highByte;
        } byte;
        } PC;
        std::pair<std::uint8_t, std::uint8_t> BitAddressDecoder(std::uint8_t BitAddress);
};
