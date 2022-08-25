#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_tile.h"
#include "sys.h"

#include "z80_pcm1ch.h"
#include "z80_psgpcm.h"

#include "mdzdrv_c.h"

#include "test_menu.h"

//====================
extern void convertPCM_F_TBL2(void);
extern u8 PCM_F_TBL[];
extern u16 PCM_F_TBL2[];
//====================
#define PCMZ80_DATA			0x50
#define PCMZ80_FMOUT		(PCMZ80_DATA+0x00)
#define PCMZ80_FMREAD		(PCMZ80_DATA+0x02)
#define PCMZ80_PAN			(PCMZ80_DATA+0x04)
#define PCMZ80_MUTE			(PCMZ80_DATA+0x06)
#define PCMZ80_CNL				0x60
#define PCMZ80_CNL_IN_START		 0
#define PCMZ80_CNL_IN_END		 3
#define PCMZ80_CNL_IN_ADD_L		 6
#define PCMZ80_CNL_IN_ADD_H		 9
#define PCMZ80_CNL_NOW_ADR		12
#define PCMZ80_CNL_NOW_ADR_L	14
#define PCMZ80_CNL_END_ADR		15
#define PCMZ80_CNL_ADD_L		18
#define PCMZ80_CNL_ADD_H		21
#define PCMZ80_CNL_PLAY_FLG		24
#define PCMZ80_CNL_VOL			26
#define PCMZ80_CNL_NEXT_TMP		27
#define PCMZ80_CNL_SIZE			0x20

static int g_def_pcm_cnl=0;
//static int g_def_pcm_cnl=1;
//static int g_def_pcm_cnl=2;

/*
#define PCMZ80_DATA			0x50
#define PCMZ80_CNL_IN_START		(PCMZ80_DATA+0x00)
#define PCMZ80_NOW			(PCMZ80_DATA+0x02)
#define PCMZ80_SIZE			(PCMZ80_DATA+0x04)
#define PCMZ80_CNL_PLAY_FLG		(PCMZ80_DATA+0x06)
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
#define PCMZ80_NOWEND1		(PCMZ80_DATA+0x1C)
#define PCMZ80_CNL_VOL			(PCMZ80_DATA+0x1E)
#define PCMZ80_FMOUT		(PCMZ80_DATA+0x20)
#define PCMZ80_FMREAD		(PCMZ80_DATA+0x22)
#define PCMZ80_CNL_ADD_L		(PCMZ80_DATA+0x24)
#define PCMZ80_CNL_ADD_H		(PCMZ80_DATA+0x26)
#define PCMZ80_NOW_TMP		(PCMZ80_DATA+0x28)
#define PCMZ80_PAN			(PCMZ80_DATA+0x2A)
*/

#define PCMZ80_PLAYMODE_MAX	7
#define PCMZ80_PSGBUFF		0x1a00
#define PCMZ80_VOLTBL		0x0a00
#define PCMZ80_VOLTBL_H		0x0a
#define PCMZ80_FMBUFF		0x1a00
#define PCMZ80_FMBUFF_B		0x1d00

static int g_pcmz80_mode=1;

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
static void writeU8_Z80(u16 off,u8 n){
	Z80_write(off,(u8)n);
}
static void writeAdr16_Z80(u16 off,u32 adr){
	u16 n=(((u32)adr) >> 8);
	writeU16_Z80(off,n);
}
static void writeAdr16S_Z80(u16 off,u32 adr){
	u16 n=(((u32)adr) >> 8);
	u8 n0=(u8)( n       & 0x7f) | 0x80;
	u8 n1=(u8)((n >> 7) & 0xff);
	Z80_write(off+0,n0);
	Z80_write(off+1,n1);
}
static void writeAdr16S3_Z80(u16 off,u32 adr){
	u16 n=(((u32)adr) >> 8);
	u8 nn=(u8)(adr & 0xff);
	u8 n0=(u8)( n       & 0x7f) | 0x80;
	u8 n1=(u8)((n >> 7) & 0xff);
	Z80_write(off+0,nn);
	Z80_write(off+1,n0);
	Z80_write(off+2,n1);
}

//====================
typedef struct PCM_BIN_DATA{
	const void *data;
	u32 size;
}PCM_BIN_DATA;

typedef struct MDZ_BIN_DATA{
	const void *data;
	u32 size;
	const PCM_BIN_DATA *pcm_tbl;
	int vol;
}MDZ_BIN_DATA;

