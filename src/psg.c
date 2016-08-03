#include "psg.h"
#include "hal.h"

// pw: previous write
static uint8_t pw;
static int p = 10;

inline uint8_t psg_ior(uint16_t addr){
	// 0x4016 controller 1 reg
	// TODO avoid magic numbers
	if(addr == 0x4016){
		if(p++ < 9){
			return hal_key_state(p);
		}
	}
	return 0; // why not void
}

inline void psg_iow(uint16_t addr, uint8_t b){
	if(addr == 0x4016){
		// strobe high to low
		if((b&1)==0 && pw==1){
			p = 0;
		}
	}
	pw = b&1;
}
