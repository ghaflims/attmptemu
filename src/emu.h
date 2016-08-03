#ifndef _EMU_H_
#define _EMU_H_
#include "cpu.h"
#include <stdint.h>

void exec_loop();

void emu_init(char* args);
void emu_update_screen();
void emu_run();
void emu_free();
#endif
