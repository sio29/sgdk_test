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
void initTestBG(void){
	u16 palette[64];
	u16 ind;
	
	// load background
	ind = TILE_USERINDEX;
	VDP_drawImageEx(BG_B, &bgb_image  , TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
	ind += bgb_image.tileset->numTile;
	VDP_drawImageEx(BG_A, &check_image, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
	ind += check_image.tileset->numTile;
	// prepare palettes
	memcpy(&palette[ 0], bgb_image.palette->data  , 16 * 2);
	memcpy(&palette[16], check_image.palette->data, 16 * 2);
	// fade in
//	VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);
	VDP_setPaletteColors(0,palette,4*16);
}

//====================
static void setBgMode(int n);
static void nextBgMode(void);
static void prevBgMode(void);
static void joyEvent_BGTest(u16 joy, u16 changed, u16 state);
static void mainLoop_BGTest(void);
static void endLoop_BGTest(void);
//
static s16 g_line_sc3[NUM_LINES];					// Needed for VDP_setHorizontalScrollLine
static s16 g_tile_sc3[NUM_COLUMNS];					// Needed for VDP_setHorizontalScrollLine
static int g_bg_mode=0;
//
void initBGTest(void){
	initTestBG();
	for(int i=0;i<NUM_LINES;i++){
		g_line_sc3[i]=-i;
	}
	for(int i=0;i<NUM_COLUMNS;i++){
		g_tile_sc3[i]=-i*2; 
	}
	VDP_setHorizontalScrollLine(BG_A, 0, g_line_sc3, NUM_LINES, 1);
	VDP_setVerticalScrollTile(BG_A, 0, g_tile_sc3, NUM_COLUMNS, 1);
	
	nextBgMode();
	JOY_setEventHandler(joyEvent_BGTest);
	g_mainloop_callback=&mainLoop_BGTest;
}
static void endLoop_BGTest(void){
	returnMenu();
}
static void mainLoop_BGTest(void){
}
static void joyEvent_BGTest(u16 joy, u16 changed, u16 state){
	if (changed & state & (BUTTON_B)){
		//VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
		nextBgMode();
	}
	if (changed & state & (BUTTON_C)){
		//VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_COLUMN);
		//VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_COLUMN);
		prevBgMode();
	}
	if (changed & state & (BUTTON_START)){
		endLoop_BGTest();
	}
}
#define BG_MODE_MAX	4
static void setBgMode(int n){
	int h=HSCROLL_PLANE;
	int v=VSCROLL_PLANE;
	switch(n){
		case 0:
			break;
		case 1:
			h=HSCROLL_LINE;
			v=VSCROLL_PLANE;
			break;
		case 2:
			h=HSCROLL_PLANE;
			v=VSCROLL_COLUMN;
			break;
		case 3:
			h=HSCROLL_LINE;
			v=VSCROLL_COLUMN;
			break;
	}
	VDP_setScrollingMode(h,v);
}
static void nextBgMode(void){
	setBgMode(g_bg_mode);
	g_bg_mode++;
	if(g_bg_mode>=BG_MODE_MAX)g_bg_mode=0;
}
static void prevBgMode(void){
	setBgMode(g_bg_mode);
	g_bg_mode--;
	if(g_bg_mode<0)g_bg_mode=BG_MODE_MAX-1;
}