static void joyEvent_MDZTest(u16 joy, u16 changed, u16 state);
static void mainLoop_MDZTest(void);
static void endLoop_MDZTest(void);
static void playBgm_MDZTest(void);
static void stopBgm_MDZTest(void);

static CMDZ_BgmData *cmdz_bgmdata;
static CMDZ_WORK *cmdz_work;
static CMDZ_FMChip cmdz_fmchip;
static CMDZ_PcmDriver cmdz_pcmdriver;
static bool g_mdz_initret=false;
static bool g_mdz_playret=false;
static int g_mdz_tempo=0;
static int g_mdz_basecnt=0;
static int g_mdz_cnt=0;
static int g_wait_num=0;
//static int g_mdz_logx=0;
//static int g_mdz_logy=0;
//static int g_mdz_logmy=24;
//static int g_ssgreg[0x10];
//static int g_port0[0x100];
//static int g_port1[0x100];
static const int g_psg_vol_tbl[]={0, 1, 1, 1, 1, 1, 2, 3, 4, 6, 8, 9,10,12,14,15};
static bool g_use_psgpcm=true;
static bool g_use_dacpcm=false;
static const PCM_BIN_DATA *g_pcm_tbl=NULL;
static int g_adpcm_vol_scale=256;
static int g_rithm_vol_scale=196;

typedef struct PcmInfo{
	int flg;
	int cnl;
	int vol;
	int oto;
	int oto_old;
	int note;
	int pan;
}PcmInfo;
#define PCMINFO_MAX	3
PcmInfo g_pcminfo[PCMINFO_MAX];
/*
static int g_pcm_cnl=0;
static int g_pcm_vol=0;
static int g_pcm_oto=-1;
static int g_pcm_oto_old=-1;
static int g_pcm_note=0;
static int g_pcm_pan=0;
*/
static void initPcmInfo(PcmInfo *info){
	info->flg=0;
	info->cnl=0;
	info->vol=0;
	info->oto=-1;
	info->oto_old=-1;
	info->note=0;
	info->pan=0;
}
static void initPcmInfoList(void){
	for(int i=0;i<PCMINFO_MAX;i++){
		initPcmInfo(&g_pcminfo[i]);
	}
}


#define VOL_MF		50
#define VOL_KGB		100
#define VOL_XP		90
#define VOL_YK		40
#define VOL_PCM01	40

const PCM_BIN_DATA g_pcm_rt[]={
	{&ym2608_bd_wav ,sizeof(ym2608_bd_wav) },
	{&ym2608_sd_wav ,sizeof(ym2608_sd_wav) },
	{&ym2608_top_wav,sizeof(ym2608_top_wav)},
	{&ym2608_hh_wav ,sizeof(ym2608_hh_wav) },
	{&ym2608_tom_wav,sizeof(ym2608_tom_wav)},
	{&ym2608_rym_wav,sizeof(ym2608_rym_wav)},
};

