#ifndef _PPU_H_
#define _PPU_H_
#include <stdbool.h>
#include <stdint.h>
typedef struct {
	uint8_t PPUCTRL;
	uint8_t PPUMASK;
	uint8_t PPUSTATUS;
	uint8_t OAMADDR;
	uint8_t OAMDATA;
	//for scroll reg should I use union?
	/*
	union{
		uint16_t PPUSCROLL;
		struct{
			uint8_t PPUSCROLL_X;
			uint8_t PPUSCROLL_Y;
		};
	};
	*/
	uint16_t PPUSCROLL;
	uint16_t PPUADDR;
	uint8_t PPUDATA;
	// pesudo reg
	uint8_t PPUSCROLL_X;
	uint8_t PPUSCROLL_Y;
	bool rdy;
	bool scroll_received_x;
	bool addr_received_high_byte;
	int mirroring_xor;
	int scanline;
}ppu_t;
ppu_t ppu;

static const uint16_t ppu_base_nametable_addrs[4] = {0x2000,0x2400,0x2800,0x2c00};
//16KB of ppu memory.. should I make it a pointer and use malloc
uint8_t ppu_ram[0x4000];
//256 bytes for sprite memory
uint8_t ppu_oam[0x100];
// cached values for the low and high pattern this is to speed up it's just a look up..
// span all the possibilities from 0 - 255
uint8_t ppu_l_h_cache[256][256][8];
uint8_t ppu_l_h_flip_cache[256][256][8];
void ppu_init(void);
//ior=io read,iow=io write
uint8_t ppu_ior(uint16_t addr);
void ppu_iow(uint16_t addr, uint8_t data);
void ppu_set_mirroring(uint8_t mirroring);
bool ppu_is_nmi_enabled(void);
void ppu_cpy(uint16_t dst, uint8_t* src, int len);
void ppu_run(int cycles);
void ppu_cycle();
uint8_t ppu_rb(uint16_t addr);
void ppu_wb(uint16_t addr,uint8_t b);
bool ppu_is_show_bg();
bool ppu_is_show_sprites();
void ppu_oam_wb(uint8_t data);
#endif
