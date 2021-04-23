#pragma once
#include <iostream>
#include <string>
#include <functional>
#include "Sfr.h"

union Byte
{
    std::uint8_t byte;
    std::int8_t signed_byte;

    struct
    {
        std::uint8_t n0 : 4;
        std::uint8_t n1 : 4;
    };

    struct
    {
        std::uint8_t b0 : 1;
        std::uint8_t b1 : 1;
        std::uint8_t b2 : 1;
        std::uint8_t b3 : 1;
        std::uint8_t b4 : 1;
        std::uint8_t b5 : 1;
        std::uint8_t b6 : 1;
        std::uint8_t b7 : 1;
    };
};

union Word
{
	std::uint16_t word;
	struct
	{
		Byte lowByte;
		Byte highByte;
	};
};

class Core8051
{
    public:
        Core8051();
        virtual ~Core8051();
        LoadHex(std::string file);

    private:
        std::pair<std::uint8_t, std::uint8_t> BitAddressDecoder(std::uint8_t BitAddress);
        Byte& Register(std::uint8_t r);

        static std::function<void(Core8051* pt)> InstructionDecoder[0x100];
        Byte code[0x10000];
        Byte ram[0x100];
        Byte externalRam[0x10000];
        Word PC;

        //sfr's
        Byte& B;
        Byte& ACC;
        Byte& PSW;
        Byte& IP;
        Byte& P3;
        Byte& IE;
        Byte& P2;
        Byte& SBUF;
        Byte& SCON;
        Byte& P1;
        Byte& TH1;
        Byte& TH0;
        Byte& TL1;
        Byte& TL0;
        Byte& TMOD;
        Byte& TCON;
        Byte& PCON;
        Byte& DPH;
        Byte& DPL;
        Byte& SP;
        Byte& P0;
};
