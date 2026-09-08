//this example code shows how to put some text in nametable
//it assumes that you have ASCII-encoded font in the CHR tiles $00-$3f

#include "neslib.h"
//this macro is used remove need of calculation of the nametable address in runtime


static unsigned char x,y,z;

void main(void)
{
	x=100;
	y=2;
	z=x/y;
	ppu_on_all();
	while(1);//do nothing, infinite loop
}
