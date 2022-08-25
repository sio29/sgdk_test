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

//====================
static void joyEvent_PcmTest(u16 joy, u16 changed, u16 state);
static void mainLoop_PcmTest(void);
static void endLoop_PcmTest(void);
static void playPcmTest();
static void stopPcmTest();
static void HBlankFunction_PcmTest_NONE(void);
static void HBlankFunction_PcmTest_DAC(void);
static void HBlankFunction_PcmTest_PSG1(void);
static void HBlankFunction_PcmTest_PSG3(void);
void convertPCM_F_TBL();
void convertPCM_F_TBL2();
//
static int g_pcmtest_play=0;
static int g_pcmtest_now=0;
static u8 *g_pcmtest_adr=0;
static int g_pcmtest_size=0;
static int g_pcmtest_add=1;
static int g_pcmtest_outtype=0;

static char *g_pcmtest_outtype_m[]={
	"DAC ",
	"PSG3",
	"PSG1",
};

u8 PCM_F_TBL[]={
	 0, 0, 1, 2, 3, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7	,
	 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9	,
	 9,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11	,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11	,
	11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12	,
	12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13	,
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13	,
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13	,
	13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14	,
	14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14	,
	14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14	,
	14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15	,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15	,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15	,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15	,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15	
};
u16 PCM_F_TBL2[]={
	//  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	0x000,0x000,0x111,0x014,0x115,0x054,0x155,0x346,0x337,0x028,0x276,0x467,0x119,0x049,0x159,0x668,
	0x559,0x488,0x24A,0x35A,0x498,0x17A,0x199,0x12B,0x33B,0x25B,0x26B,0x46B,0x37B,0x66B,0x88A,0x79A,
	0x77B,0x24C,0x6AA,0x26C,0x07C,0x1AB,0x57C,0x38C,0x6BA,0x77C,0x39C,0x78C,0x11D,0x04D,0x15D,0x9AB,
	0x55D,0x6AC,0x47D,0x18D,0x0BC,0x58D,0x19D,0x49D,0x59D,0x9D6,0x0AD,0x8BC,0xAAC,0x1CC,0x12E,0x5CC,
	0x99D,0x26E,0x46E,0x37E,0x66E,0x28E,0x48E,0x9CC,0x29E,0x4E9,0x3DC,0x69E,0x1AE,0x3AE,0x5AE,0xABD,
	0x8CD,0x7AE,0x0BE,0x3BE,0x5BE,0xBBD,0x1DD,0x7BE,0x33F,0x25F,0x26F,0x4F6,0x37F,0x6F6,0xBCD,0x48F,
	0x77F,0x29F,0x8CE,0x59F,0x69F,0x1AF,0x3AF,0xBBE,0x2DE,0x4DE,0x7AF,0x0BF,0x3BF,0x5BF,0x6BF,0x9AF,
	0x7BF,0x9DE,0x8BF,0xAFA,0x0CF,0x3CF,0xDEA,0x6CF,0xCCE,0x0EE,0x3EE,0x8CF,0xBDE,0x6EE,0x7EE,0x9CF,
	0xBBF,0x2DF,0x4DF,0x5DF,0x6DF,0xCDE,0x7DF,0x8DF,0xAEE,0xBCF,0x9DF,0x9DF,0x9DF,0xBEE,0xBEE,0xADF,
	0xADF,0xCCF,0x0EF,0x3EF,0x5EF,0xBDF,0x6FE,0x7EF,0x7EF,0x8EF,0x8EF,0x9EF,0x9EF,0x9EF,0xCDF,0xCDF,
	0xAEF,0xAEF,0xAEF,0xDEE,0xDEE,0xDEE,0xBEF,0xBEF,0xBEF,0xDDF,0xDDF,0xFF0,0x3FF,0x5FF,0x6FF,0x6FF,
	0x7FF,0x7FF,0x8FF,0x8FF,0xEEE,0xEEE,0x9FF,0x9FF,0xAFF,0xAFF,0xAFF,0xAFF,0xDEF,0xDEF,0xDEF,0xBFF,
	0xBFF,0xBFF,0xBFF,0xBFF,0xBFF,0xFFC,0xFFC,0xFFC,0xFFC,0xCFF,0xCFF,0xCFF,0xEEF,0xEEF,0xEEF,0xEEF,
	0xEEF,0xEEF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xDFF,0xEFF,
	0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,0xEFF,
	0xEFF,0xEFF,0xEFF,0xEFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,
};