const PCM_BIN_DATA g_pcm_xp[]={
	{&xp_0_wav,sizeof(xp_0_wav)},
	{&xp_1_wav,sizeof(xp_1_wav)},
	{&xp_2_wav,sizeof(xp_2_wav)},
	{&xp_3_wav,sizeof(xp_3_wav)},
	{&xp_4_wav,sizeof(xp_4_wav)},
	{&xp_5_wav,sizeof(xp_5_wav)},
	{&xp_6_wav,sizeof(xp_6_wav)},
	{&xp_7_wav,sizeof(xp_7_wav)},
	{&xp_8_wav,sizeof(xp_8_wav)},
	{&xp_9_wav,sizeof(xp_9_wav)},
	{&xp_10_wav,sizeof(xp_10_wav)},
	{&xp_11_wav,sizeof(xp_11_wav)},
	{&xp_12_wav,sizeof(xp_12_wav)},
	{&xp_13_wav,sizeof(xp_13_wav)},
	{&xp_14_wav,sizeof(xp_14_wav)},
	{&xp_15_wav,sizeof(xp_15_wav)},
	{&xp_16_wav,sizeof(xp_16_wav)},
	{&xp_17_wav,sizeof(xp_17_wav)},
	{&xp_18_wav,sizeof(xp_18_wav)},
	{&xp_19_wav,sizeof(xp_19_wav)},
	{&xp_20_wav,sizeof(xp_20_wav)}
};
const PCM_BIN_DATA g_pcm_mf[]={
	{&mf_0_wav,sizeof(mf_0_wav)},
	{&mf_1_wav,sizeof(mf_1_wav)},
	{&mf_2_wav,sizeof(mf_2_wav)},
	{&mf_3_wav,sizeof(mf_3_wav)},
	{&mf_4_wav,sizeof(mf_4_wav)},
	{&mf_5_wav,sizeof(mf_5_wav)},
	{&mf_6_wav,sizeof(mf_6_wav)},
	{&mf_7_wav,sizeof(mf_7_wav)},
	{&mf_8_wav,sizeof(mf_8_wav)},
	{&mf_9_wav,sizeof(mf_9_wav)},
	{&mf_10_wav,sizeof(mf_10_wav)},
	{&mf_11_wav,sizeof(mf_11_wav)},
	{&mf_12_wav,sizeof(mf_12_wav)},
	{&mf_13_wav,sizeof(mf_13_wav)},
	{&mf_14_wav,sizeof(mf_14_wav)},
	{&mf_15_wav,sizeof(mf_15_wav)},
	{&mf_16_wav,sizeof(mf_16_wav)},
};
const PCM_BIN_DATA g_pcm_kgb[]={
	{&kgb_0_wav,sizeof(kgb_0_wav)},
	{&kgb_1_wav,sizeof(kgb_1_wav)},
	{&kgb_2_wav,sizeof(kgb_2_wav)},
	{&kgb_3_wav,sizeof(kgb_3_wav)},
	{&kgb_4_wav,sizeof(kgb_4_wav)},
	{&kgb_5_wav,sizeof(kgb_5_wav)},
	{&kgb_6_wav,sizeof(kgb_6_wav)},
	{&kgb_7_wav,sizeof(kgb_7_wav)},
	{&kgb_8_wav,sizeof(kgb_8_wav)},
};
const PCM_BIN_DATA g_pcm_yk[]={
	{&yk_0_wav,sizeof(yk_0_wav)},
	{&yk_1_wav,sizeof(yk_1_wav)},
	{&yk_2_wav,sizeof(yk_2_wav)},
	{&yk_3_wav,sizeof(yk_3_wav)},
	{&yk_4_wav,sizeof(yk_4_wav)},
	{&yk_5_wav,sizeof(yk_5_wav)},
	{&yk_6_wav,sizeof(yk_6_wav)},
	{&yk_7_wav,sizeof(yk_7_wav)},
	{&yk_8_wav,sizeof(yk_8_wav)},
	{&yk_9_wav,sizeof(yk_9_wav)},
	{&yk_10_wav,sizeof(yk_10_wav)},
	{&yk_11_wav,sizeof(yk_11_wav)},
	{&yk_12_wav,sizeof(yk_12_wav)},
	{&yk_13_wav,sizeof(yk_13_wav)},
	{&yk_14_wav,sizeof(yk_14_wav)},
	{&yk_15_wav,sizeof(yk_15_wav)},
	{&yk_16_wav,sizeof(yk_16_wav)},
	{&yk_17_wav,sizeof(yk_17_wav)},
	{&yk_18_wav,sizeof(yk_18_wav)},
	{&yk_19_wav,sizeof(yk_19_wav)},
	{&yk_20_wav,sizeof(yk_20_wav)},
	{&yk_21_wav,sizeof(yk_21_wav)},
	{&yk_22_wav,sizeof(yk_22_wav)},
	{&yk_23_wav,sizeof(yk_23_wav)},
	{&yk_24_wav,sizeof(yk_24_wav)},
	{&yk_25_wav,sizeof(yk_25_wav)},
	{&yk_26_wav,sizeof(yk_26_wav)},
};

