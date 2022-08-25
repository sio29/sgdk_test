#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_tile.h"
#include "sys.h"

#include "test_menu.h"
#include "test_pcmz80.h"

//#define BGM_MAX	15
#define BGM_MAX	4
static const u8 *bgm_tbl[]={
//	NULL,
	bgm01_music,
	bgm02_music,
	bgm03_music,
	bgm04_music,
//	bgm05_music,
//	bgm06_music,
//	bgm07_music,
//	bgm08_music,
//	bgm09_music,
//	bgm10_music,
//	bgm11_music,
//	bgm12_music,
//	bgm13_music,
//	bgm14_music,
};


//====================
static void joyEvent_BgmTest(u16 joy, u16 changed, u16 state);
static void playBGM(int n);
//static void nextPlayBGM(void);
static void mainLoop_BgmTest(void);
static void endLoop_BgmTest(void);
static void stopBGM();
//
static int g_bgm_num=0;
static int g_bgm_play=0;
//
void initBgmTest(void){
//	initTestBG();
	VDP_setWindowVPos(0,3);
	VDP_drawTextBG(WINDOW,"XGM BGM Test", 0, 0);
	
	SYS_disableInts();
	//stopZ80();
	//XGM_init();
	Z80_init();
	Z80_clear(0, Z80_RAM_LEN, FALSE);
	Z80_loadDriver(Z80_DRIVER_XGM,0);
	Z80_requestBus(1);
	
	Z80_startReset();
	Z80_releaseBus();
	while(Z80_isBusTaken());
	Z80_endReset();
	
	
	u8 *pcmtest_adr =(u8 *)&v0002_sfx;
	u32 pcmtest_size=sizeof(v0002_sfx);
	XGM_setPCM(60,pcmtest_adr,pcmtest_size);
	
	SYS_enableInts();
	
	JOY_setEventHandler(joyEvent_BgmTest);
	g_mainloop_callback=&mainLoop_BgmTest;
}
static void endLoop_BgmTest(void){
	XGM_stopPlay();
	SYS_disableInts();
	stopZ80();
	SYS_enableInts();
	
	returnMenu();
}
static void mainLoop_BgmTest(void){

	char m[80];
	sprintf(m,"%d:xgm(%d)",g_bgm_num,SND_isPlaying_XGM());
	VDP_drawTextBG(WINDOW,m, 0, 1);
	if(g_bgm_play){
		strcpy(m,"play");
	}else{
		strcpy(m,"stop");
	}
	VDP_drawTextBG(WINDOW,m, 0, 2);
}
static void joyEvent_BgmTest(u16 joy, u16 changed, u16 state){
	if(changed & state & (BUTTON_RIGHT)){
		g_bgm_num++;
		if(g_bgm_num>(BGM_MAX-1))g_bgm_num=0;
	}
	if(changed & state & (BUTTON_LEFT)){
		g_bgm_num--;
		if(g_bgm_num<0)g_bgm_num=(BGM_MAX-1);
	}
	if(changed & state & (BUTTON_A)){
		//nextPlayBGM();
		playBGM(g_bgm_num);
	}
	if(changed & state & (BUTTON_B)){
		stopBGM();
	}
	if(changed & state & (BUTTON_C)){
		XGM_startPlayPCM(60,5,SOUND_PCM_CH4);
	}
	if (changed & state & (BUTTON_START)){
		endLoop_BgmTest();
	}
}
static void stopBGM(){
	XGM_stopPlay();
	XGM_stopPlay();
	XGM_stopPlay();
	XGM_stopPlay();
	XGM_stopPlay();
	XGM_stopPlay();
	g_bgm_play=0;
}

static void playBGM(int n){
//	SND_startPlay_XGM(sonic_music);
//		stopBGM();
//	if(n==0){
//		stopBGM();
//		return;
//	}
	const u8 *music=bgm_tbl[n];
	if(!music){
		stopBGM();
		return;
	}
	XGM_startPlay(music);
	g_bgm_play=1;
}
