#include "ppu.h"
#include "cpu.h"
#include "emu.h"
#include "helper.h"
#include "hal.h"
#include <string.h>
#include <stdio.h>

// cpu cycles
uint32_t cpu_cyc_b;
uint32_t cpu_cyc_e;
uint32_t cpu_cyc_frame;
// should these be defined as static?
// oam,oam2 address overflow and sprite overflow (of:overflow)
bool sprite_in_range, overflow_detection, oam_addr_of, oam2_addr_of, sprite_of, s0_on_next_scanline,s0_on_cur_scanline;

// should below be static?
uint8_t sprite_attribs[8];
uint8_t sprite_x[8];
uint8_t sprite_l[8];
uint8_t sprite_h[8];
uint8_t sprite_y,sprite_index;
uint64_t frame_count;
uint8_t copy_sprite_signal;

uint8_t oam2_addr;
uint8_t t_nt,t_at,t_lo,t_hi;
uint64_t tiledata;
// there was a bug because not using unsigned when calculating in_range scanline was 32 and the data was 0x77 resulting in -87 because it was signed substraction :/
unsigned int scanline;
int cycle;
bool odd_frame;
// just discover a huge bug in the ppu Jul 22 2016 from not reading the documentation correctly about ppu io reg 2007
// below is needed to fix this bug
// todo explain better
bool ppu_2007_1st_read;
// these variables will be used to implement smooth scrolling for the background
// TODO: better documentation refer to the nes documentation and the forum to explain loopy scroll 
// lt:loopyT (Temp Vram address), lv:loopyV (current Vram address), lx:loopyX (fine x scroll 3bits 0-7 pixels), lw:loopyW (1st/2nd write toggle)
// Remember 0yyy NNYY YYYX XXXX (y:fine y scroll 0-7 pixels, N:name table, Y: coarseY or Tile Y scroll, X: coarseX or Tile X scroll)
uint16_t lt,lv;
uint8_t lx;
bool lw;
                                                                                          
// used to construct the 16 bit address from 2x high low bytes
uint8_t addr_latch;
uint8_t data_latch;

// for sprite0 hit checks
uint8_t ppu_bg[264][248];
bool ppu_sprite0_hit_occured;
// TODO make below functions inline
static inline uint16_t ppu_get_bg_pattern_table_addr(){
	// refer to NES documentation.. TODO: no magic numbers and explain
	return BIT_CHECK(ppu.PPUCTRL,4) ? 0x1000 : 0x0000;
}

