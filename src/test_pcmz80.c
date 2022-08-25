#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_tile.h"
#include "sys.h"

#include "z80_pcm1ch.h"

#include "test_menu.h"
#include "test_pcm68.h"

//====================

#define PCMZ80_DATA			0x50
#define PCMZ80_START		(PCMZ80_DATA+0x00)
#define PCMZ80_NOW			(PCMZ80_DATA+0x02)
#define PCMZ80_SIZE			(PCMZ80_DATA+0x04)
#define PCMZ80_STATE		(PCMZ80_DATA+0x06)
#define PCMZ80_NOWBANK		(PCMZ80_DATA+0x08)
#define PCMZ80_NOWOFF		(PCMZ80_DATA+0x0A)
#define PCMZ80_NOWLOOP		(PCMZ80_DATA+0x0B)
#define PCMZ80_NOWEND		(PCMZ80_DATA+0x0C)
#define PCMZ80_NEXTBANK		(PCMZ80_DATA+0x0E)
#define PCMZ80_NEXTOFF		(PCMZ80_DATA+0x10)
#define PCMZ80_ENDBANK		(PCMZ80_DATA+0x12)
#define PCMZ80_ENDOFF		(PCMZ80_DATA+0x14)
#define PCMZ80_NOWADR		(PCMZ80_DATA+0x16)
#define PCMZ80_NEXTNOW1		(PCMZ80_DATA+0x18)
#define PCMZ80_PLAYMODE		(PCMZ80_DATA+0x1A)

#define PCMZ80_PLAYMODE_MAX	7
#define PCMZ80_PSGBUFF		0x1a00

static void joyEvent_PcmZ80(u16 joy, u16 changed, u16 state);
static void mainLoop_PcmZ80(void);
static void endLoop_PcmZ80(void);
static void playPcmZ80();
static void stopPcmZ80();
static u16 readU16_Z80(u16 off);
static u32 readU32_Z80(u16 off);
static void writeU32_Z80(u16 off,u32 n);
static void writeU16_Z80(u16 off,u16 n);
static void writeAdr16_Z80(u16 off,u32 n);
static void writeBytes_Z80(u16 off,const u8 *data,u32 size);
static void readBytes_Z80(u16 off,u8 *data,u32 size);
static void VBlankFunction_PcmZ80(void);
//
static int g_pcmz80_play=0;
static int g_pcmz80_mode=0;
static int g_pcmz80_vsync=0;

//
void initPcmZ80(void){
	VDP_setWindowVPos(0,28);
	VDP_drawTextBG(WINDOW,"Z80 PCM Test", 0, 0);
	//
	YM2612_enableDAC();
	
	SYS_disableInts();
	Z80_init();
	Z80_clear(0, Z80_RAM_LEN, FALSE);
	
	Z80_requestBus(1);
	
	const void *z80_prog=z80_pcm1ch;
	u32 z80_prog_size=sizeof(z80_pcm1ch);
	writeBytes_Z80(0,z80_prog,z80_prog_size);
	
	void *pcmtest_adr=(void *)&v0002_sfx;
	u32 pcmtest_size=sizeof(v0002_sfx);
	void *pcmtest_bottom=(void *)((u32)pcmtest_adr+pcmtest_size);
	Z80_write(PCMZ80_PLAYMODE,g_pcmz80_mode);
	writeAdr16_Z80(PCMZ80_START,(u32)pcmtest_adr);
	writeAdr16_Z80(PCMZ80_SIZE ,(u32)pcmtest_bottom);
	
	convertPCM_F_TBL2();
	for(int i=0;i<256;i++){
		u16 n=PCM_F_TBL2[i];
		//writeU16_Z80(PCMZ80_PSGBUFF+i*2,n);
		u8 n0= n       & 0x0f;
		u8 n1=(n >> 4) & 0x0f;
		u8 n2=(n >> 8) & 0x0f;
		n0+=0x90;
		n1+=0xB0;
		n2+=0xD0;
		Z80_write(PCMZ80_PSGBUFF      +i,n0);
		Z80_write(PCMZ80_PSGBUFF+0x100+i,n1);
		Z80_write(PCMZ80_PSGBUFF+0x200+i,n2);
		
	}
	
	Z80_startReset();
	Z80_releaseBus();
	
	while(Z80_isBusTaken());
	Z80_endReset();
	
	SYS_setVIntCallback(&VBlankFunction_PcmZ80);
	
	SYS_enableInts();
	
	JOY_setEventHandler(joyEvent_PcmZ80);
	g_mainloop_callback=&mainLoop_PcmZ80;
}
void stopZ80(void){
	Z80_init();
	Z80_clear(0, Z80_RAM_LEN, FALSE);
	Z80_requestBus(1);
}
static void VBlankFunction_PcmZ80(void){
	g_pcmz80_vsync++;
}
static void getAdrString(char *m,void *adr){
	u16 h=(u16)(((u32)adr >> 16) & 0xffff);
	u16 l=(u16)( (u32)adr        & 0xffff);
	sprintf(m,"%04x%04x",h,l);
}
static void getU16String(char *m,u16 adr){
	sprintf(m,"%04x",adr);
}
static void getU16BankOffString(char *m,u16 adr){
	sprintf(m,"%04x:bank(%04x),off(%02x)",adr,(adr & 0xff80),(adr & 0x7f));
}

