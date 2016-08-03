#ifndef _PSG_H_
#define _PSG_H_

#include <stdint.h>
uint8_t psg_ior(uint16_t addr);
void psg_iow(uint16_t addr, uint8_t b);
#endif