static inline uint16_t ppu_get_sprite_pattern_table_addr(){
	// refer to NES documentation.. TODO: no magic numbers and explain
	return BIT_CHECK(ppu.PPUCTRL,3) ? 0x1000 : 0x0000;
}
static inline uint16_t ppu_get_nametable_base_addr(){
	return ppu_base_nametable_addrs[ppu.PPUCTRL&0x3];
}
bool ppu_is_bg_showed_in_leftmost_8px(){
	return BIT_CHECK(ppu.PPUMASK,1);
}
bool ppu_is_sprite_showed_in_leftmost_8px(){
	return BIT_CHECK(ppu.PPUMASK,2);
}
bool ppu_is_nmi_enabled(void){
	return BIT_CHECK(ppu.PPUCTRL,7);
}
static inline void ppu_set_vblank(bool yesno){
	if(yesno){
		BIT_SET(ppu.PPUSTATUS,7);
	}else{
		BIT_CLEAR(ppu.PPUSTATUS,7);
	}
}
static inline bool ppu_is_vblank_enabled(){
	return BIT_CHECK(ppu.PPUCTRL,7);
}
static inline void ppu_set_sprite0_hit(bool yesno){
	if(yesno){
		BIT_SET(ppu.PPUSTATUS,6);
	}else{
		BIT_CLEAR(ppu.PPUSTATUS,6);
	}	
}
inline bool ppu_is_show_bg(){
	return BIT_CHECK(ppu.PPUMASK,3);
}
inline bool ppu_is_show_sprites(){
	return BIT_CHECK(ppu.PPUMASK,4);
}
static inline uint8_t ppu_get_sprite_height(){
	return BIT_CHECK(ppu.PPUCTRL,5) ? 16:8;
}
static inline void ppu_set_sprite_overflow(bool yesno){
	if(yesno){
		BIT_SET(ppu.PPUSTATUS,5);
	}else{
		BIT_CLEAR(ppu.PPUSTATUS,5);
	}	
}
static inline bool is_rendering_enabled(){
	return ppu_is_show_bg() || ppu_is_show_sprites();
}
void ppu_init(){
	scanline = 240;
	frame_count = 0;
	odd_frame = false;
	cycle = 340;
	copy_sprite_signal = 0;
	memset(&ppu,0,sizeof(ppu));
	// why..? avoid magic values..
	ppu.PPUSTATUS = 0xa0;
	//temp
	ppu.PPUCTRL |= 0x80;
	ppu.scanline = -1;
	ppu_2007_1st_read = true;
	data_latch = 0;
// 	printf("%d",ppu.scanline);
	int l,h,x;
	for(h=0;h<256;h++){
		for(l=0;l<256;l++){
			for(x=0;x<8;x++){
				ppu_l_h_cache[l][h][x] = (((h>>(7-x))&1)<<1) | ((l>>(7-x))&1);
				ppu_l_h_flip_cache[l][h][x] = (((h >> x) & 1) << 1) | ((l >> x) & 1);
			}
		}
	}
	
}
// set H/V mirroring depending on game rom.. called from the main init function
void ppu_set_mirroring(uint8_t mirroring){
	ppu.mirroring_xor = 0x400 << mirroring;
}
// this function will take care of ppu memory mirroring adresses
static inline uint16_t ppu_get_real_address(uint16_t addr){
	// hmm can it be better optimized to do one check rather than two: <0x2000 <0x3000 etc
	// FIXME it can be better optimized
	// FIXME avoid magic numbers
	
	if(addr >= 0x3000 && addr < 0x3f00){
		return addr - 0x1000;
	}else if(addr >= 0x3f00 && addr < 0x4000){
		addr = 0x3f00 | (addr & 0x1f);
		if(addr==0x3f10 || addr==0x3f14 || addr==0x3f18 || addr==0x3f1c)
			return addr - 0x10;
		return addr;
	}else{
		return addr;
	}
}
inline uint8_t ppu_rb(uint16_t addr){
	return ppu_ram[ppu_get_real_address(addr)];
}
void ppu_wb(uint16_t addr, uint8_t data){
	ppu_ram[ppu_get_real_address(addr)] = data;
}

