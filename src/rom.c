#include "rom.h"
#include "mmc.h"
#include "ppu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void rom_init(void){
	// allocate 1MB for reading the rom file.. too much ??!!
	rom = calloc(1*1024*1024,sizeof(uint8_t));
	
}
// what are you about to see is really bad.. but I promise it's temprary.. this will not help porting..
int readrom(char* file){
	// this is bad.. fix later never hard code the file name
	FILE* fp = fopen(file,"rb");
	if(fp==NULL){
		fprintf(stderr,"Couldn't open the rom file\n");
		exit(1);
	}
	// read the header..
	// TODO check if the read is successful & validiate the rom signature
	// this is really bad
	
	
	int readbytes = fread(rom,1,1*1024*1024,fp);
	
	if(readbytes == 0 && ferror(fp)){
		fprintf(stderr,"Reading the rom file failed");
		exit(1);
	}
	memcpy(&ines,rom,sizeof(ines_t));
	//fread(&ines,sizeof(ines_t),1,fp);
	mmc_id = (ines.flag6 >> 4) & 0x0f;
	//set mirroring
	//avoid magic numbers
	//TODO better documentation.. refer to nes Documentation
	// this was a bug at first I didn't set mirroring.. :(
	ppu_set_mirroring(ines.flag6&1);
	int prg_size = ines.prg_count * 0x4000; // 1prg page = 16kb
	//check mmc_id
	if(mmc_id == 0 || mmc_id ==3){
		if(ines.prg_count == 1){
			// if one prg page then repeat it..
			// 0x8000 upward is the program area
			mmc_cpy(0x8000,rom+sizeof(ines_t),0x4000);
			mmc_cpy(0xc000,rom+sizeof(ines_t),0x4000);
		}else{
			mmc_cpy(0x8000,rom+sizeof(ines_t),0x8000);
		}
	}else{
		return -1; // something wrong.. maybe wrong mapper
	}
	
	int i;
	int pos = sizeof(ines_t)+prg_size;
	for(i=0;i<ines.chr_count;i++){
		pos+=i*0x2000; // 8kb
		mmc_append_to_chr_pg(rom+pos);
		if(i==0)
			ppu_cpy(0x0000,rom+pos,0x2000);
	}
	return 0;


}
void rom_free(void){
	free(rom);
}
