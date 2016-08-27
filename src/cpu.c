#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "ppu.h"
#include "helper.h"
#include "debug.h"

uint8_t cpu_ram[0x8000] = {0};
static inline uint16_t nmi_vector_address(){
	return (rb(NMI_VEC) | (rb(NMI_VEC+1)<<8)); 
}
static inline uint16_t reset_vector_address(){
	return (rb(RST_VEC) | (rb(RST_VEC+1)<<8));
}
static inline uint16_t irq_vector_address(){
	return (rb(IRQ_VEC) | (rb(IRQ_VEC+1)<<8));
}

void cpu_init(cpu_t* cpu){
	//*cpu = {.cyc=0,.pc=0,.x=0,.y=0,.a=0,.sp=00,.sr=0x24};
	memset(cpu,0,sizeof(cpu_t));
	cpu->sr = 0x24;
}
void cpu_reset(cpu_t* cpu){
	cpu->pc =  reset_vector_address();
	cpu->sp -= 3;
	cpu->sr |= IF;
}
void cpu_trigger_nmi(cpu_t* cpu){
	if(ppu_is_nmi_enabled()){
		cpu->sr |= IF;
		BIT_CLEAR(cpu->sr,5);
		
		push(cpu,(cpu->pc>>8)&0xff);
		push(cpu,cpu->pc&0xff);
		push(cpu,cpu->sr);
		cpu->pc=nmi_vector_address();
	}
}

inline uint8_t cpu_ram_ior(uint16_t addr){
	return cpu_ram[addr&0x7ff];
}
inline void cpu_ram_iow(uint16_t addr, uint8_t data){
	cpu_ram[addr&0x7ff] = data;
}