uint8_t ppu_ior(uint16_t addr){
	switch(addr & 7){
		// use curly braces to scop variable decleration.. becasue case work with statment not variable decleration
		case 2:{
			//save the current value of the reg before modifying it
			uint8_t value = ppu.PPUSTATUS;
			BIT_CLEAR(ppu.PPUSTATUS,7);
			
			// are we sure that bit 6 is cleared?
			//BIT_CLEAR(ppu.PPUSTATUS,6);
			ppu.scroll_received_x = false;
			ppu.PPUSCROLL = 0;
			// the line below was the bug :( one of them at least
			// thank you gdb
			lt = 0;
			ppu.addr_received_high_byte = false;
			addr_latch = 0;
			ppu_2007_1st_read = true;
			//related to loopy scrolling
			lw = false;
			return value;
			break; // old habit die hard..
		}
		case 4:
			return ppu_oam[ppu.OAMADDR];
			break;
		case 7:{
			uint8_t value; 
			
			if(lv >= 0x3f00)
				data_latch = ppu_rb(0x3f00 | (lv & 0x1f));
			value = data_latch;
			data_latch = ppu_rb(lv & 0x3fff);
			lv += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			return value;
			break;
		}
		default:
			return 0xff;
	}
}
// TODO: better documentation for loopy scrolling and avoid Magic numbers but hey it's just a basic masking :/
void ppu_iow(uint16_t addr, uint8_t data){
	addr &= 7;
	switch(addr){
		case 0:
			ppu.PPUCTRL = data;
			// scroll related register
			lt &= ~0x0c00; // masking NN bits
			lt |= (data&0x3) << 10;
			break;
		case 1:
			ppu.PPUMASK = data;
			break;
		case 3:
			ppu.OAMADDR = data;
			break;
		case 4:
			ppu_oam[ppu.OAMADDR++] = data;
			break;
		case 5:{
			if(ppu.scroll_received_x)
				ppu.PPUSCROLL_Y = data;
			else{
				ppu.PPUSCROLL_X = data;
			//	printf("SCROLL_X: %02x\n",ppu.PPUSCROLL_X);
			}
				
			ppu.scroll_received_x ^= 1;
			if(!lw){ // lw is 0 false
				lt &= 0xffe0;
				lt |= (data&0xf8) >> 3;
				ppu.PPUADDR &= 0xffe0;
				ppu.PPUADDR |= (data&0xf8) >> 3;
				lx = data&0x7;
			}else{
				lt &=0x8c1f;
				lt |= (data&0xf8) << 2;
				lt |= (data&0x7) << 12;
				ppu.PPUADDR &= 0x8c1f;
				ppu.PPUADDR |= (data&0xf8) << 2;
				ppu.PPUADDR |= (data&0x7) << 12;
			}
			lw ^= 1;
			break;
		}
		case 6:{
			if(ppu.addr_received_high_byte)
				ppu.PPUADDR = (addr_latch << 8) | data;
			else
				addr_latch = data;
			ppu.addr_received_high_byte ^= 1;
			
			if(!lw){ // lw is 0 false
				lt &= 0x00ff;
				lt |= (data&0x3f) << 8;
			}else{
				lt &= 0xff00;
				lt |= data;
				lv = lt;
			}
			lw^=1;
			ppu_2007_1st_read = true;
			break;
		}
		case 7:{
			
				//lv &= 0x3FFF;
				// take care of mirroring V/H
				if((lv & 0x3fff) >= 0x2000 && (lv & 0x3fff) < 0x4000 ){
					ppu_wb((lv & 0x3fff) ^ ppu.mirroring_xor, data);
					ppu_wb((lv & 0x3fff), data);
				}else{
					ppu_wb((lv & 0x3fff), data);
				}
				// this was a bug I didn't increament when writing only when reading..
				// TODO avoid magic numbers and put it in function
				// refer to nes documentation
			
			lv += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			break;
			
		}
	}
}

void inline ppu_cpy(uint16_t dst, uint8_t* src, int len){
	memcpy(&ppu_ram[dst],src,len);
}


#define PRE_LINE        scanline==261
#define VISIBLE_LINE    scanline<240
#define RENDER_LINE     (scanline==261 || scanline<240)
#define PRE_FETCHCYCLE  (cycle>=321 && cycle<=336)
#define VISIBLE_CYCLE   (cycle>=1 && cycle<=256)
#define FETCH_CYCLE     (PRE_FETCHCYCLE||VISIBLE_CYCLE)

#define VBLANK_START    scanline==241 && cycle==1
#define VBLANK_END scanline==261 && cycle==1

// FIXME bad practice cpu instance should be inside the cpu and encabsulated 
extern cpu_t cpu;

