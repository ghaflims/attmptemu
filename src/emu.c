#include "emu.h"
#include "ppu.h"
#include "hal.h"
#include "mem.h"
#include "opcodes.h"
#include "helper.h"
#include "debug.h"
#include "rom.h"
#include "mmc.h"
#include <stdio.h>
#include <stdbool.h>

bool emu_running;
cpu_t cpu;

// pixel buffer to be drawn to the screen
// bg:background, bbg:behind background (for sprites that have low priority), fg:sprites
pbuf_t bg,bbg,fg;

void exec_loop(){
	mem_init();
	rom_init();
	readrom("nestest.nes");
	cpu_init(&cpu);
	cpu_reset(&cpu);
	ppu_init();
	cpu_exec(&cpu,18);
	cpu_trigger_nmi(&cpu);
	cpu_exec(&cpu,30);
		//check for interrupts
		//mem_dump(mem,8);
		//test = 0;
		//printf("test = %d",test);
}

void wait_for_frame();
void parse_events();
void emu_run(){
	while(emu_running){
		wait_for_frame();
		int scanlines = 262;
		while(scanlines-- > 0){
			ppu_run(1);
			cpu_exec(&cpu,1364/12);
		}
		parse_events();
		
	}	
}

void emu_init(char* args){
	hal_init();
//	mem_init();
// init the ppu before rom otherwise set_mirroring will not work
	ppu_init();
	rom_init();
	readrom(args);
	cpu_init(&cpu);
	
	cpu_reset(&cpu);
	emu_running = true;
}

void emu_update_screen(){
	// TODO avoid magic numbers 0xf00 is the first color in the palette entry the bg color
	int idx = ppu_rb(0x3f00);
	hal_set_bg_color(idx);
	
	// flushing sequence.. sprites behind background -> background -> sprites
	if(ppu_is_show_sprites())
		hal_flush_buf(&bbg);
	if(ppu_is_show_bg())
		hal_flush_buf(&bg);
	if(ppu_is_show_sprites())
		hal_flush_buf(&fg);
	
	hal_flip_display();
	
	pixelbuf_clear(bbg);
	pixelbuf_clear(bg);
	pixelbuf_clear(fg);
//	hal_set_bg_color(int c);
//	hal_flush_buf(void* buf);
//	hal_flip_display();
}

void emu_free(){
	hal_free();
	rom_free();
//	mem_free();
}