void cpu_exec(cpu_t* cpu, long cycles){
	while(cycles>0){
		uint8_t op = rb(cpu->pc++);
		// update the cycle timing.. this will not take in account page crossing or special additional timing..
		cpu->cyc+=ticktable[op];
		// temp address to be used inside case statment
		uint16_t ta=0;
		// temp value to be used inside case statment
		uint8_t tv=0;
		switch(op){
			case ADC_AB:
				add(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0)));
				cpu->pc+=2;
				//cyc+=4;
			break;
			case ADC_ABX:
				add(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x)));
				cpu->pc+=2;
			break;
			case ADC_ABY:
				add(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y)));
				cpu->pc+=2;
			break;
			case ADC_IMM:
				add(cpu,rb(cpu->pc++));
			break;
			case ADC_INX:
				add(cpu,rb(mem_inx_ind(rb(cpu->pc++),cpu->x)));
			break;
			case ADC_INY:
				add(cpu,rb(mem_ind_inx(rb(cpu->pc++),cpu->y)));
			break;
			case ADC_ZP:
				add(cpu,rb(rb(cpu->pc++)));
			break;
			case ADC_ZPX:
				add(cpu,rb(ZP(rb(cpu->pc++) + cpu->x)));
			break;
			case AND_AB:
				cpu->a &= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case AND_ABX:
				cpu->a &= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case AND_ABY:
				cpu->a &= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case AND_IMM:
				cpu->a &= rb(cpu->pc++);
				set_flags(cpu,cpu->a);
			break;
			case AND_INX:
				cpu->a &= rb(mem_inx_ind(rb(cpu->pc++),cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case AND_INY:
				cpu->a &= rb(mem_ind_inx(rb(cpu->pc++),cpu->y));
				set_flags(cpu,cpu->a);
			break;
			case AND_ZP:
				cpu->a &= rb(rb(cpu->pc++));
				set_flags(cpu,cpu->a);
			break;
			case AND_ZPX:
				cpu->a &= rb(ZP(rb(cpu->pc++) + cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case ASL_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				set_flag(cpu,CF,tv&0x80);
				tv = (tv<<1) & 0xfe;
				set_flags(cpu,tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case ASL_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				set_flag(cpu,CF,tv&0x80);
				tv = (tv<<1) & 0xfe;
				set_flags(cpu,tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case ASL_ACC:
				set_flag(cpu,CF,cpu->a&0x80);
				cpu->a = (cpu->a<<1) & 0xfe;
				set_flags(cpu,cpu->a);
			break;
			case ASL_ZP:
				tv = rb(cpu->pc);
				set_flag(cpu,CF,tv&0x80);
				// is masking unneeded step?
				tv = (tv<<1) & 0xfe;
				set_flags(cpu,tv);
				wb(cpu->pc++,tv);
			break;
			case ASL_ZPX:
				tv = rb(ZP(cpu->pc + cpu->x));
				set_flag(cpu,CF,tv&0x80);
				tv = (tv<<1) & 0xfe;
				set_flags(cpu,tv);
				wb(ZP(cpu->pc++ + cpu->x),tv);			
			break;
			case BCC_REL:
				// need to cast value from memory to signed value to support forward and backward brancing :)
				// don't use ? t:f favor readablity force yourself not no >_<"
				if(!cpu->f.c){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BCS_REL:
				if(cpu->f.c){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BEQ_REL:
				if(cpu->f.z){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BIT_AB:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				set_flag(cpu,ZF,!(tv&cpu->a));
				set_flag(cpu,OF,tv&0x40);
				set_flag(cpu,NF,tv&0x80);
				cpu->pc+=2;
			break;
			case BIT_ZP:
				tv = rb(rb(cpu->pc++));
				set_flag(cpu,ZF,!(tv&cpu->a));
				set_flag(cpu,OF,tv&0x40);
				set_flag(cpu,NF,tv&0x80);
			break;
			case BMI_REL:
				if(cpu->f.n){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BNE_REL:
				if(!cpu->f.z){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BPL_REL:
				if(!cpu->f.n){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BRK:
				set_flag(cpu,BF,1);
			break;
			case BVC_REL:
				if(!cpu->f.v){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case BVS_REL:
				if(cpu->f.v){
					cpu->pc+=(int8_t)rb(cpu->pc);
				}
				cpu->pc++;
			break;
			case CLC:
				set_flag(cpu,CF,0);
			break;
			case CLD:
				set_flag(cpu,DF,0);
			break;
			case CLI:
				set_flag(cpu,IF,0);
			break;
			case CLV:
				set_flag(cpu,OF,0);
			break;
			case CMP_AB:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cmp(cpu,tv,cpu->a);
				cpu->pc+=2;
			break;
			case CMP_ABX:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cmp(cpu,tv,cpu->a);
				cpu->pc+=2;
			break;
			case CMP_ABY:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cmp(cpu,tv,cpu->a);
				cpu->pc+=2;
			break;
			case CMP_IMM:
				cmp(cpu,rb(cpu->pc++),cpu->a);
			break;
			case CMP_INX:
				cmp(cpu,rb(mem_inx_ind(rb(cpu->pc++),cpu->x)),cpu->a);
			break;
			case CMP_INY:
				cmp(cpu,rb(mem_ind_inx(rb(cpu->pc++),cpu->y)),cpu->a);
			break;
			case CMP_ZP:
				cmp(cpu,rb(rb(cpu->pc++)),cpu->a);
			break;
			case CMP_ZPX:
				cmp(cpu,rb(ZP(rb(cpu->pc++)+cpu->x)),cpu->a);
			break;
			case CPX_AB:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cmp(cpu,tv,cpu->x);
				cpu->pc+=2;
			break;
			case CPX_IMM:
				cmp(cpu,rb(cpu->pc++),cpu->x);
			break;
			case CPX_ZP:
				cmp(cpu,rb(rb(cpu->pc++)),cpu->x);
			break;
			case CPY_AB:
				tv = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cmp(cpu,tv,cpu->y);
				cpu->pc+=2;
			break;
			case CPY_IMM:
				cmp(cpu,rb(cpu->pc++),cpu->y);
			break;
			case CPY_ZP:
				cmp(cpu,rb(rb(cpu->pc++)),cpu->y);
			break;
			case DEC_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				set_flags(cpu,--tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case DEC_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				set_flags(cpu,--tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case DEC_ZP:
				ta = rb(cpu->pc++);
				tv = rb(ta);
				set_flags(cpu,--tv);
				wb(ta,tv);
			break;
			case DEC_ZPX:
				ta = ZP(rb(cpu->pc++) + cpu->x);
				tv = rb(ta);
				set_flags(cpu,--tv);
				wb(ta,tv);
			break;
			case DEX:
				set_flags(cpu,--cpu->x);
			break;
			case DEY:
				set_flags(cpu,--cpu->y);
			break;
			case EOR_AB:
				cpu->a ^= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case EOR_ABX:
				cpu->a ^= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case EOR_ABY:
				cpu->a ^= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case EOR_IMM:
				cpu->a ^= rb(cpu->pc++);
				set_flags(cpu,cpu->a);
			break;
			case EOR_INX:
				cpu->a ^= rb(mem_inx_ind(rb(cpu->pc++),cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case EOR_INY:
				cpu->a ^= rb(mem_ind_inx(rb(cpu->pc++),cpu->y));
				set_flags(cpu,cpu->a);
			break;
			case EOR_ZP:
				cpu->a ^= rb(rb(cpu->pc++));
				set_flags(cpu,cpu->a);
			break;
			case EOR_ZPX:
				cpu->a ^= rb(ZP(rb(cpu->pc++) + cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case INC_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				set_flags(cpu,++tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case INC_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				set_flags(cpu,++tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case INC_ZP:
				ta = rb(cpu->pc++);
				tv = rb(ta);
				set_flags(cpu,++tv);
				wb(ta,tv);
			break;
			case INC_ZPX:
				ta = ZP(rb(cpu->pc++) + cpu->x);
				tv = rb(ta);
				set_flags(cpu,++tv);
				wb(ta,tv);
			break;
			case INX:
				set_flags(cpu,++cpu->x);
			break;
			case INY:
				set_flags(cpu,++cpu->y);
			break;
			case JMP_AB:
				cpu->pc = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
			break;
			case JMP_IN:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				// todo: better code than below and avoid magic numbers this is to fix page wrapping
				cpu->pc = mem_abs(rb(ta),rb((ta&0xff00)|((ta+1)&0xff)),0);
			break;
			case JSR_AB:
				// again is masking unccessry step?
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				//push(cpu,((cpu->pc+2)>>8)&0xff);
				push(cpu,((cpu->pc+1)>>8)&0xff);
				//push(cpu,(cpu->pc+2)&0xff);
				push(cpu,(cpu->pc+1)&0xff);
				cpu->pc = ta;
			break;
			case LDA_AB:
				cpu->a = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case LDA_ABX:
				cpu->a = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case LDA_ABY:
				cpu->a = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case LDA_IMM:
				cpu->a = rb(cpu->pc++);
				set_flags(cpu,cpu->a);
			break;
			case LDA_INX:
				cpu->a = rb(mem_inx_ind(rb(cpu->pc++),cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case LDA_INY:
				cpu->a = rb(mem_ind_inx(rb(cpu->pc++),cpu->y));
				set_flags(cpu,cpu->a);
			break;
			case LDA_ZP:
				cpu->a = rb(rb(cpu->pc++));
				set_flags(cpu,cpu->a);
			break;
			case LDA_ZPX:
				cpu->a = rb(ZP(rb(cpu->pc++) + cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case LDX_AB:
				cpu->x = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->x);
			break;
			case LDX_ABY:
				cpu->x = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cpu->pc+=2;
				set_flags(cpu,cpu->x);
			break;
			case LDX_IMM:
				cpu->x = rb(cpu->pc++);
				set_flags(cpu,cpu->x);
			break;
			case LDX_ZP:
				cpu->x = rb(rb(cpu->pc++));
				set_flags(cpu,cpu->x);
			break;
			case LDX_ZPY:
				cpu->x = rb(ZP(rb(cpu->pc++) + cpu->y));
				set_flags(cpu,cpu->x);
			break;
			case LDY_AB:
				cpu->y = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->y);
			break;
			case LDY_ABX:
				cpu->y = rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cpu->pc+=2;
				set_flags(cpu,cpu->y);
			break;
			case LDY_IMM:
				cpu->y = rb(cpu->pc++);
				set_flags(cpu,cpu->y);
			break;
			case LDY_ZP:
				cpu->y = rb(rb(cpu->pc++));
				set_flags(cpu,cpu->y);
			break;
			case LDY_ZPX:
				cpu->y = rb(ZP(rb(cpu->pc++) + cpu->x));
				set_flags(cpu,cpu->y);
			break;
			case LSR_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				set_flag(cpu,CF,tv&0x01);
				tv = (tv>>1) & 0x7f;
				set_flags(cpu,tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case LSR_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				set_flag(cpu,CF,tv&0x01);
				tv = (tv>>1) & 0x7f;
				set_flags(cpu,tv);
				wb(ta,tv);
				cpu->pc+=2;
			break;
			case LSR_ACC:
				set_flag(cpu,CF,cpu->a&0x01);
				cpu->a = (cpu->a>>1) & 0x7f;
				set_flags(cpu,cpu->a);
			break;
			case LSR_ZP:
				tv = rb(cpu->pc);
				set_flag(cpu,CF,tv&0x01);
				// is masking unneeded step?
				tv = (tv>>1) & 0x7f;
				set_flags(cpu,tv);
				wb(cpu->pc++,tv);
			break;
			case LSR_ZPX:
				tv = rb(ZP(cpu->pc + cpu->x));
				set_flag(cpu,CF,tv&0x01);
				tv = (tv>>1) & 0x7f;
				set_flags(cpu,tv);
				wb(ZP(cpu->pc++ + cpu->x),tv);
			break;
			case ORA_IMM:
				cpu->a |= rb(cpu->pc++);
				set_flags(cpu,cpu->a);
			break;
			case ORA_ZP:
				cpu->a |= rb(rb(cpu->pc++));
				set_flags(cpu,cpu->a);
			break;
			case ORA_ZPX:
				cpu->a |= rb(ZP(rb(cpu->pc++) + cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case ORA_AB:
				cpu->a |= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case ORA_ABX:
				cpu->a |= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case ORA_ABY:
				cpu->a |= rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y));
				cpu->pc+=2;
				set_flags(cpu,cpu->a);
			break;
			case ORA_INX:
				cpu->a |= rb(mem_inx_ind(rb(cpu->pc++),cpu->x));
				set_flags(cpu,cpu->a);
			break;
			case ORA_INY:
				cpu->a |= rb(mem_ind_inx(rb(cpu->pc++),cpu->y));
				set_flags(cpu,cpu->a);
			break;
			case NOP:break;
			case PHA:
				push(cpu,cpu->a);
			break;
			case PHP:
				push(cpu,cpu->sr);
			break;
			case PLA:
				cpu->a = pop(cpu);
				set_flags(cpu,cpu->a);
			break;
			case PLP:
				cpu->sr= pop(cpu);
			break;
			case ROL_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				wb(ta,((tv<<1) & 0xfe) | cpu->f.c);
				set_flag(cpu,CF,tv&0x80);
				set_flags(cpu,rb(ta));
				cpu->pc+=2;
			break;
			case ROL_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				wb(ta,((tv<<1) & 0xfe) | cpu->f.c);
				set_flag(cpu,CF,tv&0x80);
				set_flags(cpu,rb(ta));
				cpu->pc+=2;
			break;
			case ROL_ACC:
				tv = cpu->a & 0x80;
				cpu->a = ((cpu->a<<1) & 0xfe) | cpu->f.c;
				set_flag(cpu,CF,tv);
				set_flags(cpu,cpu->a);
			break;
			case ROL_ZP:
				ta = rb(cpu->pc++);
				tv = rb(ta);
				wb(ta,((tv<<1) & 0xfe) | cpu->f.c);
				set_flag(cpu,CF,tv&0x80);
				set_flags(cpu,rb(ta));
			break;
			case ROL_ZPX:
				ta = ZP(rb(cpu->pc++) + cpu->x);
				tv = rb(ta);
				wb(ta,((tv<<1) & 0xfe) | cpu->f.c);
				set_flag(cpu,CF,tv&0x80);
				set_flags(cpu,rb(ta));
			break;
			case ROR_AB:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),0);
				tv = rb(ta);
				wb(ta,((tv>>1) & 0x7f) | cpu->f.c << 7);
				set_flag(cpu,CF,tv&0x01);
				set_flags(cpu,rb(ta));
				cpu->pc+=2;
			break;
			case ROR_ABX:
				ta = mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x);
				tv = rb(ta);
				wb(ta,((tv>>1) & 0x7f) | cpu->f.c << 7);
				set_flag(cpu,CF,tv&0x01);
				set_flags(cpu,rb(ta));
				cpu->pc+=2;
			break;
			case ROR_ACC:
				tv = cpu->a & 0x01;
				cpu->a = ((cpu->a>>1) & 0x7f) | cpu->f.c << 7;
				set_flag(cpu,CF,tv);
				set_flags(cpu,cpu->a);
			break;
			case ROR_ZP:
				ta = rb(cpu->pc++);
				tv = rb(ta);
				wb(ta,((tv>>1) & 0x7f) | cpu->f.c << 7);
				set_flag(cpu,CF,tv&0x01);
				set_flags(cpu,rb(ta));
			break;
			case ROR_ZPX:
				ta = ZP(rb(cpu->pc++) + cpu->x);
				tv = rb(ta);
				wb(ta,((tv>>1) & 0x7f) | cpu->f.c << 7);
				set_flag(cpu,CF,tv&0x01);
				set_flags(cpu,rb(ta));
			break;
			case RTI:
				cpu->sr = pop(cpu);
				tv = pop(cpu);
				cpu->pc = mem_abs(tv,pop(cpu),0);
			break;
			case RTS:
				tv = pop(cpu);
				cpu->pc = mem_abs(tv,pop(cpu),0) + 1;
			break;
			case SBC_IMM:
				sub(cpu,rb(cpu->pc++));
			break;
			case SBC_ZP:
				sub(cpu,rb(rb(cpu->pc++)));
			break;
			case SBC_ZPX:
				sub(cpu,rb(ZP(rb(cpu->pc++) + cpu->x)));
			break;
			case SBC_AB:
				sub(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0)));
				cpu->pc+=2;
			break;
			case SBC_ABX:
				sub(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x)));
				cpu->pc+=2;
			break;
			case SBC_ABY:
				sub(cpu,rb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y)));
				cpu->pc+=2;
			break;
			case SBC_INX:
				sub(cpu,rb(mem_inx_ind(rb(cpu->pc++),cpu->x)));
			break;
			case SBC_INY:
				sub(cpu,rb(mem_ind_inx(rb(cpu->pc++),cpu->y)));
			break;
			case SEC:
				set_flag(cpu,CF,1);
			break;
			case SED:
				set_flag(cpu,DF,1);
			break;
			case SEI:
				set_flag(cpu,IF,1);
			break;
			case STA_AB:
				wb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0),cpu->a);
				cpu->pc+=2;
			break;
			case STA_ABX:
				wb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->x),cpu->a);
				cpu->pc+=2;
			break;
			case STA_ABY:
				wb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),cpu->y),cpu->a);
				cpu->pc+=2;
			break;
			case STA_INX:
				wb(mem_inx_ind(rb(cpu->pc++),cpu->x),cpu->a);
			break;
			case STA_INY:
				wb(mem_ind_inx(rb(cpu->pc++),cpu->y),cpu->a);
			break;
			case STA_ZP:
				wb(rb(cpu->pc++),cpu->a);
			break;
			case STA_ZPX:
				wb(ZP(rb(cpu->pc++) + cpu->x),cpu->a);
			break;
			case STX_ZP:
				wb(rb(cpu->pc++),cpu->x);
			break;
			case STX_ZPY:
				wb(ZP(rb(cpu->pc++) + cpu->y),cpu->x);
			break;
			case STX_AB:
				wb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0),cpu->x);
				cpu->pc+=2;
			break;
			case STY_ZP:
				wb(rb(cpu->pc++),cpu->y);
			break;
			case STY_ZPX:
				wb(ZP(rb(cpu->pc++) + cpu->x), cpu->y);
			break;
			case STY_AB:
				wb(mem_abs(rb(cpu->pc),rb(cpu->pc+1),0),cpu->y);
			break;
			case TAX:
				cpu->x = cpu->a;
				set_flags(cpu,cpu->x);
			break;
			case TAY:
				cpu->y = cpu->a;
				set_flags(cpu,cpu->y);
			break;
			case TSX:
				cpu->x = cpu->sp;
				set_flags(cpu,cpu->x);
			break;
			case TXA:
				cpu->a = cpu->x;
				set_flags(cpu,cpu->a);
			break;
			case TXS:
				cpu->sp = cpu->x;
			break;
			case TYA:
				cpu->a = cpu->y;
				set_flags(cpu,cpu->a);
			break;
			case WAI:break;

			default:
				break;
				
		}
	// run cpu until the requested cycles..
	cycles-=ticktable[op];
	// todo: tick the ppu 3x times as the value in ticktable[op]
	int i = 0;
	for(i=0;i<ticktable[op]*3;i++)
		ppu_tick();
	
//	print_debug(cpu,op);
	}
}
