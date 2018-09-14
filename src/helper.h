#ifndef _HELPER_H_
#define _HELPER_H_

#include "cpu.h"
#include "mem.h"

#define ZP(x) ((uint8_t) (x))

#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))


#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) ((x) & (y))
static inline void set_flag(cpu_t* cpu, uint8_t f, uint16_t set){
	if(set){
		cpu->sr |= f;
	}else{
		cpu->sr &= ~f;
	}
}

static inline uint16_t mem_abs(uint8_t l, uint8_t h , uint8_t off){
	//cpu->extra_cyc+=(((l | (h<<8)) & 0xff00) !=  ((uint16_t)off + (l | (h<<8)))) ? 1:0;
	return (uint16_t)off + (l | (h<<8));
}

static inline uint16_t mem_ind_inx(uint8_t addr, uint8_t off){
	return mem_abs(rb(addr),rb(addr+1),off);
}

static inline uint16_t mem_inx_ind(uint8_t addr, uint8_t off){
	// below code will not wrap around page zero.. the correct henc commented and corrected
	//return mem_abs(rb(addr+off),rb(addr+off+1),0);
	return mem_abs(rb(ZP(addr+off)),rb(ZP(addr+off+1)),0);
}

static inline void set_flags(cpu_t* cpu, uint8_t val){
	set_flag(cpu,ZF,!val);
	set_flag(cpu,NF,val&0x80);
}

static inline void add(cpu_t* cpu,uint16_t val){
	uint8_t tv = (uint8_t)val;
	uint8_t s = tv + cpu->a + cpu->f.c;
	val+= cpu->a + cpu->f.c;
	set_flag(cpu,CF,(val & 0x0100));
	// below old method for OF is wrong
	//set_flag(cpu,OF,(cpu->a & 0x80) != (val & 0x80));
	set_flag(cpu,OF, (cpu->a ^ s) & (tv ^ s) & 0x80);
	cpu->a = (uint8_t)val;
	set_flags(cpu,cpu->a);
}

static inline void sub(cpu_t* cpu,uint16_t val){
	uint8_t tv = (uint8_t)val;
	//uint8_t s = 
	val = cpu->a - val - !cpu->f.c;
	//set_flag(cpu,OF, val & 0xff00);
	set_flag(cpu,OF, (cpu->a ^ tv) & (cpu->a ^ val) & 0x80);
	set_flag(cpu,CF,!(val & 0x8000));
	cpu->a = (uint8_t)val;
	set_flags(cpu,cpu->a);
}

static inline void cmp(cpu_t* cpu, uint8_t m, uint8_t r){
	set_flag(cpu,CF,r>=m);
	set_flag(cpu,ZF,r==m);
	// is masking unnecessary step?
	set_flag(cpu,NF,0x80&(r-m));
}

static inline void push(cpu_t* cpu,uint8_t val){
	wb(cpu->sp-- + STACK_START,val);
}
static inline uint8_t pop(cpu_t* cpu){
	return rb(++cpu->sp + STACK_START);
}
#endif
