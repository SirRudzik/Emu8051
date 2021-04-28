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
   { [](Core8051* pt){pt->PC++;} }, //NOP	00
   { [](Core8051* pt){pt->PC++; Word_t temp{0x0}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	01
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.highByte = pt->code[pt->PC++]; temp.lowByte = pt->code[pt->PC]; pt->PC = temp; pt->PC++;} }, //LJMP code addr		02
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b0; pt->ACC.byte >>= 1; pt->ACC.b7 = temp;} }, //RR A	 03
   { [](Core8051* pt){pt->PC++; pt->ACC++;} }, //INC A		04
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]]++; pt->PC++;} }, //INC data addr	05
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]++;} }, //INC @R0	06
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]++;} }, //INC @R1	07
   { [](Core8051* pt){pt->PC++; pt->Register(0)++;} }, //INC R0		08
   { [](Core8051* pt){pt->PC++; pt->Register(1)++;} }, //INC R1		09
   { [](Core8051* pt){pt->PC++; pt->Register(2)++;} }, //INC R2		0A
   { [](Core8051* pt){pt->PC++; pt->Register(3)++;} }, //INC R3		0B
   { [](Core8051* pt){pt->PC++; pt->Register(4)++;} }, //INC R4		0C
   { [](Core8051* pt){pt->PC++; pt->Register(5)++;} }, //INC R5		0D
   { [](Core8051* pt){pt->PC++; pt->Register(6)++;} }, //INC R6		0E
   { [](Core8051* pt){pt->PC++; pt->Register(7)++;} }, //INC R7		0F
   { [](Core8051* pt){pt->PC++; if(pt->GetBit(pt->code[pt->PC++]){pt->ClearBit(pt->code[pt->PC - Byte_t{1}]; pt->PC += pt->code[pt->PC++].signedByte}else{pt->PC++;}} }, //JBC bt addr, code addr		10
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr		11
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.highByte = pt->code[pt->PC++]; temp.lowByte = pt->code[pt->PC++]; pt->SP++; pt->ram[pt->SP] = pt->PC.lowByte; pt->SP++; pt->ram[pt->SP] = pt->PC.highByte;} }, //LCALL code addr		12
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7; pt->PSW.b7 = pt->ACC.b0; pt->ACC.byte >>= 1; pt->ACC.b7 = temp;} }, //RRC A	13
   { [](Core8051* pt){pt->PC++; pt->ACC--;} }, //DEC A		14
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]]--; pt->PC++;} }, //DEC data addr	15
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)]--;} }, //DEC @R0	16
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)]--;} }, //DEC @R1	17
   { [](Core8051* pt){pt->PC++; pt->Register(0)--;} }, //DEC R0		18
   { [](Core8051* pt){pt->PC++; pt->Register(1)--;} }, //DEC R1		19
   { [](Core8051* pt){pt->PC++; pt->Register(2)--;} }, //DEC R2		1A
   { [](Core8051* pt){pt->PC++; pt->Register(3)--;} }, //DEC R3		1B
   { [](Core8051* pt){pt->PC++; pt->Register(4)--;} }, //DEC R4		1C
   { [](Core8051* pt){pt->PC++; pt->Register(5)--;} }, //DEC R5		1D
   { [](Core8051* pt){pt->PC++; pt->Register(6)--;} }, //DEC R6		1E
   { [](Core8051* pt){pt->PC++; pt->Register(7)--;} }, //DEC R7		1F
   { [](Core8051* pt){pt->PC++;} }, //JB bit addr, code addr		20
   { [](Core8051* pt){pt->PC++; Word_t temp{0x100}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	21
   { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--]; pt->PC.lowByte = pt->ram[pt->SP--];} }, //RET	22
   { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b7; pt->ACC.byte <<= 1; pt->ACC.b0 = temp;} }, //RL A	23
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC]; pt->PC++;} }, //ADD A,#data	24
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ADD A,data addr		25
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)];} }, //ADD A,@R0	26
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)];} }, //ADD A,@R1	27
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0);} }, //ADD A,R0	28
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1);} }, //ADD A,R1	29
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2);} }, //ADD A,R2	2A
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3);} }, //ADD A,R3	2B
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4);} }, //ADD A,R4	2C
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5);} }, //ADD A,R5	2D
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6);} }, //ADD A,R6	2E
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7);} }, //ADD A,R7	2F
   { [](Core8051* pt){pt->PC++;} }, //JNB bit addr, codeaddr	30
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr		31
   { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--]; pt->PC.lowByte = pt->ram[pt->SP--];} }, //RETI	32
   { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7; pt->PSW.b7 = pt->ACC.b7; pt->ACC.byte <<= 1; pt->ACC.b0 = temp;} }, //RLC A	33
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->code[pt->PC] += Byte_t{pt->PSW.b7}; pt->PC++;} }, //ADDC A,#data		34
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->code[pt->PC]] += Byte_t{pt->PSW.b7}; pt->PC++;} }, //ADDC A,data addr		35
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(0)] += Byte_t{pt->PSW.b7};} }, //ADDC A,@R0	36
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->ram[pt->Register(1)] += Byte_t{pt->PSW.b7};} }, //ADDC A,@R1	37
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(0) += Byte_t{pt->PSW.b7};} }, //ADDC A,R0	38
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(1) += Byte_t{pt->PSW.b7};} }, //ADDC A,R1	39
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(2) += Byte_t{pt->PSW.b7};} }, //ADDC A,R2	3A
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(3) += Byte_t{pt->PSW.b7};} }, //ADDC A,R3	3B
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(4) += Byte_t{pt->PSW.b7};} }, //ADDC A,R4	3C
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(5) += Byte_t{pt->PSW.b7};} }, //ADDC A,R5	3D
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(6) += Byte_t{pt->PSW.b7};} }, //ADDC A,R6	3E
   { [](Core8051* pt){pt->PC++; pt->ACC += pt->Register(7) += Byte_t{pt->PSW.b7};} }, //ADDC A,R7	3F
   { [](Core8051* pt){pt->PC++; if(PSW.b7){pt->PC += pt->code[pt->PC++].signedByte}else{pt->PC++;};} }, //JC code addr	40
   { [](Core8051* pt){pt->PC++; Word_t temp{0x200}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	41
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] &= pt->ACC; pt->PC++;} }, //ORL data addr,A	42
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] &= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //ORL data addr,#data	43
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->code[pt->PC]; pt->PC++;} }, //ORL data A,#data		44
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ORL A,data addr		45
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(0)];} }, //ORL data A,@R0	46
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(1)];} }, //ORL data A,@R1	47
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(0);} }, //ORL data A,R0		48
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(1);} }, //ORL data A,R1		49
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(2);} }, //ORL data A,R2		4A
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(3);} }, //ORL data A,R3		4B
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(4);} }, //ORL data A,R4		4C
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(5);} }, //ORL data A,R5		4D
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(6);} }, //ORL data A,R6		4E
   { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(7);} }, //ORL data A,R7		4F
   { [](Core8051* pt){pt->PC++; if(!PSW.b7){pt->PC += pt->code[pt->PC++].signedByte}else{pt->PC++;}} }, //JNC code addr	50
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr	51
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] |= pt->ACC; pt->PC++;} }, //ANL data addr,A		52
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] |= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //ANL data addr,#data	53
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->code[pt->PC]; pt->PC++;} }, //ANL A,#data	54
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //ANL A,data addr		55
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(0)];} }, //ANL A,@R0		56
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(1)];} }, //ANL A,@R1		57
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(0);} }, //ANL A,R0	58
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(1);} }, //ANL A,R1	59
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(2);} }, //ANL A,R2	5A
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(3);} }, //ANL A,R3	5B
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(4);} }, //ANL A,R4	5C
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(5);} }, //ANL A,R5	5D
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(6);} }, //ANL A,R6	5E
   { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(7);} }, //ANL A,R7	5F
   { [](Core8051* pt){pt->PC++; if(pt->ACC){pt->PC += pt->code[++(pt->PC)];} else{pt->PC++;};} }, //JZ code addr	60
   { [](Core8051* pt){pt->PC++; Word_t temp{0x300}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP codeaddr	61
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] ^= pt->ACC; pt->PC++;} }, //XRL data addr,A	62
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] ^= pt->code[pt->PC + Byte_t{1}]; pt->PC + Word_t{2};} }, //XRL data addr,#data	63
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->code[pt->PC]; pt->PC++;} }, //XRL A,#data	64
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //XRL A,data addr	65
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(0)];} }, //XRL A.@R0	66
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->ram[pt->Register(1)];} }, //XRL A.@R1	67
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(0);} }, //XRL A.R0	68
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(1);} }, //XRL A.R1	69
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(2);} }, //XRL A.R2	6A
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(3);} }, //XRL A.R3	6B
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(4);} }, //XRL A.R4	6C
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(5);} }, //XRL A.R5	6D
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(6);} }, //XRL A.R6	6E
   { [](Core8051* pt){pt->PC++; pt->ACC ^= pt->Register(7);} }, //XRL A.R7	6F
   { [](Core8051* pt){pt->PC++; if(!(pt->ACC)){pt->PC += pt->code[++(pt->PC)];} else{pt->PC++;};} }, //JNZ code addr	70
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr	71
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) && pt->GetBit(pt->code[pt->PC]))); pt->PC++;} }, //ORL C, !bit addr	72
   { [](Core8051* pt){pt->PC++; pt->PC = pt->ACC + pt->DPTR;} }, //JMP @A+DPTR	73
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC]; pt->PC++ ;} }, //MOV A,#data	74
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->code[pt->PC + Byte_t{1}]; pt->PC += Word_t{2};} }, //MOV data addr,#data	75
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->code[pt->PC]; pt->PC++;} }, //MOV @R0,#data	76
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->code[pt->PC]; pt->PC++;} }, //MOV @R1,#data	77
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->code[pt->PC]; pt->PC++;} }, //MOV R0,#data	78
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->code[pt->PC]; pt->PC++;} }, //MOV R1,#data	79
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->code[pt->PC]; pt->PC++;} }, //MOV R2,#data	7A
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->code[pt->PC]; pt->PC++;} }, //MOV R3,#data	7B
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->code[pt->PC]; pt->PC++;} }, //MOV R4,#data	7C
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->code[pt->PC]; pt->PC++;} }, //MOV R5,#data	7D
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->code[pt->PC]; pt->PC++;} }, //MOV R6,#data	7E
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->code[pt->PC]; pt->PC++;} }, //MOV R7,#data	7F
   { [](Core8051* pt){pt->PC++; pt->PC + pt->code[pt->PC++].signedByte;} }, //SJMP code addr	80
   { [](Core8051* pt){pt->PC++; Word_t temp{0x400}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	81
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) || pt->GetBit(pt->code[pt->PC]))); pt->PC++;} }, //ANL C bit addr	82
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC + pt->ACC];} }, //MOVC A,@A+PC	83
   { [](Core8051* pt){pt->PC++;} }, //DIV AB	84
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->code[pt->PC].byte + 1]; pt->PC += Word_t{2};} }, //MOV data addr,data addr	85
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->Register(0)]; pt->PC++;} }, //MOV data addr,@R0		86
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ram[pt->Register(1)]; pt->PC++;} }, //MOV data addr,@R1		87
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(0); pt->PC++;} }, //MOV data addr,R0	88
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(1); pt->PC++;} }, //MOV data addr,R1	89
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(2); pt->PC++;} }, //MOV data addr,R2	8A
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(3); pt->PC++;} }, //MOV data addr,R3	8B
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(4); pt->PC++;} }, //MOV data addr,R4	8C
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(5); pt->PC++;} }, //MOV data addr,R5	8D
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(6); pt->PC++;} }, //MOV data addr,R6	8E
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->Register(7); pt->PC++;} }, //MOV data addr,R7	8F
   { [](Core8051* pt){pt->PC++; pt->DPTR += pt->code[pt->PC]; pt->PC += Word_t{2};} }, //MOV DPTR,#data		90
   { [](Core8051* pt){pt->PC++;} }, //ACALL codeaddr	91
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC], pt->GetBit(PSW_7)); pt->PC++;} }, //MOV bit addr,C	92
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->DPTR + pt->ACC];} }, //MOVC A,@A+DPTR	93
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,#data	94
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,data addr	95
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,@R0	96
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,@R1	97
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R0		98
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R1		99
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R2		9A
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R3		9B
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R4		9C
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R5		9D
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R6		9E
   { [](Core8051* pt){pt->PC++;} }, //SUBB A,R7		9F
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) && !(pt->GetBit(pt->code[pt->PC])))); pt->PC++;} }, //ORL C,!bit addr	A0
   { [](Core8051* pt){pt->PC++; Word_t temp{0x500}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	A1
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, pt->GetBit(pt->code[pt->PC])); pt->PC++;} }, //MOV C,bit addr	A2
   { [](Core8051* pt){pt->PC++; pt->DPTR++;} }, //INC DPTR	A3
   { [](Core8051* pt){pt->PC++; Word_t temp; temp.word = pt->ACC.byte * pt->B.byte; pt->ACC = temp.lowByte; pt->B = temp.highByte;} }, //MUL AB		A4
   { [](Core8051* pt){pt->PC++;} }, //reserved		A5
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV @R0,data addr		A6
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV @R1,data addr		A7
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R0,data addr		A8
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R1,data addr		A9
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R2,data addr		AA
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R3,data addr		AB
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R4,data addr		AC
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R5,data addr		AD
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R6,data addr		AE
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV R7,data addr		AF
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, (pt->GetBit(PSW_7) || !(pt->GetBit(pt->code[pt->PC])))); pt->PC++;} }, //ANL C,!bit addr	B0
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr	B1
   { [](Core8051* pt){pt->PC++; pt->WriteBit(pt->code[pt->PC], !pt->GetBit(pt->code[pt->PC])); pt->PC++;} }, //CPL bit addr	B2
   { [](Core8051* pt){pt->PC++; pt->WriteBit(PSW_7, !pt->GetBit(PSW_7));} }, //CPL C	B3
   { [](Core8051* pt){pt->PC++;} }, //CJNE A,#data,code addr	B4
   { [](Core8051* pt){pt->PC++;} }, //CJNE A,data addr,code addr	B5
   { [](Core8051* pt){pt->PC++;} }, //CJNE @R0,#data,code addr		B6
   { [](Core8051* pt){pt->PC++;} }, //CJNE @R1,#data,code addr		B7
   { [](Core8051* pt){pt->PC++;} }, //CJNE R0,#data,code addr		B8
   { [](Core8051* pt){pt->PC++;} }, //CJNE R1,#data,code addr		B9
   { [](Core8051* pt){pt->PC++;} }, //CJNE R2,#data,code addr		BA
   { [](Core8051* pt){pt->PC++;} }, //CJNE R3,#data,code addr		BB
   { [](Core8051* pt){pt->PC++;} }, //CJNE R4,#data,code addr		BC
   { [](Core8051* pt){pt->PC++;} }, //CJNE R5,#data,code addr		BD
   { [](Core8051* pt){pt->PC++;} }, //CJNE R6,#data,code addr		BE
   { [](Core8051* pt){pt->PC++;} }, //CJNE R6,#data,code addr		BF
   { [](Core8051* pt){pt->PC++; pt->ram[++(pt->SP)] = pt->ram[pt->code[++(pt->PC)]];} }, //PUSH data addr	C0
   { [](Core8051* pt){pt->PC++; Word_t temp{0x600}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;;} }, //AJMP code addr	C1
   { [](Core8051* pt){pt->PC++; pt->ClearBit(pt->code[pt->PC]); pt->PC++;} }, //CLR bit addr	C2
   { [](Core8051* pt){pt->PC++; pt->ClearBit(PSW_7);} }, //CLR C	C3
   { [](Core8051* pt){pt->PC++; Byte_t temp; temp.n0 = pt->ACC.n1; temp.n1 = pt->ACC.n0; pt->ACC.byte = temp.byte;} }, //SWAP A		C4
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->code[pt->PC]]; pt->ram[pt->code[pt->PC]] = temp; pt->PC++;} }, //XCH A,data addr	C5
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(0)]; pt->ram[pt->Register(0)] = temp;} }, //XCH A,@R0	C6
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(1)]; pt->ram[pt->Register(1)] = temp;} }, //XCH A,@R1	C7
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(0); pt->Register(0) = temp;} }, //XCH A,R0	C8
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(1); pt->Register(1) = temp;} }, //XCH A,R1	C9
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(2); pt->Register(2) = temp;} }, //XCH A,R2	CA
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(3); pt->Register(3) = temp;} }, //XCH A,R3	CB
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(4); pt->Register(4) = temp;} }, //XCH A,R4	CC
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(5); pt->Register(5) = temp;} }, //XCH A,R5	CD
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(6); pt->Register(6) = temp;} }, //XCH A,R6	CE
   { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(7); pt->Register(7) = temp;} }, //XCH A,R7	CF
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[++(pt->PC)]] = pt->ram[pt->SP--];} }, //POP data addr		D0
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr		D1
   { [](Core8051* pt){pt->PC++; pt->SetBit(pt->code[pt->PC]); pt->PC++;} }, //SETB bit addr	D2
   { [](Core8051* pt){pt->PC++; pt->SetBit(PSW_7);} }, //SETB C		D3
   { [](Core8051* pt){pt->PC++;} }, //DA A	D4
   { [](Core8051* pt){pt->PC++;} }, //DJNZ data addr, code addr		D5
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(0)].n0;} }, //XCHD A,@R0	D6
   { [](Core8051* pt){pt->PC++; pt->ACC.n0 = pt->ram[pt->Register(1)].n0;} }, //XCHD A,@R1	D7
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R0, code addr	D8
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R1, code addr	D9
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R2, code addr	DA
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R3, code addr	DB
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R4, code addr	DC
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R5, code addr	DD
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R6, code addr	DE
   { [](Core8051* pt){pt->PC++;} }, //DJNZ R7, code addr	DF
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->DPTR];} }, //MOVX A,@DPTR		E0
   { [](Core8051* pt){pt->PC++; Word_t temp{0x700}; temp.lowByte = pt->code[pt->PC++]; pt->PC = temp;} }, //AJMP code addr	E1
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} }, //MOVX A,@R0	E2
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} }, //MOVX A,@R1	E3
   { [](Core8051* pt){pt->PC++; pt->ACC = Byte_t{0x0};} }, //CLR A	E4
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->code[pt->PC]]; pt->PC++;} }, //MOV A,data addr	E5
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} }, //MOV A,@R0	E6
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} }, //MOV A,@R1	E7
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(0);} }, //MOV A,R0	E8
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(1);} }, //MOV A,R1	E9
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(2);} }, //MOV A,R2	EA
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(3);} }, //MOV A,R3	EB
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(4);} }, //MOV A,R4	EC
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(5);} }, //MOV A,R5	ED
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(6);} }, //MOV A,R6	EE
   { [](Core8051* pt){pt->PC++; pt->ACC = pt->Register(7);} }, //MOV A,R7	EF
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->DPTR].byte = pt->ACC.byte;} }, //MOVX @DPTR,A	F0
   { [](Core8051* pt){pt->PC++;} }, //ACALL code addr		F1
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(0)] = pt->ACC;} }, //MOVX @R0,A	F2
   { [](Core8051* pt){pt->PC++; pt->externalRam[pt->Register(1)] = pt->ACC;} }, //MOVX @R1,A	F3
   { [](Core8051* pt){pt->PC++; pt->ACC.byte = ~(pt->ACC.byte);} }, //CPL A	F4
   { [](Core8051* pt){pt->PC++; pt->ram[pt->code[pt->PC]] = pt->ACC; pt->PC++;} }, //MOV data addr,A	F5
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(0)] = pt->ACC;} }, //MOV @R0,A	F6
   { [](Core8051* pt){pt->PC++; pt->ram[pt->Register(1)] = pt->ACC;} }, //MOV @R1,A	F7
   { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ACC;} }, //MOV R0,A	F8
   { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ACC;} }, //MOV R1,A	F9
   { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ACC;} }, //MOV R2,A	FA
   { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ACC;} }, //MOV R3,A	FB
   { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ACC;} }, //MOV R4,A	FC
   { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ACC;} }, //MOV R5,A	FD
   { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ACC;} }, //MOV R6,A	FE
   { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ACC;} }  //MOV R7,A	FF
};
