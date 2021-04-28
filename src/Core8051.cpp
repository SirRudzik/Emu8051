#include "Core8051.h"

//Byte_t operators definitions
Byte_t& Byte_t::operator++(){byte++; return *this;}
Byte_t Byte_t::operator++(int){Byte_t temp = *this; operator++(); return temp;}
Byte_t& Byte_t::operator--(){byte--; return *this;}
Byte_t Byte_t::operator--(int){Byte_t temp = *this; operator--(); return temp;}
Byte_t& Byte_t::operator+=(const Byte_t& rhs){byte += rhs.byte; return *this;}
Byte_t& Byte_t::operator-=(const Byte_t& rhs){byte -= rhs.byte; return *this;}
Byte_t Byte_t::operator+(const Byte_t& rhs){Byte_t temp = *this; temp += rhs; return temp;}
Byte_t Byte_t::operator-(const Byte_t& rhs){Byte_t temp = *this; temp -= rhs; return temp;}
Byte_t& Byte_t::operator|=(const Byte_t& rhs){byte |= rhs.byte; return *this;}
Byte_t& Byte_t::operator&=(const Byte_t& rhs){byte &= rhs.byte; return *this;}
Byte_t& Byte_t::operator^=(const Byte_t& rhs){byte ^= rhs.byte; return *this;}

Byte_t::operator int(){return byte;}
Byte_t::operator Word_t(){return Word_t{byte};}

//Word_t operators definitions
Word_t& Word_t::operator++(){word++; return *this;}
Word_t Word_t::operator++(int){Word_t temp = *this; operator++(); return temp;}
Word_t& Word_t::operator--(){word--; return *this;}
Word_t Word_t::operator--(int){Word_t temp = *this; operator--(); return temp;}
Word_t& Word_t::operator+=(const Word_t& rhs){word += rhs.word; return *this;}
Word_t& Word_t::operator-=(const Word_t& rhs){word -= rhs.word; return *this;}
Word_t Word_t::operator+(const Word_t& rhs){Word_t temp = *this; temp += rhs; return temp;}
Word_t Word_t::operator-(const Word_t& rhs){Word_t temp = *this; temp -= rhs; return temp;}

Word_t::operator int(){return word;}
Word_t::operator Byte_t(){return lowByte;}

//Core8051 methods  definitions

Core8051::Core8051() :
    code{},
    ram{},
    externalRam{},
    B(ram[0xF0]),
    ACC(ram[0xE0]),
    PSW(ram[0xD0]),
    IP(ram[0xC0]),
    P3(ram[0xB0]),
    IE(ram[0xA8]),
    P2(ram[0xA0]),
    SBUF(ram[0x99]),
    SCON(ram[0x98]),
    P1(ram[0x90]),
    TH1(ram[0x8D]),
    TH0(ram[0x8C]),
    TL1(ram[0x8B]),
    TL0(ram[0x8A]),
    TMOD(ram[0x89]),
    TCON(ram[0x88]),
    PCON(ram[0x87]),
    DPH(ram[0x83]),
    DPL(ram[0x82]),
    SP(ram[0x81]),
    P0(ram[0x80]),
    DPTR(reinterpret_cast<Word_t&>(ram[0x82]))
{
    //ctor
}

Core8051::~Core8051()
{
    //dtor
}


std::pair<std::uint8_t, std::uint8_t> Core8051::BitAddressDecoder(std::uint8_t bitAddress)
{
    if(bitAddress < 0x80)
    {
        return std::pair<std::uint8_t, std::uint8_t>( 0x20 + (bitAddress /  8), (bitAddress % 8));
    }
    else
    {
        Byte_t temp;
        temp.byte = bitAddress;
        std::uint8_t cell = (temp.n1 * 0x10) + ( temp.n0 < 8 ? 0 : 8);
        std::uint8_t bit = (temp.n0 < 8 ? temp.n0 : temp.n0 - 8);
        return std::pair<std::uint8_t, std::uint8_t>(cell, bit);
    }
}

