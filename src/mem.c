#include "mem.h"
#include "mmc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ppu.h"
#include "cpu.h"
#include "psg.h"
//#include <stdio.h>

void mem_init(void){
 // mem = (uint8_t*) malloc(MEM_SIZE);
/*
  memset(mem,0xff,MEM_SIZE);
  memcpy(mem+PRG_START,prg,sizeof(prg));
  *((uint16_t*) &mem[0xfffc]) = PRG_START; 
  
  memcpy(mem+NMI_START,nmiprg,sizeof(nmiprg));
  *((uint16_t*) &mem[0xfffa]) = NMI_START; 
*/
}

void mem_free(void){
//  free(mem);
}

uint8_t rb(uint16_t addr){
	uint8_t r;
	switch(addr >> 13){
		case 0:
			r = cpu_ram_ior(addr&0x07ff);
			if(debug_flag){
				printf("R:\tAddress:%04X\tValue:%02X\n",addr,r);
			}
			return r;
			break;
		case 1:
			r = ppu_ior(addr);
			if(debug_flag){
				printf("R:\tAddress:%04X\tValue:%02X\n",addr,r);
			}
			return r;
			break;
		case 2:
			return psg_ior(addr);
			break;
		case 3:
			return cpu_ram_ior(addr&0x17ff);
			break;
		default:
			return mmc_ior(addr);
		//	return mem[addr];
	}

//	return mem[addr];
}
void wb(uint16_t addr, uint8_t b){
	if(debug_flag){
		printf("W:\tAddress:%04X\tValue:%02X\n",addr,b);
	}
	// DMA operation..
	int i;
	// TODO avoid magic numbers 0x4014 is the DMA reg
	if(addr == 0x4014){
		for(i=0;i<256;i++){
			ppu_oam_wb(cpu_ram_ior((0x100 * b) + i));
		}
		return;
	}
	switch(addr >> 13){
		case 0:
			cpu_ram_iow(addr&0x07ff,b);
			break;
		case 1:
			ppu_iow(addr, b);
			break;
		case 2:
			psg_iow(addr,b);
			break;
		case 3:
			cpu_ram_iow(addr&0x1fff,b);
			break;
		default:
			mmc_iow(addr,b);
		//	mem[addr] = b;
	}

//mem[addr] = b;
}
