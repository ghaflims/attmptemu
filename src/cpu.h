#ifndef _CPU_H_
#define _CPU_H_
#include "opcodes.h"
#include <stdint.h>
#define STACK_START 0x100
#define NMI_VEC 0xfffa
#define RST_VEC 0xfffc
#define IRQ_VEC 0xfffe
#define NF 0x80
#define OF 0x40
#define BF 0x10
#define DF 0x08
#define IF 0x04
#define ZF 0x02
#define CF 0x01

static const uint32_t ticktable[256] = {
/*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |     */
/* 0 */      7,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    4,    4,    6,    6,  /* 0 */
/* 1 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 1 */
/* 2 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    4,    4,    6,    6,  /* 2 */
/* 3 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 3 */
/* 4 */      6,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    3,    4,    6,    6,  /* 4 */
/* 5 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 5 */
/* 6 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    5,    4,    6,    6,  /* 6 */
/* 7 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 7 */
/* 8 */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* 8 */
/* 9 */      2,    6,    2,    6,    4,    4,    4,    4,    2,    5,    2,    5,    5,    5,    5,    5,  /* 9 */
/* A */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* A */
/* B */      2,    5,    2,    5,    4,    4,    4,    4,    2,    4,    2,    4,    4,    4,    4,    4,  /* B */
/* C */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* C */
/* D */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* D */
/* E */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* E */
/* F */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7   /* F */
};

static const char opcode_str[256][15] = {
"BRK impl","ORA X,ind","??? ---","??? ---","??? ---","ORA zpg","ASL zpg","??? ---","PHP impl","ORA #","ASL A","??? ---","??? ---","ORA abs","ASL abs","??? ---",
"BPL rel","ORA ind,Y","??? ---","??? ---","??? ---","ORA zpg,X","ASL zpg,X","??? ---","CLC impl","ORA abs,Y","??? ---","??? ---","??? ---","ORA abs,X","ASL abs,X","??? ---",
"JSR abs","AND X,ind","??? ---","??? ---","BIT zpg","AND zpg","ROL zpg","??? ---","PLP impl","AND #","ROL A","??? ---","BIT abs","AND abs","ROL abs","??? ---",
"BMI rel","AND ind,Y","??? ---","??? ---","??? ---","AND zpg,X","ROL zpg,X","??? ---","SEC impl","AND abs,Y","??? ---","??? ---","??? ---","AND abs,X","ROL abs,X","??? ---",
"RTI impl","EOR X,ind","??? ---","??? ---","??? ---","EOR zpg","LSR zpg","??? ---","PHA impl","EOR #","LSR A","??? ---","JMP abs","EOR abs","LSR abs","??? ---",
"BVC rel","EOR ind,Y","??? ---","??? ---","??? ---","EOR zpg,X","LSR zpg,X","??? ---","CLI impl","EOR abs,Y","??? ---","??? ---","??? ---","EOR abs,X","LSR abs,X","??? ---",
"RTS impl","ADC X,ind","??? ---","??? ---","??? ---","ADC zpg","ROR zpg","??? ---","PLA impl","ADC #","ROR A","??? ---","JMP ind","ADC abs","ROR abs","??? ---",
"BVS rel","ADC ind,Y","??? ---","??? ---","??? ---","ADC zpg,X","ROR zpg,X","??? ---","SEI impl","ADC abs,Y","??? ---","??? ---","??? ---","ADC abs,X","ROR abs,X","??? ---",
"??? ---","STA X,ind","??? ---","??? ---","STY zpg","STA zpg","STX zpg","??? ---","DEY impl","??? ---","TXA impl","??? ---","STY abs","STA abs","STX abs","??? ---",
"BCC rel","STA ind,Y","??? ---","??? ---","STY zpg,X","STA zpg,X","STX zpg,Y","??? ---","TYA impl","STA abs,Y","TXS impl","??? ---","??? ---","STA abs,X","??? ---","??? ---",
"LDY #","LDA X,ind","LDX #","??? ---","LDY zpg","LDA zpg","LDX zpg","??? ---","TAY impl","LDA #","TAX impl","??? ---","LDY abs","LDA abs","LDX abs","??? ---",
"BCS rel","LDA ind,Y","??? ---","??? ---","LDY zpg,X","LDA zpg,X","LDX zpg,Y","??? ---","CLV impl","LDA abs,Y","TSX impl","??? ---","LDY abs,X","LDA abs,X","LDX abs,Y","??? ---",
"CPY #","CMP X,ind","??? ---","??? ---","CPY zpg","CMP zpg","DEC zpg","??? ---","INY impl","CMP #","DEX impl","??? ---","CPY abs","CMP abs","DEC abs","??? ---",
"BNE rel","CMP ind,Y","??? ---","??? ---","??? ---","CMP zpg,X","DEC zpg,X","??? ---","CLD impl","CMP abs,Y","??? ---","??? ---","??? ---","CMP abs,X","DEC abs,X","??? ---",
"CPX #","SBC X,ind","??? ---","??? ---","CPX zpg","SBC zpg","INC zpg","??? ---","INX impl","SBC #","NOP impl","??? ---","CPX abs","SBC abs","INC abs","??? ---",
"BEQ rel","SBC ind,Y","??? ---","??? ---","??? ---","SBC zpg,X","INC zpg,X","??? ---","SED impl","SBC abs,Y","??? ---","??? ---","??? ---","SBC abs,X","INC abs,X","??? ---"
};

typedef struct{
  uint32_t cyc;
  uint16_t pc;
  uint8_t x,y,a,sp;
  union{
      uint8_t sr;
      struct{
        uint32_t c:1;
        uint32_t z:1;
        uint32_t i:1;
        uint32_t d:1;
        uint32_t b:1;
        uint32_t na:1;
        uint32_t v:1;
        uint32_t n:1;
      } f;
  };

} cpu_t;

void cpu_init(cpu_t* cpu);
void cpu_reset(cpu_t* cpu);
void cpu_trigger_nmi(cpu_t* cpu);
void cpu_exec(cpu_t* cpu, long cycles);
uint8_t cpu_ram_ior(uint16_t addr);
void cpu_ram_iow(uint16_t addr, uint8_t data);
#endif
