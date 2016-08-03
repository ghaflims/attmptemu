#ifndef _MMC_H_
#define _MMC_H_
#include <stdint.h>
// define maximum pages
#define MMC_MAX_PGS 32


uint8_t mmc_id;

uint8_t mmc_ior(uint16_t addr);
void mmc_iow(uint16_t addr, uint8_t data);
void mmc_append_to_chr_pg(uint8_t* src);
void mmc_cpy(uint16_t addr,uint8_t* src, int len);
#endif
