#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_tile.h"
#include "sys.h"

#include "test_menu.h"

extern void initIntHRaster(void);
extern void initHwHRaster(void);
extern void initDemo(void);
extern void initBGTest(void);
extern void initBgmTest(void);
extern void initPcmTest(void);
extern void initPcmZ80(void);
extern void initTestBG(void);
extern void initMDZTest(void);

//====================
#define CMD_INTHRAS		0
#define CMD_HWHRAS		1
#define CMD_DEMO		2
#define CMD_BGTEST		3
#define CMD_BGMTEST		4
#define CMD_PCMTEST		5
#define CMD_PCMZ80		6
#define CMD_MDZTEST		7
#define CMD_MAX			8

//static int g_cmd_num=0;
static InitCmdProc g_cmd_tbl[]={
//	&initHRaster,
	&initIntHRaster,	//0
	&initHwHRaster,		//1
	&initDemo,			//2
	&initBGTest,		//3
	&initBgmTest,		//4
	&initPcmTest,		//5
	&initPcmZ80,		//6
	&initMDZTest,		//7
//	NULL
};
MainLoopCallback g_mainloop_callback=NULL;

static void joyEvent_Menu(u16 joy, u16 changed, u16 state);
static void mainLoop_Menu(void);
static void clearMenuCursor(void);
static void drawMenuCursor(void);
static void endMenu(void);
//
static int g_menu_index=7;
static int g_menu_ok=0;
static int cursor_index=1;
static const char *cursor_tbl[]={">","*","<","@"};
//
void initMenu(void){
	g_menu_ok=0;
//	VDP_clearPlane(WINDOW,0);
	VDP_clearPlane(WINDOW,0);
	VDP_clearPlane(BG_A,0);
	VDP_clearPlane(BG_B,0);
	VDP_setWindowVPos(0,10);
	VDP_drawTextBG(WINDOW,"[Menu]"    , 1, 0);	//
	VDP_drawTextBG(WINDOW,"IntHRaster", 1, 1);	//0
	VDP_drawTextBG(WINDOW,"HwHRaster" , 1, 2);	//1
	VDP_drawTextBG(WINDOW,"Demo"      , 1, 3);	//2
	VDP_drawTextBG(WINDOW,"BGTest"    , 1, 4);	//3
	VDP_drawTextBG(WINDOW,"BgmTest"   , 1, 5);	//4
	VDP_drawTextBG(WINDOW,"PcmTest"   , 1, 6);	//5
	VDP_drawTextBG(WINDOW,"PcmZ80"    , 1, 7);	//6
	VDP_drawTextBG(WINDOW,"MDZTest"   , 1, 8);	//7
	drawMenuCursor();
	//
	g_mainloop_callback=mainLoop_Menu;
	JOY_setEventHandler(joyEvent_Menu);
}
static void endMenu(void){
	VDP_clearPlane(WINDOW,0);
	VDP_setWindowVPos(0,0);
}
void returnMenu(void){
	initMenu();
}
static void mainLoop_Menu(void){
	drawMenuCursor();
	if(g_menu_ok){
		endMenu();
		InitCmdProc init_proc=g_cmd_tbl[g_menu_index];
		if(init_proc)(*init_proc)();
	}
}
static void clearMenuCursor(){
	VDP_drawTextBG(WINDOW," ", 0, 1+g_menu_index);
}
static void drawMenuCursor(){
//	VDP_drawTextBG(WINDOW,">", 0, 1+g_menu_index);
	VDP_drawTextBG(WINDOW,cursor_tbl[cursor_index], 0, 1+g_menu_index);
	cursor_index++;
	cursor_index&=3;
}

static void joyEvent_Menu(u16 joy, u16 changed, u16 state){
	char m[80];
	sprintf(m,"joy(%d),cng(%04x),sts(%04x)",joy, changed, state);
	VDP_drawTextBG(WINDOW,m, 0, 0);
	if (changed & state & (BUTTON_A)){
		g_menu_ok=1;
	}
	if (changed & state & (BUTTON_UP)){
		clearMenuCursor();
		g_menu_index--;if(g_menu_index<0)g_menu_index=0;
		drawMenuCursor();
		//VDP_setHIntCounter(g_hcnt_num);
	}
	if (changed & state & (BUTTON_DOWN)){
		clearMenuCursor();
		g_menu_index++;if(g_menu_index>(CMD_MAX-1))g_menu_index=(CMD_MAX-1);
		drawMenuCursor();
		//VDP_setHIntCounter(g_hcnt_num);
	}
}
