#include "ppu.h"
#include "cpu.h"
#include "emu.h"
#include "helper.h"
#include "hal.h"
#include <string.h>
#include <stdio.h>
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
void ppu_init(){
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
			BIT_CLEAR(ppu.PPUSTATUS,6);
			ppu.scroll_received_x = false;
			ppu.PPUSCROLL = 0;
			// the line below was the bug :( one of them at least
			// thank you gdb
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
			/*
			if(lv >= 0x3f00)
				data_latch = ppu_rb(0x3f00 | (lv & 0x1f));
			value = data_latch;
			data_latch = ppu_rb(lv & 0x3fff);
			lv += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			lv &= 0x7fff;
			*/
			
			if(ppu.PPUADDR >= 0x3f00)
				data_latch = ppu_rb(ppu.PPUADDR);
			value = data_latch;
			data_latch = ppu_rb(ppu.PPUADDR);
			// should this be more clear.. refer to ppu docs
			// the if condition is to fix the bug
			//if(ppu_2007_1st_read){
				ppu_2007_1st_read = false;
			//}else{
				ppu.PPUADDR += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			//}
			
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
			/*
			if(ppu_is_show_bg()){
				
			}else{
				//lv &= 0x3FFF;
				// take care of mirroring V/H
				if(lv >= 0x2000 && lv < 0x4000 ){
					ppu_wb((lv & 0x3fff) ^ ppu.mirroring_xor, data);
					ppu_wb((lv & 0x3fff), data);
				}else{
					ppu_wb((lv & 0x3fff), data);
				}
				// this was a bug I didn't increament when writing only when reading..
				// TODO avoid magic numbers and put it in function
				// refer to nes documentation
			}
			lv += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			lv &= 0x7FFF;
			break;
			*/
			
			ppu.PPUADDR &= 0x3FFF;
			// take care of mirroring V/H
			if(ppu.PPUADDR >= 0x2000 && ppu.PPUADDR < 0x4000 ){
				ppu_wb(ppu.PPUADDR ^ ppu.mirroring_xor, data);
				ppu_wb(ppu.PPUADDR, data);
			}else{
				ppu_wb(ppu.PPUADDR, data);
			}
			// this was a bug I didn't increament when writing only when reading..
			// TODO avoid magic numbers and put it in function
			// refer to nes documentation
			ppu.PPUADDR += BIT_CHECK(ppu.PPUCTRL,2) ? 32:1;
			//ppu.PPUADDR &= 0x3FFF;
			break;
			
		}
	}
}

void inline ppu_cpy(uint16_t dst, uint8_t* src, int len){
	memcpy(&ppu_ram[dst],src,len);
}

