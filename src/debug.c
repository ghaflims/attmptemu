#include "debug.h"
void print_debug(cpu_t* cpu,uint8_t op){
	printf("----------------CYC: [%08d]------------------\n",cpu->cyc);
	printf("-------------PRE_CYC: [%08d]------------------\n",cpu->pre_cyc);
	printf("A:  [0x%02X]\t",cpu->a);
	printf("X:  [0x%02X]\t",cpu->x);
	printf("Y:  [0x%02X]\n",cpu->y);
	printf("PC: [0x%04X]\t",cpu->pc);
	printf("SP: [0x%02X]\t",cpu->sp);
	printf("SR: [");
	printf("%c",(cpu->f.n) ? 'N':'n');
	printf("%c",(cpu->f.v) ? 'V':'v');
	printf("-");
	printf("%c",(cpu->f.b) ? 'B':'b');
	printf("%c",(cpu->f.d) ? 'D':'d');
	printf("%c",(cpu->f.i) ? 'I':'i');
	printf("%c",(cpu->f.z) ? 'Z':'z');
	printf("%c",(cpu->f.c) ? 'C':'c');
	printf("]\n");
	//printf("Cycle: [%d]\n",cpu->cyc);

	//printf("----------------OP: [0x%02X]------------------------\n\n",op);
	printf("----------------OP: [0x%02X] %s ---------------------\n\n",op,opcode_str[op]);

}

void mem_dump(const void* mem,size_t len){
	const int COL_SIZE = 8;
	size_t i;
	for(i=0; i< len + ((len % COL_SIZE) ? (COL_SIZE - len % COL_SIZE) : 0); ++i){
		//the offset
		if(i % COL_SIZE == 0){
			printf("0x%04lx: ",i);
		}
		//the data
		if(i < len){
			printf("%02x ",((uint8_t*)mem)[i]);
		}else{ // less than COL_SIZE just align
			printf("   ");
		}
		// move to next row
		if(i % COL_SIZE == (COL_SIZE -1))
			putchar('\n');
	}
}

void ppu_dump(int x, int y, uint16_t nt, uint16_t lv){
	printf("y:%03d\tx:%03d\tnt:0x%04x\tlv:0x%04x\n",y,x,nt,lv);
}

void debug_ppu(ppu_t* ppu){
	printf("***********************************************\n");
	printf("PPUCTRL:  [0x%02X]\t",ppu->PPUCTRL);
	printf("PPUMASK:  [0x%02X]\n",ppu->PPUMASK);
	printf("PPUSTATUS:  [0x%02X]\n",ppu->PPUSTATUS);
	printf("***********************************************\n");
}
