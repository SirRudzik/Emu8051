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

void Core8051::AccAdd(const Byte_t& num)
{
}

void Core8051::AccAddC(const Byte_t& num)
{
}

void Core8051::AccSubb(const Byte_t& num)
{
}

void Core8051::CalcBitP()
{
	PSW.b0 = ACC.b0 ^ ACC.b1 ^ ACC.b2 ^ ACC.b3 ^ ACC.b4 ^ ACC.b5 ^ ACC.b6 ^ ACC.b7;
}

std::function<void(Core8051* pt)> Core8051::InstructionDecoder[] =
{
    //NOP  0x00
   { [](Core8051* pt){pt->PC++;} },
    //AJMP code addr  0x01
   { [](Core8051* pt){pt->PC++; Word_t temp{0x0};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //LJMP code addr  0x02
   { [](Core8051* pt){pt->PC++; Word_t temp;
                                temp.highByte = pt->code[pt->PC++];
                                temp.lowByte = pt->code[pt->PC];
                                pt->PC = temp; pt->PC++;} },
    //RR A  0x03
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b0;
                                pt->ACC.byte >>= 1;
                                pt->ACC.b7 = temp;} },
    //INC A  0x04
   { [](Core8051* pt){pt->PC++; pt->ACC++;} },
    //INC data addr   0x05
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]]++;
                                pt->PC++;} },
    //INC @Ri  0x06
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]++;} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]++;} },
    //INC Ri  0x08
   { [](Core8051* pt){pt->PC++; pt->Register(0)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(1)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(2)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(3)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(4)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(5)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(6)++;} },
   { [](Core8051* pt){pt->PC++; pt->Register(7)++;} },
    //JBC bt addr, code addr  0x10
   { [](Core8051* pt){pt->PC++; if(pt->GetBit(pt->code[pt->PC++]))
                                {
                                    pt->ClearBit(pt->code[pt->PC - 1]);
                                    pt->PC.word += pt->code[pt->PC++].signedByte;
                                }
                                else
                                    pt->PC++;} },
    //ACALL code addr  0x11
   { [](Core8051* pt){pt->PC++;} },
    //LCALL code addr  0x12
   { [](Core8051* pt){pt->PC++; Word_t temp;
                                temp.highByte = pt->code[pt->PC++];
                                temp.lowByte = pt->code[pt->PC++];
                                pt->ram[++pt->SP] = pt->PC.lowByte;
                                pt->ram[++pt->SP] = pt->PC.highByte;} },
    //RRC A  0x13
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7;
                                pt->PSW.b7 = pt->ACC.b0;
                                pt->ACC.byte >>= 1;
                                pt->ACC.b7 = temp;} },
    //DEC A 0x14
   { [](Core8051* pt){pt->PC++; pt->ACC--;} },
    //DEC data addr  0x15
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]]--;} },
    //DEC @Ri  0x16
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]--;} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]--;} },
    //DEC R0  0x18
   { [](Core8051* pt){pt->PC++; pt->Register(0)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(1)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(2)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(3)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(4)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(5)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(6)--;} },
   { [](Core8051* pt){pt->PC++; pt->Register(7)--;} },
    //JB bit addr, code addr  0x20
   { [](Core8051* pt){pt->PC++;} },
    //AJMP code addr  0x21
   { [](Core8051* pt){pt->PC++; Word_t temp{0x100};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //RET  0x22
   { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--];
                                pt->PC.lowByte = pt->ram[pt->SP--];} },
    //RL A  0x23
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b7;
                                pt->ACC.byte <<= 1;
                                pt->ACC.b0 = temp;} },
    //ADD A,#data  0x24
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC++];} },
    //ADD A,data addr  0x25
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC++]];} },
    //ADD A,@Ri  0x26
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)];} },
    //ADD A,Ri  0x28
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7);} },
    //JNB bit addr, codeaddr  0x30
   { [](Core8051* pt){pt->PC++;} },
    //ACALL code addr  0x31
   { [](Core8051* pt){pt->PC++;} },
    //RETI  0x32
   { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--];
                                pt->PC.lowByte = pt->ram[pt->SP--];} },
    //RLC A  0x33
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7;
                                pt->PSW.b7 = pt->ACC.b7;
                                pt->ACC.byte <<= 1;
                                pt->ACC.b0 = temp;} },
    //ADDC A,#data  0x34
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC++] += Byte_t{pt->PSW.b7};} },
    //ADDC A,data addr  0x35
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC++]] += Byte_t{pt->PSW.b7};} },
    //ADDC A,@R0  0x36
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)] += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)] += Byte_t{pt->PSW.b7};} },
    //ADDC A,R0  0x38
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6) += Byte_t{pt->PSW.b7};} },
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7) += Byte_t{pt->PSW.b7};} },
    //JC code addr  0x40
   { [](Core8051* pt){pt->PC++; if(pt->PSW.b7)
                                    pt->PC.word += pt->code[pt->PC++].signedByte;
                                else
                                    pt->PC++;} },
    //AJMP code addr  0x41
   { [](Core8051* pt){pt->PC++; Word_t temp{0x200};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //ORL data addr,A  0x42
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] &= pt->ACC; pt->PC++;} },
    //ORL data addr,#data  0x43
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] &= pt->code[pt->PC + 1]; pt->PC++;} },
    //ORL data A,#data  0x44
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->code[pt->PC++];} },
    //ORL A,data addr  0x45
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->code[pt->PC++]];} },
    //ORL data A,@Ri  0x46
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(1)];} },
    //ORL data A,Ri  0x48
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(7);} },
    //JNC code addr	 0x50
   { [](Core8051* pt){pt->PC++; if(!pt->PSW.b7)
                                    pt->PC.word += pt->code[pt->PC++].signedByte;
                                else
                                    pt->PC++;} },
    //ACALL code addr  0x51
   { [](Core8051* pt){pt->PC++;} },
    //ANL data addr,A  0x52
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] |= pt->ACC;} },
    //ANL data addr,#data  0x53
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] |= pt->code[pt->PC + 1]; pt->PC++;} },
    //ANL A,#data  0x54
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->code[pt->PC++];} },
    //ANL A,data addr  0x55
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->code[pt->PC++]];} },
    //ANL A,@Ri  0x56
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(1)];} },
    //ANL A,Ri  0x58
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(7);} },
    //JZ code addr  0x60
   { [](Core8051* pt){pt->PC++; if(pt->ACC)
                                   pt->PC += pt->code[pt->PC++];
                                else
                                    pt->PC++;} },
    //AJMP codeaddr  0x61
   { [](Core8051* pt){pt->PC++; Word_t temp{0x300};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //XRL data addr,A  0x62
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] ^= pt->ACC;} },
    //XRL data addr,#data  0x63
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] ^= pt->code[pt->PC + 1]; pt->PC++;} },
    //XRL A,#data  0x64
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->code[pt->PC++];} },
    //XRL A,data addr  0x65
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->code[pt->PC++]];} },
    //XRL A.@Ri  0x66
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(1)];} },
    //XRL A.R0  0x68
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(7);} },
    //JNZ code addr  0x70
   { [](Core8051* pt){pt->PC++; if(!pt->ACC)
                                    pt->PC.word += pt->code[pt->PC++].signedByte;
                                else
                                    pt->PC++;} },
    //ACALL code addr  0x71
   { [](Core8051* pt){pt->PC++;} },
    //ORL C, !bit addr  0x72
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) && pt->GetBit(pt->code[pt->PC++]));} },
    //JMP @A+DPTR  0x73
   { [](Core8051* pt){pt->PC++; pt->PC = pt->ACC + pt->DPTR;} },
    //MOV A,#data  0x74
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC++];} },
    //MOV data addr,#data  0x75
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->code[pt->PC + 1];
                                pt->PC += Word_t{2};} },
    //MOV @R1,#data  0x76
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->code[pt->PC++];} },
    //MOV Ri,#data  0x78
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->code[pt->PC++];} },
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->code[pt->PC++];} },
    //SJMP code addr  0x0
   { [](Core8051* pt){pt->PC++; pt->PC.word += pt->code[pt->PC++].signedByte;} },
    //AJMP code addr  0x81
   { [](Core8051* pt){pt->PC++; Word_t temp{0x400};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //ANL C bit addr  0x82
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) || pt->GetBit(pt->code[pt->PC++]));} },
    //MOVC A,@A+PC  0x83
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC + pt->ACC];} },
    //DIV AB  0x84
   { [](Core8051* pt){pt->PC++;} },
    //MOV data addr,data addr  0x85
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->ram[pt->code[pt->PC + 2]]; pt->PC++;} },
    //MOV data addr,@Ri  0x86
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->ram[pt->Register(1)];} },
    //MOV data addr,Ri  0x88
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->Register(7);} },
    //MOV DPTR,#data  0x90
   { [](Core8051* pt){pt->PC++; pt->DPTR.highByte = pt->code[pt->PC++]; pt->DPTR.lowByte = pt->code[pt->PC++];} },
    //ACALL codeaddr  0x91
   { [](Core8051* pt){pt->PC++;} },
    //MOV bit addr,C  0x92
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC++], pt->GetBit(PSW_7));} },
    //MOVC A,@A+DPTR  0x93
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->DPTR + pt->ACC];} },
    //SUBB A,#data  0x4
   { [](Core8051* pt){pt->PC++;} },
    //SUBB A,data addr  0x95
   { [](Core8051* pt){pt->PC++;} },
    //SUBB A,@Ri  0x96
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
    //SUBB A,Ri  0x98
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
    //ORL C,!bit addr  0xA0
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) && !pt->GetBit(pt->code[pt->PC++]));} },
    //AJMP code addr  0xA1
   { [](Core8051* pt){pt->PC++; Word_t temp{0x500}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} },
    //MOV C,bit addr  0xA2
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(pt->code[pt->PC++]));} },
    //INC DPTR  0xA3
   { [](Core8051* pt){pt->PC++; pt->DPTR++;} },
    //MUL AB  0xA4
   { [](Core8051* pt){pt->PC++; Word_t temp;
                                temp.word = pt->ACC.byte * pt->B.byte;
                                pt->ACC = temp.lowByte;
                                pt->B = temp.highByte;} },
    //reserved  0xA5
   { [](Core8051* pt){pt->PC++;} },
    //MOV @Ri,data addr	 0xA6
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ram[pt->code[pt->PC++]];} },
    //MOV Ri,data addr  0xA8
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ram[pt->code[pt->PC++]];} },
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ram[pt->code[pt->PC++]];} },
    //ANL C,!bit addr  0xB0
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) || !pt->GetBit(pt->code[pt->PC++]));} },
    //ACALL code addr  0xB1
   { [](Core8051* pt){pt->PC++;} },
    //CPL bit addr  0xB2
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC], !pt->GetBit(pt->code[pt->PC])); pt->PC++;} },
    //CPL C  0xB3
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, !pt->GetBit(PSW_7));} },
    //CJNE A,#data,code addr  0xB4
   { [](Core8051* pt){pt->PC++;} },
    //CJNE A,data addr,code addr  0xB5
   { [](Core8051* pt){pt->PC++;} },
    //CJNE @Ri,#data,code addr  0xB6
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
    //CJNE Ri,#data,code addr  0xB8
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
    //PUSH data addr  0xC0
   { [](Core8051* pt){pt->PC++; pt->ram[++(pt->SP)] = pt->ram[pt->code[pt->PC++]];} },
    //AJMP code addr  0xC1
   { [](Core8051* pt){pt->PC++; Word_t temp{0x600}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} },
    //CLR bit addr  0xC2
   { [](Core8051* pt){pt->PC++; pt->ClearBit(pt->code[pt->PC++]);} },
    //CLR  0xC3
   { [](Core8051* pt){pt->PC++; pt->ClearBit(PSW_7);} },
    //SWAP A  0xC4
   { [](Core8051* pt){pt->PC++; Byte_t temp;
                                temp.n0 = pt->ACC.n1;
                                temp.n1 = pt->ACC.n0;
                                pt->ACC.byte = temp.byte;} },
    //XCH A,data addr  0xC5
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->code[pt->PC]]; pt->ram[pt->code[pt->PC]] = temp; pt->PC++;} },
    //XCH A,@Ri  0xC6
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(0)]; pt->ram[pt->Register(0)] = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(1)]; pt->ram[pt->Register(1)] = temp;} },
    //XCH A,Ri  0xC8
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(0); pt->Register(0) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(1); pt->Register(1) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(2); pt->Register(2) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(3); pt->Register(3) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(4); pt->Register(4) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(5); pt->Register(5) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(6); pt->Register(6) = temp;} },
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(7); pt->Register(7) = temp;} },
    //POP data addr  0xD0
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC++]] = pt->ram[pt->SP--];} },
    //ACALL code addr  0xD1
   { [](Core8051* pt){pt->PC++;} },
    //SETB bit addr  0xD2
   { [](Core8051* pt){pt->PC++; pt->SetBit(pt->code[pt->PC++]);} },
    //SETB C  0xD3
   { [](Core8051* pt){pt->PC++; pt->SetBit(PSW_7);} },
    //DA A  0xD4
   { [](Core8051* pt){pt->PC++;} },
    //DJNZ data addr, code addr  0xD5
   { [](Core8051* pt){pt->PC++;} },
    //XCHD A,@Ri  0xD6
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(0)].n0;} },
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(1)].n0;} },
    //DJNZ Ri, code addr  0xD8
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
   { [](Core8051* pt){pt->PC++;} },
    //MOVX A,@DPTR	 0xE0
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->DPTR];} },
    //AJMP code addr  0xE1
   { [](Core8051* pt){pt->PC++; Word_t temp{0x700};
                                temp.lowByte = pt->code[pt->PC++];
                                pt->PC = temp;} },
    //MOVX A,@Ri  0xE2
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} },
    //CLR A  0x4
   { [](Core8051* pt){pt->PC++; pt->ACC = Byte_t{0};} },
    //MOV A,data addr  0xE5
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->code[pt->PC++]];} },
    //MOV A,@Ri  0xE6
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} },
    //MOV A,Ri  0xE8
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(0);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(1);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(2);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(3);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(4);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(5);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(6);} },
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(7);} },
    //MOVX @DPTR,A  0xF0
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->DPTR] = pt->ACC;} },
    //ACALL code addr  0xF1
   { [](Core8051* pt){pt->PC++;} },
    //MOVX @Ri,A  0xF2
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(0)] = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(1)] = pt->ACC;} },
    //CPL A  0xF4
   { [](Core8051* pt){pt->PC++; pt->ACC.byte = ~(pt->ACC.byte);} },
    //MOV data addr,A  0xF5
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ACC; pt->PC++;} },
   //MOV @Ri,A  0xF6
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ACC;} },
    //MOV Ri,A  0xF8
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ACC;} },
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ACC;} }
};