const PCM_BIN_DATA g_pcm_pcm01[]={
	{&pcm01_0_wav,sizeof(pcm01_0_wav)},
	{&pcm01_1_wav,sizeof(pcm01_1_wav)},
	{&pcm01_2_wav,sizeof(pcm01_2_wav)},
	{&pcm01_3_wav,sizeof(pcm01_3_wav)},
	{&pcm01_4_wav,sizeof(pcm01_4_wav)},
	{&pcm01_5_wav,sizeof(pcm01_5_wav)},
	{&pcm01_6_wav,sizeof(pcm01_6_wav)},
	{&pcm01_7_wav,sizeof(pcm01_7_wav)},
	{&pcm01_8_wav,sizeof(pcm01_8_wav)},
	{&pcm01_9_wav,sizeof(pcm01_9_wav)},
	{&pcm01_10_wav,sizeof(pcm01_10_wav)},
	{&pcm01_11_wav,sizeof(pcm01_11_wav)},
	{&pcm01_12_wav,sizeof(pcm01_12_wav)},
	{&pcm01_13_wav,sizeof(pcm01_13_wav)},
	{&pcm01_14_wav,sizeof(pcm01_14_wav)},
	{&pcm01_15_wav,sizeof(pcm01_15_wav)},
	{&pcm01_16_wav,sizeof(pcm01_16_wav)},
	{&pcm01_17_wav,sizeof(pcm01_17_wav)},
	{&pcm01_18_wav,sizeof(pcm01_18_wav)},
	{&pcm01_19_wav,sizeof(pcm01_19_wav)},
	{&pcm01_20_wav,sizeof(pcm01_20_wav)},
	{&pcm01_21_wav,sizeof(pcm01_21_wav)},
};

const MDZ_BIN_DATA g_mdz_bin_data[]={
	{mf01_mdz,sizeof(mf01_mdz),g_pcm_mf,VOL_MF},			//0
	{mf02_mdz,sizeof(mf02_mdz),g_pcm_mf,VOL_MF},			//1
//	{mf11_mdz,sizeof(mf11_mdz),g_pcm_mf,VOL_MF},			//2
	{mf17_mdz,sizeof(mf17_mdz),g_pcm_mf,VOL_MF},			//3
	{fg_mdz,sizeof(fg_mdz),g_pcm_kgb,VOL_KGB},				//4
	{bigblue_mdz,sizeof(bigblue_mdz),g_pcm_xp,VOL_XP},		//5
	{spb_05_mdz,sizeof(spb_05_mdz),g_pcm_kgb,VOL_KGB},		//5
	{spb_10_mdz,sizeof(spb_10_mdz),g_pcm_yk,VOL_YK},		//5
	{spb_12_mdz,sizeof(spb_12_mdz),g_pcm_yk,VOL_YK},		//5
	{xex_st1_mdz,sizeof(xex_st1_mdz),g_pcm_kgb,VOL_KGB},	//5
	{bos_fla_mdz,sizeof(bos_fla_mdz),g_pcm_pcm01,VOL_PCM01},	//5
};
#define MDZ_BIN_DATA_MAX	(sizeof(g_mdz_bin_data)/sizeof(MDZ_BIN_DATA))

char g_mute_buff[512];
void g_initMute(void){
	memset(g_mute_buff,0,512);
}

//static int g_mdz_data_index=0;
static int g_mdz_data_index=4;
//static int g_mdz_data_index=5;
//static int g_mdz_data_index=0;


bool g_usePSGPcm(void){
	return g_use_psgpcm;
}
bool g_useDACPcm(void){
	return g_use_dacpcm;
}
static bool g_hasPort(int port){
	return true;
}
#define DMst 	3579545.0
#define OMst 	7987200.0
#define PSG_TONE_SCALE_SHIFT	10
#define PSG_TONE_SCALE_MUL		(int)((DMst / OMst * 2.0)*(1 << PSG_TONE_SCALE_SHIFT))
static int g_toneScale(int n){
	int r = (n*PSG_TONE_SCALE_MUL) >> PSG_TONE_SCALE_SHIFT;
	if(r<    0)r=0;
	if(r>0x3ff)r=0x3ff;
	return r;
}
void g_outputPsgTone(int reg,int tone){
	if(g_use_psgpcm)return;
	tone&=0xfff;
	tone=g_toneScale(tone);
	int n0=reg | (tone & 0x0f);
	int n1=(tone >> 4) & 0x3f;
	vu8 *pb=(u8*) PSG_PORT;
	*pb = n0;
	*pb = n1;
}
void g_outputPsgVolume(int reg,int vol){
	if(g_use_psgpcm)return;
	vol=g_psg_vol_tbl[vol & 0x0f];
	int n0=reg | ((15-vol) & 0x0f);
	vu8 *pb=(u8*) PSG_PORT;
	*pb = n0;
}
void g_stopPsgVolume(void){
	vu8 *pb=(u8*) PSG_PORT;
	*pb = 0x90;
	*pb = 0xB0;
	*pb = 0xD0;
	//
	*pb = 0x80;
	*pb = 0x00;
	*pb = 0xA0;
	*pb = 0x00;
	*pb = 0xC0;
	*pb = 0x00;
}
#define FM_NOTE_SCALE_SHIFT	10
#define FM_NOTE_SCALE_MUL	(int)(1.049*(1 << FM_NOTE_SCALE_SHIFT))
int g_calcFMNoteScale(int _wave){
	int oct =(_wave >> 11);
	int note= _wave & 0x7ff;
	note<<=oct;
	note=(note*FM_NOTE_SCALE_MUL) >> FM_NOTE_SCALE_SHIFT;
	note>>=oct;
	if(note>0x7ff)note=0x7ff;
	_wave=(oct << 11) | note;
	return _wave;
}

