#include "Core8051.h"

//Byte_t operators definitions
Byte_t& Byte_t::operator++(){byte++; return *this;}
Byte_t Byte_t::operator++(int){Byte_t temp = *this; byte++; return temp;}
Byte_t& Byte_t::operator--(){byte--; return *this;}
Byte_t Byte_t::operator--(int){Byte_t temp = *this; byte--; return temp;}
Byte_t& Byte_t::operator+=(const Byte_t& rhs){byte += rhs.byte; return *this;}
Byte_t& Byte_t::operator-=(const Byte_t& rhs){byte -= rhs.byte; return *this;}
Byte_t Byte_t::operator+(const Byte_t& rhs){Byte_t temp = *this; temp += rhs; return temp;}
Byte_t Byte_t::operator-(const Byte_t& rhs){Byte_t temp = *this; temp -= rhs; return temp;}
Byte_t Byte_t::operator/(const Byte_t& rhs){Byte_t temp = *this; temp.byte = byte / rhs.byte; return temp;}
Byte_t Byte_t::operator%(const Byte_t& rhs){Byte_t temp = *this; temp.byte = byte % rhs.byte; return temp;}
Byte_t& Byte_t::operator|=(const Byte_t& rhs){byte |= rhs.byte; return *this;}
Byte_t& Byte_t::operator&=(const Byte_t& rhs){byte &= rhs.byte; return *this;}
Byte_t& Byte_t::operator^=(const Byte_t& rhs){byte ^= rhs.byte; return *this;}
std::ostream& operator<<(std::ostream& out, const Byte_t& b){std::cout << std::hex << static_cast<unsigned short>(b.n1) << static_cast<unsigned short>(b.n0); return out;}

Byte_t::operator int(){return byte;}
Byte_t::operator Word_t(){return Word_t{byte};}

//Word_t operators definitions
Word_t& Word_t::operator++(){word++; return *this;}
Word_t Word_t::operator++(int){Word_t temp = *this; word++; return temp;}
Word_t& Word_t::operator--(){word--; return *this;}
Word_t Word_t::operator--(int){Word_t temp = *this; word--; return temp;}
Word_t& Word_t::operator+=(const Word_t& rhs){word += rhs.word; return *this;}
Word_t& Word_t::operator-=(const Word_t& rhs){word -= rhs.word; return *this;}
Word_t Word_t::operator+(const Word_t& rhs){Word_t temp = *this; temp += rhs; return temp;}
Word_t Word_t::operator-(const Word_t& rhs){Word_t temp = *this; temp -= rhs; return temp;}
std::ostream& operator<<(std::ostream& out, const Word_t& w){std::cout << w.highByte << w.lowByte; return out;}

Word_t::operator int(){return word;}
Word_t::operator Byte_t(){return lowByte;}

//Core8051 methods  definitions

Core8051::Core8051() :
    code{},
    ram{},
    PC{0},
    cycleCounter(0),
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
    SP = Byte_t{0x07};
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
    temp.byte = ~temp.byte;
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
    if(bit == true)
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

void Core8051::AccAdd(Byte_t num)
{
        union
    {
        std::uint8_t b01 : 2;
    struct
    {
        std::uint8_t b0 : 1;
        std::uint8_t b1 : 1;
    };
    }temp;

    temp.b01 = 0;

    temp.b01 = ACC.b0 + num.b0 + temp.b1;
    ACC.b0 = temp.b0;

    temp.b01 = ACC.b1 + num.b1 + temp.b1;
    ACC.b1 = temp.b0;

    temp.b01 = ACC.b2 + num.b2 + temp.b1;
    ACC.b2 = temp.b0;

    temp.b01 = ACC.b3 + num.b3 + temp.b1;
    ACC.b3 = temp.b0;
    PSW.b6 = temp.b1;

    temp.b01 = ACC.b4 + num.b4 + temp.b1;
    ACC.b4 = temp.b0;

    temp.b01 = ACC.b5 + num.b5 + temp.b1;
    ACC.b5 = temp.b0;

    temp.b01 = ACC.b6 + num.b6 + temp.b1;
    ACC.b6 = temp.b0;
    PSW.b2 = temp.b1;

    temp.b01 = ACC.b7 + num.b7 + temp.b1;
    ACC.b7 = temp.b0;
    PSW.b7 = temp.b1;
}

void Core8051::AccAddC(Byte_t num)
{
    AccAdd(Byte_t{PSW.b7});
    AccAdd(num);
}

void Core8051::AccSubb(Byte_t num)
{
    union
    {
        std::uint8_t b01 : 2;
    struct
    {
        std::uint8_t b0 : 1;
        std::uint8_t b1 : 1;
    };
    }temp;

    temp.b0 = 0;
    temp.b1 = PSW.b7;

    temp.b01 = ACC.b0 - num.b0 - temp.b1;
    ACC.b0 = temp.b0;

    temp.b01 = ACC.b1 - num.b1 - temp.b1;
    ACC.b1 = temp.b0;

    temp.b01 = ACC.b2 - num.b2 - temp.b1;
    ACC.b2 = temp.b0;

    temp.b01 = ACC.b3 - num.b3 - temp.b1;
    ACC.b3 = temp.b0;
    PSW.b6 = temp.b1;

    temp.b01 = ACC.b4 - num.b4 - temp.b1;
    ACC.b4 = temp.b0;

    temp.b01 = ACC.b5 - num.b5 - temp.b1;
    ACC.b5 = temp.b0;

    temp.b01 = ACC.b6 - num.b6 - temp.b1;
    ACC.b6 = temp.b0;
    PSW.b2 = temp.b1;

    temp.b01 = ACC.b7 - num.b7 - temp.b1;
    ACC.b7 = temp.b0;
    PSW.b7 = temp.b1;
}

void Core8051::CalcBitP()
{
	PSW.b0 = ACC.b0 ^ ACC.b1 ^ ACC.b2 ^ ACC.b3 ^ ACC.b4 ^ ACC.b5 ^ ACC.b6 ^ ACC.b7;
}

void Core8051::ProgramRun(std::uint64_t cycles, bool step)
{
    do
    {
        PrintCore();
        if(step)
            std::cin.get();
        Cycle();
    }while(!cycles ? true : cycles > cycleCounter);
}