//
void initPcmTest(void){
	VDP_setWindowVPos(0,3);
	VDP_drawTextBG(WINDOW,"68k PCM Test", 0, 0);
	
//	Z80_init();
//	Z80_loadDriver(Z80_DRIVER_NULL,0);
	SYS_disableInts();
	stopZ80();
	
	//Z80_unloadDriver();
	//
	convertPCM_F_TBL();
	convertPCM_F_TBL2();
	
	g_pcmtest_play=0;
	g_pcmtest_now=0;
	g_pcmtest_adr =(u8 *)&v0002_sfx;
	g_pcmtest_size=sizeof(v0002_sfx);
	//
	YM2612_enableDAC();
	YM2612_write(0,0x2a);
	//
//	internalHIntCB=&HBlankFunction_PcmTest_NONE;
	SYS_setHIntCallback(&HBlankFunction_PcmTest_NONE);
	VDP_setHIntCounter((g_pcmtest_add-1));
	VDP_setHInterrupt(1);
	
	SYS_enableInts();
	//
	JOY_setEventHandler(joyEvent_PcmTest);
	g_mainloop_callback=&mainLoop_PcmTest;
}
static void endLoop_PcmTest(void){
	SYS_setHIntCallback(NULL);
	VDP_setHInterrupt(0);
	returnMenu();
}
static void mainLoop_PcmTest(void){
	char m[80];
	
	
	if(g_pcmtest_play){
		sprintf(m,"play(%08x/%08x)",g_pcmtest_now,g_pcmtest_size);
	}else{
		strcpy(m,"stop");
	}
	VDP_drawTextBG(WINDOW,g_pcmtest_outtype_m[g_pcmtest_outtype], 0, 1);
	VDP_drawTextBG(WINDOW,m, 0, 2);
}
static u8 g_pcmftbl_flg=0;
void convertPCM_F_TBL(){
	if(g_pcmftbl_flg)return;
	g_pcmftbl_flg=1;
	for(int i=0;i<256;i++){
		u8 d=PCM_F_TBL[i];
		d=15-d;
		PCM_F_TBL[i]=d;
	}
}
static u8 g_pcmftbl2_flg=0;
void convertPCM_F_TBL2(){
	if(g_pcmftbl2_flg)return;
	g_pcmftbl2_flg=1;
	for(int i=0;i<256;i++){
		u16 d=PCM_F_TBL2[i];
		u8 d0=  d       & 0xf;
		u8 d1=((d >> 4) & 0xf);
		u8 d2=((d >> 8) & 0xf);
		d0=15-d0;
		d1=15-d1;
		d2=15-d2;
		d=(d2 << 8) | (d1 << 4) | d0;
		PCM_F_TBL2[i]=d;
	}
}

static void HBlankFunction_PcmTest_NONE(void){
}
static void HBlankFunction_PcmTest_DAC(void){
	if(!g_pcmtest_play)return;
	u8 data=(g_pcmtest_adr[g_pcmtest_now]+0x80) &0xff;
	YM2612_write(0, 0x2a);
	YM2612_write(1, data);

//	u8 *pb = (u8*) (YM2612_BASEPORT+1);
//	*pb = data;

	g_pcmtest_now+=g_pcmtest_add;
	if(g_pcmtest_now>=g_pcmtest_size)g_pcmtest_play=0;
}
static void HBlankFunction_PcmTest_PSG1(void){
	if(!g_pcmtest_play)return;
	u8 data=(g_pcmtest_adr[g_pcmtest_now]+0x80) &0xff;
	u8 d=PCM_F_TBL[data];
	vu8 *pb=(u8*) PSG_PORT;
	*pb = 0x90 | d;
	g_pcmtest_now+=g_pcmtest_add;
	if(g_pcmtest_now>=g_pcmtest_size)g_pcmtest_play=0;
}
static void HBlankFunction_PcmTest_PSG3(void){
	if(!g_pcmtest_play)return;
	u8 data=(g_pcmtest_adr[g_pcmtest_now]+0x80) &0xff;
	u16 d=PCM_F_TBL2[data];
	u8 d0=  d       & 0xf;
	u8 d1=((d >> 4) & 0xf);
	u8 d2=((d >> 8) & 0xf);
	vu8 *pb=(u8*) PSG_PORT;
	*pb = 0x90 | d0;
	*pb = 0xb0 | d1;
	*pb = 0xd0 | d2;
	g_pcmtest_now+=g_pcmtest_add;
	if(g_pcmtest_now>=g_pcmtest_size)g_pcmtest_play=0;
}
#define SFX_V0002		64
static void playPcmTest(){
	g_pcmtest_play=1;
	g_pcmtest_now=0;
//	SND_setPCM_XGM(SFX_V0002, v0002_sfx, sizeof(v0002_sfx));
//	SND_startPlayPCM_XGM(SFX_V0002, 1, SOUND_PCM_CH2);
	switch(g_pcmtest_outtype){
		case 0:
			//internalHIntCB=&HBlankFunction_PcmTest_DAC;
			SYS_setHIntCallback(&HBlankFunction_PcmTest_DAC);
			break;
		case 1:
			//internalHIntCB=&HBlankFunction_PcmTest_PSG3;
			SYS_setHIntCallback(&HBlankFunction_PcmTest_PSG3);
			break;
		case 2:
			//internalHIntCB=&HBlankFunction_PcmTest_PSG1;
			SYS_setHIntCallback(&HBlankFunction_PcmTest_PSG1);
			break;
	}

}
static void stopPcmTest(){
	g_pcmtest_play=0;
}
static void joyEvent_PcmTest(u16 joy, u16 changed, u16 state){
	if(changed & state & (BUTTON_RIGHT)){
		g_pcmtest_outtype++;
		if(g_pcmtest_outtype>2)g_pcmtest_outtype=0;
	}
	if(changed & state & (BUTTON_LEFT)){
		g_pcmtest_outtype--;
		if(g_pcmtest_outtype<0)g_pcmtest_outtype=2;
	}
	if(changed & state & (BUTTON_A)){
		playPcmTest();
	}
	if(changed & state & (BUTTON_B)){
		stopPcmTest();
	}
	if (changed & state & (BUTTON_START)){
		endLoop_PcmTest();
	}
}
