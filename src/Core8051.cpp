#include "Core8051.h"

Core8051::Core8051() :
    code{},
    ram{},
    externalRam{},
    B(ram[0xF0]),
    ACC(ram[0xF0]),
    PSW(ram[0xF0]),
    IP(ram[0xF0]),
    P3(ram[0xF0]),
    IE(ram[0xF0]),
    P2(ram[0xF0]),
    SBUF(ram[0xF0]),
    SCON(ram[0xF0]),
    P1(ram[0xF0]),
    TH1(ram[0xF0]),
    TH0(ram[0xF0]),
    TL1(ram[0xF0]),
    TL0(ram[0xF0]),
    TMOD(ram[0xF0]),
    TCON(ram[0xF0]),
    PCON(ram[0xF0]),
    DPH(ram[0xF0]),
    DPL(ram[0xF0]),
    SP(ram[0xF0]),
    P0(ram[0xF0])
{
    //ctor
}

Core8051::~Core8051()
{
    //dtor
}


std::pair<std::uint8_t, std::uint8_t> Core8051::BitAddressDecoder(std::uint8_t BitAddress)
{
    if(BitAddress < 0x80)
    {
        return std::pair<std::uint8_t, std::uint8_t>( 0x20 + (BitAddress /  8), (BitAddress % 8));
    }
    else
    {
        Byte temp;
        temp.byte = BitAddress;
        std::uint8_t cell = (temp.n1 * 0x10) + ( temp.n0 < 8 ? 0 : 8);
        std::uint8_t bit = (temp.n0 < 8 ? temp.n0 : temp.n0 - 8);
        return std::pair<std::uint8_t, std::uint8_t>(cell, bit);
    }
}
Byte& Core8051::Register(std::uint8_t r)
{
    Byte temp;
    temp.byte = r;
    temp.b3 = PSW.b3;
    temp.b4 = PSW.b4;
    return ram[temp.byte];
}

