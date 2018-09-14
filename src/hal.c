#include "hal.h"
#include "ppu.h"
#include "debug.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
extern cpu_t cpu;
extern ppu_t ppu;
// bytes per pixel
#define BPP 4
// this will hold the surface that contains the pixel.. (CPU)
// SDL_Surface* surf

// this will hold the texture that will represent the surface in (GPU)
SDL_Texture* texture;

// this will hold the render window that will contain the textures and will be passed to the window..
SDL_Renderer* rend;

// this will hold the window 
SDL_Window* win;

SDL_Event evt;
// this is library depending color mapping
// is there a point of this?
//Uint32 color_map[64];
extern bool emu_running;
void parse_events(){
	SDL_PollEvent(&evt);
	if(evt.type == SDL_QUIT)
		emu_running = false;
	if(evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_d){
		print_debug(&cpu,cpu.op);
		debug_ppu(&ppu);
	
	}
}

void wait_for_frame(){
	//SDL_Delay(16);
	SDL_Delay(16);
}
void hal_set_bg_color(int c){
	SDL_SetRenderDrawColor(rend,palette[c].r,palette[c].g,palette[c].b,0xff);
	// clear the framebuffer to the color specified
	// temp color
	uint32_t tc = cmap[c];
	int i;
	for(i=0;i<SCREEN_H * SCREEN_W;i++)
		fb[i] = tc;
}
// this should take a pixel buffer array..
// and flush it to the fb
void hal_flush_buf(pbuf_t* pb){
	int i;
	for(i=0;i<pb->pos;i++){
		fb[(pb->buf[i].y * SCREEN_W) + pb->buf[i].x] = cmap[pb->buf[i].c];
	}
}
void hal_flip_display(){
	// update the GPU Texture with the framebuffer
	SDL_UpdateTexture(texture,NULL,(void*)fb,SCREEN_W*BPP);
	// clear the render..
	SDL_RenderClear(rend);
	// copy texture to the render..
	SDL_RenderCopy(rend,texture,NULL,NULL);
	// flip the display..
	SDL_RenderPresent(rend);
}
void hal_init(){
	// TODO check for nulls/erros..
	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("NES Emulator",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	rend = SDL_CreateRenderer(win,-1,0);
	texture = SDL_CreateTexture(rend,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,SCREEN_W,SCREEN_H);
	int i;
	for(i=0;i<64;i++){
		// SDL specific RRGGBBAA
		// can this be optimized save pallete[i] to temp variable and access temp ?
		cmap[i] = (palette[i].r << 24) | (palette[i].g << 16) | (palette[i].b << 8) | 0xff;
	}
}
void hal_free(){
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);	
	SDL_Quit();
}
int  hal_key_state(int b){
	const uint8_t* state = SDL_GetKeyboardState(NULL);
	switch(b){
		case 0: //on,off
			return 1;
			break;
		case 1: // A
			return state[SDL_SCANCODE_A];
			break;
		case 2: // B
			return state[SDL_SCANCODE_S];
			break;
		case 3: // select
			return state[SDL_SCANCODE_SPACE];
			break;
		case 4: // start
			return state[SDL_SCANCODE_RETURN];
			break;
		case 5: // up
			return state[SDL_SCANCODE_UP];
			break;
		case 6: // down
			return state[SDL_SCANCODE_DOWN];
			break;
		case 7: // left
			return state[SDL_SCANCODE_LEFT];
			break;
		case 8: // right
			return state[SDL_SCANCODE_RIGHT];
			break;
		default:
				return 1;
	}
}
