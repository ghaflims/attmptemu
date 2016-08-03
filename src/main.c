//#include <stdio.h>
//#include <stdlib.h>
#include "emu.h"

int main(int argc, char *argv[]) {
	//cpu_t cpu = {.cyc=0,.pc=PRG_START,.x=0,.y=0,.a=0,.sp=0xff,.sr=0};
//	exec_loop();
	emu_init(argv[1]);
	emu_run();
	emu_free();
	return 0;
}
