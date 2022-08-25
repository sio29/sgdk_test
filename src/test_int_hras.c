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

extern void initTestBG(void);

//====================
static void joyEvent_IntHRaster(u16 joy, u16 changed, u16 state);
static void mainLoop_IntHRaster(void);
static void endLoop_IntHRaster(void);
static void BGRotate(void);
static void VBlankFunction_IntHRaster(void);
static void HBlankFunction_IntHRaster(void);
//
static s16 g_line_sc2[NUM_LINES];					// Needed for VDP_setHorizontalScrollLine
static s16 g_tile_sc2[NUM_COLUMNS];					// Needed for VDP_setHorizontalScrollLine
static s16 g_line_y[NUM_LINES];					// Needed for VDP_setHorizontalScrollLine
static int g_h_cnt=0;
static int g_bgrot2=0;
static int g_hcnt_num=0;
//
void initIntHRaster(void){
	initTestBG();
	
	for(int i=0;i<NUM_LINES;i++){
		g_line_sc2[i]=0;
		g_line_y [i]=-i;
	}
	for(int i=0;i<NUM_COLUMNS;i++){
		g_tile_sc2[i]=0;
	}
	VDP_setHorizontalScrollLine(BG_A, 0, g_line_sc2, NUM_LINES, 1);
	VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
//	SYS_setVIntPreCallback(&VBlankFunction_IntHRaster);
	SYS_setVIntCallback(&VBlankFunction_IntHRaster);
//	internalHIntCB=&HBlankFunction_IntHRaster;
	SYS_setHIntCallback(&HBlankFunction_IntHRaster);
	VDP_setHIntCounter(0);
	VDP_setHInterrupt(1);
	JOY_setEventHandler(joyEvent_IntHRaster);
	g_mainloop_callback=&mainLoop_IntHRaster;
}
static void endLoop_IntHRaster(void){
	SYS_setHIntCallback(NULL);
	VDP_setHInterrupt(0);
	returnMenu();
}
static void mainLoop_IntHRaster(void){
	BGRotate();
}
static void joyEvent_IntHRaster(u16 joy, u16 changed, u16 state){
	if (changed & state & (BUTTON_B)){
		g_hcnt_num--;if(g_hcnt_num<0)g_hcnt_num=0;
		VDP_setHIntCounter(g_hcnt_num);
	}
	if (changed & state & (BUTTON_C)){
		g_hcnt_num++;if(g_hcnt_num>16)g_hcnt_num=16;
		VDP_setHIntCounter(g_hcnt_num);
	}
	if (changed & state & (BUTTON_START)){
		endLoop_IntHRaster();
	}
}
static void VBlankFunction_IntHRaster(void){
	g_h_cnt=0;
	HBlankFunction_IntHRaster();
}
static void HBlankFunction_IntHRaster(void){
//	VDP_setHorizontalScroll(BG_A, g_line_sc2[g_h_cnt]);
//	VDP_setVerticalScroll  (BG_A, g_line_y [g_h_cnt]);
//	*(((u32 *)GFX_CTRL_PORT)) = GFX_WRITE_VRAM_ADDR(VDP_HSCROLL_TABLE);
//	*(((u32 *)GFX_CTRL_PORT)) = GFX_WRITE_VSRAM_ADDR(0);
	*(((u32 *)GFX_CTRL_PORT)) = GFX_WRITE_VSRAM_ADDR(0);
	*(((u16 *)GFX_DATA_PORT)) = g_line_y[g_h_cnt];
	g_h_cnt++;if(g_h_cnt>=NUM_LINES)g_h_cnt=0;
}
static void BGRotate(void){
	int scale=sinFix16(g_bgrot2);
	int ss=0;
	if(scale!=0){
		ss=FIX16(1)*FIX16(1)/scale;
	}
	int h=NUM_LINES/2;
	int yo=0;//255;
	for(int i=0;i<NUM_LINES;i++){
		int y=yo;
		if(ss!=0){
			y=((ss*(i-h)) >> FIX16_FRAC_BITS)+h;
			if(y<0)y=yo;
			if(y>255)y=yo;
		}
		g_line_y [i]=-i+y;
	}
	g_bgrot2+=8;
	g_bgrot2&=1023;
}