/*
static void g_initRegPsg(void){
	for(int i=0;i<0x10;i++)g_ssgreg[i]=0;
	for(int i=0;i<0x100;i++)g_port0[i]=0;
	for(int i=0;i<0x100;i++)g_port1[i]=0;
	g_ssgreg[0x07]=7;
}
*/

static void pushFMBuff_Z80(int port,int reg,int data){
	Z80_requestBus(1);
	u16 off=readU16_Z80(PCMZ80_FMOUT);
	Z80_write(off+0,port);
	Z80_write(off+1,reg);
	Z80_write(off+2,data);
	off+=3;
	if(off>=PCMZ80_FMBUFF_B)off=PCMZ80_FMBUFF;
	writeU16_Z80(PCMZ80_FMOUT,off);
	Z80_releaseBus();
}

u8 fmbuff[3*256];
int fmbuff_index=0;
int fmbuff_index_max=0;
static void pushFMBuff(int port,int reg,int data){
	int i=fmbuff_index;
	fmbuff[i*3+0]=port;
	fmbuff[i*3+1]=reg;
	fmbuff[i*3+2]=data;
	fmbuff_index++;
}
static void outFMBuff(){
	if(fmbuff_index>fmbuff_index_max){
		fmbuff_index_max=fmbuff_index;
	}
	for(int i=0;i<fmbuff_index;i++){
		int port=fmbuff[i*3+0];
		int reg =fmbuff[i*3+1];
		int data=fmbuff[i*3+2];
		YM2612_writeReg(port,reg,data);
	}
	fmbuff_index=0;
}

#define FM_NOTE_SCALE_SHIFT	10
#define FM_NOTE_SCALE_MUL	(int)(1.049*(1 << FM_NOTE_SCALE_SHIFT))
static void g_outReg(int port,int reg,int data){
//	int *g_port;
	if(port==0){
//		g_port=g_port0;
		if(reg!=0x28 && reg<0x30)return;
	}
	if(port==1){
//		g_port=g_port1;
		//ADPCM
		if(reg<=0x10)return;
		//FM6APAN
		if(reg==0xb6)return;
	}
	if(!g_use_dacpcm){
		pushFMBuff(port,reg,data);
	}else{
		pushFMBuff_Z80(port,reg,data);
	}
}
static void g_outReg0(int reg,int data){
	g_outReg(0,reg,data);
}

#define TEMPO_SCALE_SHIFT	10
#define TEMPO_SCALE_MUL		(1 << TEMPO_SCALE_SHIFT)
static u32 g_time;
static u32 g_time_add;
static void g_initTimeAdd(void){
	g_time=0;
	g_time_add=TEMPO_SCALE_MUL;
}
static void g_setTimer(int tempo,int base_cnt){
	g_mdz_tempo  =tempo;
	g_mdz_basecnt=base_cnt;
//	double _add=(double)(4*60*60)/((double)(tempo*base_cnt));
//	g_time_add=(u32)(_add*TEMPO_SCALE_MUL);
	g_time_add=(u32)(4*60*60*TEMPO_SCALE_MUL)/( (u32)tempo * (u32)base_cnt );
}

static bool g_timeAdd(int *_g_wait_num){
	g_time+=g_time_add;
	if(g_time>=TEMPO_SCALE_MUL){
		int wait_num=(g_time >> TEMPO_SCALE_SHIFT);
		*_g_wait_num=wait_num;
		g_time&=(TEMPO_SCALE_MUL-1);
		if(wait_num>0)return true;
	}
	return false;
}
static int g_getTimeAdd(void){
	return (100*g_time_add) >> TEMPO_SCALE_SHIFT;
}