// Todo this really bad.. avoid magic number.. avoid magic number..
// This will implement loopy fine scrolling refer to NES wiki documentation
// TODO better documentation..
void ppu_draw_bg_scanline(){
	//cx:coarseX , cy: coarsey , na: name address , aa: attribute address
	int cx,cy,na,aa;
	lv &= 0xfbe0;
	lv |= (lt & ~0xfbe0);
	cx = (lv & 0x1f);
	cy = (lv & 0x3e) >> 5;
	na = 0x2000 | (lv & 0x0fff);
	aa = 0x2000 + (lv & 0x0c00) + 0x03c0 + ((cy & 0xfffc) << 1) + (cx >> 2);
	bool top = (cy & 0x0002) == 0; // is it top?
	bool left = (cx & 0x0002) == 0; // is it left ?
	
	// read pa: palette attribute
	// pa:{tl,tr,bl,br} 
	uint8_t pa = ppu_rb(aa);
	// here we extract the 2bits based on the area
	if(top)
		pa>>=4;
	if(left)
		pa>>=2;
	pa&=3; // ensure only 2 bits value is there..
	
	// tc: tile count
	int tc;
	for(tc=0; tc<33; tc++){
		// TODO better explaination refer to NES documentation
		int ta = ppu_get_bg_pattern_table_addr() + (16*ppu_rb(na)) + ((lv&0x7000)>>12);
		uint8_t l = ppu_rb(ta);
		uint8_t h = ppu_rb(ta+8);
		//int i;
		// xit:x in tile;
		int xit;
		if(tc==0 && lx!=0){
			for(xit=0;xit<8-lx;xit++){
				uint8_t c = ppu_l_h_cache[l][h][xit+lx];
				ppu_bg[(tc<<3)+xit][ppu.scanline] = c;
				if(c != 0){
					uint16_t paddr = 0x3f00 + (pa<<2);
					// ci: color index
					int ci = ppu_rb(paddr + c);
					if(ppu.scanline > 7)
						putp(bg,(tc<<3) + xit,ppu.scanline-8,ci);
					else
						putp(bg,(tc<<3) + xit,ppu.scanline,ci);
				}
			}	
		}else if(tc==32 && lx!=0){
			for(xit=0;xit<lx;xit++){
				uint8_t c = ppu_l_h_cache[l][h][xit];
				ppu_bg[(tc<<3) + xit- lx][ppu.scanline] = c;
				if(c != 0){
					uint16_t paddr = 0x3f00 + (pa<<2);
					// ci: color index
					int ci = ppu_rb(paddr + c);
					if(ppu.scanline > 7)
						putp(bg,(tc<<3) + xit - lx,ppu.scanline-8,ci);
					else
						putp(bg,(tc<<3) + xit - lx,ppu.scanline,ci);
				}
			}	
		}else{
			for(xit=0;xit<8;xit++){
				uint8_t c = ppu_l_h_cache[l][h][xit];
				ppu_bg[(tc<<3)+xit -lx][ppu.scanline] = c;
				if(c != 0){
					uint16_t paddr = 0x3f00 + (pa<<2);
					// ci: color index
					int ci = ppu_rb(paddr + c);
					if(ppu.scanline > 7)
						putp(bg,(tc<<3) + xit - lx,ppu.scanline-8,ci);
					else
						putp(bg,(tc<<3) + xit - lx,ppu.scanline,ci);
				}
			}	
		}
		na++;
		cx++;
		if((cx & 0x0001) == 0){
			if((cx & 0x0003) == 0){
				if((cx&0x1f) == 0){
					na ^= 0x0400; 
					aa ^= 0x0400;
					na -= 0x0020;
					aa -= 0x0008;
					cx -= 0x0020;
				}
				aa++;
			}
			
			uint8_t pa = ppu_rb(aa);
			if(top)
				pa>>=4;
			if(left)
				pa>>=2;
			pa&=3; // ensure only 2 bits value is there..
		}

	}
	
	if((lv & 0x7000) == 0x7000)	{
		lv &= 0x8fff;
		
		if((lv & 0x03e0) == 0x03a0){
			lv ^= 0x0800;
			lv &= 0xfc1f;
		}else{
			if((lv & 0x3e0) == 0x03e0){
				lv &= 0xfc1f;
			}else{
				lv += 0x0020; 
			}
		}
	}else{
		lv +=0x1000;
	}
}
/*
// draw the background,, parameter (mirror yesno)
void ppu_draw_bg_scanline(bool m){
	//tile x
	int tx;
	
	// there is 32 tiles in the the x axis screenW/8 the tile size is 8 pixel
	// potential optimization.. just invert the boolean.. 
	for(tx=ppu_is_bg_showed_in_leftmost_8px() ? 0 : 1; tx<32; tx++){
		// skip off screen pixels
		if(((tx<<3) - ppu.PPUSCROLL_X + (m ? SCREEN_W:0)) > SCREEN_W)
			continue;
		
		// divide scanline by 8 to get the y tile
		//tile y
		int ty = ppu.scanline >> 3;
		// tile index = ti,, tile address = ta
		int ti = ppu_rb(ppu_get_nametable_base_addr() + tx + (ty << 5) + (m ? 0x400 : 0));
		// each pattern is 16 bytes hence 16*ti // TODO better explaination refer to NES documentation
		int ta = ppu_get_bg_pattern_table_addr() + (16*ti);
		
		// y inside tile range : 0 - 7
		// from the scanline.. we are intrested in the lower 3 bits
		int yit = ppu.scanline & 0x7;
		// low and high bytes that construct the tile pattern
		uint8_t l = ppu_rb(ta+yit);
		uint8_t h = ppu_rb(ta+yit+8);
		
		// x inside tile range : 0 -7
		int xit;
		for(xit=0;xit<8;xit++){
			// instead of looping each time to construct the pattern from l & h use lookup cache table
			// c for color range from 0 - 3
			// c is not the actual color one more step is needed which is getting the attribute and look at palette table
			uint8_t c = ppu_l_h_cache[l][h][xit];
			// c = 0 is transparent
			if(c != 0){
				// aa: attribute address,, 0x3c0 is the offset for the attribute table from the nametable base address TODO more documentation
				// basiclly we add attribute table offest to the nametable base.. check for mirror to move to other nametable if needed add this to tile x / 4 + scanline / 32 * 8
				// Refer to NES documentation
				uint16_t aa = (ppu_get_nametable_base_addr() + (m ? 0x400:0) + 0x3c0 + (tx >> 2) + (ppu.scanline >>5) * 8);
				
				// since each entry in attribute table represent 32x32 pixels or (4x4) tiles each 4 tile belongs to {tl,tr,bl,br} t:top,l:left,b:bottom,r:right
				// TODO better documentation.. refer to NES documentation
				// here we check in which area are we {tl,tr,bl,br} ?
				// this is needed to extract the 2-bit position in the byte:{##,##,##,##}
				// these 2-bits along with the 2-bits (c value above) construct the color index
				
				bool top = (ppu.scanline % 32) < 16; // is it top?
				bool left = (tx % 32 < 16); // is it left ?
				
				// read pa: palette attribute
				// pa:{tl,tr,bl,br} 
				uint8_t pa = ppu_rb(aa);
				// here we extract the 2bits based on the area
				if(top)
					pa>>=4;
				if(left)
					pa>>=2;
				pa&=3; // ensure only 2 bits value is there..
				
				// paddr:palette address (base is 0x3f00) TODO put is as defind or const avoid magic numbers..
				// based on the palette attribute we offset the background palette table total of 4 bg palettes
				// so the palette attribute index which bg palette to use from the four
				// each bg pallete has 4 colors inside the 2-bit (c value obtained above will index the bg pallete)
				uint16_t paddr = 0x3f00 + (pa<<2);
				// ci: color index
				int ci = ppu_rb(paddr + c);
				// now we have the color index the x,y position..
				// we can draw to the frame buffer..
				putp(bg,(tx<<3) + xit - ppu.PPUSCROLL_X + (m?256:0),ppu.scanline+1,ci);
				
				// store for sprite0 hit
				ppu_bg[(tx<<3)+xit][ppu.scanline] = c;
			}
		}
	}
}
*/
void ppu_draw_sprite_scanline(){
	// to check for sprite overflow
	int sprite_count_per_scanline = 0;
	
	int i;
	
	// loop through the oam (64 * 4 per sprites = 0x100 or 256)
	for(i=0;i<0x100;i+=4){
		// sx:sprite x location (offset 3), sy:sprite y location (offset 0)
		uint8_t sx = ppu_oam[i+3];
		uint8_t sy = ppu_oam[i];
		
		// check if the sprite on the scanline and skip if not
		if(sy > ppu.scanline || (sy + ppu_get_sprite_height()) < ppu.scanline)
			continue;
		
		sprite_count_per_scanline++;
		
		// sptires overflow.. PPU can't handle more than 8 sprites per scanline
		if(sprite_count_per_scanline > 8)
			ppu_set_sprite_overflow(true);
			
		// vf:vflip,, hf: hflip
		// TODO avoid magic numbers
		bool vf = ppu_oam[i+2] & 0x80;
		bool hf = ppu_oam[i+2] & 0x40;
		
		// offset 1 from oam is the tile index
		uint16_t ta = ppu_get_sprite_pattern_table_addr() + (16* ppu_oam[i+1]);
		
		// yit:y in tile
		int yit = ppu.scanline &0x7;
		// low and high bytes that construct the tile pattern
		uint8_t l = ppu_rb(ta + (vf ? (7-yit) : yit));
		uint8_t h = ppu_rb(ta + (vf ? (7-yit) : yit) + 8);
		
		// pa: palette attribute
		uint8_t pa = ppu_oam[i+2] & 0x3;
		// TODO avoid magic numbers 0x3f10 is the start of sprites pallette address
		uint16_t paddr = 0x3f10 + (pa << 2);
		// x: x in tile
		int xit;
		for(xit=0;xit<8;xit++){
			// c:color
			int c = hf ? ppu_l_h_flip_cache[l][h][xit] : ppu_l_h_cache[l][h][xit];
			// skip if color 0 (transparent)
			if (c != 0){
				int x = sx + xit;
				int idx = ppu_rb(paddr + c);
				
				// TODO better documentation.. refer to NES documentations
				if(ppu_oam[i+2] & 0x20){
					if(ppu.scanline>7)
						putp(bbg,x,sy+yit+1-8,idx);
					else
						putp(bbg,x,sy+yit+1,idx);
				}else{
					if(ppu.scanline>7)
						putp(fg,x,sy+yit+1-8,idx);
					else
						putp(fg,x,sy+yit+1,idx);
				}
				
				if(ppu_is_show_bg() && !ppu_sprite0_hit_occured && i==0 && ppu_bg[sx][sy+yit] != 0){
					ppu_set_sprite0_hit(true);
					ppu_sprite0_hit_occured = true;
				}
			}
		}
	}
}
void ppu_run(int cycles){
	while(cycles-- > 0){
		ppu_cycle();
	}
}