static inline void render_pixel(){
	// render pixel
	int x,y;
	x = cycle - 1;
	y = scanline;
	uint8_t pdata;
	if(ppu_is_show_bg()){
		pdata = ((tiledata>>32)>>((7-lx)*4))&0xf;
		uint8_t ci = ppu_rb(0x3f00 + pdata);
		putp(bg,x,y,ci);
	}
	if(ppu_is_show_sprites()){
		int i;
		for(i=0;i<8;i++){
			uint8_t offset = x - sprite_x[i];
			if(offset < 8){
				uint8_t l = sprite_l[i];
				uint8_t h = sprite_h[i];
				uint8_t sprite_pat = (sprite_attribs[i] & 0x40) ? ppu_l_h_flip_cache[l][h][offset]:ppu_l_h_cache[l][h][offset];
				if(sprite_pat){
					if(sprite_attribs[i] & 0x20){ // behind bg
						putp(bbg,x,scanline,ppu_rb(0x3f10 + ((sprite_attribs[i]&3)<<2) + sprite_pat));
					}else{ // in front bg
						putp(fg,x,scanline,ppu_rb(0x3f10 + ((sprite_attribs[i]&3)<<2) + sprite_pat));
					}
					if(!ppu_sprite0_hit_occured && i==0 && s0_on_cur_scanline && sprite_x[i]!=255 && (pdata&3) && ppu_is_show_bg() && (sprite_x[i] >= (ppu_is_sprite_showed_in_leftmost_8px() ? 0 : 8)) && (x >= (ppu_is_bg_showed_in_leftmost_8px() ? 0 : 8))){
						ppu_set_sprite0_hit(true);
						cpu_cyc_b = cpu.cyc;
						cpu_cyc_e = 0;
						ppu_sprite0_hit_occured = true;
						printf("Sprit Hit0 occured on: SL:%d,PPU CYC:%d\n",scanline,cycle);
					}
				}
			}
		}
	}
}
static inline void fetch_pixel(){
	int cx,cy,aa,ta,i;
	uint32_t t_data = 0;
	uint8_t shift;
	// fetch
			tiledata <<= 4;
			switch(cycle%8){
				case 1: // fetch nametable index
					t_nt = ppu_rb(0x2000 | (lv & 0x0fff));
					break;
				case 3: // fetch attribute byte
					cx = (lv & 0x1f);
					cy = (lv & 0x3e) >> 5;
					aa = 0x23c0 | (lv & 0x0c00) | ((cy & 0xfffc) << 1) | (cx >> 2);
					shift = (cx & 0x2) | ((cy&0x2)<<1);
					t_at = ((ppu_rb(aa)>>shift)&0x3)<<2;
					break;
				case 5: // fetch tile low byte
					ta = ppu_get_bg_pattern_table_addr() + (16*t_nt) + ((lv&0x7000)>>12);
					t_lo = ppu_rb(ta);
					break;
				case 7:	// fetch tile high byte
					ta = ppu_get_bg_pattern_table_addr() + (16*t_nt) + ((lv&0x7000)>>12);
					t_hi = ppu_rb(ta+8);
					break;
				case 0: // looks ugly putting zero at the end.. this is struct the full 8x8 tile
					for(i=0;i<8;++i){
						t_data <<= 4;
						t_data |= t_at | ppu_l_h_cache[t_lo][t_hi][i];
					}
					tiledata |= t_data;
					break;
			}
}

static inline void increment_x(){
	if((lv&0x1f) == 31){
		lv &= 0xffe0;
		lv ^= 0x400;
	}else
		lv++;
}

static inline void increment_y(){
	if((lv&0x7000) != 0x7000)
		lv += 0x1000;
	else{
		uint16_t y;
		lv &= 0x8fff;
		y = (lv&0x03e0)>>5;
	if(y==29){
		y = 0;
		lv ^= 0x800;
	}else
	if(y==31)
		y = 0;
	else
		y++;

	lv &= 0xfc1f;
	lv |= (y<<5);
	}
}

static inline void copy_hor_v(){
	lv &= 0xfbe0;
	lv |= (lt&0x41f);
}

static inline void copy_ver_v(){
	lv &= 0x841f;
	lv |= (lt&0x7be0);
}

static inline void increment_cycle(){
	if(++cycle == 341){
		cycle = 0;
		if(++scanline == 262){
			scanline = 0;
			odd_frame ^= 1;
			frame_count++;
		}
	}
}