bool Core8051::GetBit(std::uint8_t bitAddress)
{
    std::pair<std::uint8_t, std::uint8_t> pos = BitAddressDecoder(bitAddress);
    Byte_t temp = ram[pos.first];
    temp.byte >>= pos.second;
    return (temp.b0 == 0 ? false : true);
}

void Core8051::SetBit(std::uint8_t bitAddress)
{
    std::pair<std::uint8_t, std::uint8_t> pos = BitAddressDecoder(bitAddress);
    Byte_t temp{0};
    temp.b0 = 1;
    temp.byte <<= pos.second;
    ram[pos.first] |= temp;
}

void Core8051::ClearBit(std::uint8_t bitAddress)
{
    std::pair<std::uint8_t, std::uint8_t> pos = BitAddressDecoder(bitAddress);
    Byte_t temp{0};
    temp.b0 = 1;
    temp.byte <<= pos.second;
    ram[pos.first] &= temp;
}

void Core8051::InverseBit(std::uint8_t bitAddress)
{
    std::pair<std::uint8_t, std::uint8_t> pos = BitAddressDecoder(bitAddress);
    Byte_t temp{0};
    temp.b0 = 1;
    temp.byte <<= pos.second;
    ram[pos.first] ^= temp;
}

void Core8051::WriteBit(std::uint8_t bitAddress, bool bit)
{
    if(bit)
    {
        SetBit(bitAddress);
    }
    else
    {
        ClearBit(bitAddress);
    }
}

Byte_t& Core8051::Register(std::uint8_t r)
{
    Byte_t temp;
    temp.byte = r;
    temp.b3 = PSW.b3;
    temp.b4 = PSW.b4;
    return ram[temp];
}