// FIXME bad practice cpu instance should be inside the cpu and encabsulated 
extern cpu_t cpu;
void ppu_cycle(){
	//printf("we are in ppu cycle.. and scanline is : %d \n",ppu.scanline);
	// should I check ppu if it's ready or not ?
	// wouldn't this give cpu cycles to the initial prg to initilize the system?
	// TODO check for above
	ppu.scanline++;
	//printf("%d\n",ppu.scanline);
	if(ppu.scanline == 0)
		lv = lt;
	if(ppu_is_show_bg()){
		//printf("im rendering bg \n");
		//ppu_draw_bg_scanline(false);
		ppu_draw_bg_scanline(); // why TODO check..
	}
	
	if(ppu_is_show_sprites())
		ppu_draw_sprite_scanline();
	// TODO avoid magic numbers and explain..
	// explain about the ppu frame life cycle.. Refer to NES documentation
	if(ppu.scanline == 241){
		// we are in a vblank
		ppu_set_vblank(true);
		ppu_set_sprite0_hit(false);
		cpu_trigger_nmi(&cpu);
		//printf("Im in vblank and cpu pc is : %02X\n",cpu.pc);
	}else if(ppu.scanline >= 262){
		ppu.scanline = -1;
		ppu_sprite0_hit_occured = false;
		ppu_set_vblank(false);
		emu_update_screen();
	}
}

inline void ppu_oam_wb(uint8_t data){
	ppu_oam[ppu.OAMADDR++] = data;
}