uint16_t ppu_addr_bus;

static void move_to_next_oam_byte(){
	ppu.OAMADDR = (ppu.OAMADDR + 1) & 0xff; // is the masking needed it's just 8 bit?
	oam2_addr = (oam2_addr + 1) & 0x1f;
	
	if(ppu.OAMADDR == 0)
		oam_addr_of = true;
		
	if(oam2_addr == 0){
		oam2_addr_of = true;
		overflow_detection = true;
	}
}
static void sprite_eval(){ // do sprite evaluation
	if(cycle == 65){
		// clear overflow flags and oam2 address variables
		oam_addr_of = oam2_addr_of = sprite_of = false;
		oam2_addr = 0;
	}
	
	if(cycle & 1){
		// read oam data
		ppu.OAMDATA = ppu_oam[ppu.OAMADDR];
		return;
	}
	
	// here we need to save the orignal oam data
	// the question is why..?
	
	uint8_t orig_oam_data = ppu.OAMDATA;
	
	// on even cycles data is written to oam2
	if(!(oam_addr_of || oam2_addr_of)){
		ppu_oam2[oam2_addr] = ppu.OAMDATA;
	}else{
		ppu.OAMDATA = ppu_oam2[oam2_addr];
	}
	
	if(copy_sprite_signal > 0){
		--copy_sprite_signal;
		//move to next oam byte;
		move_to_next_oam_byte();
		return;
	}
	bool in_range = (scanline - orig_oam_data) < ppu_get_sprite_height();
	// evaluate sprite hit0 at cycle 66
	if(cycle == 66)
		s0_on_next_scanline = in_range;
	
	if(in_range && !(oam_addr_of || oam2_addr_of)){
		// we are in range copy the rest 3 bytes (index,attribute,x-pos)
		copy_sprite_signal = 3;
		move_to_next_oam_byte();
		return; // I hate this.. todo replace with else one return should be in the function
	}
	
	if(!overflow_detection){
		ppu.OAMADDR = (ppu.OAMADDR + 4) & 0xfc;
		if(ppu.OAMADDR == 0)
			oam_addr_of = true;
	}else{
		if(in_range && !oam_addr_of){
			sprite_of = true;
			overflow_detection = false;
		}else{
			ppu.OAMADDR = ((ppu.OAMADDR + 4) & 0xfc) | ((ppu.OAMADDR + 1) & 3);
			if((ppu.OAMADDR & 0xfc) == 0)
				oam_addr_of = true;
		}
	}
		
	
}

// return true if sprite is in range
static bool calc_sprite_tile_addr(uint8_t y, uint8_t index, uint8_t attrib, bool is_high){
	uint8_t diff = scanline - y;
	// check if there is a vertical flip and ajust address accordingly
	uint8_t diff_y_flip = (attrib & 0x80) ? ~diff : diff;
	if(diff<16){
		//printf("y_sprite:%d,diff:%d,y:%d,CYC:%d,F#:%lld\n",y,diff,scanline,cycle,frame_count);
	}
	if(ppu_get_sprite_height() == 8){
		ppu_addr_bus = ppu_get_sprite_pattern_table_addr() + 16*index + 8*is_high + (diff_y_flip & 7);
		return diff < 8;
	}else{
		ppu_addr_bus = 0x1000*(index&1) + 16*(index & 0xfe) + ((diff_y_flip & 8) << 1) + 8*is_high + (diff_y_flip & 7);
		return diff < 16;
	}
}

