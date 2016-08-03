#include "mmc.h"
#include "ppu.h"
#include <string.h>
// should this be dynamic allocation? 0x4000 = 16KB, 0x2000 = 8KB
//uint8_t prg_pgs[MMC_MAX_PGS][0x4000];
uint8_t chr_pgs[MMC_MAX_PGS][0x2000];
int mmc_chr_pg;

// this is really bad :/ bad bad :/
// FIXME don't use memory like candy
uint8_t mmc_mem[0x10000];
inline uint8_t mmc_ior(uint16_t addr){
	return mmc_mem[addr];
}
void mmc_iow(uint16_t addr, uint8_t data){
	switch(mmc_id){
		case 0x3:
			// bank switching..
			ppu_cpy(0x0000,&chr_pgs[data&3][0],0x2000);
			break;
	}
	// will this write to the ROM man..!! or is this just a writable rom??!!
	mmc_mem[addr] = data;
}

void mmc_append_to_chr_pg(uint8_t* src){
	//memcpy(&chr_pgs[mmc_chr_pg++][0],src,len);
	memcpy(&chr_pgs[mmc_chr_pg++][0],src,0x2000);
}
inline void mmc_cpy(uint16_t addr,uint8_t* src, int len){
	memcpy(&mmc_mem[addr],src,len);
}

