#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <fstream>
#include <bitset>
#include <array>
#include "Sfr.h"

union Word_t;

union Byte_t
{
    std::uint8_t byte;
    std::int8_t signedByte;

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
	Byte_t& operator++();
	Byte_t operator++(int);
	Byte_t& operator--();
	Byte_t operator--(int);
	Byte_t& operator+=(const Byte_t& rhs);
	Byte_t& operator-=(const Byte_t& rhs);
	Byte_t operator+(const Byte_t& rhs);
	Byte_t operator-(const Byte_t& rhs);
	Byte_t operator/(const Byte_t& rhs);
	Byte_t operator%(const Byte_t& rhs);
	Byte_t& operator|=(const Byte_t& rhs);
	Byte_t& operator&=(const Byte_t& rhs);
	Byte_t& operator^=(const Byte_t& rhs);
	friend std::ostream& operator<<(std::ostream& out, const Byte_t& b);

	operator int();
	operator Word_t();
};

union Word_t
{
	std::uint16_t word;
	struct
	{
		Byte_t lowByte;
		Byte_t highByte;
	};
	Word_t& operator++();
	Word_t operator++(int);
	Word_t& operator--();
	Word_t operator--(int);
	Word_t& operator+=(const Word_t& rhs);
	Word_t& operator-=(const Word_t& rhs);
	Word_t operator+(const Word_t& rhs);
	Word_t operator-(const Word_t& rhs);
	friend std::ostream& operator<<(std::ostream& out, const Word_t& w);

	operator int();
	operator Byte_t();
};

std::uint8_t CharToUint(const char& a);

class Core8051
{
    public:
        Core8051();
        virtual ~Core8051();
        void LoadBinary(const char* filePath);
        void LoadHex(const char* filePath);
        void ProgramRun(std::uint64_t cycles, bool step);
        void Test();


    private:
        std::pair<std::uint8_t, std::uint8_t> BitAddressDecoder(std::uint8_t bitAddress);
        bool GetBit(std::uint8_t bitAddress);
        void SetBit(std::uint8_t bitAddress);
        void ClearBit(std::uint8_t bitAddress);
        void InverseBit(std::uint8_t bitAddress);
        void WriteBit(std::uint8_t bitAddress, bool bit);
        void AccAdd(Byte_t num);
        void AccAddC(Byte_t num);
        void AccSubb(Byte_t num);
        void CalcBitP();
        void Cycle();
        void PrintCore();
        Byte_t& Register(std::uint8_t r);

        static std::function<void(Core8051* pt)> InstructionDecoder[0x2][0x100];
        Byte_t code[0x10000];
        Byte_t ram[0x100];
        Byte_t externalRam[0x10000];
        Word_t PC;
        std::uint64_t cycleCounter;

        //sfr's
        Byte_t& B;
        Byte_t& ACC;
        Byte_t& PSW;
        Byte_t& IP;
        Byte_t& P3;
        Byte_t& IE;
        Byte_t& P2;
        Byte_t& SBUF;
        Byte_t& SCON;
        Byte_t& P1;
        Byte_t& TH1;
        Byte_t& TH0;
        Byte_t& TL1;
        Byte_t& TL0;
        Byte_t& TMOD;
        Byte_t& TCON;
        Byte_t& PCON;
        Byte_t& DPH;
        Byte_t& DPL;
        Byte_t& SP;
        Byte_t& P0;
        Word_t& DPTR;
};