std::function<void(Core8051* pt)> Core8051::InstructionDecoder[] =
{
   { [](Core8051* pt){pt->PC++;} }, //NOP
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.highByte = pt->code[pt->PC++]; temp.lowByte = pt->code[pt->PC]; pt->PC = temp; pt->PC++;} }, //LJMP code addr
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b0; pt->ACC.byte >>= 1; pt->ACC.b7 = temp;} }, //RR A
   { [](Core8051* pt){pt->PC++; pt->ACC++;} }, //INC A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]]++; pt->PC++;} }, //INC data addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]++;} }, //INC @R0
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]++;} }, //INC @R1
   { [](Core8051* pt){pt->PC++; pt->Register(0)++;} }, //INC R0
   { [](Core8051* pt){pt->PC++; pt->Register(1)++;} }, //INC R1
   { [](Core8051* pt){pt->PC++; pt->Register(2)++;} }, //INC R2
   { [](Core8051* pt){pt->PC++; pt->Register(3)++;} }, //INC R3
   { [](Core8051* pt){pt->PC++; pt->Register(4)++;} }, //INC R4
   { [](Core8051* pt){pt->PC++; pt->Register(5)++;} }, //INC R5
   { [](Core8051* pt){pt->PC++; pt->Register(6)++;} }, //INC R6
   { [](Core8051* pt){pt->PC++; pt->Register(7)++;} }, //INC R7
   { [](Core8051* pt){pt->PC++;} }, //JBC bt addr, code addr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++;} }, //LCALL code addr
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7; pt->PSW.b7 = pt->ACC.b0; pt->ACC.byte >>= 1; pt->ACC.b7 = temp;} }, //RRC A
   { [](Core8051* pt){pt->PC++; pt->ACC--;} }, //DEC A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]]--; pt->PC++;} }, //DEC data addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]--;} }, //DEC @R0
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]--;} }, //DEC @R1
   { [](Core8051* pt){pt->PC++; pt->Register(0)--;} }, //DEC R0
   { [](Core8051* pt){pt->PC++; pt->Register(1)--;} }, //DEC R1
   { [](Core8051* pt){pt->PC++; pt->Register(2)--;} }, //DEC R2
   { [](Core8051* pt){pt->PC++; pt->Register(3)--;} }, //DEC R3
   { [](Core8051* pt){pt->PC++; pt->Register(4)--;} }, //DEC R4
   { [](Core8051* pt){pt->PC++; pt->Register(5)--;} }, //DEC R5
   { [](Core8051* pt){pt->PC++; pt->Register(6)--;} }, //DEC R6
   { [](Core8051* pt){pt->PC++; pt->Register(7)--;} }, //DEC R7
   { [](Core8051* pt){pt->PC++;} }, //JB bit addr, code addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++;} }, //RET
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b7; pt->ACC.byte <<= 1; pt->ACC.b0 = temp;} }, //RL A
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC]; pt->PC++;} }, //ADD A,#data
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ADD A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)];} }, //ADD A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)];} }, //ADD A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0);} }, //ADD A,R0
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1);} }, //ADD A,R1
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2);} }, //ADD A,R2
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3);} }, //ADD A,R3
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4);} }, //ADD A,R4
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5);} }, //ADD A,R5
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6);} }, //ADD A,R6
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7);} }, //ADD A,R7
   { [](Core8051* pt){pt->PC++;} }, //JNB bit addr, codeaddr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++;} }, //RETI
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7; pt->PSW.b7 = pt->ACC.b7; pt->ACC.byte <<= 1; pt->ACC.b0 = temp;} }, //RLC A
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC] += Byte_t{pt->PSW.b7}; pt->PC++;} }, //ADDC A,#data
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC]] += Byte_t{pt->PSW.b7}; pt->PC++;} }, //ADDC A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)] += Byte_t{pt->PSW.b7};} }, //ADDC A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)] += Byte_t{pt->PSW.b7};} }, //ADDC A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0) += Byte_t{pt->PSW.b7};} }, //ADDC A,R0
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1) += Byte_t{pt->PSW.b7};} }, //ADDC A,R1
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2) += Byte_t{pt->PSW.b7};} }, //ADDC A,R2
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3) += Byte_t{pt->PSW.b7};} }, //ADDC A,R3
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4) += Byte_t{pt->PSW.b7};} }, //ADDC A,R4
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5) += Byte_t{pt->PSW.b7};} }, //ADDC A,R5
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6) += Byte_t{pt->PSW.b7};} }, //ADDC A,R6
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7) += Byte_t{pt->PSW.b7};} }, //ADDC A,R7
   { [](Core8051* pt){pt->PC++;} }, //JC code addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] &= pt->ACC; pt->PC++;} }, //ORL data addr,A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] &= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //ORL data addr,#data
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->code[pt->PC]; pt->PC++;} }, //ORL data A,#data
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ORL A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(0)];} }, //ORL data A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(1)];} }, //ORL data A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(0);} }, //ORL data A,R0
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(1);} }, //ORL data A,R1
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(2);} }, //ORL data A,R2
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(3);} }, //ORL data A,R3
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(4);} }, //ORL data A,R4
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(5);} }, //ORL data A,R5
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(6);} }, //ORL data A,R6
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(7);} }, //ORL data A,R7
   { [](Core8051* pt){pt->PC++;} }, //JNC code addr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] |= pt->ACC; pt->PC++;} }, //ANL data addr,A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] |= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //ANL data addr,#data
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->code[pt->PC]; pt->PC++;} }, //ANL A,#data
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ANL A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(0)];} }, //ANL A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(1)];} }, //ANL A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(0);} }, //ANL A,R0
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(1);} }, //ANL A,R1
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(2);} }, //ANL A,R2
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(3);} }, //ANL A,R3
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(4);} }, //ANL A,R4
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(5);} }, //ANL A,R5
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(6);} }, //ANL A,R6
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(7);} }, //ANL A,R7
   { [](Core8051* pt){pt->PC++; if(pt->ACC){pt->PC += pt->code[++(pt->PC)];} else{pt->PC++;};} }, //JZ code addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP codeaddr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] ^= pt->ACC; pt->PC++;} }, //XRL data addr,A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] ^= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //XRL data addr,#data
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->code[pt->PC]; pt->PC++;} }, //XRL A,#data
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //XRL A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(0)];} }, //XRL A.@R0
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(1)];} }, //XRL A.@R1
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(0);} }, //XRL A.R0
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(1);} }, //XRL A.R1
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(2);} }, //XRL A.R2
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(3);} }, //XRL A.R3
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(4);} }, //XRL A.R4
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(5);} }, //XRL A.R5
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(6);} }, //XRL A.R6
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(7);} }, //XRL A.R7
   { [](Core8051* pt){pt->PC++; if(!(pt->ACC)){pt->PC += pt->code[++(pt->PC)];} else{pt->PC++;};} }, //JNZ code addr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) && pt->GetBit(pt->code[pt->PC]))); pt->PC++;} }, //ORL C, !bit addr
   { [](Core8051* pt){pt->PC++; pt->PC = pt->ACC + pt->DPTR;} }, //JMP @A+DPTR
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC]; pt->PC++ ;} }, //MOV A,#data
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->code[pt->PC + Byte_t{1}]; pt->PC += Word_t{2};} }, //MOV data addr,#data
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->code[pt->PC]; pt->PC++;} }, //MOV @R0,#data
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->code[pt->PC]; pt->PC++;} }, //MOV @R1,#data
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->code[pt->PC]; pt->PC++;} }, //MOV R0,#data
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->code[pt->PC]; pt->PC++;} }, //MOV R1,#data
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->code[pt->PC]; pt->PC++;} }, //MOV R2,#data
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->code[pt->PC]; pt->PC++;} }, //MOV R3,#data
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->code[pt->PC]; pt->PC++;} }, //MOV R4,#data
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->code[pt->PC]; pt->PC++;} }, //MOV R5,#data
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->code[pt->PC]; pt->PC++;} }, //MOV R6,#data
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->code[pt->PC]; pt->PC++;} }, //MOV R7,#data
   { [](Core8051* pt){pt->PC++;} }, //SJMP code addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) || pt->GetBit(pt->code[pt->PC]))); pt->PC++;} }, //ANL C bit addr
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC + pt->ACC];} }, //MOVC A,@A+PC
   { [](Core8051* pt){pt->PC++;} }, //DIV AB
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->code[pt->PC].byte + 1]; pt->PC += Word_t{2};} }, //MOV data addr,data addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->Register(0)]; pt->PC++;} }, //MOV data addr,@R0
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->Register(1)]; pt->PC++;} }, //MOV data addr,@R1
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(0); pt->PC++;} }, //MOV data addr,R0
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(1); pt->PC++;} }, //MOV data addr,R1
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(2); pt->PC++;} }, //MOV data addr,R2
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(3); pt->PC++;} }, //MOV data addr,R3
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(4); pt->PC++;} }, //MOV data addr,R4
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(5); pt->PC++;} }, //MOV data addr,R5
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(6); pt->PC++;} }, //MOV data addr,R6
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(7); pt->PC++;} }, //MOV data addr,R7
   { [](Core8051* pt){pt->PC++; pt->DPTR += pt->code[pt->PC]; pt->PC += Word_t{2};} }, //MOV DPTR,#data
   { [](Core8051* pt){pt->PC++;} }, //ACALL codeaddr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC], pt->GetBit(PSW_7)); pt->PC++;} }, //MOV bit addr,C
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->DPTR + pt->ACC];} }, //MOVC A,@A+DPTR
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,#data
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,data addr
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,@R0
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,@R1
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R0
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R1
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R2
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R3
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R4
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R5
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R6
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R7
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) && !(pt->GetBit(pt->code[pt->PC])))); pt->PC++;} }, //ORL C,!bit addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(pt->code[pt->PC])); pt->PC++;} }, //MOV C,bit addr
   { [](Core8051* pt){pt->PC++; pt->DPTR++;} }, //INC DPTR
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.word = pt->ACC.byte * pt->B.byte; pt->ACC = temp.lowByte; pt->B = temp.highByte;} }, //MUL AB
   { [](Core8051* pt){pt->PC++;} }, //reserved
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV @R0,data addr
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV @R1,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R0,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R1,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R2,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R3,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R4,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R5,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R6,data addr
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R7,data addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) || !(pt->GetBit(pt->code[pt->PC])))); pt->PC++;} }, //ANL C,!bit addr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC], !pt->GetBit(pt->code[pt->PC])); pt->PC++;} }, //CPL bit addr
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, !pt->GetBit(PSW_7));} }, //CPL C
   { [](Core8051* pt){pt->PC++;} }, //CJNE A,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE A,data addr,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE @R0,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE @R1,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R0,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R1,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R2,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R3,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R4,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R5,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R6,#data,code addr
   { [](Core8051* pt){pt->PC++;} }, //CJNE R6,#data,code addr
   { [](Core8051* pt){pt->PC++; pt->ram[++(pt->SP)] = pt->ram[pt->code[++(pt->PC)]];} }, //PUSH data addr
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; pt->ClearBit(pt->code[pt->PC]); pt->PC++;} }, //CLR bit addr
   { [](Core8051* pt){pt->PC++; pt->ClearBit(PSW_7);} }, //CLR C
   { [](Core8051* pt){pt->PC++; Byte_t temp; temp.n0 = pt->ACC.n1; temp.n1 = pt->ACC.n0; pt->ACC.byte = temp.byte;} }, //SWAP A
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->code[pt->PC]]; pt->ram[pt->code[pt->PC]] = temp; pt->PC++;} }, //XCH A,data addr
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(0)]; pt->ram[pt->Register(0)] = temp;} }, //XCH A,@R0
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(1)]; pt->ram[pt->Register(1)] = temp;} }, //XCH A,@R1
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(0); pt->Register(0) = temp;} }, //XCH A,R0
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(1); pt->Register(1) = temp;} }, //XCH A,R1
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(2); pt->Register(2) = temp;} }, //XCH A,R2
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(3); pt->Register(3) = temp;} }, //XCH A,R3
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(4); pt->Register(4) = temp;} }, //XCH A,R4
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(5); pt->Register(5) = temp;} }, //XCH A,R5
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(6); pt->Register(6) = temp;} }, //XCH A,R6
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(7); pt->Register(7) = temp;} }, //XCH A,R7
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[++(pt->PC)]] = pt->ram[pt->SP--];} }, //POP data addr
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++; pt->SetBit(pt->code[pt->PC]); pt->PC++;} }, //SETB bit addr
   { [](Core8051* pt){pt->PC++; pt->SetBit(PSW_7);} }, //SETB C
   { [](Core8051* pt){pt->PC++;} }, //DA A
   { [](Core8051* pt){pt->PC++;} }, //DJNZ data addr, code addr
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(0)].n0;} }, //XCHD A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(1)].n0;} }, //XCHD A,@R1
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R0, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R1, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R2, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R3, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R4, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R5, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R6, code addr
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R7, code addr
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->DPTR];} }, //MOVX A,@DPTR
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.lowByte = pt->code[pt->PC - Byte_t{1}]; temp.word <<= 3; temp.lowByte = pt->code[pt->PC]; pt->PC++; pt->PC = temp;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} }, //MOVX A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} }, //MOVX A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC = Byte_t{0x0};} }, //CLR A
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV A,data addr
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} }, //MOV A,@R0
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} }, //MOV A,@R1
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(0);} }, //MOV A,R0
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(1);} }, //MOV A,R1
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(2);} }, //MOV A,R2
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(3);} }, //MOV A,R3
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(4);} }, //MOV A,R4
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(5);} }, //MOV A,R5
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(6);} }, //MOV A,R6
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(7);} }, //MOV A,R7
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->DPTR].byte = pt->ACC.byte;} }, //MOVX @DPTR,A
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(0)] = pt->ACC;} }, //MOVX @R0,A
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(1)] = pt->ACC;} }, //MOVX @R1,A
   { [](Core8051* pt){pt->PC++; pt->ACC.byte = ~(pt->ACC.byte);} }, //CPL A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ACC; pt->PC++;} }, //MOV data addr,A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ACC;} }, //MOV @R0,A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ACC;} }, //MOV @R1,A
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ACC;} }, //MOV R0,A
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ACC;} }, //MOV R1,A
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ACC;} }, //MOV R2,A
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ACC;} }, //MOV R3,A
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ACC;} }, //MOV R4,A
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ACC;} }, //MOV R5,A
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ACC;} }, //MOV R6,A
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ACC;} }  //MOV R7,A
};