static void endLoop_PcmZ80(void){
	returnMenu();
}
static void mainLoop_PcmZ80(void){
		char m[80]={0};
		
		u8 play_state;
		u16 now_bank;
		u8 now_off;
		u8 now_loop;
		u8 now_end;
		u16 next_bank;
		u8 next_off;
		u16 end_bank;
		u8 end_off;
		u16 now;
		u16 now_adr;
		u16 next_now;
		u8 play_mode;
		
		Z80_requestBus(1);
		play_state=Z80_read(PCMZ80_STATE);
		now      =readU16_Z80(PCMZ80_NOW);
		now_bank =readU16_Z80(PCMZ80_NOWBANK);
		now_off  =Z80_read(PCMZ80_NOWOFF);
		now_loop =Z80_read(PCMZ80_NOWLOOP);
		now_end  =Z80_read(PCMZ80_NOWEND);
		next_bank=readU16_Z80(PCMZ80_NEXTBANK);
		next_off =Z80_read(PCMZ80_NEXTOFF);
		end_bank =readU16_Z80(PCMZ80_ENDBANK);
		end_off  =Z80_read(PCMZ80_ENDOFF);
		now_adr  =readU16_Z80(PCMZ80_NOWADR);
		next_now =readU16_Z80(PCMZ80_NEXTNOW1);
		play_mode=Z80_read(PCMZ80_PLAYMODE);
		
		
		Z80_releaseBus();
		void *pcmtest_adr=(void *)&v0002_sfx;
		u32 pcmtest_size=sizeof(v0002_sfx);
		void *pcmtest_bottom=pcmtest_adr+pcmtest_size;
		int y=1;
		sprintf(m,"VSync:%5d,mode:%d,state:%d",g_pcmz80_vsync,play_mode,play_state);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		
		char pcmtest_adr_m[80];
		char pcmtest_bottom_m[80];
		char pcmtest_size_m[80];
		getU16BankOffString(pcmtest_adr_m   ,((u32)pcmtest_adr >> 8));
		getU16BankOffString(pcmtest_bottom_m,((u32)pcmtest_bottom >> 8));
		getU16String(pcmtest_size_m  ,(pcmtest_size >> 8));
		sprintf(m,"st:%s",pcmtest_adr_m);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"ed:%s",pcmtest_bottom_m);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"sz:%s",pcmtest_size_m);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"now:bank(%04x),off(%02x),loop(%02x),end(%d)",now_bank,now_off,now_loop,now_end);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"nxt:bank(%04x),off(%02x)",next_bank,next_off);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"end:bank(%04x),off(%02x)",end_bank,end_off);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"now:(%04x)",now);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"now_adr:(%04x)",now_adr);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
		sprintf(m,"next_now:(%04x)",next_now);
		VDP_drawTextBG(WINDOW,m, 0,y);y++;
}
static void writeBytes_Z80(u16 off,const u8 *data,u32 size){
	for(u32 i=0;i<size;i++){
		Z80_write(off+i,data[i]);
	}
}
static void readBytes_Z80(u16 off,u8 *data,u32 size){
	for(u32 i=0;i<size;i++){
		data[i]=Z80_read(off+i);
	}
}

static u32 readU32_Z80(u16 off){
	u8 n0=Z80_read(off+0);
	u8 n1=Z80_read(off+1);
	u8 n2=Z80_read(off+2);
	u8 n3=Z80_read(off+3);
	u32 n=(n3 << 24) | (n2 << 16) | (n1 << 8) | n0;
	return n;
}
static u16 readU16_Z80(u16 off){
	u8 n0=Z80_read(off+0);
	u8 n1=Z80_read(off+1);
	u16 n=(n1 << 8) | n0;
	return n;
}
static void writeU32_Z80(u16 off,u32 n){
	Z80_write(off+0,(u8)( n        & 0xff));
	Z80_write(off+1,(u8)((n >>  8) & 0xff));
	Z80_write(off+2,(u8)((n >> 16) & 0xff));
	Z80_write(off+3,(u8)((n >> 24) & 0xff));
}
static void writeU16_Z80(u16 off,u16 n){
	Z80_write(off+0,(u8)( n        & 0xff));
	Z80_write(off+1,(u8)((n >>  8) & 0xff));
}
static void writeAdr16_Z80(u16 off,u32 adr){
	u16 n=(((u32)adr) >> 8);
	writeU16_Z80(off,n);
}
static void playPcmZ80(){
	g_pcmz80_play=1;
	
	Z80_requestBus(1);
	Z80_write(PCMZ80_STATE,1);
	Z80_releaseBus();
}
static void stopPcmZ80(){
	g_pcmz80_play=0;
}
static void setPcmZ80Mode(int mode){
	Z80_requestBus(1);
	Z80_write(PCMZ80_PLAYMODE,mode);
	Z80_releaseBus();
}

static void nextPcmZ80Mode(void){
	g_pcmz80_mode++;
	if(g_pcmz80_mode>(PCMZ80_PLAYMODE_MAX-1))g_pcmz80_mode=0;
	setPcmZ80Mode(g_pcmz80_mode);
}
static void prevPcmZ80Mode(void){
	g_pcmz80_mode--;
	if(g_pcmz80_mode<0)g_pcmz80_mode=(PCMZ80_PLAYMODE_MAX-1);
	setPcmZ80Mode(g_pcmz80_mode);
}

static void joyEvent_PcmZ80(u16 joy, u16 changed, u16 state){
	if(changed & state & (BUTTON_A)){
		playPcmZ80();
	}
	if(changed & state & (BUTTON_B)){
		stopPcmZ80();
	}
	if (changed & state & (BUTTON_LEFT)){
		prevPcmZ80Mode();
	}
	if (changed & state & (BUTTON_RIGHT)){
		nextPcmZ80Mode();
	}
	if (changed & state & (BUTTON_START)){
		endLoop_PcmZ80();
	}
}
