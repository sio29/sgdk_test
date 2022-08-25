#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_tile.h"
#include "sys.h"

#include "test_menu.h"

#define NUM_COLUMNS	 20
#define NUM_ROWS		28
#define NUM_LINES	   NUM_ROWS * 8

int main(){
	JOY_init();
	JOY_reset();
	//cpp_test();
	// disable interrupt when accessing VDP
	SYS_disableInts();
	// initialization
	VDP_setScreenWidth320();
	// set all palette to black
//	VDP_setPaletteColors(0, (u16*) palette_black, 64);
	// init sprite engine with default parameters
	SPR_init();
	//
	initMenu();
	//
	SYS_enableInts();
	//
	while(1){
		if(g_mainloop_callback)(*g_mainloop_callback)();
		VDP_waitVSync();
		JOY_update();
	}
	return 0;
}