static void g_startPCMBuffer(){
}
static int g_getNowMakePCMSize(void){
	return 0;
}
static int g_getPCMBufferAdr(void){
	return 0;
}
static void g_allocCnl(int cnl){
}
static void g_initCnl(int cnl){
}

static void stopPcm_Z80(int cnl){
	int cnl_off=(PCMZ80_CNL+(PCMZ80_CNL_SIZE*cnl));
	writeU8_Z80(cnl_off+PCMZ80_CNL_PLAY_FLG,0);
}
static void playPcm_Z80(const PCM_BIN_DATA *pcm_tbl,int cnl,int oto,int vol,int note,int pan){
	if(!pcm_tbl)return;
	int cnl_off=(PCMZ80_CNL+(PCMZ80_CNL_SIZE*cnl));
	const void *pcmtest_adr=pcm_tbl[oto].data;
	u32   pcmtest_size     =pcm_tbl[oto].size;
	const void *pcmtest_bottom=(void *)((u32)pcmtest_adr+pcmtest_size);
	writeAdr16S_Z80(cnl_off+PCMZ80_CNL_IN_START,(u32)pcmtest_adr);
	writeAdr16S_Z80(cnl_off+PCMZ80_CNL_IN_END  ,(u32)pcmtest_bottom);
//vol=8;
	vol+=PCMZ80_VOLTBL_H;
	writeU8_Z80(cnl_off+PCMZ80_CNL_VOL,vol);
	int add_shift=12;
	u16 add_l=(note << (16-add_shift)) & 0xffff;
	u16 add_h=(note >> add_shift     ) & 0xffff;
	writeU16_Z80(cnl_off+PCMZ80_CNL_IN_ADD_L,add_l);
	writeU16_Z80(cnl_off+PCMZ80_CNL_IN_ADD_H,add_h);
	writeU16_Z80(PCMZ80_PAN,(pan & 3) << 6);
	writeU8_Z80(cnl_off+PCMZ80_CNL_PLAY_FLG,1);
}

// *************
static void g_stopCnl(int cnl){
cnl=g_def_pcm_cnl;
	if(cnl>=0 || cnl<PCMINFO_MAX){
		PcmInfo *pcminfo=&g_pcminfo[cnl];
		pcminfo->flg=1;
		pcminfo->cnl=cnl;
		pcminfo->oto=-1;
		pcminfo->vol=0;
		pcminfo->note=0;
		pcminfo->pan=0;
	}
	
	
	Z80_requestBus(1);
	stopPcm_Z80(cnl);
	Z80_releaseBus();
}

static void g_keyOnCnl(int cnl,CMDZ_PcmBankTbl *pcmbanktbl,int bank,int oto,int vol,int _note,int _pan){
cnl=g_def_pcm_cnl;
	int vol_scale=g_adpcm_vol_scale;
	if(g_use_psgpcm)vol_scale*=2;
	PCM_BIN_DATA *pcm_tbl=g_pcm_tbl;
	if(bank==-1){
		static int g_cnl=0;
		cnl=1+g_cnl;g_cnl++;g_cnl&=1;
		pcm_tbl=g_pcm_rt;
		vol_scale=g_rithm_vol_scale;
	}else{
		//return;
	}
	
	//int adpcm_vol_scale=g_adpcm_vol_scale;
	//if(g_use_psgpcm)adpcm_vol_scale*=2;
	//vol=(vol * adpcm_vol_scale) >> 8;
	vol=(vol * vol_scale) >> 8;
	if(vol>15)vol=15;
	//
	int note=_note;
	int pan=0;
	if(_pan==5){
		pan=3;
	}else if(_pan<5){
		pan=1;
	}else if(_pan>5){
		pan=2;
	}
	if(cnl>=0 || cnl<PCMINFO_MAX){
		PcmInfo *pcminfo=&g_pcminfo[cnl];
		pcminfo->flg=1;
		pcminfo->cnl=cnl;
		pcminfo->oto=oto;
		pcminfo->vol=vol;
		pcminfo->note=note;
		pcminfo->pan=pan;
	}
	Z80_requestBus(1);
	//
	playPcm_Z80(pcm_tbl,cnl,oto,vol,note,pan);
	Z80_releaseBus();
}
static void g_setPanCnl(int cnl,int pan){
}
static void g_setNoteCnl(int cnl,int wave){
}
static void g_setVolumeCnl(int cnl,int vol){
}