static void sprite_fetch(){
	//sprite number
	uint8_t sprite_n = (cycle-257)/8;
	
	if(cycle == 257)
		oam2_addr = 0;
	
	s0_on_cur_scanline = s0_on_next_scanline;
	
	switch((cycle - 1) % 8){
		case 0:
			sprite_y = ppu_oam2[oam2_addr];
			oam2_addr = (oam2_addr + 1) & 0x1f;
			break;
		case 1:
			sprite_index = ppu_oam2[oam2_addr];
			oam2_addr = (oam2_addr + 1) & 0x1f;
			break;
		case 2:
			sprite_attribs[sprite_n] = ppu_oam2[oam2_addr];
			oam2_addr = (oam2_addr + 1) & 0x1f;
			break;
		case 3:
			sprite_x[sprite_n] = ppu_oam2[oam2_addr];
			oam2_addr = (oam2_addr + 1) & 0x1f;
			break;
		case 4:
			sprite_in_range = calc_sprite_tile_addr(sprite_y, sprite_index,sprite_attribs[sprite_n],false);
			break;
		case 5:
			sprite_l[sprite_n] = sprite_in_range ? ppu_rb(ppu_addr_bus) : 0;
			break;
		case 6:
			sprite_in_range = calc_sprite_tile_addr(sprite_y, sprite_index,sprite_attribs[sprite_n],true);
			break;
		case 7:
			sprite_h[sprite_n] = sprite_in_range ? ppu_rb(ppu_addr_bus) : 0;
			break;	
	}
}

void ppu_tick(void){
    if(is_rendering_enabled()){
        if(odd_frame==1 && PRE_LINE && cycle==339)
        {
            cycle = 0;
            scanline = 0;
            odd_frame ^= 1;
            frame_count++;
            return;
        }
	}
	
	if(is_rendering_enabled()){
		if(VISIBLE_LINE && VISIBLE_CYCLE){
			render_pixel();
			
			if(cycle >= 1 && cycle <= 64){ // this is really bad optimize
				// clear oam2
				if(cycle & 1){ // on Odd cycles
					ppu.OAMDATA = 0xff;
				}else{
					ppu_oam2[oam2_addr] = ppu.OAMDATA;
					oam2_addr = (oam2_addr + 1) & 0x1f; // this is to cap increment to 31 max
				}
			}
			if(cycle >=65 && cycle <= 256){// this is really bad optimize
				sprite_eval();
			}
		}
		
		if(RENDER_LINE && FETCH_CYCLE){
			fetch_pixel();
		}
		
		if(PRE_LINE && cycle>=280 && cycle<=304){
            copy_ver_v();
        }

        // increment counters
        if(RENDER_LINE){
			if(cycle >= 257 && cycle <=320){
				sprite_fetch();
				ppu.OAMADDR = 0;
			}
            // increment X
            if(FETCH_CYCLE && cycle%8==0)
            {
               increment_x();
            }

            // increment Y
            if(cycle==256)
            {
				increment_y();
            }

            // copy X
            if(cycle==257)
            {
                copy_hor_v();
            }

		
		}
	
	}
	// start vblank logic
    if(VBLANK_START){
		ppu_set_vblank(true);
		cpu_trigger_nmi(&cpu);
    }

    // end vblank logic
    if(VBLANK_END){
		if(BIT_CHECK(ppu.PPUSTATUS,6)){
			cpu_cyc_e = cpu.cyc;
			printf("Im in\n");
		}
		ppu_set_sprite0_hit(false);
		ppu_sprite0_hit_occured = false;
		sprite_of = s0_on_next_scanline = false;
		ppu_set_vblank(false);
		emu_update_screen();
		cpu_cyc_frame = cpu_cyc_e - cpu_cyc_b;
		printf("frame#:%lld, CPU cyc:%d, SL:%d, PPU cyc:%d\n",frame_count,cpu_cyc_frame,scanline,cycle);
	}
	
	
	
	increment_cycle();
	
	/*
	if(scanline < 240){ // visible scanlines
		
	}else if(scanline = 240){ // post render scanline
		
	}else if(scanline >= 241 && scanline <=260){ // Vblank scanlines
		
	}else if(scanline = 260){ // pre-render scanline
		
	}
	*/
}


inline void ppu_oam_wb(uint8_t data){
	ppu_oam[ppu.OAMADDR++] = data;
}
