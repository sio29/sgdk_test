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
static void joyEvent_HwHRaster(u16 joy, u16 changed, u16 state);
static void mainLoop_HwHRaster(void);
static void endLoop_HwHRaster(void);
static void BGRotate2(void);
//
static s16 g_line_sc1[NUM_LINES];					// Needed for VDP_setHorizontalScrollLine
static s16 g_tile_sc1[NUM_COLUMNS];					// Needed for VDP_setHorizontalScrollLine
static int g_bgrot=4;
#define RSC_TBL_NUM		6
//static int g_rsc_tbl[]={1,2,4,8,16,32};
static int g_rsc_mode=0;
//
void initHwHRaster(void){
	initTestBG();
	
	for(int i=0;i<NUM_LINES;i++){
		g_line_sc1[i]=-i;
	}
	for(int i=0;i<NUM_COLUMNS;i++){
		g_tile_sc1[i]=-i*2; 
	}
	VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_COLUMN);
	VDP_setHorizontalScrollLine(BG_A, 0, g_line_sc1, NUM_LINES, 1);
	VDP_setVerticalScrollTile(BG_A, 0, g_tile_sc1, NUM_COLUMNS, 1);
	//
	JOY_setEventHandler(joyEvent_HwHRaster);
	g_mainloop_callback=mainLoop_HwHRaster;
}
static void endLoop_HwHRaster(void){
	returnMenu();
}
static void mainLoop_HwHRaster(void){
	BGRotate2();
}
static void joyEvent_HwHRaster(u16 joy, u16 changed, u16 state){
	if (changed & state & (BUTTON_B)){
		g_rsc_mode--;
		if(g_rsc_mode<0)g_rsc_mode=(RSC_TBL_NUM-1);
	}
	if (changed & state & (BUTTON_C)){
		g_rsc_mode++;
		if(g_rsc_mode>(RSC_TBL_NUM-1))g_rsc_mode=0;
	}
	if (changed & state & (BUTTON_START)){
		endLoop_HwHRaster();
	}
}
static void BGRotate2(void){
//	int rsc=32;
	int rsc=16;
//	int rsc=8;
//	int rsc=g_rsc_tbl[g_rsc_mode];

	int rr=(g_bgrot-512);
	if(rr<0)rr=-rr;
	rr=(rr-256) << 1;
	int rot=(rr/rsc) & 1023;
	int ss= sinFix16(rot);
	int cs= ss;//cosFix16(rot);
	int h=NUM_LINES/2;
	int offx=(320-256)/2;
//	int offy=(NUM_LINES-256)/2;
	for(int i=0;i<NUM_LINES;i++){
		int x=((ss*(i-h)) >> FIX16_FRAC_BITS);
		g_line_sc1[i]=x+offx;
	}
	for(int i=0;i<NUM_COLUMNS;i++){
		//g_tile_sc[i]=-i+offy;
		int y=((cs*(i*16-h)) >> FIX16_FRAC_BITS);
		g_tile_sc1[i]=y;
	}
	VDP_setHorizontalScrollLine(BG_A, 0, g_line_sc1, NUM_LINES, 1);
	VDP_setVerticalScrollTile(BG_A, 0, g_tile_sc1, NUM_COLUMNS, 1);
	g_bgrot+=1*rsc/4;
	g_bgrot&=1023;
}