#define VOL_BASE_DIV	1.122
static void makeVolTbl(){
	int mul=256;
	int scale=(int)(256.0*1.0/VOL_BASE_DIV);
	for(int vol=15;vol>=1;vol--){
		for(int i=0;i<256;i++){
			int p;
			if(i<128){
				p=i;
			}else{
				p=i-256;
			}
			int n0=(p*mul) >> 8;
			Z80_write(PCMZ80_VOLTBL+0x100*vol+i,n0);
		}
		mul=(mul*scale) >> 8;
	}
	for(int i=0;i<256;i++){
		Z80_write(PCMZ80_VOLTBL+i,0);
	}
}
static int psgScale(int n){
	return n;
}
void initMDZTest(void){
	VDP_setWindowVPos(0,28);
	//
	g_initMute();
	initPcmInfoList();
	//
	SYS_disableInts();
	Z80_init();
	Z80_clear(0, Z80_RAM_LEN, FALSE);
	Z80_requestBus(1);
	//
	const void *z80_prog=z80_pcm1ch;
	size_t z80_prog_size=sizeof(z80_pcm1ch);
	writeBytes_Z80(0,z80_prog,z80_prog_size);
	//
	if(g_pcmz80_mode==6){
	//PSG PCM
		g_use_psgpcm=true;
		g_use_dacpcm=false;
	}else{
	//DAC PCM
		g_use_psgpcm=false;
		g_use_dacpcm=true;
	}
	//
	convertPCM_F_TBL2();
	for(int i=0;i<256;i++){
		u16 n=PCM_F_TBL2[i];
		int n0= n       & 0x0f;
		int n1=(n >> 4) & 0x0f;
		int n2=(n >> 8) & 0x0f;
		n0=psgScale(n0);
		n1=psgScale(n1);
		n2=psgScale(n2);
		n0+=0x90;
		n1+=0xB0;
		n2+=0xD0;
		Z80_write(PCMZ80_PSGBUFF      +i,n0);
		Z80_write(PCMZ80_PSGBUFF+0x100+i,n1);
		Z80_write(PCMZ80_PSGBUFF+0x200+i,n2);
	}
	makeVolTbl();
	
	writeAdr16S_Z80(PCMZ80_MUTE,(u32)g_mute_buff);
	
	
	if(g_use_dacpcm){
		YM2612_enableDAC();
	}else{
		YM2612_disableDAC();
	}
	
	//
	
	Z80_startReset();
	Z80_releaseBus();
	while(Z80_isBusTaken());
	Z80_endReset();
	
	//
	SYS_enableInts();
	//
//	g_initRegPsg();
	//
	YM2612_reset();
	YM2612_enableDAC();
//	YM2612_disableDAC();
	//
	cmdz_bgmdata=(CMDZ_BgmData *)malloc(sizeof(CMDZ_BgmData));
	cmdz_work=(CMDZ_WORK *)malloc(sizeof(CMDZ_WORK));
	CMDZ_WORK_init(cmdz_work);
	//
	cmdz_fmchip.hasPort=&g_hasPort;
	cmdz_fmchip.outReg0=&g_outReg0;
	cmdz_fmchip.outReg =&g_outReg;
	CMDZ_WORK_setFMChip(cmdz_work,&cmdz_fmchip);
	//
	cmdz_pcmdriver.startPCMBuffer=&g_startPCMBuffer;
	cmdz_pcmdriver.setTimer=&g_setTimer;
	cmdz_pcmdriver.getNowMakePCMSize=&g_getNowMakePCMSize;
	cmdz_pcmdriver.getPCMBufferAdr=&g_getPCMBufferAdr;
	cmdz_pcmdriver.allocCnl=&g_allocCnl;
	cmdz_pcmdriver.initCnl=&g_initCnl;
	cmdz_pcmdriver.stopCnl=&g_stopCnl;
	cmdz_pcmdriver.keyOnCnl=&g_keyOnCnl;
	cmdz_pcmdriver.setPanCnl=&g_setPanCnl;
	cmdz_pcmdriver.setNoteCnl=&g_setNoteCnl;
	cmdz_pcmdriver.setVolumeCnl=&g_setVolumeCnl;
	CMDZ_WORK_setPcmDriver(cmdz_work,&cmdz_pcmdriver);
	//
	outFMBuff();
	//
	JOY_setEventHandler(joyEvent_MDZTest);
	g_mainloop_callback=&mainLoop_MDZTest;
}
static void playBgm_MDZTest(void){
	for(int cnl=0;cnl<PCMINFO_MAX;cnl++){
		PcmInfo *pcminfo=&g_pcminfo[cnl];
		pcminfo->flg=1;
		pcminfo->cnl=cnl;
		pcminfo->oto=-1;
		pcminfo->vol=0;
		pcminfo->note=0;
		pcminfo->pan=0;
	}
	fmbuff_index_max=0;
	const MDZ_BIN_DATA *mdz_bin=&g_mdz_bin_data[g_mdz_data_index];
	const void *data=mdz_bin->data;
	size_t size=mdz_bin->size;
	g_pcm_tbl=mdz_bin->pcm_tbl;
	g_adpcm_vol_scale=mdz_bin->vol;
	g_mdz_initret=CMDZ_BgmData_initSet(cmdz_bgmdata,data,size,false);
	g_mdz_playret=CMDZ_WORK_playBgm(cmdz_work,cmdz_bgmdata);
	g_mdz_cnt=0;
	g_wait_num=0;
	outFMBuff();
}
static void fadeBgm_MDZTest(void){
	CMDZ_WORK_fadeBgm(cmdz_work,5);
}
static void stopBgm_MDZTest(void){
	g_initTimeAdd();
	Z80_requestBus(1);
//	stopPcm_Z80(0);
	Z80_releaseBus();
	g_stopPsgVolume();
	CMDZ_WORK_stopBgm(cmdz_work);
	outFMBuff();
	CMDZ_WORK_delete(cmdz_work);
	CMDZ_BgmData_delete(cmdz_bgmdata);
	g_mdz_playret=0;
}
static void endLoop_MDZTest(void){
	free(cmdz_bgmdata);
	free(cmdz_work);
	returnMenu();
}
static void mainLoop_MDZTest(void){
	char m[80];
	sprintf(m,"MDZ Test:%5d",g_mdz_cnt);
	VDP_drawTextBG(WINDOW,m, 0, 0);
	
	char *pcm_filename="---";
	if(cmdz_bgmdata){
		CMDZ_BgmPcmFile *pcmfile=CMDZ_BgmData_getPcmFile(cmdz_bgmdata,1);
		if(pcmfile){
			pcm_filename=pcmfile->file;
		}
	}
	
	sprintf(m,"index:%2d:%5s",g_mdz_data_index,pcm_filename);
	VDP_drawTextBG(WINDOW,m, 0, 1);
	for(int cnl=0;cnl<PCMINFO_MAX;cnl++){
		PcmInfo *pcminfo=&g_pcminfo[cnl];
		if(pcminfo->flg){
			sprintf(m,"cnl(%d),pcm(%2d),vol(%2d),pan(%d),note(%04x)",pcminfo->cnl,pcminfo->oto,pcminfo->vol,pcminfo->pan,pcminfo->note);
			VDP_drawTextBG(WINDOW,m, 0, 2+cnl);
			pcminfo->flg=0;
		}
	}
	
	if(g_mdz_initret && g_mdz_playret){
		g_wait_num--;
		if(g_wait_num<=0){
			while(1){
				CMDZ_WORK_playMain(cmdz_work);
				
				outFMBuff();
				
				g_mdz_cnt++;
				//
				if(g_timeAdd(&g_wait_num)){
					break;
				}
			}
		}
	}
}

static void joyEvent_MDZTest(u16 joy, u16 changed, u16 state){
	int mdz_bottom=(MDZ_BIN_DATA_MAX-1);
	if(changed & state & (BUTTON_RIGHT)){
		g_mdz_data_index++;
		if(g_mdz_data_index>mdz_bottom)g_mdz_data_index=0;
	}
	if(changed & state & (BUTTON_LEFT)){
		g_mdz_data_index--;
		if(g_mdz_data_index<0)g_mdz_data_index=mdz_bottom;
	}
	if(changed & state & (BUTTON_A)){
		stopBgm_MDZTest();
		playBgm_MDZTest();
	}
	if(changed & state & (BUTTON_B)){
		//stopBgm_MDZTest();
		fadeBgm_MDZTest();
	}
	if(changed & state & (BUTTON_C)){
		g_def_pcm_cnl++;
		g_def_pcm_cnl&=3;
	}
	if (changed & state & (BUTTON_START)){
		stopBgm_MDZTest();
		endLoop_MDZTest();
	}
}