std::function<void(Core8051* pt)> Core8051::InstructionDecoder[] =
{
   { [](Core8051* pt){pt->PC.word++;} }, //NOP
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word++;} }, //LJMP code addr
   { [](Core8051* pt){pt->PC.word++;} }, //RR A
   { [](Core8051* pt){pt->PC.word++; pt->ACC.byte++;} }, //INC A
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte].byte++; pt->PC.word++;} }, //INC data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //INC @R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //INC @R1
   { [](Core8051* pt){pt->PC.word++; pt->Register(0).byte++;} }, //INC R0
   { [](Core8051* pt){pt->PC.word++; pt->Register(1).byte++;} }, //INC R1
   { [](Core8051* pt){pt->PC.word++; pt->Register(2).byte++;} }, //INC R2
   { [](Core8051* pt){pt->PC.word++; pt->Register(3).byte++;} }, //INC R3
   { [](Core8051* pt){pt->PC.word++; pt->Register(4).byte++;} }, //INC R4
   { [](Core8051* pt){pt->PC.word++; pt->Register(5).byte++;} }, //INC R5
   { [](Core8051* pt){pt->PC.word++; pt->Register(6).byte++;} }, //INC R6
   { [](Core8051* pt){pt->PC.word++; pt->Register(7).byte++;} }, //INC R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JBC bt addr, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //LCALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //RRC A
   { [](Core8051* pt){pt->PC.word++; pt->ACC.byte--;} }, //DEC A
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte].byte--; pt->PC.word++;} }, //DEC data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DEC @R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //DEC @R1
   { [](Core8051* pt){pt->PC.word++; pt->Register(0).byte--;} }, //DEC R0
   { [](Core8051* pt){pt->PC.word++; pt->Register(1).byte--;} }, //DEC R1
   { [](Core8051* pt){pt->PC.word++; pt->Register(2).byte--;} }, //DEC R2
   { [](Core8051* pt){pt->PC.word++; pt->Register(3).byte--;} }, //DEC R3
   { [](Core8051* pt){pt->PC.word++; pt->Register(4).byte--;} }, //DEC R4
   { [](Core8051* pt){pt->PC.word++; pt->Register(5).byte--;} }, //DEC R5
   { [](Core8051* pt){pt->PC.word++; pt->Register(6).byte--;} }, //DEC R6
   { [](Core8051* pt){pt->PC.word++; pt->Register(7).byte--;} }, //DEC R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JB bit addr, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //RET
   { [](Core8051* pt){pt->PC.word +=0;} }, //RL A
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADD A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JNB bit addr, codeaddr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //RETI
   { [](Core8051* pt){pt->PC.word +=0;} }, //RLC A
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //ADDC A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JC code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data addr,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data addr,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL data A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JNC code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL data addr,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL data addr,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JZ code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP codeaddr
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL data addr,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL data addr,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //XRL A.R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //JNZ code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL C,bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //JMP @A+DPTR
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++ ;} }, //MOV A,#data
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->ram[pt->code[pt->PC.word].byte + 1]; pt->PC.word +=2;} }, //MOV data addr,#data
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(0).byte] = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV @R0,#data
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(1).byte] = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV @R1,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(0) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R0,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(1) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R1,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(2) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R2,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(3) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R3,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(4) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R4,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(5) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R5,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(6) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R6,#data
   { [](Core8051* pt){pt->PC.word++; pt->Register(7) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R7,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //SJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL C.bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVC A,@A+PC
   { [](Core8051* pt){pt->PC.word +=0;} }, //DIV AB
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->ram[pt->code[pt->PC.word].byte + 1]; pt->PC.word +=2;} }, //MOV data addr,data addr
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->ram[pt->Register(0).byte]; pt->PC.word++;} }, //MOV data addr,@R0
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->ram[pt->Register(1).byte]; pt->PC.word++;} }, //MOV data addr,@R1
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(0); pt->PC.word++;} }, //MOV data addr,R0
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(1); pt->PC.word++;} }, //MOV data addr,R1
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(2); pt->PC.word++;} }, //MOV data addr,R2
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(3); pt->PC.word++;} }, //MOV data addr,R3
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(4); pt->PC.word++;} }, //MOV data addr,R4
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(5); pt->PC.word++;} }, //MOV data addr,R5
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(6); pt->PC.word++;} }, //MOV data addr,R6
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->Register(7); pt->PC.word++;} }, //MOV data addr,R7
   { [](Core8051* pt){pt->PC.word++; pt->DPH = pt->ram[pt->code[pt->PC.word].byte]; pt->DPL = pt->ram[pt->code[pt->PC.word].byte + 1];  pt->PC.word +=2;} }, //MOV DPTR,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL codeaddr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOV bit addr,C
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVC A,@A+DPTR
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,#data
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //SUBB A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //ORL C,bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOV C,bit add
   { [](Core8051* pt){pt->PC.word +=0;} }, //INC DPTR
   { [](Core8051* pt){pt->PC.word +=0;} }, //MUL AB
   { [](Core8051* pt){pt->PC.word +=0;} }, // reserved
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(0).byte] = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV @R0,data addr
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(1).byte] = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV @R1,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(0) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R0,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(1) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R1,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(2) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R2,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(3) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R3,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(4) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R4,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(5) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R5,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(6) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R6,data addr
   { [](Core8051* pt){pt->PC.word++; pt->Register(7) = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV R7,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ANL C,bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CPL bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CPL C
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE A,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE A,data addr,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE @R0,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE @R1,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R0,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R1,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R2,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R3,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R4,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R5,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R6,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CJNE R6,#data,code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //PUSH data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CLR bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //CLR C
   { [](Core8051* pt){pt->PC.word++; Byte temp; temp.n0 = pt->ACC.n1; temp.n1 = pt->ACC.n0; pt->ACC.byte = temp.byte;} }, //SWAP A
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R2
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R3
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R4
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R5
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R6
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCH A,R7
   { [](Core8051* pt){pt->PC.word +=0;} }, //POP data addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //SETB bit addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //SETB C
   { [](Core8051* pt){pt->PC.word +=0;} }, //DA A
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ data addr, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCHD A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //XCHD A,@R1
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R0, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R1, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R2, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R3, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R4, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R5, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R6, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //DJNZ R7, code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVX A,@DPTR
   { [](Core8051* pt){pt->PC.word +=0;} }, //AJMP code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVX A,@R0
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVX A,@R1
   { [](Core8051* pt){pt->PC.word++; pt->ACC.byte = 0x0;} }, //CLR A
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->ram[pt->code[pt->PC.word].byte]; pt->PC.word++;} }, //MOV A,data addr
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->ram[pt->Register(0).byte];} }, //MOV A,@R0
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->ram[pt->Register(1).byte];} }, //MOV A,@R1
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(0);} }, //MOV A,R0
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(1);} }, //MOV A,R1
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(2);} }, //MOV A,R2
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(3);} }, //MOV A,R3
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(4);} }, //MOV A,R4
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(5);} }, //MOV A,R5
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(6);} }, //MOV A,R6
   { [](Core8051* pt){pt->PC.word++; pt->ACC = pt->Register(7);} }, //MOV A,R7
   { [](Core8051* pt){pt->PC.word++; Word temp; temp.lowByte = pt->DPL; temp.highByte = pt->DPH; pt->externalRam[temp.word].byte = pt->ACC.byte;} }, //MOVX @DPTR,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //ACALL code addr
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVX @R0,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //MOVX @R1,A
   { [](Core8051* pt){pt->PC.word +=0;} }, //CPL A
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->code[pt->PC.word].byte] = pt->ACC; pt->PC.word++;} }, //MOV data addr,A
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(0).byte] = pt->ACC;} }, //MOV @R0,A
   { [](Core8051* pt){pt->PC.word++; pt->ram[pt->Register(1).byte] = pt->ACC;} }, //MOV @R1,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(0) = pt->ACC;} }, //MOV R0,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(1) = pt->ACC;} }, //MOV R1,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(2) = pt->ACC;} }, //MOV R2,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(3) = pt->ACC;} }, //MOV R3,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(4) = pt->ACC;} }, //MOV R4,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(5) = pt->ACC;} }, //MOV R5,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(6) = pt->ACC;} }, //MOV R6,A
   { [](Core8051* pt){pt->PC.word++; pt->Register(7) = pt->ACC;} }  //MOV R7,A
};
