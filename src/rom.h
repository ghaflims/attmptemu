#ifndef _ROM_H_
#define _ROM_H_
#include <stdint.h>
uint8_t* rom;
typedef struct {
	uint8_t signature[4];
	uint8_t prg_count;
	uint8_t chr_count;
	uint8_t flag6;
	uint8_t flag7;
	uint8_t reserved[8];
} ines_t;
ines_t ines;

// refactor later 
void rom_init(void);
int readrom(char* file);
//choose either rom_init or init_rom and keep standard across the project :/
void rom_free(void);
#endif
