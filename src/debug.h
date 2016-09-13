#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"

void print_debug(cpu_t* cpu,uint8_t op);
void mem_dump(const void* mem, size_t len);
void ppu_dump(int x, int y, uint16_t nt, uint16_t lv);
#endif
