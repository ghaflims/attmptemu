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
void ppu_tick(void);


#endif