void Core8051::Cycle()
{
    InstructionDecoder[0][code[PC]](this);
    CalcBitP();
    cycleCounter++;
}

void Core8051::LoadHex(const char* filePath)
{
    std::fstream hexFile;
    hexFile.open(filePath, std::ios::in | std::ios::binary);
    if(hexFile.good())
    {
        std::cout << "file open success" << std::endl;
        hexFile.read(reinterpret_cast<char*>(code), 0x10000 * sizeof(Byte_t));
        hexFile.close();
    }
    else
        std::cout << "file not open" << std::endl;
}

void Core8051::PrintCore()
{
    system("cls");
    std::cout   << "B    " << B    << ' ' << std::bitset<8>(B.byte)    << "    " << "ACC  " << ACC  << ' ' << std::bitset<8>(ACC.byte)  << "    " << "PSW  " << PSW  << ' ' << std::bitset<8>(PSW.byte)  << "    " << std::endl
                << "SP   " << SP   << ' ' << std::bitset<8>(SP.byte)   << "    " << "IP   " << IP   << ' ' << std::bitset<8>(IP.byte)   << "    " << "IE   " << IE   << ' ' << std::bitset<8>(IE.byte)   << "    " << std::endl
                << "DPH  " << DPH  << ' ' << std::bitset<8>(DPH.byte)  << "    " << "DPL  " << DPL  << ' ' << std::bitset<8>(DPL.byte)  << "    " << "P0   " << P0   << ' ' << std::bitset<8>(P0.byte)   << "    " << std::endl
                << "P1   " << P1   << ' ' << std::bitset<8>(P1.byte)   << "    " << "P2   " << P2   << ' ' << std::bitset<8>(P2.byte)   << "    " << "P3   " << P3   << ' ' << std::bitset<8>(P3.byte)   << "    " << std::endl
                << "TMOD " << TMOD << ' ' << std::bitset<8>(TMOD.byte) << "    " << "TCON " << TCON << ' ' << std::bitset<8>(TCON.byte) << "    " << "PCON " << PCON << ' ' << std::bitset<8>(PCON.byte) << "    " << std::endl
                << "TH0  " << TH0  << ' ' << std::bitset<8>(TH0.byte)  << "    " << "TL0  " << TL0  << ' ' << std::bitset<8>(TL0.byte)  << "    " << "SCON " << SCON << ' ' << std::bitset<8>(SCON.byte) << "    " << std::endl
                << "TH1  " << TH1  << ' ' << std::bitset<8>(TH1.byte)  << "    " << "TL1  " << TL1  << ' ' << std::bitset<8>(TL1.byte)  << "    " << "SBUF " << SBUF << ' ' << std::bitset<8>(SBUF.byte) << "    " << std::endl;

    std::cout   << "R0 " << Register(0) << "  " << "R1 " << Register(1) << "  " << "R2 " << Register(2) << "  " << "R3 " << Register(3) << std::endl
                << "R4 " << Register(4) << "  " << "R5 " << Register(5) << "  " << "R6 " << Register(6) << "  " << "R7 " << Register(7) << std::endl;

    std::cout   << std::endl << "PC  " << PC << "  " << "  " << std::bitset<16>(PC) << "  " << code[PC] << "  ";
                InstructionDecoder[1][code[PC]](this);
    std::cout   << std::endl << "Cycle " << std::dec << cycleCounter << std::endl;

    std::cout   << "    ";
    for(std::uint8_t i = 0; i < 0x10; i++)
    {
        std::cout << 0 << std::hex << static_cast<short>(i) << "  ";
    }

    for(std::uint8_t i = 0; i < 0x80; i++)
    {

        if(i % 16 == 0)
                std::cout << std::endl << std::hex << i / 16 << 0  << "  ";
        std::cout << ram[i] << "  ";
    }
    std::cout << std::endl;

}
std::function<void(Core8051* pt)> Core8051::InstructionDecoder[0x2][0x100] =
{
    {
    //NOP  0x00
    { [](Core8051* pt){pt->PC++;} },
    //AJMP code addr  0x01
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x0};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //LJMP code addr  0x02
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                Word_t temp;
                                temp.highByte = pt->code[pt->PC - 2];
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC = temp;} },
    //RR A  0x03
    { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b0;
                                pt->ACC.byte >>= 1;
                                pt->ACC.b7 = temp;} },
    //INC A  0x04
    { [](Core8051* pt){pt->PC++; pt->ACC++;} },
    //INC data addr   0x05
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]]++;} },
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
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->GetBit(pt->code[pt->PC - 2]))
                                {
                                    pt->ClearBit(pt->code[pt->PC - 2]);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }
                                else
                                    pt->PC++;} },
    //ACALL code addr  0x11
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x0};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //LCALL code addr  0x12
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                Word_t temp;
                                temp.highByte = pt->code[pt->PC - 2];
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC = temp;} },
    //RRC A  0x13
    { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7;
                                pt->PSW.b7 = pt->ACC.b0;
                                pt->ACC.byte >>= 1;
                                pt->ACC.b7 = temp;} },
    //DEC A 0x14
    { [](Core8051* pt){pt->PC++; pt->ACC--;} },
    //DEC data addr  0x15
    { [](Core8051* pt){pt->PC += Byte_t{2}; ; pt->ram[pt->code[pt->PC]]--;} },
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
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->GetBit(pt->code[pt->PC - 2]))
                                    pt->PC.word = pt->PC.word + pt->code[pt->PC - 1].signedByte;} },
    //AJMP code addr  0x21
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x100};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //RET  0x22
    { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--];
                                pt->PC.lowByte = pt->ram[pt->SP--];} },
    //RL A  0x23
    { [](Core8051* pt){pt->PC++; bool temp = pt->ACC.b7;
                                pt->ACC.byte <<= 1;
                                pt->ACC.b0 = temp;} },
    //ADD A,#data  0x24
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccAdd(pt->code[pt->PC - 1]);} },
    //ADD A,data addr  0x25
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccAdd(pt->ram[pt->code[pt->PC - 1]]);} },
    //ADD A,@Ri  0x26
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->ram[pt->Register(0)]);} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->ram[pt->Register(1)]);} },
    //ADD A,Rn  0x28
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(0));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(1));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(2));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(3));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(4));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(5));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(6));} },
    { [](Core8051* pt){pt->PC++; pt->AccAdd(pt->Register(7));} },
    //JNB bit addr, codeaddr  0x30
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(!pt->GetBit(pt->code[pt->PC - 2]))
                                    pt->PC.word = pt->PC.word + pt->code[pt->PC - 1].signedByte;} },
    //ACALL code addr  0x31
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x100};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //RETI  0x32
    { [](Core8051* pt){pt->PC++; pt->PC.highByte = pt->ram[pt->SP--];
                                pt->PC.lowByte = pt->ram[pt->SP--];} },
    //RLC A  0x33
    { [](Core8051* pt){pt->PC++; bool temp = pt->PSW.b7;
                                pt->PSW.b7 = pt->ACC.b7;
                                pt->ACC.byte <<= 1;
                                pt->ACC.b0 = temp;} },
    //ADC A,#data  0x34
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccAddC(pt->code[pt->PC - 1]);} },
    //ADC A,data addr  0x35
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccAddC(pt->ram[pt->code[pt->PC - 1]]);} },
    //ADC A,@Ri  0x36
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->ram[pt->Register(0)]);} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->ram[pt->Register(1)]);} },
    //ADC A,Rn  0x38
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(0));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(1));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(2));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(3));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(4));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(5));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(6));} },
    { [](Core8051* pt){pt->PC++; pt->AccAddC(pt->Register(7));} },
    //JC code addr  0x40
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                if(pt->PSW.b7)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    //AJMP code addr  0x41
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x200};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //ORL data addr,A  0x42
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] |= pt->ACC;} },
    //ORL data addr,#data  0x43
    { [](Core8051* pt){pt->PC += Byte_t{3}; pt->ram[pt->code[pt->PC - 2]] |= pt->code[pt->PC - 1];} },
    //ORL data A,#data  0x44
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC |= pt->code[pt->PC - 1];} },
    //ORL A,data addr  0x45
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC |= pt->ram[pt->code[pt->PC - 1]];} },
    //ORL data A,@Ri  0x46
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(0)];} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->ram[pt->Register(1)];} },
    //ORL data A,Ri  0x48
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(0);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(1);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(2);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(3);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(4);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(5);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(6);} },
    { [](Core8051* pt){pt->PC++; pt->ACC |= pt->Register(7);} },
    //JNC code addr	 0x50
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                if(!pt->PSW.b7)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    //ACALL code addr  0x51
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x200};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //ANL data addr,A  0x52
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] &= pt->ACC;} },
    //ANL data addr,#data  0x53
    { [](Core8051* pt){pt->PC += Byte_t{3}; pt->ram[pt->code[pt->PC - 2]] &= pt->code[pt->PC - 1];} },
    //ANL A,#data  0x54
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC &= pt->code[pt->PC - 1];} },
    //ANL A,data addr  0x55
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC &= pt->ram[pt->code[pt->PC - 1]];} },
    //ANL A,@Ri  0x56
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(0)];} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->ram[pt->Register(1)];} },
    //ANL A,Ri  0x58
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(0);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(1);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(2);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(3);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(4);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(5);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(6);} },
    { [](Core8051* pt){pt->PC++; pt->ACC &= pt->Register(7);} },
    //JZ code addr  0x60
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                if(pt->ACC)
                                   pt->PC += pt->code[pt->PC - 1];} },
    //AJMP codeaddr  0x61
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x300};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //XRL data addr,A  0x62
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] ^= pt->ACC;} },
    //XRL data addr,#data  0x63
    { [](Core8051* pt){pt->PC += Byte_t{3}; pt->ram[pt->code[pt->PC - 2]] ^= pt->code[pt->PC - 1];} },
    //XRL A,#data  0x64
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC ^= pt->code[pt->PC - 1];} },
    //XRL A,data addr  0x65
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC ^= pt->ram[pt->code[pt->PC - 1]];} },
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
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                if(!pt->ACC)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    //ACALL code addr  0x71
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x300};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //ORL C,bit addr  0x72
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) || pt->GetBit(pt->code[pt->PC - 1]));} },
    //JMP @A+DPTR  0x73
    { [](Core8051* pt){pt->PC++; pt->PC = pt->ACC + pt->DPTR;} },
    //MOV A,#data  0x74
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC = pt->code[pt->PC - 1];} },
    //MOV data addr,#data  0x75
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                pt->ram[pt->code[pt->PC - 2]] = pt->code[pt->PC - 1];} },
    //MOV @Ri,#data  0x76
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->Register(0)] = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->Register(1)] = pt->code[pt->PC - 1];} },
    //MOV Rn,#data  0x78
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(0) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(1) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(2) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(3) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(4) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(5) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(6) = pt->code[pt->PC - 1];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(7) = pt->code[pt->PC - 1];} },
    //SJMP code addr  0x80
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    //AJMP code addr  0x81
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x400};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //ANL C,bit addr  0x82
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) && pt->GetBit(pt->code[pt->PC - 1]));} },
    //MOVC A,@A+PC  0x83
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->PC + pt->ACC];} },
    //DIV AB  0x84
    { [](Core8051* pt){pt->PC++; if(!pt->B)
                                {
                                    pt->ACC = pt->B = Byte_t{0x0};
                                    pt->SetBit(PSW_2);
                                    pt->ClearBit(PSW_7);
                                }
                                else
                                {
                                    Byte_t tempAcc, tempB;
                                    tempAcc = pt->ACC / pt->B;
                                    tempB = pt->ACC % pt->B;
                                    pt->ACC = tempAcc;
                                    pt->B = tempB;
                                    pt->ClearBit(PSW_7);
                                    pt->ClearBit(PSW_2);
                                    }} },
    //MOV data addr,data addr  0x85
    { [](Core8051* pt){pt->PC += Byte_t{3}; pt->ram[pt->code[pt->PC - 2]] = pt->ram[pt->code[pt->PC - 1]];} },
    //MOV data addr,@Ri  0x86
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->ram[pt->Register(0)];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->ram[pt->Register(1)];} },
    //MOV data addr,Ri  0x88
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(0);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(1);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(2);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(3);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(4);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(5);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(6);} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->Register(7);} },
    //MOV DPTR,#data  0x90
    { [](Core8051* pt){pt->PC += Byte_t{3}; pt->DPTR.highByte = pt->code[pt->PC - 2]; pt->DPTR.lowByte = pt->code[pt->PC - 1];} },
    //ACALL codeaddr  0x91
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x400};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //MOV bit addr,C  0x92
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(pt->code[pt->PC - 1], pt->GetBit(PSW_7));} },
    //MOVC A,@A+DPTR  0x93
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->code[pt->DPTR + pt->ACC];} },
    //SUBB A,#data  0x94
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccSubb(pt->code[pt->PC - 1]);} },
    //SUBB A,data addr  0x95
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->AccSubb(pt->ram[pt->code[pt->PC - 1]]);} },
    //SUBB A,@Ri  0x96
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->ram[pt->Register(0)]);} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->ram[pt->Register(1)]);} },
    //SUBB A,Rn  0x98
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(0));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(1));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(2));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(3));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(4));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(5));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(6));} },
    { [](Core8051* pt){pt->PC++; pt->AccSubb(pt->Register(7));} },
    //ORL C,/bit addr  0xA0
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) || !pt->GetBit(pt->code[pt->PC - 1]));} },
    //AJMP code addr  0xA1
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x500};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //MOV C,bit addr  0xA2
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(PSW_7, pt->GetBit(pt->code[pt->PC - 1]));} },
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
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->Register(0)] = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->Register(1)] = pt->ram[pt->code[pt->PC - 1]];} },
    //MOV Rn,data addr  0xA8
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(0) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(1) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(2) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(3) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(4) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(5) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(6) = pt->ram[pt->code[pt->PC - 1]];} },
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->Register(7) = pt->ram[pt->code[pt->PC - 1]];} },
    //ANL C,!bit addr  0xB0
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->WriteBit(PSW_7, pt->GetBit(PSW_7) && !pt->GetBit(pt->code[pt->PC - 1]));} },
    //ACALL code addr  0xB1
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x500};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //CPL bit addr  0xB2
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->InverseBit(pt->code[pt->PC - 1]);} },
    //CPL C  0xB3
    { [](Core8051* pt){pt->PC++; pt->InverseBit(PSW_7);} },
    //CJNE A,#data,code addr  0xB4
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->ACC != pt->code[pt->PC - 2])
                                {
                                    if(pt->ACC < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    //CJNE A,data addr,code addr  0xB5
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->ACC != pt->ram[pt->code[pt->PC - 2]])
                                {
                                    if(pt->ACC < pt->ram[pt->code[pt->PC - 2]])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    //CJNE @Ri,#data,code addr  0xB6
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->ram[pt->Register(0)] != pt->code[pt->PC - 2])
                                {
                                    if(pt->ram[pt->Register(0)] < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->ram[pt->Register(1)] != pt->code[pt->PC - 2])
                                {
                                    if(pt->ram[pt->Register(1)] < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    //CJNE Rn,#data,code addr  0xB8
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(0) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(0) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(1) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(1) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(2) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(2) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(3) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(3) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(4) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(4) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(5) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(5) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(6) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(6) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    { [](Core8051* pt){pt->PC += Byte_t{3};
                                if(pt->Register(7) != pt->code[pt->PC - 2])
                                {
                                    if(pt->Register(7) < pt->code[pt->PC - 2])
                                        pt->SetBit(PSW_7);
                                    else
                                        pt->ClearBit(PSW_7);
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;
                                }} },
    //PUSH data addr  0xC0
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->SP++; pt->ram[(pt->SP)] = pt->ram[pt->code[pt->PC - 1]];} },
    //AJMP code addr  0xC1
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x600};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //CLR bit addr  0xC2
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ClearBit(pt->code[pt->PC - 1]);} },
    //CLR C 0xC3
    { [](Core8051* pt){pt->PC++; pt->ClearBit(PSW_7);} },
    //SWAP A  0xC4
    { [](Core8051* pt){pt->PC++; Byte_t temp;
                                temp.n0 = pt->ACC.n1;
                                temp.n1 = pt->ACC.n0;
                                pt->ACC.byte = temp.byte;} },
    //XCH A,data addr  0xC5
    { [](Core8051* pt){pt->PC += Byte_t{2}; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->code[pt->PC - 1]]; pt->ram[pt->code[pt->PC - 1]] = temp;} },
    //XCH A,@Ri  0xC6
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(0)]; pt->ram[pt->Register(0)] = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->ram[pt->Register(1)]; pt->ram[pt->Register(1)] = temp;} },
    //XCH A,Rn  0xC8
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(0); pt->Register(0) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(1); pt->Register(1) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(2); pt->Register(2) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(3); pt->Register(3) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(4); pt->Register(4) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(5); pt->Register(5) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(6); pt->Register(6) = temp;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC = pt->Register(7); pt->Register(7) = temp;} },
    //POP data addr  0xD0
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ram[pt->code[pt->PC - 1]] = pt->ram[pt->SP]; pt->SP--;} },
    //ACALL code addr  0xD1
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x600};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //SETB bit addr  0xD2
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->SetBit(pt->code[pt->PC - 1]);} },
    //SETB C  0xD3
    { [](Core8051* pt){pt->PC++; pt->SetBit(PSW_7);} },
    //DA A  0xD4
    { [](Core8051* pt){pt->PC++; if(pt->ACC.n0 > 9 || pt->GetBit(PSW_6))
                                    pt->AccAdd(Byte_t{0x6});
                                 if(pt->ACC.n1 > 9 || pt->GetBit(PSW_7))
                                    pt->AccAdd(Byte_t{0x60});
                                 } }, // <-----------------Check this
    //DJNZ data addr, code addr  0xD5
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->ram[pt->code[pt->PC - 1]]--;
                                if(pt->ram[pt->code[pt->PC - 1]] != 0)
                                    pt->PC.word += pt->code[pt->PC].signedByte;} },
    //XCHD A,@Ri  0xD6
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC.n0 = pt->ram[pt->Register(0)].n0; pt->ram[pt->Register(0)].n0 = temp.n0;} },
    { [](Core8051* pt){pt->PC++; Byte_t temp = pt->ACC; pt->ACC.n0 = pt->ram[pt->Register(1)].n0; pt->ram[pt->Register(1)].n0 = temp.n0;} },
    //DJNZ Rn, code addr  0xD8
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(0)--;
                                if(pt->Register(0) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(1)--;
                                if(pt->Register(1) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(2)--;
                                if(pt->Register(2) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(3)--;
                                if(pt->Register(3) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(4)--;
                                if(pt->Register(4) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(5)--;
                                if(pt->Register(5) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(6)--;
                                if(pt->Register(6) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                pt->Register(7)--;
                                if(pt->Register(7) != 0)
                                    pt->PC.word += pt->code[pt->PC - 1].signedByte;} },
    //MOVX A,@DPTR	 0xE0
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->DPTR];} },
    //AJMP code addr  0xE1
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x700};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
    //MOVX A,@Ri  0xE2
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->Register(0)];} },
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->externalRam[pt->Register(1)];} },
    //CLR A  0xE4
    { [](Core8051* pt){pt->PC++; pt->ACC = Byte_t{0};} },
    //MOV A,data addr  0xE5
    { [](Core8051* pt){pt->PC += Byte_t{2}; pt->ACC = pt->ram[pt->code[pt->PC - 1]];} },
    //MOV A,@Ri  0xE6
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(0)];} },
    { [](Core8051* pt){pt->PC++; pt->ACC = pt->ram[pt->Register(1)];} },
    //MOV A,Rn  0xE8
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
    { [](Core8051* pt){pt->PC += Byte_t{2};
                                Word_t temp{0x700};
                                temp.lowByte = pt->code[pt->PC - 1];
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.lowByte;
                                pt->SP++;
                                pt->ram[pt->SP] = pt->PC.highByte;
                                pt->PC.highByte.b2 = temp.highByte.b2;
                                pt->PC.highByte.b1 = temp.highByte.b1;
                                pt->PC.highByte.b0 = temp.highByte.b0;
                                pt->PC.lowByte = temp.lowByte;} },
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
    //MOV Rn,A  0xF8
    { [](Core8051* pt){pt->PC++; pt->Register(0) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(1) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(2) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(3) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(4) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(5) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(6) = pt->ACC;} },
    { [](Core8051* pt){pt->PC++; pt->Register(7) = pt->ACC;} }
    },
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
    //NOP  0x00
    { [](Core8051* pt){std::cout << "NOP";} },
    //AJMP code addr  0x01
    { [](Core8051* pt){Word_t temp{0x0000}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //LJMP code addr  0x02
    { [](Core8051* pt){std::cout << "LJMP";} },
    //RR A  0x03
    { [](Core8051* pt){std::cout << "RR A";} },
    //INC A  0x04
    { [](Core8051* pt){std::cout << "INC A";} },
    //INC data addr   0x05
    { [](Core8051* pt){std::cout << "INC " << pt->code[pt->PC + 1];} },
    //INC @Ri  0x06
    { [](Core8051* pt){std::cout << "INC @R0";} },
    { [](Core8051* pt){std::cout << "INC @R1";} },
    //INC Ri  0x08
    { [](Core8051* pt){std::cout << "INC R0";} },
    { [](Core8051* pt){std::cout << "INC R1";} },
    { [](Core8051* pt){std::cout << "INC R2";} },
    { [](Core8051* pt){std::cout << "INC R3";} },
    { [](Core8051* pt){std::cout << "INC R4";} },
    { [](Core8051* pt){std::cout << "INC R5";} },
    { [](Core8051* pt){std::cout << "INC R6";} },
    { [](Core8051* pt){std::cout << "INC R7";} },
    //JBC bt addr, code addr  0x10
    { [](Core8051* pt){std::cout << "JBC bit addr,code addr";} },
    //ACALL code addr  0x11
    { [](Core8051* pt){Word_t temp{0x0000}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //LCALL code addr  0x12
    { [](Core8051* pt){std::cout << "LCALL code addr";} },
    //RRC A  0x13
    { [](Core8051* pt){std::cout << "RRC A";} },
    //DEC A 0x14
    { [](Core8051* pt){std::cout << "DEC A";} },
    //DEC data addr  0x15
    { [](Core8051* pt){std::cout << "DEC " << pt->code[pt->PC + 1];} },
    //DEC @Ri  0x16
    { [](Core8051* pt){std::cout << "DEC @R0";} },
    { [](Core8051* pt){std::cout << "DEC @R1";} },
    //DEC R0  0x18
    { [](Core8051* pt){std::cout << "DEC R0";} },
    { [](Core8051* pt){std::cout << "DEC R1";} },
    { [](Core8051* pt){std::cout << "DEC R2";} },
    { [](Core8051* pt){std::cout << "DEC R3";} },
    { [](Core8051* pt){std::cout << "DEC R4";} },
    { [](Core8051* pt){std::cout << "DEC R5";} },
    { [](Core8051* pt){std::cout << "DEC R6";} },
    { [](Core8051* pt){std::cout << "DEC R7";} },
    //JB bit addr, code addr  0x20
     { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "JB " << Byte_t{temp.first} << '.' << Byte_t{temp.second} << ',' << pt->code[pt->PC + 2];} },
    //AJMP code addr  0x21
    { [](Core8051* pt){Word_t temp{0x0100}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //RET  0x22
    { [](Core8051* pt){std::cout << "RET";} },
    //RL A  0x23
    { [](Core8051* pt){std::cout << "RL A";} },
    //ADD A,#data  0x24
    { [](Core8051* pt){std::cout << "ADD A,#" << pt->code[pt->PC + 1];} },
    //ADD A,data addr  0x25
    { [](Core8051* pt){std::cout << "ADD A," << pt->code[pt->PC + 1];} },
    //ADD A,@Ri  0x26
    { [](Core8051* pt){std::cout << "ADD A,@R0";} },
    { [](Core8051* pt){std::cout << "ADD A,@R1";} },
    //ADD A,Rn  0x28
    { [](Core8051* pt){std::cout << "ADD A,R0";} },
    { [](Core8051* pt){std::cout << "ADD A,R1";} },
    { [](Core8051* pt){std::cout << "ADD A,R2";} },
    { [](Core8051* pt){std::cout << "ADD A,R3";} },
    { [](Core8051* pt){std::cout << "ADD A,R4";} },
    { [](Core8051* pt){std::cout << "ADD A,R5";} },
    { [](Core8051* pt){std::cout << "ADD A,R6";} },
    { [](Core8051* pt){std::cout << "ADD A,R7";} },
    //JNB bit addr, codeaddr  0x30
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "JNB " << Byte_t{temp.first} << '.' << Byte_t{temp.second} << ',' << pt->code[pt->PC + 2];} },
    //ACALL code addr  0x31
    { [](Core8051* pt){Word_t temp{0x0100}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //RETI  0x32
    { [](Core8051* pt){std::cout << "RETI";} },
    //RLC A  0x33
    { [](Core8051* pt){std::cout << "RLC A";} },
    //AddC A,#data  0x34
    { [](Core8051* pt){std::cout << "ADDC A,#" << pt->code[pt->PC + 1];} },
    //AddC A,data addr  0x35
     { [](Core8051* pt){std::cout << "ADDC A," << pt->code[pt->PC + 1];} },
    //AddC A,@Ri  0x36
    { [](Core8051* pt){std::cout << "ADDC A,@R0";} },
    { [](Core8051* pt){std::cout << "ADDC A,@R1";} },
    //AddC A,Rn  0x38
    { [](Core8051* pt){std::cout << "ADDC A,R0";} },
    { [](Core8051* pt){std::cout << "ADDC A,R1";} },
    { [](Core8051* pt){std::cout << "ADDC A,R2";} },
    { [](Core8051* pt){std::cout << "ADDC A,R3";} },
    { [](Core8051* pt){std::cout << "ADDC A,R4";} },
    { [](Core8051* pt){std::cout << "ADDC A,R5";} },
    { [](Core8051* pt){std::cout << "ADDC A,R6";} },
    { [](Core8051* pt){std::cout << "ADDC A,R7";} },
    //JC code addr  0x40
    { [](Core8051* pt){std::cout << "JC " << pt->code[pt->PC + 1];} },
    //AJMP code addr  0x41
    { [](Core8051* pt){Word_t temp{0x0200}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //ORL data addr,A  0x42
    { [](Core8051* pt){std::cout << "ORL " << pt->code[pt->PC + 1] << ",A";} },
    //ORL data addr,#data  0x43
    { [](Core8051* pt){std::cout << "ORL " << pt->code[pt->PC + 1] << ",#" << pt->code[pt->PC + 2];} },
    //ORL A,#data  0x44
    { [](Core8051* pt){std::cout << "ORL A,#" << pt->code[pt->PC + 1];} },
    //ORL A,data addr  0x45
    { [](Core8051* pt){std::cout << "ORL A," << pt->code[pt->PC + 1];} },
    //ORL A,@Ri  0x46
    { [](Core8051* pt){std::cout << "ORL A,@R0";} },
    { [](Core8051* pt){std::cout << "ORL A,@R1";} },
    //ORL data A,Ri  0x48
    { [](Core8051* pt){std::cout << "ORL A,R0";} },
    { [](Core8051* pt){std::cout << "ORL A,R1";} },
    { [](Core8051* pt){std::cout << "ORL A,R2";} },
    { [](Core8051* pt){std::cout << "ORL A,R3";} },
    { [](Core8051* pt){std::cout << "ORL A,R4";} },
    { [](Core8051* pt){std::cout << "ORL A,R5";} },
    { [](Core8051* pt){std::cout << "ORL A,R6";} },
    { [](Core8051* pt){std::cout << "ORL A,R7";} },
    //JNC code addr	 0x50
    { [](Core8051* pt){std::cout << "JNC " << pt->code[pt->PC + 1];} },
    //ACALL code addr  0x51
    { [](Core8051* pt){Word_t temp{0x0200}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //ANL data addr,A  0x52
    { [](Core8051* pt){std::cout << "ANL " << pt->code[pt->PC + 1] << ",A";} },
    //ANL data addr,#data  0x53
    { [](Core8051* pt){std::cout << "ANL " << pt->code[pt->PC + 1] << ",#" << pt->code[pt->PC + 2];} },
    //ANL A,#data  0x54
    { [](Core8051* pt){std::cout << "ANL A,#" << pt->code[pt->PC + 1];} },
    //ANL A,data addr  0x55
    { [](Core8051* pt){std::cout << "ANL A," << pt->code[pt->PC + 1];} },
    //ANL A,@Ri  0x56
    { [](Core8051* pt){std::cout << "ANL A,@R0";} },
    { [](Core8051* pt){std::cout << "ANL A,@R1";} },
    //ANL A,Ri  0x58
    { [](Core8051* pt){std::cout << "ANL A,R0";} },
    { [](Core8051* pt){std::cout << "ANL A,R1";} },
    { [](Core8051* pt){std::cout << "ANL A,R2";} },
    { [](Core8051* pt){std::cout << "ANL A,R3";} },
    { [](Core8051* pt){std::cout << "ANL A,R4";} },
    { [](Core8051* pt){std::cout << "ANL A,R5";} },
    { [](Core8051* pt){std::cout << "ANL A,R6";} },
    { [](Core8051* pt){std::cout << "ANL A,R7";} },
    //JZ code addr  0x60
    { [](Core8051* pt){std::cout << "JZ " << pt->code[pt->PC + 1];} },
    //AJMP codeaddr  0x61
    { [](Core8051* pt){Word_t temp{0x0300}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //XRL data addr,A  0x62
    { [](Core8051* pt){std::cout << "XRL " << pt->code[pt->PC + 1] << ",A";} },
    //XRL data addr,#data  0x63
    { [](Core8051* pt){std::cout << "XRL " << pt->code[pt->PC + 1] << ",#" << pt->code[pt->PC + 2];} },
    //XRL A,#data  0x64
    { [](Core8051* pt){std::cout << "XRL A,#" << pt->code[pt->PC + 1];} },
    //XRL A,data addr  0x65
    { [](Core8051* pt){std::cout << "XRL A," << pt->code[pt->PC + 1];} },
    //XRL A,@Ri  0x66
    { [](Core8051* pt){std::cout << "XRL A,@R0";} },
    { [](Core8051* pt){std::cout << "XRL A,@R1";} },
    //XRL A.R0  0x68
    { [](Core8051* pt){std::cout << "XRL A,R0";} },
    { [](Core8051* pt){std::cout << "XRL A,R1";} },
    { [](Core8051* pt){std::cout << "XRL A,R2";} },
    { [](Core8051* pt){std::cout << "XRL A,R3";} },
    { [](Core8051* pt){std::cout << "XRL A,R4";} },
    { [](Core8051* pt){std::cout << "XRL A,R5";} },
    { [](Core8051* pt){std::cout << "XRL A,R6";} },
    { [](Core8051* pt){std::cout << "XRL A,R7";} },
    //JNZ code addr  0x70
    { [](Core8051* pt){std::cout << "JNZ " << pt->code[pt->PC + 1];} },
    //ACALL code addr  0x71
    { [](Core8051* pt){Word_t temp{0x0300}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //ORL C,bit addr  0x72
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "ORL C," << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //JMP @A+DPTR  0x73
    { [](Core8051* pt){std::cout << "JMP @A+DPTR";} },
    //MOV A,#data  0x74
    { [](Core8051* pt){std::cout << "MOV A,#" << pt->code[pt->PC + 1];} },
    //MOV data addr,#data  0x75
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",#" << pt->code[pt->PC + 2];} },
    //MOV @Ri,#data  0x76
    { [](Core8051* pt){std::cout << "MOV @R0,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV @R1#" << pt->code[pt->PC + 1];} },
    //MOV Rn,#data  0x78
    { [](Core8051* pt){std::cout << "MOV R0,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R1,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R2,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R3,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R4,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R5,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R6,#" << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R7,#" << pt->code[pt->PC + 1];} },
    //SJMP code addr  0x80
    { [](Core8051* pt){std::cout << "SJMP code addr";} },
    //AJMP code addr  0x81
    { [](Core8051* pt){Word_t temp{0x0400}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //ANL C,bit addr  0x82
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "ANL C," << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //MOVC A,@A+PC  0x83
    { [](Core8051* pt){std::cout << "MOVC A,@A+PC";} },
    //DIV AB  0x84
    { [](Core8051* pt){std::cout << "DIV AB";} },
    //MOV data addr,data addr  0x85
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ',' << pt->code[pt->PC + 2];} },
    //MOV data addr,@Ri  0x86
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",@R0";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",@R1";} },
    //MOV data addr,Ri  0x88
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R0";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R1";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R2";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R3";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R4";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R5";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R6";} },
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",R7";} },
    //MOV DPTR,#data  0x90
    { [](Core8051* pt){std::cout << "MOV DPTR," << pt->code[pt->PC + 1] << pt->code[pt->PC + 2];} },
    //ACALL codeaddr  0x91
    { [](Core8051* pt){Word_t temp{0x0400}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //MOV bit addr,C  0x92
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "MOV" << Byte_t{temp.first} << '.' <<static_cast<unsigned short>(temp.second) << ",C";} },
    //MOVC A,@A+DPTR  0x93
    { [](Core8051* pt){std::cout << "MOVC A,@A+DPTR";} },
    //SUBB A,#data  0x4
    { [](Core8051* pt){std::cout << "SUBB A,#" << pt->code[pt->PC + 1];} },
    //SUBB A,data addr  0x95
    { [](Core8051* pt){std::cout << "SUBB A," << pt->code[pt->PC + 1];} },
    //SUBB A,@Ri  0x96
    { [](Core8051* pt){std::cout << "SUBB A,@R0";} },
    { [](Core8051* pt){std::cout << "SUBB A,@R1";} },
    //SUBB A,Rn  0x98
    { [](Core8051* pt){std::cout << "SUBB A,R0";} },
    { [](Core8051* pt){std::cout << "SUBB A,R1";} },
    { [](Core8051* pt){std::cout << "SUBB A,R2";} },
    { [](Core8051* pt){std::cout << "SUBB A,R3";} },
    { [](Core8051* pt){std::cout << "SUBB A,R4";} },
    { [](Core8051* pt){std::cout << "SUBB A,R5";} },
    { [](Core8051* pt){std::cout << "SUBB A,R6";} },
    { [](Core8051* pt){std::cout << "SUBB A,R7";} },
    //ORL C,/bit addr  0xA0
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "ORL C,/" << Byte_t{temp.first} << '.' <<static_cast<unsigned short>(temp.second);} },
    //AJMP code addr  0xA1
    { [](Core8051* pt){Word_t temp{0x0500}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //MOV C,bit addr  0xA2
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "MOV C," << Byte_t{temp.first} << '.' <<static_cast<unsigned short>(temp.second);} },
    //INC DPTR  0xA3
    { [](Core8051* pt){std::cout << "INC DPTR";} },
    //MUL AB  0xA4
    { [](Core8051* pt){std::cout << "MUL AB";} },
    //reserved  0xA5
    { [](Core8051* pt){std::cout << "RESERVED";} },
    //MOV @Ri,data addr	 0xA6
    { [](Core8051* pt){std::cout << "MOV @R0," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV @R1," << pt->code[pt->PC + 1];} },
    //MOV Rn,data addr  0xA8
    { [](Core8051* pt){std::cout << "MOV R0," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R1," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R2," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R3," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R4," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R5," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R6," << pt->code[pt->PC + 1];} },
    { [](Core8051* pt){std::cout << "MOV R7," << pt->code[pt->PC + 1];} },
    //ANL C,!bit addr  0xB0
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "ANL C,/" << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //ACALL code addr  0xB1
    { [](Core8051* pt){Word_t temp{0x0500}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //CPL bit addr  0xB2
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "CPL " << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //CPL C  0xB3
    { [](Core8051* pt){std::cout << "CPL C";} },
    //CJNE A,#data,code addr  0xB4
    { [](Core8051* pt){std::cout << "CJNE A,#data,code addr";} },
    //CJNE A,data addr,code addr  0xB5
    { [](Core8051* pt){std::cout << "CJNE A,data addr,code addr";} },
    //CJNE @Ri,#data,code addr  0xB6
    { [](Core8051* pt){std::cout << "CJNE @R0,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE @R1,data addr,code addr";} },
    //CJNE Rn,#data,code addr  0xB8
    { [](Core8051* pt){std::cout << "CJNE R0,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R1,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R2,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R3,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R4,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R5,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R6,data addr,code addr";} },
    { [](Core8051* pt){std::cout << "CJNE R7,data addr,code addr";} },
    //PUSH data addr  0xC0
    { [](Core8051* pt){std::cout << "PUSH " << pt->code[pt->PC + 1];} },
    //AJMP code addr  0xC1
    { [](Core8051* pt){Word_t temp{0x0600}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //CLR bit addr  0xC2
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "CLR " << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //CLR C  0xC3
    { [](Core8051* pt){std::cout << "CLR C";} },
    //SWAP A  0xC4
    { [](Core8051* pt){std::cout << "SWAP A";} },
    //XCH A,data addr  0xC5
    { [](Core8051* pt){std::cout << "XCH A," << pt->code[pt->PC + 1];} },
    //XCH A,@Ri  0xC6
    { [](Core8051* pt){std::cout << "XCH A,@R0";} },
    { [](Core8051* pt){std::cout << "XCH A,@R1";} },
    //XCH A,Rn  0xC8
    { [](Core8051* pt){std::cout << "XCH A,R0";} },
    { [](Core8051* pt){std::cout << "XCH A,R1";} },
    { [](Core8051* pt){std::cout << "XCH A,R2";} },
    { [](Core8051* pt){std::cout << "XCH A,R3";} },
    { [](Core8051* pt){std::cout << "XCH A,R4";} },
    { [](Core8051* pt){std::cout << "XCH A,R5";} },
    { [](Core8051* pt){std::cout << "XCH A,R6";} },
    { [](Core8051* pt){std::cout << "XCH A,R7";} },
    //POP data addr  0xD0
    { [](Core8051* pt){std::cout << "POP " << pt->code[pt->PC + 1];} },
    //ACALL code addr  0xD1
    { [](Core8051* pt){Word_t temp{0x0600}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //SETB bit addr  0xD2
    { [](Core8051* pt){std::pair<std::uint8_t, std::uint8_t> temp = pt->BitAddressDecoder(pt->code[pt->PC + 1]);
                        std::cout << "SETB " << Byte_t{temp.first} << '.' << static_cast<unsigned short>(temp.second);} },
    //SETB C  0xD3
    { [](Core8051* pt){std::cout << "SETB C";} },
    //DA A  0xD4
    { [](Core8051* pt){std::cout << "DA";} },
    //DJNZ data addr, rel  0xD5
    { [](Core8051* pt){std::cout << "DJNZ " << pt->code[pt->PC + 1] << ',' << pt->code[pt->PC + 1];} },
    //XCHD A,@Ri  0xD6
    { [](Core8051* pt){std::cout << "XCHD A,@R0";} },
    { [](Core8051* pt){std::cout << "XCHD A,@R1";} },
    //DJNZ Rn, rel  0xD8
    { [](Core8051* pt){std::cout << "DJNZ R0,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R1,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R2,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R3,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R4,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R5,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R6,code addr";} },
    { [](Core8051* pt){std::cout << "DJNZ R7,code addr";} },
    //MOVX A,@DPTR	 0xE0
    { [](Core8051* pt){std::cout << "MOVX A,@DPTR";} },
    //AJMP code addr  0xE1
    { [](Core8051* pt){Word_t temp{0x0700}; temp += pt->code[pt->PC + 1]; std::cout << "AJMP " << temp;} },
    //MOVX A,@Ri  0xE2
    { [](Core8051* pt){std::cout << "MOVX A,@R0";} },
    { [](Core8051* pt){std::cout << "MOVX A,@R1";} },
    //CLR A  0x4
    { [](Core8051* pt){std::cout << "CLR A";} },
    //MOV A,data addr  0xE5
    { [](Core8051* pt){std::cout << "MOV A," << pt->code[pt->PC + 1];} },
    //MOV A,@Ri  0xE6
    { [](Core8051* pt){std::cout << "MOV A,@R0";} },
    { [](Core8051* pt){std::cout << "MOV A,@R1";} },
    //MOV A,Rn  0xE8
    { [](Core8051* pt){std::cout << "MOV A,R0";} },
    { [](Core8051* pt){std::cout << "MOV A,R1";} },
    { [](Core8051* pt){std::cout << "MOV A,R2";} },
    { [](Core8051* pt){std::cout << "MOV A,R3";} },
    { [](Core8051* pt){std::cout << "MOV A,R4";} },
    { [](Core8051* pt){std::cout << "MOV A,R5";} },
    { [](Core8051* pt){std::cout << "MOV A,R6";} },
    { [](Core8051* pt){std::cout << "MOV A,R7";} },
    //MOVX @DPTR,A  0xF0
    { [](Core8051* pt){std::cout << "MOVX @DPTR,A";} },
    //ACALL code addr  0xF1
    { [](Core8051* pt){Word_t temp{0x0700}; temp += pt->code[pt->PC + 1]; std::cout << "ACALL " << temp;} },
    //MOVX @Ri,A  0xF2
    { [](Core8051* pt){std::cout << "MOVX @R0,A";} },
    { [](Core8051* pt){std::cout << "MOVX @R1,A";} },
    //CPL A  0xF4
    { [](Core8051* pt){std::cout << "CPL A";} },
    //MOV data addr,A  0xF5
    { [](Core8051* pt){std::cout << "MOV " << pt->code[pt->PC + 1] << ",A";} },
    //MOV @Ri,A  0xF6
    { [](Core8051* pt){std::cout << "MOV @R0,A";} },
    { [](Core8051* pt){std::cout << "MOV @R1,A";} },
    //MOV Rn,A  0xF8
    { [](Core8051* pt){std::cout << "MOV R0,A";} },
    { [](Core8051* pt){std::cout << "MOV R1,A";} },
    { [](Core8051* pt){std::cout << "MOV R2,A";} },
    { [](Core8051* pt){std::cout << "MOV R3,A";} },
    { [](Core8051* pt){std::cout << "MOV R4,A";} },
    { [](Core8051* pt){std::cout << "MOV R5,A";} },
    { [](Core8051* pt){std::cout << "MOV R6,A";} },
    { [](Core8051* pt){std::cout << "MOV R7,A";} }
    }
};

void Core8051::Test()
{
    SetBit(PSW_7);
    ACC = Byte_t{0x0};
    std::cout << std::dec << "  " << static_cast<short>(ACC.signedByte) << " "  << ACC << "  " << std::bitset<8>(ACC.byte) << "  " << std::bitset<8>(PSW.byte) << std::endl;
    for(int i = 0; i < 255; i++ )
    {
        AccAddC(Byte_t{1});
        std::cout << std::dec << "  " << static_cast<short>(ACC.signedByte) << " "  << ACC << "  " << std::bitset<8>(ACC.byte) << "  " << std::bitset<8>(PSW.byte) << std::endl;
    }
}

