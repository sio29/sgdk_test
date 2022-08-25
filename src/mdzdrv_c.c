/*-----------------------------------------------------------------------------
;	mdzdrv_c.cpp
-----------------------------------------------------------------------------*/
//#define SGDK

#include "mdzdrv_c.h"

#ifdef SGDK
#else
extern "C" {
#endif

/*****************************************************************************
;	define
******************************************************************************/
#define FM_NERIRO_SIZE		25//26
#define FM_NERIRO_OFF		0//1
#define FM_NERIRO_SIZE2		25
#define SSG_NERIRO_SIZE		6
#define ADPCM_NERIRO_SIZE	6
#define PPZ8_NERIRO_SIZE	6

typedef void (CMDZ_CNL_COMMAND)(CMDZ_CNL *cnl,CMDZ_WORK *work);

/*****************************************************************************
;	
******************************************************************************/
//パンテーブル
const static uint8_t g_ppz_pan_tbl[]={0,1,9,5};

//FM音源の音程テーブル(618,1272,2.05825)
const static uint16_t fm_ontei_tbl[]={
	0x026A,0x028F,0x02B6,0x02DF,0x030B,0x0339,
	0x036A,0x039E,0x03D5,0x0410,0x044E,0x048F
};
//SSGの音程テーブル(3816,2022,1.8872403560830860534124629080119)
const static uint16_t ssg_ontei_tbl[]={
	0x0EE8,0x0E12,0x0D48,0x0C89,0x0BD5,0x0B2B,
	0x0A8A,0x09F3,0x0964,0x08DD,0x085E,0x07E6
};
/*
//ADPCMの音程テーブル
const static uint16_t adpcm_ontei_tbl[]={
	0x0000,0x0463,0x0909,0x0DF6,0x132D,0x1864,	//o1
	0x1E8A,0x24BD,0x2B4E,0x3244,0x39A3,0x4173,
	0x49BA,0x527E,0x5BC8,0x65A0,0x700D,0x7b19,	//o2
	0x86CC,0x9336,0xA057,0xAE42,0xBD01,0xCCA2,
	0xC8B4,0xC9CC,0xCAF5,0xCC30,0xCD7E,0xCEDF,	//o3
	0xD056,0xD1E3,0xD387,0xD544,0xD71C,0xD911,
	0xDB22,0xDD53,0xDFA6,0xE21C,0xE4B7,0xE777,	//o4
	0xEA65,0xED7F,0xF0C8,0xF443,0xF7F4,0xFBDC,
	0x0000,0x0463,0x0909,0x0DF6,0x132D,0x1864,	//o5
	0x1E8A,0x24BD,0x2B4E,0x3244,0x39A3,0x4173,
	0x49BA,0x527E,0x5BC8,0x65A0,0x700D,0x7b19,	//o6
	0x86CC,0x9336,0xA057,0xAE42,0xBD01,0xCCA2,
	0xC8B4,0xC9CC,0xCAF5,0xCC30,0xCD7E,0xCEDF,	//o7
	0xD056,0xD1E3,0xD387,0xD544,0xD71C,0xD911,
};
*/
//音程テーブル
const static uint16_t ppz8_ontei_tbl[]={
	0x8000,		//00 c
	0x87A6,		//01 d-
	0x8FB3,		//02 d
	0x9838,		//03 e-
	0xA146,		//04 e
	0xAADE,		//05 f
	0xB4FF,		//06 g-
	0xBFCC,		//07 g
	0xCB34,		//08 a-
	0xD747,		//09 a
	0xE418,		//10 b-
	0xF1A5		//11 b	(1.887847900390625)
};

//FM音源の音量テーブル
const static uint8_t fm_vol_tbl[]={
	0x38,0x34,0x30,0x2A,0x28,0x25,0x22,0x20,
	0x1D,0x1A,0x18,0x15,0x12,0x10,0x0D,0x09,
};
//SSGのミキサー出力データ作成テーブル
const static uint8_t ssg_mixer_tbl[]={
	0x09,	//00001001B,	//0 NONE
	0x08,	//00001000B,	//1 TONE
	0x01,	//00000001B,	//2 NOISE
	0x00,	//00000000B,	//3 TONE+NOISE
};
const static uint8_t ssg_mixer_mask[]={
	0xf6,	//(1111_0110B)
	0xed,	//(1110_1101B)
	0xdb,	//(1101_1011B)
};
//FM音源のアルゴリズムテーブル
const static uint8_t fm_alg_tbl[]={
	0x08,	//00001000B,	//0
	0x08,	//00001000B,	//1
	0x08,	//00001000B,	//2
	0x08,	//00001000B,	//3
	0x0c,	//00001100B,	//4
	0x0e,	//00001110B,	//5
	0x0e,	//00001110B,	//6
	0x0f,	//00001111B,	//7
};

const static uint8_t adpcm_em_vol[]={
//	 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4,	//00
	 4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,	//10
	 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,	//20
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	//30
	 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,	//40
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,	//50
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,	//60
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,	//70
	10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,	//80
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,	//90
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,	//a0
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,	//b0
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,	//c0
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,	//d0
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,	//e0
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,	//f0
};



/*****************************************************************************
;	extern
******************************************************************************/
//CMDZ_Tools
void CMDZ_Tools_stringToLowerCase(char *_dst,const char *_src);
void CMDZ_Tools_removeCommentDarer(char *_dst,const char *_src);
const char *CMDZ_Tools_getCommandName(int command);
const char *CMDZ_Tools_getCnlCateName(int cnl_cate);
//CMDZ_OffsetTbl
void CMDZ_OffsetTbl_init(CMDZ_OffsetTbl *offtbl);
size_t CMDZ_OffsetTbl_size(CMDZ_OffsetTbl *offtbl);
int CMDZ_OffsetTbl_getIndex(CMDZ_OffsetTbl *offtbl,int off);
int CMDZ_OffsetTbl_getBottom(CMDZ_OffsetTbl *offtbl,int off);
int CMDZ_OffsetTbl_getOffset(CMDZ_OffsetTbl *offtbl,int index);
void CMDZ_OffsetTbl_add(CMDZ_OffsetTbl *offtbl,int off,const char *name);
void CMDZ_OffsetTbl_sort(CMDZ_OffsetTbl *offtbl);
void CMDZ_OffsetTbl_print(CMDZ_OffsetTbl *offtbl);
//CMDZ_BinData
void CMDZ_BinData_init(CMDZ_BinData *bindata);
void CMDZ_BinData_free(CMDZ_BinData *bindata);
void CMDZ_BinData_delete(CMDZ_BinData *bindata);
void CMDZ_BinData_alloc(CMDZ_BinData *bindata,size_t _size);
void *CMDZ_BinData_getData(const CMDZ_BinData *bindata);
size_t CMDZ_BinData_getSize(const CMDZ_BinData *bindata);
void CMDZ_BinData_setData(CMDZ_BinData *bindata,const void *_data,size_t _size,bool releaseble_flg);
bool CMDZ_BinData_checkOffset(const CMDZ_BinData *bindata,int offset);
uint8_t CMDZ_BinData_readUint8(const CMDZ_BinData *bindata,uintptr_t off);
uint16_t CMDZ_BinData_readUint16(const CMDZ_BinData *bindata,uintptr_t off);
uint32_t CMDZ_BinData_readUint32(const CMDZ_BinData *bindata,uintptr_t off);
int8_t CMDZ_BinData_readSint8(const CMDZ_BinData *bindata,uintptr_t off);
int16_t CMDZ_BinData_readSint16(const CMDZ_BinData *bindata,uintptr_t off);
int32_t CMDZ_BinData_readSint32(const CMDZ_BinData *bindata,uintptr_t off);
size_t CMDZ_BinData_readBytes(const CMDZ_BinData *bindata,uintptr_t off,void *_data,size_t _size);
size_t CMDZ_BinData_readStr(const CMDZ_BinData *bindata,uintptr_t off,void *_data);
size_t CMDZ_BinData_readStrN(const CMDZ_BinData *bindata,uintptr_t off,void *_data,size_t _size);
//CMDZ_BgmPcmFile
void CMDZ_BgmPcmFile_init(CMDZ_BgmPcmFile *pcmfile);
//CMDZ_BgmComment
void CMDZ_BgmComment_init(CMDZ_BgmComment *comment);
//CMDZ_WAVEOCT
void CMDZ_WAVEOCT_init(CMDZ_WAVEOCT *waveoct,int _wave,int _oct);
//CMDZ_PZILT
void CMDZ_PZILT_init(CMDZ_PZILT *pzit);
//CMDZ_PZIL
void CMDZ_PZIL_init(CMDZ_PZIL *pzil,int _tbl_num);
void CMDZ_PZIL_delete(CMDZ_PZIL *pzil);
//CMDZ_MakePara
void CMDZ_MakePara_init(CMDZ_MakePara *makepara);
//CMDZ_CNLFLG
void CMDZ_CNLFLG_init(CMDZ_CNLFLG *cnlflg);
void CMDZ_CNLFLG_setLFO(CMDZ_CNLFLG *cnlflg,int i,bool n);
bool CMDZ_CNLFLG_getLFO(CMDZ_CNLFLG *cnlflg,int i);
//CMDZ_LFO 
void CMDZ_LFO_init(CMDZ_LFO *lfo);
//CMDZ_ALFO 
void CMDZ_ALFO_init(CMDZ_ALFO *alfo);
//CMDZ_HLFO 
void CMDZ_HLFO_init(CMDZ_HLFO *hlfo);
//CMDZ_VEND 
void CMDZ_VEND_init(CMDZ_VEND *vend);
//CMDZ_ENVFLG 
void CMDZ_ENVFLG_init(CMDZ_ENVFLG *envflg);
//CMDZ_ENV 
void CMDZ_ENV_init(CMDZ_ENV *env);
//CMDZ_APAN 
void CMDZ_APAN_init(CMDZ_APAN *apan);
//CMDZ_PcmTone
void CMDZ_PcmTone_init(CMDZ_PcmTone *pcmtone);
void CMDZ_PcmTone_set(CMDZ_PcmTone *pcmtone,CMDZ_PcmBankTbl *_pzit,CMDZ_PZILT *pzilt);
//CMDZ_PcmInfo
void CMDZ_PcmInfo_init(CMDZ_PcmInfo *pcminfo);
//CMDZ_PcmBank
CMDZ_PcmBankTbl *CMDZ_PcmBank_getTbl(const CMDZ_PcmBank *pcmbank,int index);
int CMDZ_PcmBank_getTblNum(const CMDZ_PcmBank *pcmbank);
//CMDZ_PcmBankTbl
void CMDZ_PcmBankTbl_setLoopStart(CMDZ_PcmBankTbl *tbl,int loop_start);
void CMDZ_PcmBankTbl_setLoopEnd(CMDZ_PcmBankTbl *tbl,int loop_end);
void CMDZ_PcmBankTbl_setLoopFlg(CMDZ_PcmBankTbl *tbl,int flg);
void CMDZ_PcmBankTbl_setStart(CMDZ_PcmBankTbl *tbl,int start);
void CMDZ_PcmBankTbl_setSampleNum(CMDZ_PcmBankTbl *tbl,int end);
void CMDZ_PcmBankTbl_setRate(CMDZ_PcmBankTbl *tbl,int rate);
int CMDZ_PcmBankTbl_getLoopStart(const CMDZ_PcmBankTbl *tbl);
int CMDZ_PcmBankTbl_getLoopEnd(const CMDZ_PcmBankTbl *tbl);

//CMDZ_BgmDataCnl
void CMDZ_BgmDataCnl_init(CMDZ_BgmDataCnl *cnl);
//CMDZ_BgmData
void CMDZ_SetOffsetTbl(CMDZ_OffsetTbl *offtbl,const void *data,size_t _size,bool releaseble_flg);
void CMDZ_BgmData_init(CMDZ_BgmData *bgmr);
bool CMDZ_BgmData_initSet(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg);
void CMDZ_BgmData_delete(CMDZ_BgmData *bgmr);
void CMDZ_BgmData_release(CMDZ_BgmData *bgmr);
void CMDZ_BgmData_setData(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg);
void *CMDZ_BgmData_getDataAdr(const CMDZ_BgmData *bgmr);
size_t CMDZ_BgmData_getDataSize(CMDZ_BgmData *bgmr);
int CMDZ_BgmData_getQFlg(const CMDZ_BgmData *bgmr);
int CMDZ_BgmData_getBaseCnt(const CMDZ_BgmData *bgmr);
bool CMDZ_BgmData_getAdpcmNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmAdpcmNeiro *neiro);
bool CMDZ_BgmData_getFmNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmFmNeiro *neiro);
bool CMDZ_BgmData_getSsgNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmSsgNeiro *neiro);
int CMDZ_BgmData_getFmNeiroNum(const CMDZ_BgmData *bgmr);
int CMDZ_BgmData_getSsgNeiroNum(const CMDZ_BgmData *bgmr);
int CMDZ_BgmData_getAdpcmNeiroNum(const CMDZ_BgmData *bgmr);
int CMDZ_BgmData_getCnlNum(CMDZ_BgmData *bgmr);
CMDZ_BgmDataCnl *CMDZ_BgmData_getCnlTbl(CMDZ_BgmData *bgmr,int i);
CMDZ_BgmPcmFile *CMDZ_BgmData_getPcmFile(CMDZ_BgmData *bgmr,int i);
void CMDZ_BgmData_setPcmBank(CMDZ_BgmData *bgmr,int i,CMDZ_PcmBank *pcmbank);
void CMDZ_BgmData_setComment(CMDZ_BgmData *bgmr,const CMDZ_BgmComment *_comment);
const CMDZ_BgmComment *CMDZ_BgmData_getComment(CMDZ_BgmData *bgmr);
CMDZ_PcmBank *CMDZ_BgmData_getPcmBank(const CMDZ_BgmData *bgmr,int i);
bool CMDZ_BgmData_parseHeader(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg);
//==========================
int CMDZ_addWave(int wave,int add);
//==========================
//CMDZ_WORK
void CMDZ_WORK_init(CMDZ_WORK *work);
void CMDZ_WORK_delete(CMDZ_WORK *work);
void CMDZ_WORK_initSub(CMDZ_WORK *work);
void *CMDZ_WORK_getDataAdr(const CMDZ_WORK *work);
int CMDZ_WORK_getQFlg(CMDZ_WORK *work);
int CMDZ_WORK_allocPcmDriverCnl(CMDZ_WORK *work,int num);
void CMDZ_WORK_initLoopOne(CMDZ_WORK *work,int bank);
bool CMDZ_WORK_stopBgm(CMDZ_WORK *work);
void CMDZ_WORK_feedbackPcmDriverTimer(CMDZ_WORK *work);
void CMDZ_WORK_initFMReg(CMDZ_WORK *work);
void CMDZ_WORK_initSSGReg(CMDZ_WORK *work);
void CMDZ_WORK_stopFM(CMDZ_WORK *work,int port);
void CMDZ_WORK_stopFM2(CMDZ_WORK *work,int port);
void CMDZ_WORK_keySubFM2(CMDZ_WORK *work,int port,int fm_cnl,int n);
void CMDZ_WORK_outSSGMixer(CMDZ_WORK *work);
void CMDZ_WORK_calcTempo(CMDZ_WORK *work);
bool CMDZ_WORK_execFade(CMDZ_WORK *work);

//==========================
//CMDZ_CNL
void CMDZ_CNL_init(CMDZ_CNL *cnl);
void CMDZ_CNL_delete(CMDZ_CNL *cnl);
void CMDZ_CNL_setStopFlg(CMDZ_CNL *cnl,bool n);
bool CMDZ_CNL_getStopFlg(CMDZ_CNL *cnl);
bool CMDZ_CNL_getOneLoopFlg(CMDZ_CNL *cnl);
void CMDZ_CNL_initSub(CMDZ_CNL *cnl);
bool CMDZ_CNL_initCnl(CMDZ_CNL *cnl,CMDZ_WORK *work,const CMDZ_BgmDataCnl *cnltbl,int _cnl_index);
bool CMDZ_CNL_getCnlInfoMess(CMDZ_CNL *cnl,char *m);
bool CMDZ_CNL_stopBgmCnl(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_driverCom(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_keyOff(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_execLFO(CMDZ_CNL *cnl,CMDZ_WORK *work);
bool CMDZ_CNL_parseCommand(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_execOnchoCommand(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_execCommand(CMDZ_CNL *cnl,CMDZ_WORK *work,int command);
void CMDZ_CNL_outRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_rest(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_restSkip(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_outKeyOn(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_getOntei(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei);
void CMDZ_CNL_initAllLFO(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_keyOn(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_keySubFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n);
bool CMDZ_CNL_stopCnl(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_execLFOSub(CMDZ_CNL *cnl,CMDZ_WORK *work,int type);
void CMDZ_CNL_execPitchVend(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_execAutoPan(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_outOnteiVol(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_outOntei(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_outVol(CMDZ_CNL *cnl,CMDZ_WORK *work);
int CMDZ_CNL_execSoftEnv(CMDZ_CNL *cnl,CMDZ_WORK *work);
void CMDZ_CNL_volSubRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n,int and_n);
void CMDZ_CNL_initAPan(CMDZ_CNL *cnl,CMDZ_WORK *work);


/******************************************************************************
;	CMDZ_Tools
******************************************************************************/
int CMDZ_strncmp(const char *m1,const char *m2,size_t num){
	for(int i=0;i<num;i++){
		if(m1[i]!=m2[i])return 1;
	}
	return 0;
}
void *CMDZ_malloc(size_t size){
#ifndef SGDK
	return malloc(size);
#else
	return MEM_alloc((u16)size);
#endif
}
void CMDZ_free(void *p){
#ifndef SGDK
	free(p);
#else
	MEM_free(p);
#endif
}
/******************************************************************************
;	CMDZ_Tools
******************************************************************************/
void CMDZ_Tools_stringToLowerCase(char *_dst,const char *_src){
	const char *src=_src;
	char *dst=_dst;
	while(1){
		char m=*src++;
		if(m>='A' && m<='Z')m=m-'A'+'a';
		*dst++=m;
		if(!m)break;
	}
}
void CMDZ_Tools_removeCommentDarer(char *_dst,const char *_src){
	const char *src=_src;
	char *dst=_dst;
	while(1){
		char m=*src++;
		if(!m)break;
		if(m=='$')break;
		*dst++=m;
	}
	*dst++=0;
}

const char *CMDZ_Tools_getCommandName(int command){
	switch(command){
		case 0x81:return "VolSet";
		case 0x82:return "VolUp";
		case 0x83:return "VolDown";
		case 0x84:return "TEMPO_SET";
		case 0x85:return "JumpCom";
		case 0x86:return "LoopCom";
		case 0x87:return "LoopOut";
		case 0x88:return "QuotaSet";
		case 0x89:return "DetuneSet";
		case 0x8A:return "LFOSet";
		case 0x8B:return "LFOCom";
		case 0x8C:return "OPN_WRITE";
		case 0x8D:return "OtoCom";
		case 0x8E:return "PanSet";
		case 0x8F:return "NOISE_SET";
		case 0x90:return "EnvSet";
		case 0x91:return "VolSet2";
		case 0x92:return "TaiSet";
		case 0x93:return "LoopSet";
		case 0x94:return "SYNC_COM";
		case 0x95:return "WAIT_COM";
		case 0x96:return "FadeCom";
		case 0x97:return "VEND_SET";
		case 0x98:return "PCM_F_SET";
		case 0x99:return "SentDataCom";
		case 0x9A:return "SIcho";
		case 0x9B:return "SuraSet";
		case 0x9C:return "DefLenSet";
		case 0x9D:return "BankSel";
		case 0x9E:return "MIDIEffect";
		case 0x9F:return "VendRange";
		case 0xA0:return "VelocityUp";
		case 0xA1:return "VelocityDown";
		case 0xA2:return "setChannelIndex";
		case 0xA3:return "PPZ_PanSet";
		case 0xA4:return "VelocitySet";
		case 0xA5:return "AutoPanSet";
		case 0xA6:return "RestOffSet";
		case 0xA7:return "TIMERA_COM";
		case 0xA8:return "TempoSet";
		case 0xA9:return "VendSet";
		case 0xAA:return "LoopPointSet";
		case 0xAB:return "LoopIs";
	}
	return NULL;
}
const char *CMDZ_Tools_getCnlCateName(int cnl_cate){
	switch(cnl_cate){
		case _FM_F:return "FM";
		case _SSG_F:return "SSG";
		case _RITHM_F:return "RITHM";
		case _ADPCM_F:return "ADPCM";
		case _PCM68_F:return "PCM68";
		case _PCM86_F:return "PCM86";
		case _PPZ_F:return "PPZ";
		case _PPZ86_F:return "PPZ86";
		case _PPZ8_F:return "PPZ8";
		case _MIDI_F:return "MIDI";
		case _OPX_F:return "OPX";
		case _OPL4_FM_F:return "OPL4_FM";
		case _OPL4_RITHM_F:return "OPL4_RITHM";
		case _OPL4_PCM_F:return "OPL4_PCM";
	}
	return NULL;
}

void CMDZ_Tools_getCommandName2(char *name,int cmd){
	if(cmd==_STOP_COM){
		strcpy(name,"stop");
	}else if(cmd<_COM_TOP){
		if(cmd==_REST_COM){
			strcpy(name,"reset");
		}else{
			strcpy(name,"ontei");
		}
	}else{
		const char *cmd_name=CMDZ_Tools_getCommandName(cmd);
		if(cmd_name){
			strcpy(name,cmd_name);
		}else{
			strcpy(name,"***");
		}
	}
}
/*****************************************************************************
;	CMDZ_OffsetTbl
******************************************************************************/
void CMDZ_OffsetTbl_init(CMDZ_OffsetTbl *offtbl){
#ifndef SGDK
printf("CMDZ_OffsetTbl_init\n");
#endif
	offtbl->tbl_num=0;
}
size_t CMDZ_OffsetTbl_size(CMDZ_OffsetTbl *offtbl){
	return offtbl->tbl_num;
}
int CMDZ_OffsetTbl_getIndex(CMDZ_OffsetTbl *offtbl,int off){
	int num=(int)offtbl->tbl_num;
	if(num>0){
		for(int i=0;i<num;i++){
			if(offtbl->tbl[i].off==off)return i;
		}
	}
	return -1;
}
int CMDZ_OffsetTbl_getBottom(CMDZ_OffsetTbl *offtbl,int off){
	int num=(int)offtbl->tbl_num;
	int index=CMDZ_OffsetTbl_getIndex(offtbl,off);
	if(index<0)return 0;
	if(index>=(num-1))return 0;
	return offtbl->tbl[index+1].off;
}
int CMDZ_OffsetTbl_getOffset(CMDZ_OffsetTbl *offtbl,int index){
	return offtbl->tbl[index].off;
}
void CMDZ_OffsetTbl_add(CMDZ_OffsetTbl *offtbl,int off,const char *name){
	int index=CMDZ_OffsetTbl_getIndex(offtbl,off);
	if(index!=-1)return;
	CMDZ_OffsetTblOne *t=&offtbl->tbl[offtbl->tbl_num];
	t->off=off;
	strcpy(t->name,name);
	offtbl->tbl_num++;
}
void CMDZ_OffsetTbl_sort(CMDZ_OffsetTbl *offtbl){
	int num=(int)offtbl->tbl_num;
	for(int i=0;i<num-1;i++){
		for(int j=i+1;j<num;j++){
			if(offtbl->tbl[i].off > offtbl->tbl[j].off){
				CMDZ_OffsetTblOne t=offtbl->tbl[i];
				offtbl->tbl[i]=offtbl->tbl[j];
				offtbl->tbl[j]=t;
			}
		}
	}
}
void CMDZ_OffsetTbl_print(CMDZ_OffsetTbl *offtbl){
#ifndef SGDK
	int num=(int)offtbl->tbl_num;
	for(int i=0;i<num;i++){
		size_t size=0;
		if(i<(num-1))size=offtbl->tbl[i+1].off-offtbl->tbl[i].off;
		printf("tbloff[%2d]:off(%04x),size(%5d):[%s]\n",i,offtbl->tbl[i].off,(int)size,offtbl->tbl[i].name);
	}
#endif
}
/*****************************************************************************
;	CMDZ_BinData
******************************************************************************/
void CMDZ_BinData_init(CMDZ_BinData *bindata){
	bindata->buff=NULL;
	bindata->buff_size=0;
	bindata->alloc_flg=false;
}
void CMDZ_BinData_free(CMDZ_BinData *bindata){
	if(bindata->buff && bindata->alloc_flg){
		CMDZ_free(bindata->buff);
		bindata->buff=NULL;
	}
}
void CMDZ_BinData_delete(CMDZ_BinData *bindata){
	CMDZ_BinData_free(bindata);
}
void CMDZ_BinData_alloc(CMDZ_BinData *bindata,size_t _size){
	bindata->buff_size=_size;
	bindata->buff=CMDZ_malloc(_size);
	bindata->alloc_flg=true;
}
void *CMDZ_BinData_getData(const CMDZ_BinData *bindata){
	return bindata->buff;
}
size_t CMDZ_BinData_getSize(const CMDZ_BinData *bindata){
	return bindata->buff_size;
}

void CMDZ_BinData_setData(CMDZ_BinData *bindata,const void *_data,size_t _size,bool releaseble_flg){
	if(!releaseble_flg){
		bindata->buff_size=_size;
		bindata->buff=(void *)_data;
		bindata->alloc_flg=false;
	}else{
		CMDZ_BinData_alloc(bindata,_size);
		memcpy(bindata->buff,_data,_size);
		bindata->alloc_flg=true;
	}
}
bool CMDZ_BinData_checkOffset(const CMDZ_BinData *bindata,int offset){
	if(offset<0 || offset>=bindata->buff_size)return false;
	return true;
}

uint8_t CMDZ_BinData_readUint8(const CMDZ_BinData *bindata,uintptr_t off){
	uint8_t *src=(uint8_t *)((uintptr_t)bindata->buff+off);
	uint8_t n=src[0];
	return n;
}
uint16_t CMDZ_BinData_readUint16(const CMDZ_BinData *bindata,uintptr_t off){
	uint8_t *src=(uint8_t *)((uintptr_t)bindata->buff+off);
	uint8_t n0=src[0] & 0xff;
	uint8_t n1=src[1] & 0xff;
	uint16_t n=(n1 << 8) | n0;
	return n;
}
uint32_t CMDZ_BinData_readUint32(const CMDZ_BinData *bindata,uintptr_t off){
	uint8_t *src=(uint8_t *)((uintptr_t)bindata->buff+off);
	uint8_t n0=src[0] & 0xff;
	uint8_t n1=src[1] & 0xff;
	uint8_t n2=src[2] & 0xff;
	uint8_t n3=src[3] & 0xff;
	uint32_t n=(n3 << 24) | (n2 << 16) | (n1 << 8) | n0;
	return n;
}
int8_t CMDZ_BinData_readSint8(const CMDZ_BinData *bindata,uintptr_t off){
	int8_t *src=(int8_t *)((uintptr_t)bindata->buff+off);
	int8_t n=src[0];
	return n;
}
int16_t CMDZ_BinData_readSint16(const CMDZ_BinData *bindata,uintptr_t off){
	int8_t *src=(int8_t *)((uintptr_t)bindata->buff+off);
	uint8_t n0=src[0] & 0xff;
	int8_t  n1=src[1];
	int16_t n=(n1 << 8) | n0;
	return n;
}
int32_t CMDZ_BinData_readSint32(const CMDZ_BinData *bindata,uintptr_t off){
	int8_t *src=(int8_t *)((uintptr_t)bindata->buff+off);
	uint8_t n0=src[0] & 0xff;
	uint8_t n1=src[1] & 0xff;
	uint8_t n2=src[2] & 0xff;
	int8_t  n3=src[3];
	uint32_t n=(n3 << 24) | (n2 << 16) | (n1 << 8) | n0;
	return n;
}
size_t CMDZ_BinData_readBytes(const CMDZ_BinData *bindata,uintptr_t off,void *_data,size_t _size){
	if(!_size)return 0;
	uint8_t *src=(uint8_t *)((uintptr_t)bindata->buff+off);
	memcpy(_data,src,_size);
	return _size;
}
size_t CMDZ_BinData_readStr(const CMDZ_BinData *bindata,uintptr_t off,void *_data){
	int i=0;
	char *moji=(char *)_data;
	while(1){
		uint8_t m=CMDZ_BinData_readUint8(bindata,off+i);i++;
		*moji++=m;
		if(m==0)break;
	}
	return i;
}
size_t CMDZ_BinData_readStrN(const CMDZ_BinData *bindata,uintptr_t off,void *_data,size_t _size){
	char *moji=(char *)_data;
	for(size_t i=0;i<_size;i++){
		uint8_t m=CMDZ_BinData_readUint8(bindata,off+i);
		*moji++=m;
	}
	return _size;
}

/*****************************************************************************
;	
******************************************************************************/
void CMDZ_SetOffsetTbl(CMDZ_OffsetTbl *offtbl,const void *data,size_t _size,bool releaseble_flg){
	CMDZ_BinData bindata;
	CMDZ_BinData_setData(&bindata,data,_size,releaseble_flg);
	CMDZ_OffsetTbl_add(offtbl,0,"top");
	CMDZ_OffsetTbl_add(offtbl,(int)_size,"size");
	
	
	int _fm_oto=CMDZ_BinData_readUint16(&bindata,MDZH_FM_OTO);
	if(_fm_oto<_size){
		CMDZ_OffsetTbl_add(offtbl,_fm_oto,"fm_oto");
	}
	int _ssg_oto=CMDZ_BinData_readUint16(&bindata,MDZH_SSG_OTO);
	if(_ssg_oto<_size){
		CMDZ_OffsetTbl_add(offtbl,_ssg_oto,"ssg_oto");
	}
	int _adpcm_oto=CMDZ_BinData_readUint16(&bindata,MDZH_ADPCM_OTO);
	if(_adpcm_oto<_size){
		CMDZ_OffsetTbl_add(offtbl,_adpcm_oto,"adpcm_oto");
	}
	int pcm1_name_offset=CMDZ_BinData_readUint16(&bindata,MDZH_PCM1_NAME);
	if(pcm1_name_offset!=0xffff){
		CMDZ_OffsetTbl_add(offtbl,pcm1_name_offset,"pcm1");
	}
	int pcm2_name_offset=CMDZ_BinData_readUint16(&bindata,MDZH_PCM2_NAME);
	if(pcm2_name_offset!=0xffff){
		CMDZ_OffsetTbl_add(offtbl,pcm2_name_offset,"pcm2");
	}
	int _comment_offset=CMDZ_BinData_readUint16(&bindata,MDZH_COMMENT);
	if(_comment_offset!=0xffff){
		CMDZ_OffsetTbl_add(offtbl,_comment_offset,"comment");
	}
	int _data_top2=CMDZ_BinData_readUint16(&bindata,MDZH_BGM_DATA2_TOP);
	if(_data_top2!=0xffff){
		CMDZ_OffsetTbl_add(offtbl,_data_top2,"data_top2");
	}
	int _ppz8_oto=CMDZ_BinData_readUint16(&bindata,MDZH_PPZ8_OTO);
	if(_ppz8_oto!=0xffff){
		CMDZ_OffsetTbl_add(offtbl,_ppz8_oto,"ppz8_oto");
	}
	CMDZ_OffsetTbl_add(offtbl,MDZ_HEADER_SIZE,"header_bottom");
	//
	int src_cnl_num=CMDZ_BinData_readUint16(&bindata,MDZH_CNL_NUM);
	if(src_cnl_num>BGM_CNL_MAX)src_cnl_num=BGM_CNL_MAX;
	//
	for(int i=0;i<src_cnl_num;i++){
		int cnl_tbl_index=MDZ_HEADER_SIZE+(i*4);
		int cnl_type  =CMDZ_BinData_readSint8(&bindata,cnl_tbl_index+0);
		int cnl_index =CMDZ_BinData_readUint8(&bindata,cnl_tbl_index+1);
		int cnl_offset=CMDZ_BinData_readUint16(&bindata,cnl_tbl_index+2);
		if(cnl_offset!=0 && cnl_offset!=0xffff){
			char cnl_m[256];
			sprintf(cnl_m,"cnl%d(%s/%d)",i,CMDZ_Tools_getCnlCateName(cnl_type),cnl_index);
			CMDZ_OffsetTbl_add(offtbl,cnl_offset,cnl_m);
		}
	}
	CMDZ_OffsetTbl_sort(offtbl);
	CMDZ_OffsetTbl_print(offtbl);
}


/*****************************************************************************
;	CMDZ_BgmData
******************************************************************************/
void CMDZ_BgmData_init(CMDZ_BgmData *bgmr){
	bgmr->data_top2    =0;
	bgmr->pri          =0;
	bgmr->ex_flg       =0;
	bgmr->qflg         =1;
	bgmr->base_cnt     =192;				//ベースカウント
	bgmr->cnl_num      =0;
	bgmr->fm_oto       =0;
	bgmr->ssg_oto      =0;
	bgmr->adpcm_oto    =0;
	bgmr->ppz8_oto     =0;
	bgmr->fm_oto_num   =0;
	bgmr->ssg_oto_num  =0;
	bgmr->adpcm_oto_num=0;
	bgmr->ppz8_oto_num =0;
	CMDZ_BgmComment_init(&bgmr->comment);
	CMDZ_OffsetTbl_init(&bgmr->offtbl);
	for(int i=0;i<PZI_BANK_MAX;i++){
		bgmr->pzi_tbl[i]=NULL;
		CMDZ_BgmPcmFile_init(&bgmr->pcm_file[i]);
	}
	for(int i=0;i<BGM_CNL_MAX;i++){
		CMDZ_BgmDataCnl_init(&bgmr->cnltbl[i]);
	}
}
bool CMDZ_BgmData_initSet(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg){
	CMDZ_BgmData_init(bgmr);
	return CMDZ_BgmData_parseHeader(bgmr,_data,_size,releaseble_flg);
}
void CMDZ_BgmData_delete(CMDZ_BgmData *bgmr){
	CMDZ_BgmData_release(bgmr);
}
void CMDZ_BgmData_release(CMDZ_BgmData *bgmr){
	CMDZ_BinData_free(&bgmr->bindata);
	
}
void CMDZ_BgmData_setData(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg){
	CMDZ_BgmData_parseHeader(bgmr,_data,_size,releaseble_flg);
}

void *CMDZ_BgmData_getDataAdr(const CMDZ_BgmData *bgmr){
	return CMDZ_BinData_getData(&bgmr->bindata);
}
size_t CMDZ_BgmData_getDataSize(CMDZ_BgmData *bgmr){
	return CMDZ_BinData_getSize(&bgmr->bindata);
}
int CMDZ_BgmData_getQFlg(const CMDZ_BgmData *bgmr){
	return bgmr->qflg;
}
int CMDZ_BgmData_getBaseCnt(const CMDZ_BgmData *bgmr){
	return bgmr->base_cnt;
}
bool CMDZ_BgmData_getAdpcmNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmAdpcmNeiro *neiro){
	if(index<0 || index>=0x80)return false;
	int adr=bgmr->adpcm_oto+(index*ADPCM_NERIRO_SIZE);
	neiro->start=CMDZ_BinData_readUint16(&bgmr->bindata,adr+0);
	neiro->end  =CMDZ_BinData_readUint16(&bgmr->bindata,adr+2);
	neiro->rate =CMDZ_BinData_readUint16(&bgmr->bindata,adr+4);
	return true;
}
bool CMDZ_BgmData_getFmNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmFmNeiro *neiro){
	if(index<0 || index>=0x80)return false;
	int adr=bgmr->fm_oto+(index*FM_NERIRO_SIZE)+FM_NERIRO_OFF;
	for(int i=0;i<FM_NERIRO_SIZE2;i++){
		neiro->data[i]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+i);
	}
	return true;
}
bool CMDZ_BgmData_getSsgNeiro(const CMDZ_BgmData *bgmr,int index,CMDZ_BgmSsgNeiro *neiro){
	if(index<0 || index>=0x80)return false;
	int adr=bgmr->ssg_oto+(index*SSG_NERIRO_SIZE);
	neiro->data[0]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+0);
	neiro->data[1]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+1);
	neiro->data[2]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+2);
	neiro->data[3]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+3);
	neiro->data[4]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+4);
	neiro->data[5]=CMDZ_BinData_readUint8(&bgmr->bindata,adr+5);
	return true;
}
int CMDZ_BgmData_getFmNeiroNum(const CMDZ_BgmData *bgmr){
	return bgmr->fm_oto_num;
}
int CMDZ_BgmData_getSsgNeiroNum(const CMDZ_BgmData *bgmr){
	return bgmr->ssg_oto_num;
}
int CMDZ_BgmData_getAdpcmNeiroNum(const CMDZ_BgmData *bgmr){
	return bgmr->adpcm_oto_num;
}
int CMDZ_BgmData_getCnlNum(CMDZ_BgmData *bgmr){
//	return BGM_CNL_MAX;
	return bgmr->cnl_num;
}
CMDZ_BgmDataCnl *CMDZ_BgmData_getCnlTbl(CMDZ_BgmData *bgmr,int i){
	if(i<0 || i>=CMDZ_BgmData_getCnlNum(bgmr))return NULL;
	return (CMDZ_BgmDataCnl *)&bgmr->cnltbl[i];
}
CMDZ_BgmPcmFile *CMDZ_BgmData_getPcmFile(CMDZ_BgmData *bgmr,int i){
	if(i<0 || i>=PZI_BANK_MAX)return NULL;
	return (CMDZ_BgmPcmFile *)&bgmr->pcm_file[i];
}

void CMDZ_BgmData_setPcmBank(CMDZ_BgmData *bgmr,int i,CMDZ_PcmBank *pcmbank){
	if(i<0 || i>=PZI_BANK_MAX)return;
	bgmr->pzi_tbl[i]=pcmbank;
}
void CMDZ_BgmData_setComment(CMDZ_BgmData *bgmr,const CMDZ_BgmComment *_comment){
	bgmr->comment=*_comment;
}
const CMDZ_BgmComment *CMDZ_BgmData_getComment(CMDZ_BgmData *bgmr){
	return &bgmr->comment;
}

CMDZ_PcmBank *CMDZ_BgmData_getPcmBank(const CMDZ_BgmData *bgmr,int i){
	if(i<0 || i>=PZI_BANK_MAX)return NULL;
	return bgmr->pzi_tbl[i];
}

#ifndef SGDK
extern bool g_usePSGPcm(void);
extern bool g_useDACPcm(void);
#endif
//MDZヘッダーの獲得
bool CMDZ_BgmData_parseHeader(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg){
	if(!_data || !_size){
#ifndef SGDK
		printf("CMDZ_BgmData_parseHeader error!!:data==NULL or size==0 \n");
#endif
		return false;
	}
	CMDZ_BinData_setData(&bgmr->bindata,_data,_size,releaseble_flg);
	//MDZHEAD mdzh=data;
	//if(CMDZ_	strncmp(mdzh.m,"MDZS",4)!=0)return false;
	char head_m[4+1];
	CMDZ_BinData_readStrN(&bgmr->bindata,MDZH_STRING,head_m,4);
	head_m[4]=0;
	if(CMDZ_strncmp(head_m,"MDZS",4)!=0){
#ifndef SGDK
		printf("CMDZ_BgmData_parseHeader error!!:Header String Error:%s\n",head_m);
#endif
		return false;
	}
	//int ver=mdzh.ver;
	int ver=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_VER);
	if(ver!=_MDZS_VER){
#ifndef SGDK
		printf("CMDZ_BgmData_parseHeader error!!:Header Version Error\n");
#endif
		return false;
	}
	CMDZ_OffsetTbl_add(&bgmr->offtbl,0,"top");
	CMDZ_OffsetTbl_add(&bgmr->offtbl,(int)_size,"size");
//printf("ver(%d)\n",ver);
	//拡張フラグ
	bgmr->ex_flg=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_EX_FLG);
//printf("ex_flg(%d)\n",ex_flg);
	//Q
	int _qflg=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_QFLG);
	if(_qflg==0){
		bgmr->qflg=8;
	}else{
		bgmr->qflg=1;
	}
//printf("qflg(%d)\n",bgmr->qflg);
	bgmr->fm_oto   =0;
	bgmr->ssg_oto  =0;
	bgmr->adpcm_oto=0;
	bgmr->ppz8_oto =0;
	bgmr->fm_oto_num   =0;
	bgmr->ssg_oto_num  =0;
	bgmr->adpcm_oto_num=0;
	bgmr->ppz8_oto_num =0;
	//FM音
	int _fm_oto=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_FM_OTO);
	if(_fm_oto<_size){
		bgmr->fm_oto=_fm_oto;
		CMDZ_OffsetTbl_add(&bgmr->offtbl,bgmr->fm_oto,"fm_oto");
#ifndef SGDK
printf("fm_oto(%08x)\n",bgmr->fm_oto);
#endif
	}
	//SSG音
	int _ssg_oto=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_SSG_OTO);
	if(_ssg_oto<_size){
		bgmr->ssg_oto=_ssg_oto;
		CMDZ_OffsetTbl_add(&bgmr->offtbl,bgmr->ssg_oto,"ssg_oto");
#ifndef SGDK
printf("ssg_oto(%08x)\n",bgmr->ssg_oto);
#endif
	}
	//ADPCM音
	int _adpcm_oto=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_ADPCM_OTO);
	if(_adpcm_oto<_size){
		bgmr->adpcm_oto=_adpcm_oto;
		CMDZ_OffsetTbl_add(&bgmr->offtbl,bgmr->adpcm_oto,"adpcm_oto");
#ifndef SGDK
printf("adpcm_oto(%08x)\n",bgmr->adpcm_oto);
#endif
	}
	//PCM1名前
	int pcm1_name_offset=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_PCM1_NAME);
	if(pcm1_name_offset!=0xffff){
		CMDZ_OffsetTbl_add(&bgmr->offtbl,pcm1_name_offset,"pcm1");
		CMDZ_BinData_readStr(&bgmr->bindata,pcm1_name_offset,bgmr->pcm_file[0].file);
		CMDZ_Tools_stringToLowerCase(bgmr->pcm_file[0].file,bgmr->pcm_file[0].file);
#ifndef SGDK
printf("pcm1(%s)\n",bgmr->pcm_file[0].file);
#endif
	}
	//PCM1種別
	int _pcm1_cate=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_PCM1_CATE);
	bgmr->pcm_file[0].cate=_pcm1_cate & 1;
//printf("pcm1_cate(%d)\n",bgmr->pcm_file[0].cate);
	//PCM2名前
	int pcm2_name_offset=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_PCM2_NAME);
	if(pcm2_name_offset!=0xffff){
		CMDZ_OffsetTbl_add(&bgmr->offtbl,pcm2_name_offset,"pcm2");
		CMDZ_BinData_readStr(&bgmr->bindata,pcm2_name_offset,bgmr->pcm_file[1].file);
		CMDZ_Tools_stringToLowerCase(bgmr->pcm_file[1].file,bgmr->pcm_file[1].file);
#ifndef SGDK
printf("pcm2(%s)\n",bgmr->pcm_file[1].file);
#endif
	}
	//PCM2種別
	bgmr->pcm_file[1].cate=0;
	//コメント
	int _comment_offset=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_COMMENT);
	if(_comment_offset!=0xffff){
		CMDZ_OffsetTbl_add(&bgmr->offtbl,_comment_offset,"comment");
		CMDZ_BinData_readStr(&bgmr->bindata,_comment_offset,bgmr->comment.comment);
		CMDZ_Tools_removeCommentDarer(bgmr->comment.comment,bgmr->comment.comment);
#ifndef SGDK
printf("comment(%s)\n",bgmr->comment.comment);
#endif
	}
	//ベース
	int _base_cnt=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_BASE);
	//if(_base_cnt==0xffff)_base_cnt=192;
	if(_base_cnt==0xffff)_base_cnt=96;
	bgmr->base_cnt=_base_cnt;
#ifndef SGDK
printf("base_cnt(%d)\n",bgmr->base_cnt);
#endif
	//チャネル数
	int src_cnl_num=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_CNL_NUM);
	if(src_cnl_num>BGM_CNL_MAX)src_cnl_num=BGM_CNL_MAX;
#ifndef SGDK
printf("src_cnl_num(%d)\n",src_cnl_num);
#endif
	//プライオリティ
	int _pri=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_PRIORYTY_FLG);
	if(_pri!=0xffff){
		bgmr->pri=_pri;
#ifndef SGDK
printf("pri(%d)\n",bgmr->pri);
#endif
	}
	//データ先頭2
	int _data_top2=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_BGM_DATA2_TOP);
	if(_data_top2!=0xffff){
		bgmr->data_top2=_data_top2;
#ifndef SGDK
printf("data_top2(%d)\n",bgmr->data_top2);
#endif
		CMDZ_OffsetTbl_add(&bgmr->offtbl,bgmr->data_top2,"data_top2");
	}
	//
	int _ppz8_oto=CMDZ_BinData_readUint16(&bgmr->bindata,MDZH_PPZ8_OTO);
	if(_ppz8_oto!=0xffff){
		bgmr->ppz8_oto=_ppz8_oto;
#ifndef SGDK
printf("ppz8_oto(%d)\n",bgmr->ppz8_oto);
#endif
		CMDZ_OffsetTbl_add(&bgmr->offtbl,bgmr->ppz8_oto,"ppz8_oto");
	}
	//
	for(int i=0;i<BGM_CNL_MAX;i++){
		CMDZ_BgmDataCnl_init(&bgmr->cnltbl[i]);
	}
	//
	CMDZ_OffsetTbl_add(&bgmr->offtbl,MDZ_HEADER_SIZE,"header_bottom");
	
	bgmr->cnl_num=0;
	for(int i=0;i<src_cnl_num;i++){
		int cnl_tbl_index=MDZ_HEADER_SIZE+(i*4);
		int cnl_type  =CMDZ_BinData_readSint8(&bgmr->bindata,cnl_tbl_index+0);
		int cnl_index =CMDZ_BinData_readUint8(&bgmr->bindata,cnl_tbl_index+1);
		int cnl_offset=CMDZ_BinData_readUint16(&bgmr->bindata,cnl_tbl_index+2);
		if(cnl_offset!=0 && cnl_offset!=0xffff){
			char cnl_m[256];
			sprintf(cnl_m,"cnl%d",i);
			CMDZ_OffsetTbl_add(&bgmr->offtbl,cnl_offset,cnl_m);
		}
		if(cnl_type==-1)continue;
		//PPZ8チャネル以外スキップ
#ifndef SGDK
if(cnl_type==_FM_F){
	if(g_useDACPcm()){
		if(cnl_index>=5)continue;
	}
}else if(cnl_type==_SSG_F){
	if(g_usePSGPcm()){
		continue;
	}
}else if(cnl_type==_ADPCM_F){
//	continue;
}else if(cnl_type==_RITHM_F){
//	continue;
}else{
	continue;
}
#endif
//if(cnl_type!=_PPZ8_F && cnl_type!=_ADPCM_F && cnl_type!=_RITHM_F && cnl_type!=_FM_F && cnl_type!=_SSG_F)continue;
//if(cnl_type!=_ADPCM_F && cnl_type!=_RITHM_F )continue;
		//チャンネルオーバー
		if(cnl_index>=BGM_CNL_MAX)continue;
		//最初のデータが停止だったら
		if(!CMDZ_BinData_checkOffset(&bgmr->bindata,cnl_offset))continue;
		int first_data=CMDZ_BinData_readUint8(&bgmr->bindata,cnl_offset);
		if(first_data==0xff)continue;
		//
		bgmr->cnltbl[bgmr->cnl_num].type  =cnl_type;
		bgmr->cnltbl[bgmr->cnl_num].index =cnl_index;
		bgmr->cnltbl[bgmr->cnl_num].offset=(void *)((uintptr_t)CMDZ_BinData_getData(&bgmr->bindata)+cnl_offset);
		bgmr->cnl_num++;
		//
#ifndef SGDK
		printf(" cnl[%d]:type(%s),index(%d),offset(%08x)\n",bgmr->cnl_num-1,CMDZ_Tools_getCnlCateName(cnl_type),cnl_index,cnl_offset);
#endif
	}
	//
	CMDZ_OffsetTbl_sort(&bgmr->offtbl);
	CMDZ_OffsetTbl_print(&bgmr->offtbl);
	if(bgmr->fm_oto){
		int bottom=CMDZ_OffsetTbl_getBottom(&bgmr->offtbl,bgmr->fm_oto);
		bgmr->fm_oto_num=(bottom-bgmr->fm_oto)/FM_NERIRO_SIZE;
#ifndef SGDK
		int dif=(bottom-bgmr->fm_oto);
		printf("fm_oto(%04x-%04x),dif(%d),num(%d)\n",bgmr->fm_oto,bottom,dif,bgmr->fm_oto_num);
#endif
	}
	if(bgmr->ssg_oto){
		int bottom=CMDZ_OffsetTbl_getBottom(&bgmr->offtbl,bgmr->ssg_oto);
		bgmr->ssg_oto_num=(bottom-bgmr->ssg_oto)/SSG_NERIRO_SIZE;
#ifndef SGDK
		int dif=(bottom-bgmr->ssg_oto);
		printf("ssg_oto(%04x-%04x),dif(%d),num(%d)\n",bgmr->ssg_oto,bottom,dif,bgmr->ssg_oto_num);
#endif
	}
	if(bgmr->adpcm_oto){
		int bottom=CMDZ_OffsetTbl_getBottom(&bgmr->offtbl,bgmr->adpcm_oto);
		bgmr->adpcm_oto_num=(bottom-bgmr->adpcm_oto)/ADPCM_NERIRO_SIZE;
#ifndef SGDK
		int dif=(bottom-bgmr->adpcm_oto);
		printf("adpcm_oto(%04x-%04x),dif(%d),num(%d)\n",bgmr->adpcm_oto,bottom,dif,bgmr->adpcm_oto_num);
#endif
	}
	if(bgmr->ppz8_oto){
		int bottom=CMDZ_OffsetTbl_getBottom(&bgmr->offtbl,bgmr->ppz8_oto);
		bgmr->ppz8_oto_num=(bottom-bgmr->ppz8_oto)/PPZ8_NERIRO_SIZE;
#ifndef SGDK
		int dif=(bottom-bgmr->ppz8_oto);
		printf("ppz8_oto(%04x-%04x),dif(%d),num(%d)\n",bgmr->ppz8_oto,bottom,dif,bgmr->ppz8_oto_num);
#endif
	}
	
	
//	printf("CMDZ_BgmData_parseHeader ok!!\n");
	return true;
}
/*****************************************************************************
;	CMDZ_BgmDataCnl 
******************************************************************************/
void CMDZ_BgmDataCnl_init(CMDZ_BgmDataCnl *cnl){
	cnl->type  =-1;
	cnl->index =-1;
	cnl->offset=NULL;
}
/*****************************************************************************
;	CMDZ_BgmPcmFile
******************************************************************************/
void CMDZ_BgmPcmFile_init(CMDZ_BgmPcmFile *pcmfile){
	pcmfile->file[0]=0;
	pcmfile->cate=0;
}
/*****************************************************************************
;	CMDZ_BgmComment
******************************************************************************/
void CMDZ_BgmComment_init(CMDZ_BgmComment *comment){
	comment->comment[0]=0;
}
/*****************************************************************************
;	CMDZ_WAVEOCT
******************************************************************************/
void CMDZ_WAVEOCT_init(CMDZ_WAVEOCT *waveoct,int _wave,int _oct){
	waveoct->wave=_wave;
	waveoct->oct=_oct;
}
/*****************************************************************************
;	CMDZ_PZILT
******************************************************************************/
void CMDZ_PZILT_init(CMDZ_PZILT *pzit){
	pzit->loop_start=-1;
	pzit->loop_end  =-1;
}
/*****************************************************************************
;	CMDZ_PZIL
******************************************************************************/
/*
void CMDZ_PZIL_init(CMDZ_PZIL *pzil){
	pzil->pcmbank=NULL;
	pzil->tbl=NULL;
	pzil->tbl_num=0;
}
*/
void CMDZ_PZIL_init(CMDZ_PZIL *pzil,int _tbl_num){
	pzil->pcmbank=NULL;
	pzil->tbl_num=_tbl_num;
	pzil->tbl=(CMDZ_PZILT *)CMDZ_malloc(sizeof(CMDZ_PZILT)*_tbl_num);
	for(int i=0;i<pzil->tbl_num;i++){
		CMDZ_PZILT_init(&pzil->tbl[i]);
	}
}
void CMDZ_PZIL_delete(CMDZ_PZIL *pzil){
	if(pzil->tbl){
		CMDZ_free(pzil->tbl);
		pzil->tbl=NULL;
	}
}
/*****************************************************************************
;	CMDZ_MakePara
******************************************************************************/
void CMDZ_MakePara_init(CMDZ_MakePara *makepara){
}
/*****************************************************************************
;	CMDZ_CNLFLG 
******************************************************************************/
void CMDZ_CNLFLG_init(CMDZ_CNLFLG *cnlflg){
	cnlflg->tai=false;			//タイ
	cnlflg->tai2=false;			//タイ2
	cnlflg->stop=true;			//停止
	cnlflg->rest=false;			//休符
	cnlflg->kon=false;			//キーオン
	cnlflg->koff=false;			//キーオフ
	cnlflg->start=false;		//スタート
	cnlflg->vend=false;			//ベンド
	cnlflg->lfo=0;
	cnlflg->alfo=false;			//A_LFO
	cnlflg->hlfo=false;			//H_LFO
	cnlflg->kon_r=false;		//キーオン
	cnlflg->sura=false;			//スラー
	cnlflg->sura2=false;		//スラー２
	cnlflg->apan=false;			//APAN
	cnlflg->rest_off=false;		//REST_OFF
	cnlflg->oneloop=false;		//1周した
}
void CMDZ_CNLFLG_setLFO(CMDZ_CNLFLG *cnlflg,int i,bool n){
	int f=1 << i;
	if(n){
		cnlflg->lfo|=f;
	}else{
		cnlflg->lfo&=(~f & 7);
	}
}
bool CMDZ_CNLFLG_getLFO(CMDZ_CNLFLG *cnlflg,int i){
	int f=1 << i;
	return cnlflg->lfo & f;
}
/*****************************************************************************
;	CMDZ_LFO
******************************************************************************/
void CMDZ_LFO_init(CMDZ_LFO *lfo){
	lfo->md=0;
	lfo->md_cnt=0;
}
/*****************************************************************************
;	CMDZ_ALFO
******************************************************************************/
void CMDZ_ALFO_init(CMDZ_ALFO *alfo){
}
/*****************************************************************************
;	CMDZ_HLFO
******************************************************************************/
void CMDZ_HLFO_init(CMDZ_HLFO *hlfo){
}
/*****************************************************************************
;	CMDZ_VEND
******************************************************************************/
void CMDZ_VEND_init(CMDZ_VEND *vend){
	vend->md_cnt=0;
}
/*****************************************************************************
;	CMDZ_ENVFLG
******************************************************************************/
void CMDZ_ENVFLG_init(CMDZ_ENVFLG *envflg){
	envflg->ar=false;
	envflg->dr=false;
	envflg->sr=false;
}
/*****************************************************************************
;	CMDZ_ENV
******************************************************************************/
void CMDZ_ENV_init(CMDZ_ENV *env){
	CMDZ_ENVFLG_init(&env->flg);
	env->vol2=0;
	env->sv=0xff;				//SV
	env->ar=0xff;				//AR
	env->dr=0xff;				//DR
	env->sl=0xff;				//SL
	env->sr=0;					//SR
	env->rr=0xff;				//RR
}
/*****************************************************************************
;	CMDZ_APAN
******************************************************************************/
void CMDZ_APAN_init(CMDZ_APAN *apan){
	apan->md=0;
	apan->md_cnt=0;
}
/*****************************************************************************
;	CMDZ_PcmTone
******************************************************************************/
void CMDZ_PcmTone_init(CMDZ_PcmTone *pcmtone){
	pcmtone->pzit=NULL;
	pcmtone->start=0;
	pcmtone->end  =0;
}
void CMDZ_PcmTone_set(CMDZ_PcmTone *pcmtone,CMDZ_PcmBankTbl *_pzit,CMDZ_PZILT *pzilt){
	pcmtone->pzit=_pzit;
}
/*****************************************************************************
;	CMDZ_PcmInfo
******************************************************************************/
void CMDZ_PcmInfo_init(CMDZ_PcmInfo *pcminfo){
}
/*****************************************************************************
;	CMDZ_PcmBank
******************************************************************************/
//※
CMDZ_PcmBankTbl *CMDZ_PcmBank_getTbl(const CMDZ_PcmBank *pcmbank,int index){
	//pcmbank->
	return NULL;
}
int CMDZ_PcmBank_getTblNum(const CMDZ_PcmBank *pcmbank){
	return 0;
}
/*****************************************************************************
;	CMDZ_PcmBankTbl
******************************************************************************/
void CMDZ_PcmBankTbl_setLoopStart(CMDZ_PcmBankTbl *tbl,int loop_start){
}
void CMDZ_PcmBankTbl_setLoopEnd(CMDZ_PcmBankTbl *tbl,int loop_end){
}
void CMDZ_PcmBankTbl_setLoopFlg(CMDZ_PcmBankTbl *tbl,int flg){
}
void CMDZ_PcmBankTbl_setStart(CMDZ_PcmBankTbl *tbl,int start){
}
void CMDZ_PcmBankTbl_setSampleNum(CMDZ_PcmBankTbl *tbl,int end){
}
void CMDZ_PcmBankTbl_setRate(CMDZ_PcmBankTbl *tbl,int rate){
}
int CMDZ_PcmBankTbl_getLoopStart(const CMDZ_PcmBankTbl *tbl){
	return 0;
}
int CMDZ_PcmBankTbl_getLoopEnd(const CMDZ_PcmBankTbl *tbl){
	return 0;
}

/*****************************************************************************
;	CMDZ_WORK
******************************************************************************/
void CMDZ_WORK_init(CMDZ_WORK *work){
	work->pcmdriver=NULL;
	work->fmchip=NULL;
	work->rithm_pcmbank=NULL;
	work->bgmr=NULL;
	work->cnl_num=0;
	for(int i=0;i<PZI_BANK_MAX;i++)work->pzi_lp[i]=NULL;
	for(int i=0;i<BGM_CNL_MAX;i++)work->cnlwork[i]=NULL;
	for(int i=0;i<BGM_CNL_MAX;i++)work->keyon_tbl[i]=0;
	CMDZ_WORK_initSub(work);
}
void CMDZ_WORK_delete(CMDZ_WORK *work){
	CMDZ_WORK_releaseCnlWork(work);
	
	
	
}
//初期化
void CMDZ_WORK_initSub(CMDZ_WORK *work){
	//initBgmWork();
	work->bgm_state=0;					//BGM停止
	work->soutai_tempo=0;					//相対テンポ
	work->fade_cnt=0;						//フェードカウンタ
	work->fade_cnt2=0;					//
	work->fade_vol =0;					//
	work->src_tempo=DEF_TEMPO;			//元のテンポ
	work->now_tempo=DEF_TEMPO;			//実際のテンポ
	work->init_flg=1;						//初期化された
	work->bgm_state=0;					//BGM停止
	work->loop_is_flg=0;					//ループクリア
	work->loop_is_adr=-1;					//ループだアドレス
	work->end_is_adr=-1;					//演奏終了アドレス
	work->pause_flg=0;					//
	work->one_loop_flg=0;					//
	work->sent_data=0;					//外部出力データ
	work->key_check_flg=0;				//キーチェックフラグ
}

void CMDZ_WORK_setFMChip(CMDZ_WORK *work,CMDZ_FMChip *_fmchip){
	work->fmchip=_fmchip;
}
void CMDZ_WORK_setPcmDriver(CMDZ_WORK *work,CMDZ_PcmDriver *_pcmdriver){
	work->pcmdriver=_pcmdriver;
}
void *CMDZ_WORK_getDataAdr(const CMDZ_WORK *work){
	if(!work->bgmr)return NULL;
	return CMDZ_BgmData_getDataAdr(work->bgmr);
}
int CMDZ_WORK_getQFlg(CMDZ_WORK *work){
	if(!work->bgmr)return 1;
	return CMDZ_BgmData_getQFlg(work->bgmr);
}
int CMDZ_WORK_getBaseCnt(CMDZ_WORK *work){
	if(!work->bgmr)return 96;
	return CMDZ_BgmData_getBaseCnt(work->bgmr);
}
CMDZ_PcmBank *CMDZ_WORK_getPcmBank(const CMDZ_WORK *work,int bank){
	if(bank<0 || bank>=PZI_BANK_MAX)return NULL;
	if(!work->bgmr)return NULL;
	return CMDZ_BgmData_getPcmBank(work->bgmr,bank);
}
CMDZ_PcmBankTbl *CMDZ_WORK_getPcmBankTbl(const CMDZ_WORK *work,int bank,int index){
	CMDZ_PcmBank *pcmbank=CMDZ_WORK_getPcmBank(work,bank);
	if(!pcmbank)return NULL;
	return CMDZ_PcmBank_getTbl(pcmbank,index);
}

bool CMDZ_WORK_getAdpcmNeiro(CMDZ_WORK *work,int index,CMDZ_BgmAdpcmNeiro *neiro){
	if(!work->bgmr)return false;
	return CMDZ_BgmData_getAdpcmNeiro(work->bgmr,index,neiro);
}
bool CMDZ_WORK_getFmNeiro(CMDZ_WORK *work,int index,CMDZ_BgmFmNeiro *neiro){
	if(!work->bgmr)return false;
	return CMDZ_BgmData_getFmNeiro(work->bgmr,index,neiro);
}
bool CMDZ_WORK_getSsgNeiro(CMDZ_WORK *work,int index,CMDZ_BgmSsgNeiro *neiro){
	if(!work->bgmr)return false;
	return CMDZ_BgmData_getSsgNeiro(work->bgmr,index,neiro);
}
CMDZ_PZIL *CMDZ_WORK_getPziLoop(CMDZ_WORK *work,int bank){
	if(bank<0 || bank>=PZI_BANK_MAX)return NULL;
	return work->pzi_lp[bank];
}
CMDZ_PZILT *CMDZ_WORK_getPziLoopTable(CMDZ_WORK *work,int bank,int index){
	CMDZ_PZIL *pzil=CMDZ_WORK_getPziLoop(work,bank);
	if(!pzil)return NULL;
	if(index<0 || index>=pzil->tbl_num)return NULL;
	if(!pzil->tbl)return NULL;
	return &pzil->tbl[index];
}
int CMDZ_WORK_getFMVolFlg(CMDZ_WORK *work){
	return work->fm_vol_flg;
}
int CMDZ_WORK_getFadeVol(CMDZ_WORK *work){
	if(work->fade_cnt!=0){
		return work->fade_vol;
	}else{
		return 0;
	}
}
void CMDZ_WORK_setSrcTempo(CMDZ_WORK *work,int n){
	work->src_tempo=n;
}
void CMDZ_WORK_setLoopIsFlg(CMDZ_WORK *work,int n){
	work->loop_is_flg=n;
}
bool CMDZ_WORK_getOneLoopFlg(CMDZ_WORK *work){
	return work->one_loop_flg;
}
int CMDZ_WORK_getP8Data(CMDZ_WORK *work){
	return work->p8_data;
}
void CMDZ_WORK_setSsgNoise(CMDZ_WORK *work,int n){
	work->ssg_noise=n;
}
int CMDZ_WORK_getSsgMixer(CMDZ_WORK *work){
	return work->ssg_mixer;
}
void CMDZ_WORK_setSsgMixer(CMDZ_WORK *work,int n){
	work->ssg_mixer=n;
}
void CMDZ_WORK_setSendData(CMDZ_WORK *work,int n){
	work->sent_data=n;
}
int CMDZ_WORK_getCnlNum(CMDZ_WORK *work){
	return work->cnl_num;
}
void CMDZ_WORK_clearCnlNum(CMDZ_WORK *work){
	work->cnl_num=0;
}
CMDZ_CNL *CMDZ_WORK_getCnlWork(CMDZ_WORK *work,int i){
	if(i<0 || i>=CMDZ_WORK_getCnlNum(work))return NULL;
	return (CMDZ_CNL *)work->cnlwork[i];
}
CMDZ_CNL *CMDZ_WORK_newCnlWork(CMDZ_WORK *work,const CMDZ_BgmDataCnl *cnltbl,int index){
	CMDZ_CNL *cnl=(CMDZ_CNL *)CMDZ_malloc(sizeof(CMDZ_CNL));
	CMDZ_CNL_initCnl(cnl,work,cnltbl,index);
	work->cnlwork[work->cnl_num]=cnl;
	work->cnl_num++;
	return cnl;
}
bool CMDZ_WORK_getCnlInfoMess(CMDZ_WORK *work,int i,char *m){
	CMDZ_CNL *cnl=CMDZ_WORK_getCnlWork(work,i);
	if(!cnl)return false;
	char mm[256]={0};
	CMDZ_CNL_getCnlInfoMess(cnl,mm);
	sprintf(m,"cnl[%2d]:%s",i,mm);
	return true;
}
void CMDZ_WORK_setRithmPcmBank(CMDZ_WORK *work,CMDZ_PcmBank *pcmbank){
	work->rithm_pcmbank=pcmbank;
}
CMDZ_PcmBank *CMDZ_WORK_getRithmPcmBank(CMDZ_WORK *work){
	return work->rithm_pcmbank;
}
//pcm loop設定
void CMDZ_WORK_setPcmLoopPoint(CMDZ_WORK *work,int bank,int index,int loop_start,int loop_end){
	CMDZ_PcmBank *pzi=CMDZ_WORK_getPcmBank(work,bank);
	if(!pzi)return;
	CMDZ_PcmBankTbl *tbl=CMDZ_PcmBank_getTbl(pzi,index);
	if(!tbl)return;
	CMDZ_PcmBankTbl_setLoopStart(tbl,loop_start);
	CMDZ_PcmBankTbl_setLoopEnd(tbl,loop_end);
	CMDZ_PcmBankTbl_setLoopFlg(tbl,1);
}
void CMDZ_WORK_setAdpcmNeiro(CMDZ_WORK *work,int bank,int index,const CMDZ_BgmAdpcmNeiro *adpcm_neiro){
		CMDZ_PcmBank *pzi=CMDZ_WORK_getPcmBank(work,bank);
		if(!pzi)return;
		CMDZ_PcmBankTbl *tbl=CMDZ_PcmBank_getTbl(pzi,index);
		if(!tbl)return;
		int start=adpcm_neiro->start*64;
		int end  =((adpcm_neiro->end-adpcm_neiro->start)*64);
		int rate =adpcm_neiro->rate >> 1;
		CMDZ_PcmBankTbl_setStart(tbl,start);
		CMDZ_PcmBankTbl_setSampleNum(tbl,end);
		CMDZ_PcmBankTbl_setRate(tbl,rate);
}

void CMDZ_WORK_releaseCnlWork(CMDZ_WORK *work){
	for(int i=0;i<CMDZ_WORK_getCnlNum(work);i++){
		CMDZ_CNL *cnl=CMDZ_WORK_getCnlWork(work,i);
		if(cnl){
			CMDZ_CNL_init(cnl);
			CMDZ_CNL_setStopFlg(cnl,true);
			CMDZ_CNL_delete(cnl);
			CMDZ_free(cnl);
			work->cnlwork[i]=NULL;
		}
	}
	work->cnl_num=0;
}
//BGM_WORKの初期化
void CMDZ_WORK_initBgmWork(CMDZ_WORK *work){
	work->bgm_state=0;					//BGM停止
	CMDZ_WORK_releaseCnlWork(work);
}
//すべてのPZIループポインタの初期化
void CMDZ_WORK_initLoopAll(CMDZ_WORK *work){
	for(int i=0;i<PZI_BANK_MAX;i++){
		CMDZ_WORK_initLoopOne(work,i);
	}
}
//一つのPZIのループポインタの初期化
void CMDZ_WORK_initLoopOne(CMDZ_WORK *work,int bank){
	if(bank<0 || bank>=PZI_BANK_MAX)return;
	if(!work->bgmr)return;
	CMDZ_PcmBank *pzi=CMDZ_BgmData_getPcmBank(work->bgmr,bank);
	if(!pzi)return;
	int tbl_num=CMDZ_PcmBank_getTblNum(pzi);
	CMDZ_PZIL *pzil=(CMDZ_PZIL *)CMDZ_malloc(sizeof(CMDZ_PZIL)*tbl_num);
	pzil->pcmbank=pzi;
	work->pzi_lp[bank]=pzil;
	if(!pzil)return;
	for(int i=0;i<tbl_num;i++){
		CMDZ_PcmBankTbl *tbl=CMDZ_PcmBank_getTbl(pzi,i);
		if(!tbl)continue;
		int loop_start=-1;
		int loop_end  =-1;
		if(tbl){
			loop_start=CMDZ_PcmBankTbl_getLoopStart(tbl);
			loop_end  =CMDZ_PcmBankTbl_getLoopEnd(tbl);
		}
		pzil->tbl[i].loop_start=loop_start;
		pzil->tbl[i].loop_end  =loop_end;
	}
}
void CMDZ_WORK_initKeyOnTbl(CMDZ_WORK *work){
	for(int i=0;i<CMDZ_WORK_getCnlNum(work);i++){
		work->keyon_tbl[i]=0;
	}
}

bool CMDZ_WORK_hasPort(CMDZ_WORK *work,int port){
	if(!work->fmchip)return false;
	return (*work->fmchip->hasPort)(port);
}
void CMDZ_WORK_outReg0(CMDZ_WORK *work,int reg,int data){
	if(!work->fmchip)return;
	(*work->fmchip->outReg0)(reg,data);
}
void CMDZ_WORK_outReg(CMDZ_WORK *work,int port,int reg,int data){
	if(!work->fmchip)return;
	(*work->fmchip->outReg)(port,reg,data);
}
void CMDZ_WORK_startPcmDriverBuffer(CMDZ_WORK *work){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->startPCMBuffer)();
}
void CMDZ_WORK_setPcmDriverTimer(CMDZ_WORK *work,int tempo,int base_cnt){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->setTimer)(tempo,base_cnt);
}
void CMDZ_WORK_allocCnlsPcmDriver(CMDZ_WORK *work,int num){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->allocCnl)(num);
}
void CMDZ_WORK_initPcmDriverCnl(CMDZ_WORK *work,int i){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->initCnl)(i);
}
int CMDZ_WORK_allocPcmDriverCnl(CMDZ_WORK *work,int num){
	int index=work->pcm_cnl_cnt;
	work->pcm_cnl_cnt+=num;
	for(int j=0;j<num;j++){
		CMDZ_WORK_initPcmDriverCnl(work,index+j);
	}
	return index;
}
void CMDZ_WORK_stopPcmDriverCnl(CMDZ_WORK *work,int cnl){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->stopCnl)(cnl);
}
int CMDZ_WORK_getPcmDriverNowMakePCMSize(CMDZ_WORK *work){
	if(!work->pcmdriver)return 0;
	return (*work->pcmdriver->getNowMakePCMSize)();
}
int CMDZ_WORK_getPcmDriverBufferAdr(CMDZ_WORK *work){
	if(!work->pcmdriver)return 0;
	return (*work->pcmdriver->getPCMBufferAdr)();
}
//void CMDZ_WORK_keyOnPcmDriverCnl(CMDZ_WORK *work,int cnl,CMDZ_PcmBankTbl *pcmbanktbl,int vol,int note,int pan){
void CMDZ_WORK_keyOnPcmDriverCnl(CMDZ_WORK *work,int cnl,CMDZ_PcmBankTbl *pcmbanktbl,int bank,int oto,int vol,int note,int pan){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->keyOnCnl)(cnl,pcmbanktbl,bank,oto,vol,note,pan);
}
void CMDZ_WORK_setPcmDriverCnlVol(CMDZ_WORK *work,int cnl,int n){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->setVolumeCnl)(cnl,n);
}
void CMDZ_WORK_setPcmDriverCnlPan(CMDZ_WORK *work,int cnl,int n){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->setPanCnl)(cnl,n);
}
void CMDZ_WORK_setPcmDriverCnlNote(CMDZ_WORK *work,int cnl,int n){
	if(!work->pcmdriver)return;
	(*work->pcmdriver->setNoteCnl)(cnl,n);
}
/******************************************************************************
;	○ 演奏開始
;	void mus_play(void *mdz_data);
;	out	0 正常終了
;		1 異常終了
******************************************************************************/
bool CMDZ_WORK_playBgm(CMDZ_WORK *work,CMDZ_BgmData *_bgmdata){
	if(!_bgmdata){
#ifndef SGDK
		printf("playBgm error !!(01)\n");
#endif
		return false;
	}
	//
	//演奏停止
	CMDZ_WORK_stopBgm(work);
	//BGMワークの初期化
	CMDZ_WORK_initBgmWork(work);
	work->p8_data=true;
	//ヘッダーチェック
	work->bgmr=_bgmdata;
	work->pcm_cnl_cnt=0;
	//BGMワークを初期化
	CMDZ_WORK_clearCnlNum(work);
	for(int i=0;i<CMDZ_BgmData_getCnlNum(work->bgmr);i++){
		const CMDZ_BgmDataCnl *cnltbl=CMDZ_BgmData_getCnlTbl(work->bgmr,i);
		if(!cnltbl)continue;
		int cnl_type  =cnltbl->type;
		//int cnl_index =cnltbl->index;
		if(cnl_type==-1)continue;
		if(cnl_type==_FM_F || cnl_type==_SSG_F || cnl_type==_RITHM_F || cnl_type==_ADPCM_F){
			work->p8_data=false;
		}
		CMDZ_CNL *cnl=CMDZ_WORK_newCnlWork(work,cnltbl,i);
	}
	//
	CMDZ_WORK_allocCnlsPcmDriver(work,work->pcm_cnl_cnt);
	
	//キーオン&ボリュームテーブルクリア
	CMDZ_WORK_initKeyOnTbl(work);
	//PZIループポインタの初期化
	CMDZ_WORK_initLoopAll(work);
	//その他全体ワーク初期化
	work->soutai_tempo=0;					//相対テンポ
	work->fade_cnt=0;						//フェードカウンタ
	work->src_tempo=DEF_TEMPO;			//元のテンポ
	work->now_tempo=DEF_TEMPO;			//実際のテンポ
	work->old_tempo   =0;
	work->old_base_cnt=0;
	CMDZ_WORK_feedbackPcmDriverTimer(work);
	work->bgm_state=1;					//演奏フラグ 1
	work->loop_is_flg=0;					//ループクリア
	work->loop_is_adr=-1;					//ループだアドレス
	work->end_is_adr=-1;					//演奏終了アドレス
	//
	CMDZ_WORK_startPcmDriverBuffer(work);
	//FMパン初期化
	CMDZ_WORK_initFMReg(work);
	//SSGミキサー初期化
	CMDZ_WORK_initSSGReg(work);
	///
	return true;
}
//FMレジスタ初期化
void CMDZ_WORK_initFMReg(CMDZ_WORK *work){
	CMDZ_WORK_outReg(work,0,0xb4,0xc0);
	CMDZ_WORK_outReg(work,0,0xb5,0xc0);
	CMDZ_WORK_outReg(work,0,0xb6,0xc0);
	CMDZ_WORK_outReg(work,1,0xb4,0xc0);
	CMDZ_WORK_outReg(work,1,0xb5,0xc0);
	CMDZ_WORK_outReg(work,1,0xb6,0xc0);
}
//SSGレジスタ初期化
void CMDZ_WORK_initSSGReg(CMDZ_WORK *work){
#ifdef SGDK
#else
	CMDZ_WORK_outReg0(work,7,work->ssg_mixer);
#endif
	work->before_noise=-1;
	work->before_mixer=-1;
	work->ssg_mixer=0xb8;				//1011_1000B;
#ifdef SGDK
#else
	CMDZ_WORK_outReg0(work,7,work->ssg_mixer);
#endif
}
/******************************************************************************
;	演奏終了
******************************************************************************/
#ifdef SGDK
extern void g_outputPsgVolume(int reg,int vol);
extern void g_stopPsgVolume(void);
#endif
bool CMDZ_WORK_stopBgm(CMDZ_WORK *work){
	for(int i=0;i<CMDZ_WORK_getCnlNum(work);i++){
		CMDZ_CNL *cnl=CMDZ_WORK_getCnlWork(work,i);
		if(!cnl)continue;
		CMDZ_CNL_stopBgmCnl(cnl,work);
	}
	work->bgm_state=0;	//BGM停止
	//FMストップ
	CMDZ_WORK_stopFM(work,0);
	CMDZ_WORK_stopFM(work,1);
	CMDZ_WORK_stopFM2(work,0);
	CMDZ_WORK_stopFM2(work,1);
	//SSG停止
#ifdef SGDK
	g_stopPsgVolume();
//	g_outputPsgVolume(0x90,0);
//	g_outputPsgVolume(0xB0,0);
//	g_outputPsgVolume(0xD0,0);
#else
	CMDZ_WORK_outReg0(work, 8,0);
	CMDZ_WORK_outReg0(work, 9,0);
	CMDZ_WORK_outReg0(work,10,0);
#endif
	//
	return true;
}
void CMDZ_WORK_stopFM(CMDZ_WORK *work,int port){
	//TL=0
	{
	int data=127;
	int reg=0x40;
	for(int i=0;i<4;i++){
		CMDZ_WORK_outReg(work,port,reg+0,data);
		CMDZ_WORK_outReg(work,port,reg+1,data);
		CMDZ_WORK_outReg(work,port,reg+2,data);
		reg+=4;
	}
	}
	//RR停止
	{
	int data=15;
	int reg=0x80;
	for(int i=0;i<16;i++){
		CMDZ_WORK_outReg(work,port,reg,data);
		reg++;
	}
	}
}
void CMDZ_WORK_stopFM2(CMDZ_WORK *work,int port){
	for(int i=0;i<3;i++){
		CMDZ_WORK_keySubFM2(work,port,i,0);
	}
}
void CMDZ_WORK_keySubFM2(CMDZ_WORK *work,int port,int fm_cnl,int n){
	if(!CMDZ_WORK_hasPort(work,port))return;
#if 0
	n|=fm_cnl;
	CMDZ_WORK_outReg(work,port,0x28,n);
#else
	n|=fm_cnl+(port*3);
	CMDZ_WORK_outReg(work,0,0x28,n);
#endif
}
/******************************************************************************
;	ポーズON/OFF
******************************************************************************/
void CMDZ_WORK_setPauseFlg(CMDZ_WORK *work,int flg){
	work->pause_flg=flg;
}
/*******************************************************************************
;	○ フェードアウト
;	void mdz_fade(int speed);
******************************************************************************/
void CMDZ_WORK_fadeBgm(CMDZ_WORK *work,int speed){
	//フェード初期化
	if(work->fade_cnt==0){
		work->fade_cnt =speed;
		work->fade_cnt2=speed;
		work->fade_vol =0;
	}
}
/*******************************************************************************
;	○ BGM演奏チェック
;	int mdz_check_bgm(void);
;	out	0 終了
;		1 演奏中
;		2 PAUSE中
******************************************************************************/
int CMDZ_WORK_checkBgm(CMDZ_WORK *work){
	if(work->bgm_state==0)return 0;
	if(work->pause_flg==0)return 1;
	return 2; 
}
/*******************************************************************************
;	○ 相対テンポ指定
;	void mdz_add_tempo(int add_tempo);
;	add_tempo	-128~0~127相対テンポ
******************************************************************************/
void CMDZ_WORK_addTempo(CMDZ_WORK *work,int n){
	work->soutai_tempo=n;
}
/******************************************************************************
;		PCM作成のパラメータ
;	int mdz_get_makepara(int *loopadr,int *endadr,int *nowsize);
;	OUT	EAX	0:継続
;			1:終了
******************************************************************************/
void CMDZ_WORK_getMakePara(CMDZ_WORK *work,CMDZ_MakePara *para){
	para->loopadr=work->loop_is_adr;
	para->endadr =work->end_is_adr;
	para->nowsize=0;
	para->nowsize=CMDZ_WORK_getPcmDriverNowMakePCMSize(work);
	//終了アドレスが求まった?
	if(work->end_is_adr==-1){
		para->play_flg=true;		//継続
	}else{
		para->play_flg=false;		//終了
	}
}
/*******************************************************************************
;	○ PCM合成のパラメータ
;	int mdz_loop_mode(int mode);
;	mode	0:普通にループ
;		1:すべてのチャネルがループしたら終了
******************************************************************************/
void CMDZ_WORK_setLoopMode(CMDZ_WORK *work,int n){
	work->one_loop_flg=n;
}
/*******************************************************************************
;	○ 早送り
******************************************************************************/
void CMDZ_WORK_setFastForward(CMDZ_WORK *work,bool n){
	work->key_check_flg=n;
}
/******************************************************************************
;	○ テンポ計算
;	IN	RATE		サンプリングレート
;		TEMPO		テンポ
;		BASE		基本カウント値（全音符のカウント数）
;	OUT	ONE_STEP	1ｽﾃｯﾌﾟの処理回数
;	ONE_STEP = RATE/[(TEMPO*BASE)/(4*60)]
;	         = (RATE*4*60)/(TEMPO*BASE)
******************************************************************************/
void CMDZ_WORK_feedbackPcmDriverTimer(CMDZ_WORK *work){
	int tempo=work->now_tempo;
	if(work->key_check_flg){
		tempo=255;
	}
	int base_cnt=CMDZ_BgmData_getBaseCnt(work->bgmr);
	if(tempo!=work->old_tempo || base_cnt!=work->old_base_cnt){
		work->old_tempo   =tempo;
		work->old_base_cnt=base_cnt;
		CMDZ_WORK_setPcmDriverTimer(work,tempo,base_cnt);
	}
}
/*****************************************************************************
;	
******************************************************************************/
bool CMDZ_WORK_checkAllCnlStop(CMDZ_WORK *work){
	for(int i=0;i<CMDZ_WORK_getCnlNum(work);i++){
		CMDZ_CNL *cnl=CMDZ_WORK_getCnlWork(work,i);
		if(!CMDZ_CNL_getStopFlg(cnl) || !CMDZ_CNL_getOneLoopFlg(cnl))return false;
	}
	return true;
}
void CMDZ_WORK_getLoopAdr(CMDZ_WORK *work){
	//すでにループポインタを設定した?
	if(work->loop_is_adr==-1){
		//[ループだ!]があった?
		if(work->loop_is_flg!=0){
			work->loop_is_adr=CMDZ_WORK_getPcmDriverBufferAdr(work);	//ループアドレス設定
		}
	}
	//すでに終了アドレスを設定した?
	if(work->end_is_adr==-1){
		//演奏終了?
		if(work->bgm_state==0){
			work->end_is_adr=CMDZ_WORK_getPcmDriverBufferAdr(work);	//ENDアドレス設定
		}
	}
}
/******************************************************************************
;	○ BGM演奏処理呼び出し
******************************************************************************/
void CMDZ_WORK_playMain(CMDZ_WORK *work){
	if(work->bgm_state==0)return;
#ifndef SGDK
printf("CMDZ_WORK_playMain\n");
#endif
	//チャネルごとの処理へ
	for(int i=0;i<CMDZ_WORK_getCnlNum(work);i++){
		CMDZ_CNL *cnl=CMDZ_WORK_getCnlWork(work,i);
		if(CMDZ_CNL_getStopFlg(cnl))continue;
		CMDZ_CNL_driverCom(cnl,work);
	}
	//すべてのチャネルが終了またはループした
	if(CMDZ_WORK_checkAllCnlStop(work)){
		work->bgm_state=0;		//演奏フラグOFF
#ifndef SGDK
		printf("playMain:allCnlStop\n");
#endif
		return;
	}
	/*======================================
	;	ミキサー出力
	;=====================================*/
	CMDZ_WORK_outSSGMixer(work);
	/*======================================
	;	テンポ計算
	;=====================================*/
	CMDZ_WORK_calcTempo(work);
	/*======================================
	;	フェードアウト処理
	;=====================================*/
	if(CMDZ_WORK_execFade(work)){
#ifndef SGDK
		printf("playMain:fadeEnd\n");
#endif
		return;
	}
	/*======================================
	;	早送り&演奏停止
	;=====================================*/
	CMDZ_WORK_feedbackPcmDriverTimer(work);
	//ループアドレスの獲得
	CMDZ_WORK_getLoopAdr(work);
}
//テンポ計算
void CMDZ_WORK_calcTempo(CMDZ_WORK *work){
	int _tempo=work->src_tempo+work->soutai_tempo;
	if(_tempo<TEMPO_MIN)_tempo=TEMPO_MIN;
	if(_tempo>TEMPO_MAX)_tempo=TEMPO_MAX;
	work->now_tempo=_tempo;
}
//フェードアウト処理
bool CMDZ_WORK_execFade(CMDZ_WORK *work){
	if(work->fade_cnt!=0){
		work->fade_cnt2--;
		if(work->fade_cnt2==0){
			work->fade_cnt2=work->fade_cnt;
			work->fade_vol++;
			if(work->fade_vol>64){
				work->fade_cnt=0;
				CMDZ_WORK_stopBgm(work);
				return true;
			}
		}
	}
	return false;
}
//ミキサーの音出力
void CMDZ_WORK_outSSGMixer(CMDZ_WORK *work){
	if(work->ssg_noise!=work->before_noise){
		work->before_noise=work->ssg_noise;
#ifdef SGDK
#else
		CMDZ_WORK_outReg0(work,6,work->ssg_noise);
#endif
	}
	if(work->ssg_mixer!=work->before_mixer){
		work->before_mixer=work->ssg_mixer;
#ifdef SGDK
#else
		CMDZ_WORK_outReg0(work,7,work->ssg_mixer);
#endif
	}
}







/*****************************************************************************
;	CMDZ_CNL
******************************************************************************/
void CMDZ_CNL_init(CMDZ_CNL *cnl){
	cnl->data_adr_top=NULL;
	cnl->data_adr=NULL;
	CMDZ_CNL_initSub(cnl);
}
void CMDZ_CNL_delete(CMDZ_CNL *cnl){
}
void CMDZ_CNL_setStopFlg(CMDZ_CNL *cnl,bool n){
	cnl->state.stop=n;
}
bool CMDZ_CNL_getStopFlg(CMDZ_CNL *cnl){
	return cnl->state.stop;
}
bool CMDZ_CNL_getOneLoopFlg(CMDZ_CNL *cnl){
	return cnl->state.oneloop;
}
void CMDZ_CNL_initSub(CMDZ_CNL *cnl){
	cnl->state.stop  =true;
	cnl->loop_adr    =-1;			//(byte *)((int)&cnl.stack-1);//LOOP初期化
	cnl->data_adr_top=NULL;			//チャネルデータ
	cnl->data_adr    =NULL;			//チャネルデータ
	CMDZ_CNLFLG_init(&cnl->state);
	cnl->def_len     =0x30;			//デフォルトの音長
	cnl->len_wk      =1;				//音長カウンタ
	cnl->wave        =0;				//現在の周波数
	cnl->before_wave =0xffff;		//前の周波数
	cnl->quota_wk    =1;				//Qカウンタ
	cnl->detune      =0;				//ディチューン
	cnl->cnl_cate    =_PPZ8_F;		//チャネルIndex
	cnl->cnl_number  =0;				//チャネルIndex
	cnl->cnl_port_num=0;				//チャネルIndex
	cnl->quota       =1;				//Q
	cnl->now_ontei   =0;				//現在の音程
	cnl->before_ontei=0xffff;		//前の音程
	cnl->vol         =63;			//VOL
	cnl->before_vol  =-1;			//前の音量
	cnl->soutai_icho =0;				//
	cnl->pan         =5;				//
	cnl->oto_num     =-1;			//音番号
	cnl->now_cmd     =0xff;
	cnl->now_len=1;
	//
	cnl->oto_bank   =0;				//バンク番号
	cnl->pcm_work  =-1;				//使用するPCMチャネル
	for(int i=0;i<3;i++){
		CMDZ_LFO_init(&cnl->lfo[i]);
	}
	CMDZ_VEND_init(&cnl->vend);			//
	CMDZ_ENV_init(&cnl->env)	;			//エンベロープ初期化
	CMDZ_APAN_init(&cnl->apan);			//
	CMDZ_PcmTone_init(&cnl->pcmtone);	//
}
void CMDZ_CNL_errorStop(CMDZ_CNL *cnl,CMDZ_WORK *work,const char *m,...){
#ifndef SGDK
	char mm[1024];
	va_list list;
	va_start(list,m);
	vsnprintf(mm,sizeof(mm),m,list);
	va_end(list);
	
	printf("cnl(%2d:%5s:%d):errorStop !!:%s\n",cnl->cnl_index,CMDZ_Tools_getCnlCateName(cnl->cnl_cate),cnl->cnl_number,mm);
#endif
	CMDZ_CNL_stopCnl(cnl,work);
}
static int CMDZ_CNL_getCommandPointer(CMDZ_CNL *cnl){
	return (int)((uintptr_t)cnl->data_adr-(uintptr_t)cnl->data_adr_top);
}
static void CMDZ_CNL_skipCommandAdr(CMDZ_CNL *cnl,int skip_bytes){
	cnl->data_adr=(void *)((uintptr_t)cnl->data_adr+skip_bytes);
}
static void CMDZ_CNL_setCommandAdr(CMDZ_CNL *cnl,const CMDZ_WORK *work,int offset){
	cnl->data_adr=(void *)(((uintptr_t)CMDZ_WORK_getDataAdr(work))+offset);
}
static int CMDZ_CNL_readCommandUint8(CMDZ_CNL *cnl){
	int n=*(uint8_t *)cnl->data_adr;
	CMDZ_CNL_skipCommandAdr(cnl,1);
	return n;
}
static int CMDZ_CNL_readCommandSint8(CMDZ_CNL *cnl){
	int n=*(int8_t *)cnl->data_adr;
	CMDZ_CNL_skipCommandAdr(cnl,1);
	return n;
}
static int CMDZ_CNL_readCommandUint16(CMDZ_CNL *cnl){
	int n0=*((uint8_t *)((uintptr_t)cnl->data_adr+0));
	int n1=*((uint8_t *)((uintptr_t)cnl->data_adr+1));
	int n=(n1 << 8) | n0;
	CMDZ_CNL_skipCommandAdr(cnl,2);
	return n;
}
static int CMDZ_CNL_readCommandSint16(CMDZ_CNL *cnl){
	int n0=*((uint8_t *)((uintptr_t)cnl->data_adr+0));
	int n1=*(( int8_t *)((uintptr_t)cnl->data_adr+1));
	int n=(n1 << 8) | n0;
	CMDZ_CNL_skipCommandAdr(cnl,2);
	return n;
}
static int CMDZ_CNL_readCommandUint32(CMDZ_CNL *cnl){
	int n0=*((uint8_t *)((uintptr_t)cnl->data_adr+0));
	int n1=*((uint8_t *)((uintptr_t)cnl->data_adr+1));
	int n2=*((uint8_t *)((uintptr_t)cnl->data_adr+2));
	int n3=*((uint8_t *)((uintptr_t)cnl->data_adr+3));
	int n=(n3 << 24) | (n2 << 16) | (n1 << 8) | n0;
	CMDZ_CNL_skipCommandAdr(cnl,4);
	return n;
}
static int CMDZ_CNL_readCommandSint32(CMDZ_CNL *cnl){
	int n0=*((uint8_t *)((uintptr_t)cnl->data_adr+0));
	int n1=*((uint8_t *)((uintptr_t)cnl->data_adr+1));
	int n2=*((uint8_t *)((uintptr_t)cnl->data_adr+2));
	int n3=*(( int8_t *)((uintptr_t)cnl->data_adr+3));
	int n=(n3 << 24) | (n2 << 16) | (n1 << 8) | n0;
	CMDZ_CNL_skipCommandAdr(cnl,4);
	return n;
}

bool CMDZ_CNL_initCnlFM(CMDZ_CNL *cnl){
	cnl->pan=3;
	cnl->vol=0x10;
	cnl->before_vol=255;
	cnl->before_wave=0xffff;
	//
	if(cnl->cnl_number<3){
		cnl->fm_cnl_number=cnl->cnl_number;	//FM音源のチャネル
		cnl->cnl_port_num =0;			//アクセスポート
	}else{
		cnl->fm_cnl_number=cnl->cnl_number-3;	//FM音源のチャネル
		cnl->cnl_port_num =1;			//アクセスポート
	}
	return true;
}
bool CMDZ_CNL_initCnlSSG(CMDZ_CNL *cnl){
	cnl->cnl_port_num=0;			//アクセスポート
	CMDZ_ENV_init(&cnl->env);
	cnl->before_vol=0;
	cnl->before_wave=-1;
	cnl->vol=15;
	cnl->pan=1;
	return true;
}
bool CMDZ_CNL_initCnlRITHM(CMDZ_CNL *cnl){
	cnl->cnl_port_num=0;			//アクセスポート
	cnl->vol=63;
	cnl->before_vol=0;
	for(int i=0;i<6;i++){
		cnl->rt_vol[i]=0xdf;		//11011111B
	}
	return true;
}
bool CMDZ_CNL_initCnlADPCM(CMDZ_CNL *cnl){
	return true;
}
bool CMDZ_CNL_initCnlPPZ8(CMDZ_CNL *cnl){
	return true;
}
bool CMDZ_CNL_initCnl(CMDZ_CNL *cnl,CMDZ_WORK *work,const CMDZ_BgmDataCnl *cnltbl,int _cnl_index){
	CMDZ_CNL_init(cnl);
	cnl->state.stop  =false;
	cnl->data_adr    =cnltbl->offset;			//チャネルデータ
	cnl->data_adr_top=cnl->data_adr;			//データ先頭
	cnl->cnl_index   =_cnl_index;				//チャネルindex
	cnl->cnl_cate    =cnltbl->type;				//チャネル種類
	cnl->cnl_number  =cnltbl->index;			//チャネル種類内のindex
	cnl->quota       =CMDZ_WORK_getQFlg(work);	//Q
	cnl->len_wk      =1;						//音長カウンタ
	cnl->def_len     =0x30;						//デフォルトの音長
	cnl->quota_wk    =1;						//Qカウンタ
	cnl->pan         =5;						//PAN
	cnl->vol         =63;						//VOL
	cnl->loop_adr    =-1;						//(byte *)((int)&cnl.stack-1);//LOOP初期化
	cnl->soutai_icho =0;						//相対移調
	cnl->now_ontei   =0;						//現在の音程
	cnl->before_vol  =-1;						//以前のボリューム
	cnl->before_ontei=0xffff;					//前の音程
	cnl->before_wave =0xffff;					//前の周波数
	cnl->wave        =0;						//現在の周波数
	cnl->detune      =0;						//ディチューン
	cnl->oto_num     =-1;						//音番号
	CMDZ_ENV_init(&cnl->env);					//エンベロープ初期化
	for(int j=0;j<6;j++){
		cnl->rt_vol[j]=0xdf;					//11011111B
	}
	//PCMW設定
	if(work){
	//if(cnl_type==CMDZ_BGMDATA._PPZ8_F || cnl_type==CMDZ_BGMDATA._ADPCM_F){
			if(cnltbl->type!=_RITHM_F){
				cnl->pcm_work=CMDZ_WORK_allocPcmDriverCnl(work,1);
			}else{
				cnl->pcm_work=CMDZ_WORK_allocPcmDriverCnl(work,6);
			}
		//}
	}
	if(cnltbl->type==_ADPCM_F){
		cnl->oto_bank=1;
	}
	if(cnltbl->type==_FM_F){
		CMDZ_CNL_initCnlFM(cnl);
	}
	if(cnltbl->type==_SSG_F){
		CMDZ_CNL_initCnlSSG(cnl);
	}
	if(cnltbl->type==_ADPCM_F){
		CMDZ_CNL_initCnlADPCM(cnl);
	}
	if(cnltbl->type==_PPZ8_F){
		CMDZ_CNL_initCnlPPZ8(cnl);
	}
	return true;
}




/******************************************************************************
;	○ チャネルごとのコマンド実行
******************************************************************************/
void CMDZ_CNL_driverCom(CMDZ_CNL *cnl,CMDZ_WORK *work){
	//停止している?
	if(cnl->state.stop)return;
	//キーオンクリア
	cnl->state.kon=false;	//KON_F 0
	//
	if(cnl->state.start){
		if(!cnl->state.tai){
			if(cnl->quota_wk!=0){
				cnl->quota_wk--;
				if(cnl->quota_wk==0){
					cnl->state.vend=false;
					CMDZ_CNL_keyOff(cnl,work);
				}
			}
		}
		cnl->len_wk--;
		if(cnl->len_wk!=0){
			CMDZ_CNL_execLFO(cnl,work);
			return;
		}else{
			bool old_sura=cnl->state.sura;
			cnl->state.sura=false;
			if(old_sura){
				cnl->state.sura2=true;
			}else{
				cnl->state.sura2=false;
			}
			bool old_tai=cnl->state.tai;
			cnl->state.tai=false;
			if(old_tai){
				cnl->state.tai2=true;
			}else{
				cnl->state.tai2=false;
			}
		}
	}
	//
	if(!CMDZ_CNL_parseCommand(cnl,work))return;
	CMDZ_CNL_execOnchoCommand(cnl,work);
}
/******************************************************************************
;	○
******************************************************************************/
bool CMDZ_CNL_getCnlInfoMess(CMDZ_CNL *cnl,char *m){
	char cmd_name[256]={0};
	CMDZ_Tools_getCommandName2(cmd_name,cnl->now_cmd);
	sprintf(m,"%s(%d):cmd(%04x:%s),%d,oto(%d/%d),len(%d/%d),wave(%04x),vol(%2d),pan(%d)",
		CMDZ_Tools_getCnlCateName(cnl->cnl_cate),
		cnl->cnl_number,
		CMDZ_CNL_getCommandPointer(cnl),
		cmd_name,
		cnl->state.kon,
		cnl->oto_bank,cnl->oto_num,
		cnl->len_wk,cnl->now_len,
		cnl->wave,
		cnl->vol,
		cnl->pan
	);
	return true;
}
/******************************************************************************
;	○
******************************************************************************/

void CMDZ_CNL_printCommand(CMDZ_CNL *cnl,int n){
	char name[256];
	if(n==_STOP_COM){
		strcpy(name,"stop");
	}else if(n<_COM_TOP){
		if(n==_REST_COM){
			strcpy(name,"reset");
		}else{
			strcpy(name,"ontei");
		}
	}else{
		strcpy(name,CMDZ_Tools_getCommandName(n));
	}
#ifndef SGDK
	int p=CMDZ_CNL_getCommandPointer(cnl)-1;
	printf("cnl(%2d:%5s:%d):cmd:[%08x]:(0x%02x):%s\n",cnl->cnl_index,CMDZ_Tools_getCnlCateName(cnl->cnl_cate),cnl->cnl_number,p,n,name);
#endif
}
/******************************************************************************
;	○ コマンド処理
******************************************************************************/
bool CMDZ_CNL_parseCommand(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.start=true;
	while(true){
		int n=CMDZ_CNL_readCommandUint8(cnl);
		cnl->now_cmd=n;
//printCommand(n);
		if(n==_STOP_COM){
			CMDZ_CNL_stopCnl(cnl,work);
			return false;
		}else if(n<_COM_TOP){
			cnl->now_ontei=n;
			break;
		}
		//コマンドジャンプ
		CMDZ_CNL_execCommand(cnl,work,n);
	}
	return true;
}
/******************************************************************************
;	○ 音長コマンド
******************************************************************************/
void CMDZ_CNL_execOnchoCommand(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int len=cnl->def_len;
	int n=CMDZ_CNL_readCommandUint8(cnl);
	if(n!=0){
		if(n<0x80){
			len=n;
		}else{
			int n2=CMDZ_CNL_readCommandUint8(cnl);
			len=(((n & 0x7f) << 8) | n2);
		}
	}
	cnl->len_wk =len;
	cnl->now_len=len;
	//
	if(CMDZ_WORK_getQFlg(work)!=8){
		int qw=cnl->len_wk-(cnl->quota-1);
		if(qw<=0)qw=cnl->len_wk;
		cnl->quota_wk=qw;
	}else{
		if(cnl->quota==8){
			cnl->quota_wk=cnl->len_wk;
		}else{
			int qw=(cnl->len_wk >> 3)*cnl->quota;
			if(qw==0)qw=1;
			cnl->quota_wk=qw;
		}
	}
	//
	if(cnl->cnl_cate==_RITHM_F){
		//RITHM_OUT:
		CMDZ_CNL_outRITHM(cnl,work);
	}else{
		if(cnl->now_ontei==_REST_COM){
			CMDZ_CNL_rest(cnl,work);
		}else{
			CMDZ_CNL_restSkip(cnl,work);
		}
	}
}
/******************************************************************************
;	○ 音程のチェックと休符処理
******************************************************************************/
void CMDZ_CNL_rest(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->before_ontei=-1;
	cnl->state.rest=false;
	CMDZ_CNL_keyOff(cnl,work);
}
void CMDZ_CNL_restSkip(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int old_ontei=cnl->before_ontei;
	cnl->before_ontei=cnl->now_ontei;
	cnl->state.rest=true;
	//
	if(cnl->state.vend){
		//ピッチベンドの場合、前の音程
		//はベンドの目標音程となる
		cnl->before_ontei=cnl->vend.ontei;
		if(cnl->vend.ontei<cnl->now_ontei){
			cnl->vend.rate=-cnl->vend.rate;
		}
	}
	if(cnl->state.tai2){
		bool old_sura2=cnl->state.sura2;
		cnl->state.sura2=false;
		if(cnl->now_ontei==old_ontei){
			CMDZ_CNL_execLFO(cnl,work);
			return;
		}else{
			if(old_sura2){
				CMDZ_CNL_keyOff(cnl,work);
			}
		}
	}
	CMDZ_CNL_outKeyOn(cnl,work);
}
/******************************************************************************
;	○ キーオンをする
******************************************************************************/
void CMDZ_CNL_outKeyOn(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_WAVEOCT waveoct;
	CMDZ_CNL_getOntei(cnl,&waveoct,work,cnl->now_ontei);
	cnl->wave  =waveoct.wave;
	cnl->oct_wk=waveoct.oct;
	CMDZ_CNL_initAllLFO(cnl,work);
	CMDZ_CNL_keyOn(cnl,work);
}
/******************************************************************************
;	○ キーオン処理
******************************************************************************/
void CMDZ_CNL_keyOnEnv(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.kon_r=true;
	cnl->env.flg.ar=false;
	cnl->env.flg.dr=false;
	cnl->env.flg.sr=false;
	cnl->env.vol2  =cnl->env.sv;
}
void CMDZ_CNL_keyOnFM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_outOnteiVol(cnl,work);
	CMDZ_CNL_keySubFM(cnl,work,0xf0);
}
void CMDZ_CNL_keyOnSSG(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOnEnv(cnl,work);
	CMDZ_CNL_outOnteiVol(cnl,work);
}
void CMDZ_CNL_keyOnPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOnEnv(cnl,work);
	//keyOn
	CMDZ_WORK_keyOnPcmDriverCnl(work,cnl->pcm_work,cnl->pcmtone.pzit,cnl->oto_bank,cnl->oto_num,cnl->vol,cnl->wave,cnl->pan);
	//音程、ボリューム設定
	CMDZ_CNL_outOnteiVol(cnl,work);		//音程、ボリューム設定
}
void CMDZ_CNL_keyOnADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOnPPZ8(cnl,work);
}
void CMDZ_CNL_keyOn(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.kon  =true;
	cnl->state.koff =false;
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_keyOnFM(cnl,work);break;
		case _SSG_F  :CMDZ_CNL_keyOnSSG(cnl,work);break;
		case _ADPCM_F:CMDZ_CNL_keyOnADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_keyOnPPZ8(cnl,work);break;
	}
}
/******************************************************************************
;	○ 音色設定サブ
;	Break)EAX,ECX,EDX,ESI,EDI,EBP
******************************************************************************/
bool CMDZ_CNL_feedbackPcmNeiroPPZ8Sub(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int bank=cnl->oto_bank;
	int num =cnl->oto_num;
	CMDZ_PcmBankTbl *pzit=CMDZ_WORK_getPcmBankTbl(work,bank,num);
	if(!pzit){
#ifndef SGDK
		printf("SetOto pzi.GetTbl:bank(%d),num(%d) Error !!\n",bank,num);
#endif
		return false;
	}
	CMDZ_PZILT *pzilt=CMDZ_WORK_getPziLoopTable(work,bank,num);
	if(!pzilt){
#ifndef SGDK
		printf("SetOto work.pzi_lp:bank(%d),num(%d) Error !!\n",bank,num);
#endif
		return false;
	}
	CMDZ_PcmTone_set(&cnl->pcmtone,pzit,pzilt);
	return true;
}
void CMDZ_CNL_feedbackPcmNeiroPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_feedbackPcmNeiroPPZ8Sub(cnl,work);
}
void CMDZ_CNL_feedbackPcmNeiroADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->oto_num<0x80){
		CMDZ_BgmAdpcmNeiro adpcm_neiro;
		if(!CMDZ_WORK_getAdpcmNeiro(work,cnl->oto_num,&adpcm_neiro)){
			return;
		}
		CMDZ_WORK_setAdpcmNeiro(work,1,cnl->oto_num,&adpcm_neiro);
		//return;
	}else{
	}
	CMDZ_CNL_feedbackPcmNeiroPPZ8Sub(cnl,work);
}
void CMDZ_CNL_feedbackPcmNeiro(CMDZ_CNL *cnl,CMDZ_WORK *work){
	switch(cnl->cnl_cate){
		case _ADPCM_F:CMDZ_CNL_feedbackPcmNeiroADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_feedbackPcmNeiroPPZ8(cnl,work);break;
	}
}
void CMDZ_CNL_feedbackPcmNeiroPcmBankIndex(CMDZ_CNL *cnl,CMDZ_WORK *work,int _bank,int _index){
	if(cnl->cnl_cate!=_ADPCM_F && cnl->cnl_cate!=_PPZ8_F)return;
	if(cnl->state.stop)return;
	if(cnl->oto_bank==_bank && cnl->oto_num==_index){
		CMDZ_CNL_feedbackPcmNeiro(cnl,work);
	}
}
/******************************************************************************
;	○ LFOの処理
******************************************************************************/
void CMDZ_CNL_execLFO(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->cnl_cate==_RITHM_F){
	}else{
		CMDZ_CNL_execLFOSub(cnl,work,0);
		CMDZ_CNL_execLFOSub(cnl,work,1);
		CMDZ_CNL_execLFOSub(cnl,work,2);
		CMDZ_CNL_execPitchVend(cnl,work);
		CMDZ_CNL_execAutoPan(cnl,work);
		//
		CMDZ_CNL_outOnteiVol(cnl,work);
	}
}
void CMDZ_CNL_outOnteiVol(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_outOntei(cnl,work);
	CMDZ_CNL_outVol(cnl,work);
}
/******************************************************************************
;	○ LFOの演算処理
******************************************************************************/
	/*======================================
	;	[0]三角波
	;=====================================*/
void CMDZ_CNL_execLFOWave0(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	cnl->wave=CMDZ_addWave(cnl->wave,_lfo->rate_sub);
	_lfo->depth_cnt--;
	if(_lfo->depth_cnt!=0)return;
	_lfo->depth_cnt=_lfo->depth;
	_lfo->rate_sub=-_lfo->rate_sub;
}
	/*======================================
	;	[1]のこぎり波
	;=====================================*/
void CMDZ_CNL_execLFOWave1(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	cnl->wave=CMDZ_addWave(cnl->wave,_lfo->rate);
	_lfo->depth_cnt--;
	if(_lfo->depth_cnt!=0)return;
	_lfo->depth_cnt=_lfo->depth;
	_lfo->rate=-_lfo->rate_sub;
}
	/*======================================
	;	[2]方形波
	;=====================================*/
void CMDZ_CNL_execLFOWave2(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	int n=_lfo->rate_sub;
	if(_lfo->depth_cnt!=0){
		n=n >> 1;
		_lfo->depth_cnt=0;
	}
	cnl->wave=CMDZ_addWave(cnl->wave,n);
	_lfo->rate_sub=-_lfo->rate_sub;
}
	/*======================================
	;	[3]ポルタメント
	;=====================================*/
void CMDZ_CNL_execLFOWave3(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	cnl->wave=CMDZ_addWave(cnl->wave,_lfo->rate_sub);
	if(_lfo->depth_cnt!=0)return;
	_lfo->md_cnt=0;	//終了
}
	/*======================================
	;	[4]階段波
	;=====================================*/
void CMDZ_CNL_execLFOWave4(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	cnl->wave=CMDZ_addWave(cnl->wave,_lfo->rate_sub);
	_lfo->depth_cnt--;
	if(_lfo->depth_cnt==0){
		_lfo->depth_cnt=2;
	}
	_lfo->rate_sub=-_lfo->rate_sub;
}

//======================
void CMDZ_CNL_execLFOSub(CMDZ_CNL *cnl,CMDZ_WORK *work,int type){
	if(!CMDZ_CNLFLG_getLFO(&cnl->state,type))return;
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	if(_lfo->md_cnt==0)return;
	_lfo->md_cnt--;
	if(_lfo->md_cnt!=0)return;
	_lfo->md_cnt=1;
	_lfo->speed_cnt--;
	if(_lfo->speed_cnt!=0)return;
	_lfo->speed_cnt=_lfo->speed;
	//
	switch(_lfo->wave_num){
		case 0:
			CMDZ_CNL_execLFOWave0(cnl,work,type);
			break;
		case 1:
			CMDZ_CNL_execLFOWave1(cnl,work,type);
			break;
		case 2:
			CMDZ_CNL_execLFOWave2(cnl,work,type);
			break;
		case 3:
			CMDZ_CNL_execLFOWave3(cnl,work,type);
			break;
		case 4:
			CMDZ_CNL_execLFOWave4(cnl,work,type);
			break;
		case 5:
			CMDZ_CNL_execLFOWave0(cnl,work,type);
			break;
	}
}
/******************************************************************************
;	○ ピッチベンドの処理
******************************************************************************/
void CMDZ_CNL_execPitchVendEnd(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->wave      =cnl->vend.wave;
	cnl->oct_wk    =cnl->vend.oct_wk;
	cnl->state.vend=false;
	cnl->vend.rate =0;
}
void CMDZ_CNL_execPitchVendSSG(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int old_wave =cnl->wave >> cnl->oct_wk;
	int vend_wave=cnl->vend.wave >> cnl->vend.oct_wk;
	int rate=cnl->vend.rate;
	if(vend_wave<old_wave)rate=-rate;
	cnl->wave=CMDZ_addWave(cnl->wave,rate);
	int now_wave =cnl->wave >> cnl->oct_wk;
	if(rate>0){
		if(now_wave>=vend_wave){
			CMDZ_CNL_execPitchVendEnd(cnl,work);
		}
	}else{
		if(now_wave<=vend_wave){
			CMDZ_CNL_execPitchVendEnd(cnl,work);
		}
	}
}
void CMDZ_CNL_execPitchVendPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->wave=CMDZ_addWave(cnl->wave,cnl->vend.rate);
	if(cnl->vend.rate>0){
		if(cnl->wave>=cnl->vend.wave){
			cnl->wave=cnl->vend.wave;
			cnl->vend.rate=0;
			cnl->state.vend=false;
		}
	}else{
		if(cnl->wave<=cnl->vend.wave){
			cnl->wave=cnl->vend.wave;
			cnl->vend.rate=0;
			cnl->state.vend=false;
		}
	}
}
void CMDZ_CNL_execPitchVendFM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_execPitchVendPPZ8(cnl,work);
}
void CMDZ_CNL_execPitchVendADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_execPitchVendPPZ8(cnl,work);
}

void CMDZ_CNL_execPitchVend(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(!cnl->state.vend)return;
	cnl->vend.md_cnt--;
	if(cnl->vend.md_cnt!=0)return;
	cnl->vend.md_cnt=1;
	cnl->vend.speed_cnt--;
	if(cnl->vend.speed_cnt!=0)return;
	cnl->vend.speed_cnt=cnl->vend.speed;
	if(cnl->vend.rate==0)return;
	//
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_execPitchVendFM(cnl,work);break;
		case _SSG_F  :CMDZ_CNL_execPitchVendSSG(cnl,work);break;
		case _ADPCM_F:CMDZ_CNL_execPitchVendADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_execPitchVendPPZ8(cnl,work);break;
	}
}
/******************************************************************************
;	○ 音程を足す
;		IN	AX	WAVE
;			DX	加算値
;		OUT	AX	WAVE
******************************************************************************/
int CMDZ_addWave(int wave,int add){
	if(add==0)return wave;
	if(add>0){
		wave+=add;
		if(wave>0xffff)wave=0xffff;
	}else{
		wave+=add;
		if(wave<0)wave=0;
	}
	return wave;
}
/******************************************************************************
;	○ オートパンの処理
******************************************************************************/
void CMDZ_CNL_execAutoPan(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(!cnl->state.apan)return;
	if(cnl->apan.md_cnt==0)return;
	cnl->apan.md_cnt--;
	if(cnl->apan.md_cnt!=0)return;
	cnl->apan.md_cnt=1;
	cnl->apan.speed_cnt--;
	if(cnl->apan.speed_cnt!=0)return;
	cnl->apan.speed_cnt=cnl->apan.speed;
	cnl->apan.num+=cnl->apan.add;
	//
	CMDZ_WORK_setPcmDriverCnlPan(work,cnl->pcm_work,cnl->apan.num);
	//
	if(cnl->apan.dist_w!=0)return;
	if(cnl->apan.type==0){
		//デュレイカウント
		cnl->apan.md_cnt=0;
	}else{
		cnl->apan.add=-cnl->apan.add;
		int n1=cnl->apan.sorc_w;
		int n2=cnl->apan.dist_w;
		cnl->apan.sorc_w=n2;
		cnl->apan.dist_w=n1;
	}
}
/******************************************************************************
;	○ 音程からそれぞれの周波数を求める
******************************************************************************/
void CMDZ_CNL_getOnteiFM(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei){
	int oct =ontei / 12;
	int note=ontei % 12;
	int n=fm_ontei_tbl[note];
	n=CMDZ_addWave(n,cnl->detune);
	n&=0x07ff;
	n|=oct << (3+8);
	waveoct->wave=n;
	waveoct->oct =oct;
}
void CMDZ_CNL_getOnteiSSG(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei){
	int oct =ontei / 12;
	int note=ontei % 12;
	int n=ssg_ontei_tbl[note];
	n=CMDZ_addWave(n,cnl->detune);
	waveoct->wave=n;
	waveoct->oct =oct;
}
//o4c(4*12)が基準
void CMDZ_CNL_getOnteiPPZ8(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei){
	int oct =ontei / 12;
	int note=ontei % 12;
	int n=ppz8_ontei_tbl[note];
	n=n >> (7-oct);
	n=CMDZ_addWave(n,cnl->detune);
	waveoct->wave=n;
	waveoct->oct =oct;
}
void CMDZ_CNL_getOnteiADPCM(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei){
//	int oct =ontei / 12;
//	int note=ontei % 12;
	CMDZ_CNL_getOnteiPPZ8(cnl,waveoct,work,ontei);
}
void CMDZ_CNL_getOntei(CMDZ_CNL *cnl,CMDZ_WAVEOCT *waveoct,CMDZ_WORK *work,int ontei){
	ontei+=cnl->soutai_icho;
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_getOnteiFM(cnl,waveoct,work,ontei);break;
		case _SSG_F  :CMDZ_CNL_getOnteiSSG(cnl,waveoct,work,ontei);break;
		case _ADPCM_F:CMDZ_CNL_getOnteiADPCM(cnl,waveoct,work,ontei);break;
		case _PPZ8_F :CMDZ_CNL_getOnteiPPZ8(cnl,waveoct,work,ontei);break;
	}
}
/******************************************************************************
;	○ 音程の計算
******************************************************************************/
#ifdef SGDK
extern int g_calcFMNoteScale(int _wave);
#endif
void CMDZ_CNL_outOnteiFM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int _wave=cnl->wave;
	if(_wave!=cnl->before_wave){
		cnl->before_wave=_wave;
		int port=cnl->cnl_port_num;
		int reg0=cnl->fm_cnl_number+0xa0;
		int reg1=cnl->fm_cnl_number+0xa4;
#ifdef SGDK
		_wave=g_calcFMNoteScale(_wave);
#endif
		CMDZ_WORK_outReg(work,port,reg1,(_wave >> 8) & 0xff);
		CMDZ_WORK_outReg(work,port,reg0, _wave       & 0xff);
	}
}

#ifdef SGDK
extern void g_outputPsgTone(int reg,int tone);
#endif

void CMDZ_CNL_outOnteiSSG(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int _wave=cnl->wave >> cnl->oct_wk;
	if(_wave!=cnl->before_wave){
		cnl->before_wave=_wave;
#ifdef SGDK
		int mixer=work->ssg_mixer;
		if(!(mixer & (0x01 << cnl->cnl_number))){
			int reg=0x80+(cnl->cnl_number*0x20);
			g_outputPsgTone(reg,_wave);
		}
#else
		int reg=cnl->cnl_number*2;
		CMDZ_WORK_outReg0(work,reg+0, _wave       & 0xff);
		CMDZ_WORK_outReg0(work,reg+1,(_wave >> 8) & 0xff);
#endif
	}
}
void CMDZ_CNL_outOnteiPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->wave!=cnl->before_wave){
		cnl->before_wave=cnl->wave;
		int _wave=cnl->wave;
		CMDZ_WORK_setPcmDriverCnlNote(work,cnl->pcm_work,_wave);
	}
}
void CMDZ_CNL_outOnteiADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_outOnteiPPZ8(cnl,work);
}
void CMDZ_CNL_outOntei(CMDZ_CNL *cnl,CMDZ_WORK *work){
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_outOnteiFM(cnl,work);break;
		case _SSG_F  :CMDZ_CNL_outOnteiSSG(cnl,work);break;
		case _ADPCM_F:CMDZ_CNL_outOnteiADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_outOnteiPPZ8(cnl,work);break;
	}
}
/******************************************************************************
;	○ キーオフ
******************************************************************************/
void CMDZ_CNL_keyOffEnv(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->env.rr==0){
		cnl->env.vol2=0;
	}
}
void CMDZ_CNL_keySubFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	int port=cnl->cnl_port_num;
	if(!CMDZ_WORK_hasPort(work,port))return;
	CMDZ_WORK_keySubFM2(work,port,cnl->fm_cnl_number,n);
}
void CMDZ_CNL_keyOffFM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keySubFM(cnl,work,0);
}
void CMDZ_CNL_keyOffSSG(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOffEnv(cnl,work);
	CMDZ_CNL_outVol(cnl,work);
}
void CMDZ_CNL_keyOffPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOffEnv(cnl,work);
	CMDZ_CNL_outVol(cnl,work);
}
void CMDZ_CNL_keyOffADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_CNL_keyOffPPZ8(cnl,work);
}
void CMDZ_CNL_keyOffRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	//RT_KEYOFF
}
void CMDZ_CNL_keyOff(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.koff=true;
	cnl->state.kon =false;
	if(cnl->state.rest_off)return;
	//
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_keyOffFM(cnl,work);break;
		case _SSG_F  :CMDZ_CNL_keyOffSSG(cnl,work);break;
		case _ADPCM_F:CMDZ_CNL_keyOffADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_keyOffPPZ8(cnl,work);break;
		case _RITHM_F:CMDZ_CNL_keyOffRITHM(cnl,work);break;
	}
}
/******************************************************************************
;	○ ソフトエンベロープの処理と出力
******************************************************************************/
void CMDZ_CNL_outVolFM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int _vol=cnl->vol;
	
	int fade_vol=CMDZ_WORK_getFadeVol(work);
	if(fade_vol!=0){
		_vol+=fade_vol;
		if(_vol< 0)_vol=0;
		if(_vol>63)_vol=63;
	}
	
	if(_vol!=cnl->before_vol){
		cnl->before_vol=_vol;
		int port=cnl->cnl_port_num;
		int alg=cnl->fm_alg;
		int reg=cnl->fm_cnl_number+0x40;
		int adr=0;
		for(int i=0;i<4;i++){
			if((alg & (1 << i))!=0){
				//int vol2=cnl->vol;
				int vol2=_vol;
#ifdef SGDK
//vol2+=6;
#endif
				if(CMDZ_WORK_getFMVolFlg(work)==0){
					vol2+=cnl->fm_now_oto_tl[adr];
				}
#ifdef SGDK
//if(vol2<0)vol2=0;
//if(vol2>63)vol2=63;
#endif
				CMDZ_WORK_outReg(work,port,reg,vol2);
			}
			adr++;
			reg+=4;
		}
	}
}
#ifdef SGDK
extern void g_outputPsgVolume(int reg,int vol);
#endif
void CMDZ_CNL_outVolSSG(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int vol3=CMDZ_CNL_execSoftEnv(cnl,work);
	vol3=vol3*SSG_VOL_SCALE/100;
	if(vol3>15)vol3=15;
	
#ifdef SGDK
	int reg=0x90+(cnl->cnl_number*0x20);
	g_outputPsgVolume(reg,vol3);
#else
	int reg=cnl->cnl_number+8;
	CMDZ_WORK_outReg0(work,reg,vol3);
#endif
}
void CMDZ_CNL_outVolADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int vol3=adpcm_em_vol[cnl->vol & 0xff] << 2;
	vol3=vol3*ADPCM_VOL_SCALE/100;
	if(vol3>63)vol3=63;
	CMDZ_WORK_setPcmDriverCnlVol(work,cnl->pcm_work,vol3);
}
void CMDZ_CNL_outVolPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int vol3=CMDZ_CNL_execSoftEnv(cnl,work);
	//
	if(vol3!=cnl->before_vol){
		cnl->before_vol=vol3;
		CMDZ_WORK_setPcmDriverCnlVol(work,cnl->pcm_work,vol3);
	}
}
void CMDZ_CNL_outVol(CMDZ_CNL *cnl,CMDZ_WORK *work){
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_outVolFM(cnl,work);break;
		case _SSG_F  :CMDZ_CNL_outVolSSG(cnl,work);break;
		case _ADPCM_F:CMDZ_CNL_outVolADPCM(cnl,work);break;
		case _PPZ8_F :CMDZ_CNL_outVolPPZ8(cnl,work);break;
	}
}
/******************************************************************************
;	○ ソフトエンベロープの処理と出力
******************************************************************************/
int CMDZ_CNL_execSoftEnv(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int ev=cnl->env.vol2;
	if(!cnl->state.kon_r){
		if(!cnl->state.koff){
			if(!cnl->env.flg.ar){
				ev+=cnl->env.ar;
				if(ev>255){
					ev=255;
					cnl->env.flg.ar=true;
				}
			}
			if(!cnl->env.flg.dr && cnl->env.flg.ar){
				ev-=cnl->env.dr;
				if(ev<=cnl->env.sl){
					ev=cnl->env.sl;
					cnl->env.flg.dr=true;
				}
			}
			if(!cnl->env.flg.sr && cnl->env.flg.dr){
				ev-=cnl->env.sr;
				if(ev<0){
					ev=0;
					cnl->env.flg.sr=true;
				}
			}
		}else{
			if(!cnl->env.flg.sr){
				ev-=cnl->env.rr;
				if(ev<0){
					ev=0;
					cnl->env.flg.sr=true;
				}
			}
		}
	}
	//ENV_OUT
	cnl->state.kon_r=false;
	cnl->env.vol2=ev;
	//フェード
	int vol3=((ev+1)*cnl->vol >> 8);
	int fade_vol=CMDZ_WORK_getFadeVol(work);
	if(fade_vol!=0){
		vol3-=fade_vol;
		if(vol3<0)vol3=0;
	}
	return vol3;
}
/******************************************************************************
;	○ リズムの出力
;					b:00000001b
;					s:00000010b
;					c:00000100b
;					h:00001000b
;					t:00010000b
;					r:00100000b
******************************************************************************/
void CMDZ_CNL_outRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work){
//	CMDZ_PcmBank *rithm_pcmbank=CMDZ_WORK_getRithmPcmBank(work);
//	if(!rithm_pcmbank)return;
//	if(CMDZ_PcmBank_getTblNum(rithm_pcmbank)<6)return;
	int n=cnl->now_ontei;
	//RITHM_OUT:
	int bit=0x1;
	for(int i=0;i<6;i++){
		if((n & bit)!=0){
			//CMDZ_PcmBankTbl *pcmbanktbl=CMDZ_PcmBank_getTbl(rithm_pcmbank,i);
			//if(pcmbanktbl)
			{
				int rithm_pcm_work=cnl->pcm_work+i;
				int note=0x0800;
				int pan=g_ppz_pan_tbl[(cnl->rt_vol[i] >> 6) & 3];
				int _vol=cnl->rt_vol[i] & 63;
				int vol=_vol*RITHM_VOL_SCALE/100;
				if(vol>63)vol=63;
				//int rate   =44100;
				//CMDZ_WORK_keyOnPcmDriverCnl(work,rithm_pcm_work,pcmbanktbl,vol,note,pan);
				CMDZ_WORK_keyOnPcmDriverCnl(work,
					-1,
					NULL,
					-1,
					i,
					vol,
					note,
					pan);
			}
		}
		bit<<=1;
	}
}
//リズムボリューム設定
static int g_spb_pan_tbl[]={0,2,1,3};		//SPBのPAN補正
//const static int spb_pan_tbl[]={0,2,1,3};//SPBのPAN補正

void CMDZ_CNL_setPanRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	n=(g_spb_pan_tbl[n & 3] << 6) & 0xc0;
	int and_n=0x3f;
	CMDZ_CNL_volSubRITHM(cnl,work,n,and_n);
}
void CMDZ_CNL_setVol2RITHM(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	if(n< 0)n=0;
	if(n>63)n=63;
	int and_n=0xc0;
	CMDZ_CNL_volSubRITHM(cnl,work,n,and_n);
}
void CMDZ_CNL_volSubRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n,int and_n){
	int n2=CMDZ_CNL_readCommandUint8(cnl);
	int bit=1;
	for(int i=0;i<6;i++){
		if((n2 & bit)!=0){
			int v=cnl->rt_vol[i];
			v=(v & and_n) | n;
			cnl->rt_vol[i]=v;
		}
	}
}
/******************************************************************************
;	○ チャネルの停止
******************************************************************************/
bool CMDZ_CNL_stopBgmCnl(CMDZ_CNL *cnl,CMDZ_WORK *work){
	//ストップフラグ
	cnl->state.stop=true;
	//PCM停止
	CMDZ_WORK_stopPcmDriverCnl(work,cnl->pcm_work);
	return true;
}
/******************************************************************************
;	○ チャネルの停止
******************************************************************************/
bool CMDZ_CNL_stopCnl(CMDZ_CNL *cnl,CMDZ_WORK *work){
	//ストップフラグ
	cnl->state.stop=true;
	//休符処理
	CMDZ_CNL_rest(cnl,work);
	//PCM停止
	CMDZ_WORK_stopPcmDriverCnl(work,cnl->pcm_work);
	return true;
}
/******************************************************************************
;	○ 全チャネルの停止
******************************************************************************/
void CMDZ_CNL_stopCom(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_WORK_stopBgm(work);
}
/******************************************************************************
;	○ 音長設定
******************************************************************************/
void CMDZ_CNL_setDefLen(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint16(cnl);
	cnl->def_len=n;
}
/******************************************************************************
;	○ テンポ設定
******************************************************************************/
void CMDZ_CNL_setTempo(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_WORK_setSrcTempo(work,CMDZ_CNL_readCommandUint16(cnl));
}
//Timer B
void CMDZ_CNL_setTimerB(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int timer_b=CMDZ_CNL_readCommandUint16(cnl);
	int base_cnt=CMDZ_WORK_getBaseCnt(work);
	int _tempo=833333/(-timer_b+256)/base_cnt;
	CMDZ_WORK_setSrcTempo(work,_tempo);
}
//Timer A
void CMDZ_CNL_setTimerA(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int timer_a=CMDZ_CNL_readCommandUint16(cnl);
	int base_cnt=CMDZ_WORK_getBaseCnt(work);
	int _tempo=13333333/(-timer_a+1024)/base_cnt;
	CMDZ_WORK_setSrcTempo(work,_tempo);
}
/******************************************************************************
;	○ すべてのチャネルがループまたは終了したかチェック
******************************************************************************/
void CMDZ_CNL_checkOneLoop(CMDZ_CNL *cnl,CMDZ_WORK *work){
}
/******************************************************************************
;	○ ジャンプ、ループ命令
******************************************************************************/
//ループだ
void CMDZ_CNL_setLoopIs(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_WORK_setLoopIsFlg(work,1);
}
//ジャンプコマンド
void CMDZ_CNL_JumpCom(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(CMDZ_WORK_getOneLoopFlg(work)){
		cnl->state.oneloop=true;
	}
	int offset=CMDZ_CNL_readCommandUint16(cnl);
	CMDZ_CNL_setCommandAdr(cnl,work,offset);
}
//ループ左括弧
void CMDZ_CNL_setLoopStart(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->loop_adr++;
	cnl->loop_stack[cnl->loop_adr]=CMDZ_CNL_readCommandUint8(cnl);
}
//ループ右括弧
void CMDZ_CNL_setLoopEnd(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->loop_adr<0){
		return;
	}
	cnl->loop_stack[cnl->loop_adr]--;
	if(cnl->loop_stack[cnl->loop_adr]!=0){
		int offset=CMDZ_CNL_readCommandUint16(cnl);
		CMDZ_CNL_setCommandAdr(cnl,work,offset);
	}else{
		CMDZ_CNL_skipCommandAdr(cnl,2);
		cnl->loop_adr--;
	}
}
//ループアウト
void CMDZ_CNL_setLoopOut(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->loop_adr<0){
		return;
	}
	if(cnl->loop_stack[cnl->loop_adr]!=1){
		CMDZ_CNL_skipCommandAdr(cnl,2);
	}else{
		int offset=CMDZ_CNL_readCommandUint16(cnl);
		CMDZ_CNL_setCommandAdr(cnl,work,offset);
		cnl->loop_adr--;
	}
}
/******************************************************************************
;	○ ゲートタイムの設定
******************************************************************************/
void CMDZ_CNL_setQuota(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->quota=CMDZ_CNL_readCommandUint8(cnl);
}
/******************************************************************************
;	○ タイ
******************************************************************************/
void CMDZ_CNL_setTai(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.tai =true;
	cnl->state.sura=false;
}
/******************************************************************************
;	○ スラー
******************************************************************************/
void CMDZ_CNL_setSura(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->state.tai =true;
	cnl->state.sura=true;
}
/******************************************************************************
;	○ ディチューン
******************************************************************************/
void CMDZ_CNL_setDetune(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->detune=CMDZ_CNL_readCommandSint16(cnl);
}
/******************************************************************************
;	○ LFO設定
******************************************************************************/
//LFO初期化サブ
void CMDZ_CNL_initLFO(CMDZ_CNL *cnl,int type){
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	_lfo->md_cnt   =_lfo->md;
	_lfo->speed_cnt=_lfo->speed;
	_lfo->depth_cnt=(_lfo->depth >> 1)+(_lfo->depth & 1);
	int rate=_lfo->rate;
	if(!(_lfo->wave_num==0 || _lfo->wave_num==3)){
		rate=rate*_lfo->depth;
		if(_lfo->wave_num==4){
			rate=(rate >> 1);
			_lfo->depth_cnt=1;
		}
	}
	_lfo->rate_sub=rate;
}
//
void CMDZ_CNL_setLFO(CMDZ_CNL *cnl,CMDZ_WORK *work){
	//波形番号
	int type    =CMDZ_CNL_readCommandUint8(cnl);
	int md      =CMDZ_CNL_readCommandUint8(cnl);
	int speed   =CMDZ_CNL_readCommandUint8(cnl);
	int rate    =CMDZ_CNL_readCommandSint16(cnl);
	int depth   =CMDZ_CNL_readCommandUint8(cnl);
	int wave_num=CMDZ_CNL_readCommandUint8(cnl);
	//MA,MHは対象外
	if(type>=3)return;
	CMDZ_CNLFLG_setLFO(&cnl->state,type,true);
	CMDZ_LFO *_lfo=&cnl->lfo[type];
	_lfo->md      =md+1;
	_lfo->speed   =speed;
	_lfo->rate    =rate;
	_lfo->depth   =depth;
	_lfo->wave_num=wave_num;
	CMDZ_CNL_initLFO(cnl,type);
}
//LFO初期化
void CMDZ_CNL_initAllLFO(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(CMDZ_CNLFLG_getLFO(&cnl->state,0)){
		CMDZ_CNL_initLFO(cnl,0);
	}
	if(CMDZ_CNLFLG_getLFO(&cnl->state,1)){
		CMDZ_CNL_initLFO(cnl,1);
	}
	if(CMDZ_CNLFLG_getLFO(&cnl->state,2)){
		CMDZ_CNL_initLFO(cnl,2);
	}
	if(cnl->state.apan){
		CMDZ_CNL_initAPan(cnl,work);
	}
}
/******************************************************************************
;	○ LFOのON/OFF
******************************************************************************/
void CMDZ_CNL_setLFOFlg(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int type=CMDZ_CNL_readCommandUint8(cnl);
	int flg =CMDZ_CNL_readCommandUint8(cnl);
	if(type<3){
		if(flg==0){
			CMDZ_CNLFLG_setLFO(&cnl->state,type,false);
		}else{
			CMDZ_CNLFLG_setLFO(&cnl->state,type,true);
		}
	}else if(type==4){
		if(flg==0){
			cnl->state.alfo=false;
		}else{
			cnl->state.alfo=true;
		}
	}else if(type==5){
		if(flg==0){
			cnl->state.hlfo=false;
		}else{
			cnl->state.hlfo=true;
		}
	}
}
/******************************************************************************
;	○ ピッチベンドの指定
******************************************************************************/
void CMDZ_CNL_setVendSub(CMDZ_CNL *cnl,CMDZ_WORK *work,int ontei,int md_cnt,int speed,int rate){
	int new_ontei=ontei+cnl->soutai_icho;
	CMDZ_WAVEOCT waveoct;
	CMDZ_CNL_getOntei(cnl,&waveoct,work,new_ontei);
	cnl->vend.ontei    =ontei;
	cnl->vend.wave     =waveoct.wave;
	cnl->vend.oct_wk   =waveoct.oct;
	cnl->vend.md_cnt   =md_cnt+1;
	cnl->vend.speed    =speed;
	cnl->vend.speed_cnt=cnl->vend.speed;
	cnl->vend.rate     =rate;
	cnl->state.vend=true;
}
void CMDZ_CNL_setVendOld(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int ontei =CMDZ_CNL_readCommandUint8(cnl);
	int md_cnt=CMDZ_CNL_readCommandUint8(cnl);
	int speed =CMDZ_CNL_readCommandUint8(cnl);
	int rate  =CMDZ_CNL_readCommandSint8(cnl);	//byte
	CMDZ_CNL_setVendSub(cnl,work,ontei,md_cnt,speed,rate);
}
void CMDZ_CNL_setVend(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int ontei =CMDZ_CNL_readCommandUint8(cnl);
	int md_cnt=CMDZ_CNL_readCommandUint8(cnl);
	int speed =CMDZ_CNL_readCommandUint8(cnl);
	int rate  =CMDZ_CNL_readCommandSint16(cnl);	//word
	//
	CMDZ_CNL_setVendSub(cnl,work,ontei,md_cnt,speed,rate);
}
/******************************************************************************
;	○ ボリューム設定
******************************************************************************/
void CMDZ_CNL_setVolFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	if(n<0)n=0;
	if(n>15)n=15;
	cnl->vol=fm_vol_tbl[n];
}
void CMDZ_CNL_setVolSSG(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol=n;
}
void CMDZ_CNL_setVolRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol=n;
}
void CMDZ_CNL_setVolADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol=n;
}
void CMDZ_CNL_setVolPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	if(!CMDZ_WORK_getP8Data(work)){
		int _vol=(n << 2)*SSGPPZ8_VOL_SCALE/100;
		if(_vol>63)_vol=63;
		cnl->vol=_vol;
	}else{
		cnl->vol=n << 2;
	}
}
void CMDZ_CNL_setVol(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_setVolFM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_setVolSSG(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_setVolADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_setVolPPZ8(cnl,work,n);break;
		case _RITHM_F:CMDZ_CNL_setVolRITHM(cnl,work,n);break;
	}
}
//==================
void CMDZ_CNL_setVol2FM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol=n;
}
void CMDZ_CNL_setVol2SSG(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol=n;
}
void CMDZ_CNL_setVol2ADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
}
void CMDZ_CNL_setVol2PPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	if(!CMDZ_WORK_getP8Data(work)){
		int vol=(n << 2)*SSGPPZ8_VOL_SCALE/100;
		if(vol>63)vol=63;
		cnl->vol=vol;
	}else{
		cnl->vol=n << 2;
	}
}
void CMDZ_CNL_setVol2(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->cnl_cate==_RITHM_F){
		CMDZ_CNL_setVol2RITHM(cnl,work);
		return;
	}
	//
	int n=CMDZ_CNL_readCommandUint8(cnl);
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_setVol2FM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_setVol2SSG(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_setVol2ADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_setVol2PPZ8(cnl,work,n);break;
	}
}
/******************************************************************************
;	○ ボリュームのUP
******************************************************************************/
void CMDZ_CNL_upVolFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol-=n;
	if(cnl->vol<0)cnl->vol=0;
}
void CMDZ_CNL_upVolSSG(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol+=n;
	if(cnl->vol>15)cnl->vol=0;
}
void CMDZ_CNL_upVolADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol+=(n << 1);
	if(cnl->vol>255)cnl->vol=255;
}
void CMDZ_CNL_upVolPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol+=n;
	if(cnl->vol>63)cnl->vol=63;
}
void CMDZ_CNL_upVolRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol+=n;
	if(cnl->vol>63)cnl->vol=63;
}
void CMDZ_CNL_upVol(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	//
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_upVolFM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_upVolSSG(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_upVolADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_upVolPPZ8(cnl,work,n);break;
		case _RITHM_F:CMDZ_CNL_upVolRITHM(cnl,work,n);break;
	}
}
/******************************************************************************
;	○ ボリュームのDOWN
******************************************************************************/
void CMDZ_CNL_downVolFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol+=n;
	if(cnl->vol>127)cnl->vol=127;
}
void CMDZ_CNL_downVolSSG(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol-=n;
	if(cnl->vol<0)cnl->vol=0;
}
void CMDZ_CNL_downVolADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol-=(n << 1);
	if(cnl->vol<0)cnl->vol=0;
}
void CMDZ_CNL_downVolPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol-=n;
	if(cnl->vol<0)cnl->vol=0;
}
void CMDZ_CNL_downVolRITHM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->vol-=n;
	if(cnl->vol<0)cnl->vol=0;
}
void CMDZ_CNL_downVol(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	//
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_downVolFM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_downVolSSG(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_downVolADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_downVolPPZ8(cnl,work,n);break;
		case _RITHM_F:CMDZ_CNL_downVolRITHM(cnl,work,n);break;
	}
}
/******************************************************************************
;	ノイズセット
******************************************************************************/
void CMDZ_CNL_setNoise(CMDZ_CNL *cnl,CMDZ_WORK *work){
	CMDZ_WORK_setSsgNoise(work,CMDZ_CNL_readCommandUint8(cnl));
}
/******************************************************************************
;	SSGのミキサー設定
******************************************************************************/
void CMDZ_CNL_setSSGMixer(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->pan=n & 3;
	int _cnl=cnl->cnl_number;
	int al=ssg_mixer_tbl[cnl->pan] << _cnl;		//ALに出力データ
	int ah=ssg_mixer_mask[_cnl];			//AHにANDデータ(11110110B)
	
	int ssg_mixer=CMDZ_WORK_getSsgMixer(work);
	ssg_mixer&=ah;
	ssg_mixer&=al;
	CMDZ_WORK_setSsgMixer(work,ssg_mixer);
}
/******************************************************************************
;	○ PANの設定
******************************************************************************/
void CMDZ_CNL_setPanFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->pan=n;
	int port=cnl->cnl_port_num;
	int reg=cnl->fm_cnl_number+0xb4;
	int val=(cnl->pan & 3) << 6;
	CMDZ_WORK_outReg(work,port,reg,val);
}
void CMDZ_CNL_setPanPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->pan=n;
	cnl->state.apan=false;
	CMDZ_WORK_setPcmDriverCnlPan(work,cnl->pcm_work,cnl->pan);
}
void CMDZ_CNL_setPanADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	CMDZ_CNL_setPanPPZ8(cnl,work,g_ppz_pan_tbl[n]);
}
void CMDZ_CNL_setPan(CMDZ_CNL *cnl,CMDZ_WORK *work){
	if(cnl->cnl_cate==_RITHM_F){
		CMDZ_CNL_setPanRITHM(cnl,work);
		return;
	}
	int n=CMDZ_CNL_readCommandUint8(cnl);
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_setPanFM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_setSSGMixer(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_setPanADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_setPanPPZ8(cnl,work,n);break;
	}
}
/******************************************************************************
;	○ オートPANの設定
******************************************************************************/
void CMDZ_CNL_initAPan(CMDZ_CNL *cnl,CMDZ_WORK *work){
	cnl->apan.md_cnt   =cnl->apan.md;
	cnl->apan.speed_cnt=cnl->apan.speed;
	cnl->apan.sorc_w   =cnl->apan.sorc;
	cnl->apan.num      =cnl->apan.sorc;
	cnl->apan.dist_w   =cnl->apan.dist;
	if(cnl->apan.sorc==cnl->apan.dist){
		cnl->state.apan=false;
	}else{
		if(cnl->apan.sorc<cnl->apan.dist){
			cnl->apan.add= 1;
		}else{
			cnl->apan.add=-1;
		}
		CMDZ_WORK_setPcmDriverCnlPan(work,cnl->pcm_work,cnl->apan.sorc);
	}
}
void CMDZ_CNL_setAutoPan(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int md   =CMDZ_CNL_readCommandUint8(cnl);
	int speed=CMDZ_CNL_readCommandUint8(cnl);
	int sorc =CMDZ_CNL_readCommandUint8(cnl);
	int dist =CMDZ_CNL_readCommandUint8(cnl);
	int type =CMDZ_CNL_readCommandUint8(cnl);
	//オートパン開始
	cnl->apan.md   =md+1;
	cnl->apan.speed=speed;
	cnl->apan.sorc =sorc;
	cnl->apan.dist =dist;
	cnl->apan.type =type;
	cnl->state.apan=true;
	CMDZ_CNL_initAPan(cnl,work);
}
/******************************************************************************
;	○ 休符でキーオフするか
******************************************************************************/
void CMDZ_CNL_setRestOff(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	if(n==0){
		cnl->state.rest_off=false;
	}else{
		cnl->state.rest_off=true;
	}
}
/******************************************************************************
;	○ 音色切り替え
******************************************************************************/
void CMDZ_CNL_selectNeiroFM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->oto_num=n;
	//
	CMDZ_BgmFmNeiro neiro;
	if(!CMDZ_WORK_getFmNeiro(work,cnl->oto_num,&neiro))return;
	//
	//int adr=0x04;
	//TLデータの保存
	for(int i=0;i<4;i++){
		cnl->fm_now_oto_tl[i]=neiro.data[i+0x04];
	}
	int port=cnl->cnl_port_num;
	//RR End
	{
	int reg=cnl->fm_cnl_number+0x80;
	for(int i=0;i<4;i++){
		CMDZ_WORK_outReg(work,port,reg,15);
		reg+=4;
	}
	}
	//
	int adr=0;
	int reg=cnl->fm_cnl_number+0x30;
	for(int i=0;i<24;i++){
		int data=neiro.data[adr];
		CMDZ_WORK_outReg(work,port,reg,data);
		reg+=4;
		adr++;
	}
	//ALG
	int alg=neiro.data[adr];
	reg+=0x20;
	CMDZ_WORK_outReg(work,port,reg,alg);
	cnl->fm_alg=fm_alg_tbl[alg & 0x7];
	cnl->before_vol=255;
	//
	CMDZ_CNL_outVolFM(cnl,work);
}
void CMDZ_CNL_selectNeiroSSG(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	cnl->oto_num=n;
	//
	CMDZ_BgmSsgNeiro neiro;
	if(!CMDZ_WORK_getSsgNeiro(work,cnl->oto_num,&neiro))return;
	cnl->env.sv=neiro.data[0];
	cnl->env.ar=neiro.data[1];
	cnl->env.dr=neiro.data[2];
	cnl->env.sl=neiro.data[3];
	cnl->env.sr=neiro.data[4];
	cnl->env.rr=neiro.data[5];
}
void CMDZ_CNL_selectNeiroPPZ8(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	if(cnl->oto_num!=n){
		cnl->oto_num=n;
		CMDZ_CNL_feedbackPcmNeiro(cnl,work);
	}
}
void CMDZ_CNL_selectNeiroADPCM(CMDZ_CNL *cnl,CMDZ_WORK *work,int n){
	CMDZ_CNL_selectNeiroPPZ8(cnl,work,n);
}
void CMDZ_CNL_selectNeiro(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	switch(cnl->cnl_cate){
		case _FM_F   :CMDZ_CNL_selectNeiroFM(cnl,work,n);break;
		case _SSG_F  :CMDZ_CNL_selectNeiroSSG(cnl,work,n);break;
		case _ADPCM_F:CMDZ_CNL_selectNeiroADPCM(cnl,work,n);break;
		case _PPZ8_F :CMDZ_CNL_selectNeiroPPZ8(cnl,work,n);break;
	}
}
/******************************************************************************
;	○ バンク切り替え
******************************************************************************/
void CMDZ_CNL_selectBank(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint16(cnl);
	//
	if(n<0 || n>=PZI_BANK_MAX){
#ifndef SGDK
		printf("BankSel Bank[%d] Error !!",n);
#endif
		return;
	}
	if(cnl->oto_bank!=n){
		cnl->oto_bank=n;
		CMDZ_CNL_feedbackPcmNeiro(cnl,work);
	}
}
/******************************************************************************
;	○ PCMループポインタ設定
******************************************************************************/
void CMDZ_CNL_setPcmLoopPoint(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int bank      =CMDZ_CNL_readCommandUint8(cnl);	//0
	int index     =CMDZ_CNL_readCommandUint8(cnl);	//1
	int loop_start=CMDZ_CNL_readCommandUint32(cnl);	//2,3
	int loop_end  =CMDZ_CNL_readCommandUint32(cnl);	//6,7
	//
	CMDZ_WORK_setPcmLoopPoint(work,bank,index,loop_start,loop_end);
}
/******************************************************************************
;	○ エンベロープ設定
******************************************************************************/
void CMDZ_CNL_setEnv(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int sv=CMDZ_CNL_readCommandUint8(cnl);
	int ar=CMDZ_CNL_readCommandUint8(cnl);
	int dr=CMDZ_CNL_readCommandUint8(cnl);
	int sl=CMDZ_CNL_readCommandUint8(cnl);
	int sr=CMDZ_CNL_readCommandUint8(cnl);
	int rr=CMDZ_CNL_readCommandUint8(cnl);
	//
	cnl->env.sv=sv;
	cnl->env.ar=ar;
	cnl->env.dr=dr;
	cnl->env.sl=sl;
	cnl->env.sr=sr;
	cnl->env.rr=rr;
}
/******************************************************************************
;	○ 相対移調
******************************************************************************/
void CMDZ_CNL_setSoutaiIcho(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandSint8(cnl);
	if(n==0){
		cnl->soutai_icho=0;
	}else{
		cnl->soutai_icho+=n;
	}
}
/******************************************************************************
;	○ フェードアウト
******************************************************************************/
void CMDZ_CNL_setFadeCom(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	CMDZ_WORK_fadeBgm(work,n);
}
/******************************************************************************
;	○ 外部にデータを送る
******************************************************************************/
void CMDZ_CNL_sendData(CMDZ_CNL *cnl,CMDZ_WORK *work){
	int n=CMDZ_CNL_readCommandUint8(cnl);
	CMDZ_WORK_setSendData(work,n);
}
/******************************************************************************
;	○ ダミー
******************************************************************************/
void CMDZ_CNL_dummyCom(CMDZ_CNL *cnl,CMDZ_WORK *work){
}
/******************************************************************************
;	○ コマンドジャンプ
******************************************************************************/
CMDZ_CNL_COMMAND *g_cnl_command_tbl[]={
	&CMDZ_CNL_setVol,					//0x81 音量セット
	&CMDZ_CNL_upVol,					//0x82 音量アップ
	&CMDZ_CNL_downVol,					//0x83 音量ダウン
	&CMDZ_CNL_setTimerB,				//0x84 タイマーセット
	&CMDZ_CNL_JumpCom,					//0x85 ジャンプ
	&CMDZ_CNL_setLoopEnd,				//0x86 ループ
	&CMDZ_CNL_setLoopOut,				//0x87 ループアウト
	&CMDZ_CNL_setQuota,					//0x88 ゲートタイムセット
	&CMDZ_CNL_setDetune,				//0x89 ディチューン
	&CMDZ_CNL_setLFO,					//0x8A LFOセット
	&CMDZ_CNL_setLFOFlg,				//0x8B LFOON/OFF
	&CMDZ_CNL_dummyCom,					//0x8C
	&CMDZ_CNL_selectNeiro,				//0x8D 音色セット
	&CMDZ_CNL_setPan,					//0x8E PANセット
	&CMDZ_CNL_setNoise,					//0x8F ノイズセット
	&CMDZ_CNL_setEnv,					//0x90 ソフトエンベロープのセット
	&CMDZ_CNL_setVol2,					//0x91 詳細音量セット
	&CMDZ_CNL_setTai,					//0x92 タイ
	&CMDZ_CNL_setLoopStart,				//0x93 ループ左カッコ
	&CMDZ_CNL_dummyCom,					//0x94 シンクを送る
	&CMDZ_CNL_dummyCom,					//0x95 シンクを待つ
	&CMDZ_CNL_setFadeCom,				//0x96 フェードアウト
	&CMDZ_CNL_setVendOld,				//0x97 ベンド設定1
	&CMDZ_CNL_dummyCom,					//0x98 PCM_F_SET
	&CMDZ_CNL_sendData,					//0x99 データを送る
	&CMDZ_CNL_setSoutaiIcho,			//0x9A 相対移調
	&CMDZ_CNL_setSura,					//0x9B スラー
	&CMDZ_CNL_setDefLen,				//0x9C デフォルト音長の設定
	&CMDZ_CNL_selectBank,				//0x9D バンクの設定
	&CMDZ_CNL_dummyCom,					//0x9E MIDIEffect
	&CMDZ_CNL_dummyCom,					//0x9F ベンドレンジ
	&CMDZ_CNL_dummyCom,					//0xA0 ベロシティＵＰ
	&CMDZ_CNL_dummyCom,					//0xA1 ベロシティＤＯＷＮ
	&CMDZ_CNL_dummyCom,					//0xA2 チャネル番号設定
	&CMDZ_CNL_dummyCom,					//0xA3 PPZのPAN設定
	&CMDZ_CNL_dummyCom,					//0xA4 ベロシティセット
	&CMDZ_CNL_setAutoPan,				//0xA5 オートパン設定
	&CMDZ_CNL_setRestOff,				//0xA6 休符でキーオフする
	&CMDZ_CNL_dummyCom,					//0xA7 TIMERA設定
	&CMDZ_CNL_setTempo,					//0xA8 テンポ指定
	&CMDZ_CNL_setVend,					//0xA9 ベンド設定2
	&CMDZ_CNL_setPcmLoopPoint,			//0xAA ループポインタ設定
	&CMDZ_CNL_setLoopIs,				//0xAB ループだ
};
CMDZ_CNL_COMMAND *g_cnl_command_tbl_fm[]={
	&CMDZ_CNL_setVol,					//0x81 音量セット
	&CMDZ_CNL_upVol,					//0x82 音量アップ
	&CMDZ_CNL_downVol,				//0x83 音量ダウン
	&CMDZ_CNL_setTimerB,				//0x84 タイマーセット
	&CMDZ_CNL_JumpCom,					//0x85 ジャンプ
	&CMDZ_CNL_setLoopEnd,				//0x86 ループ
	&CMDZ_CNL_setLoopOut,				//0x87 ループアウト
	&CMDZ_CNL_setQuota,					//0x88 ゲートタイムセット
	&CMDZ_CNL_setDetune,				//0x89 ディチューン
	&CMDZ_CNL_setLFO,					//0x8A LFOセット
	&CMDZ_CNL_setLFOFlg,				//0x8B LFOON/OFF
	&CMDZ_CNL_dummyCom,					//0x8C
	&CMDZ_CNL_selectNeiroFM,			//0x8D 音色セット
	&CMDZ_CNL_setPanFM,					//0x8E PANセット
	&CMDZ_CNL_setNoise,					//0x8F ノイズセット
	&CMDZ_CNL_setEnv,					//0x90 ソフトエンベロープのセット
	&CMDZ_CNL_setVol2,					//0x91 詳細音量セット
	&CMDZ_CNL_setTai,					//0x92 タイ
	&CMDZ_CNL_setLoopStart,				//0x93 ループ左カッコ
	&CMDZ_CNL_dummyCom,					//0x94 シンクを送る
	&CMDZ_CNL_dummyCom,					//0x95 シンクを待つ
	&CMDZ_CNL_setFadeCom,				//0x96 フェードアウト
	&CMDZ_CNL_setVendOld,				//0x97 ベンド設定1
	&CMDZ_CNL_dummyCom,					//0x98 PCM_F_SET
	&CMDZ_CNL_sendData,					//0x99 データを送る
	&CMDZ_CNL_setSoutaiIcho,			//0x9A 相対移調
	&CMDZ_CNL_setSura,					//0x9B スラー
	&CMDZ_CNL_setDefLen,				//0x9C デフォルト音長の設定
	&CMDZ_CNL_dummyCom,					//0x9D バンクの設定
	&CMDZ_CNL_dummyCom,					//0x9E MIDIEffect
	&CMDZ_CNL_dummyCom,					//0x9F ベンドレンジ
	&CMDZ_CNL_dummyCom,					//0xA0 ベロシティＵＰ
	&CMDZ_CNL_dummyCom,					//0xA1 ベロシティＤＯＷＮ
	&CMDZ_CNL_dummyCom,					//0xA2 チャネル番号設定
	&CMDZ_CNL_dummyCom,					//0xA3 PPZのPAN設定
	&CMDZ_CNL_dummyCom,					//0xA4 ベロシティセット
	&CMDZ_CNL_dummyCom,					//0xA5 オートパン設定
	&CMDZ_CNL_setRestOff,				//0xA6 休符でキーオフする
	&CMDZ_CNL_dummyCom,					//0xA7 TIMERA設定
	&CMDZ_CNL_setTempo,					//0xA8 テンポ指定
	&CMDZ_CNL_setVend,					//0xA9 ベンド設定2
	&CMDZ_CNL_setPcmLoopPoint,			//0xAA ループポインタ設定
	&CMDZ_CNL_setLoopIs,				//0xAB ループだ
};
void CMDZ_CNL_execCommand(CMDZ_CNL *cnl,CMDZ_WORK *work,int command){
	if(command<=0x80 || command>=0xac)return;
	
	CMDZ_CNL_COMMAND **cnl_command_tbl;
	switch(cnl->cnl_cate){
		case _FM_F:
			cnl_command_tbl=g_cnl_command_tbl_fm;
			break;
		default:
			cnl_command_tbl=g_cnl_command_tbl;
	}
cnl_command_tbl=g_cnl_command_tbl;
//	CMDZ_CNL_COMMAND *cnl_cmd=g_cnl_command_tbl[command-0x81];
	CMDZ_CNL_COMMAND *cnl_cmd=cnl_command_tbl[command-0x81];
	if(!cnl_cmd)return;
	cnl_cmd(cnl,work);
}

/*
void CMDZ_CNL_execCommand(CMDZ_CNL *cnl,CMDZ_WORK *work,int command){
	if(command==0x8D){
		CMDZ_CNL_COMMAND *cnl_cmd=&CMDZ_CNL_selectNeiro;
		cnl_cmd(cnl,work);
		return;
	}
	
	switch(command){
		case 0x81:CMDZ_CNL_setVol(cnl,work);break;			//0x81 音量セット
		case 0x82:CMDZ_CNL_upVol(cnl,work);break;			//0x82 音量アップ
		case 0x83:CMDZ_CNL_downVol(cnl,work);break;			//0x83 音量ダウン
		case 0x84:CMDZ_CNL_setTimerB(cnl,work);break;		//0x84 タイマーセット
		case 0x85:CMDZ_CNL_JumpCom(cnl,work);break;			//0x85 ジャンプ
		case 0x86:CMDZ_CNL_setLoopEnd(cnl,work);break;		//0x86 ループ
		case 0x87:CMDZ_CNL_setLoopOut(cnl,work);break;		//0x87 ループアウト
		case 0x88:CMDZ_CNL_setQuota(cnl,work);break;			//0x88 ゲートタイムセット
		case 0x89:CMDZ_CNL_setDetune(cnl,work);break;		//0x89 ディチューン
		case 0x8A:CMDZ_CNL_setLFO(cnl,work);break;			//0x8A LFOセット
		case 0x8B:CMDZ_CNL_setLFOFlg(cnl,work);break;		//0x8B LFOON/OFF
//		case 0x8c:											//0x8C
		case 0x8D:CMDZ_CNL_selectNeiro(cnl,work);break;		//0x8D 音色セット
		case 0x8E:CMDZ_CNL_setPan(cnl,work);break;			//0x8E PANセット
		case 0x8F:CMDZ_CNL_setNoise(cnl,work);				//0x8F ノイズセット
		case 0x90:CMDZ_CNL_setEnv(cnl,work);break;			//0x90 ソフトエンベロープのセット
		case 0x91:CMDZ_CNL_setVol2(cnl,work);break;			//0x91 詳細音量セット
		case 0x92:CMDZ_CNL_setTai(cnl,work);break;			//0x92 タイ
		case 0x93:CMDZ_CNL_setLoopStart(cnl,work);break;		//0x93 ループ左カッコ
//		case 0x94:CMDZ_CNL_SyncCom(cnl,work);break;			//0x94 シンクを送る
//		case 0x95:CMDZ_CNL_WaitCom(cnl,work);break;			//0x95 シンクを待つ
		case 0x96:CMDZ_CNL_setFadeCom(cnl,work);break;		//0x96 フェードアウト
		case 0x97:CMDZ_CNL_setVendOld(cnl,work);break;		//0x97 ベンド設定1
//		case 0x98:CMDZ_CNL_PcmFSet(cnl,work);break;			//0x98 PCM_F_SET
		case 0x99:CMDZ_CNL_sendData(cnl,work);break;			//0x99 データを送る
		case 0x9A:CMDZ_CNL_setSoutaiIcho(cnl,work);break;	//0x9A 相対移調
		case 0x9B:CMDZ_CNL_setSura(cnl,work);break;			//0x9B スラー
		case 0x9C:CMDZ_CNL_setDefLen(cnl,work);break;		//0x9C デフォルト音長の設定
		case 0x9D:CMDZ_CNL_selectBank(cnl,work);break;		//0x9D バンクの設定
//		case 0x9E:								//0x9E MIDIEffect
//		case 0x9F:								//0x9F ベンドレンジ
//		case 0xA0:								//0xA0 ベロシティＵＰ
//		case 0xA1:								//0xA1 ベロシティＤＯＷＮ
//		case 0xA2:								//0xA2 チャネル番号設定
//		case 0xA3:								//0xA3 PPZのPAN設定
//		case 0xA4:								//0xA4 ベロシティセット
		case 0xA5:CMDZ_CNL_setAutoPan(cnl,work);break;		//0xA5 オートパン設定
		case 0xA6:CMDZ_CNL_setRestOff(cnl,work);break;		//0xA6 休符でキーオフする
//		case 0xA7:CMDZ_CNL_setTimerA(cnl,work);break;		//0xA7 TIMERA設定
		case 0xA8:CMDZ_CNL_setTempo(cnl,work);break;			//0xA8 テンポ指定
		case 0xA9:CMDZ_CNL_setVend(cnl,work);break;			//0xA9 ベンド設定2
		case 0xAA:CMDZ_CNL_setPcmLoopPoint(cnl,work);break;	//0xAA ループポインタ設定
		case 0xAB:CMDZ_CNL_setLoopIs(cnl,work);break;		//0xAB ループだ
		default:
			{
			const char *name=CMDZ_Tools_getCommandName(command);
			if(!name)name="---";
			CMDZ_CNL_errorStop(cnl,work,"Unkown Command[%02x](%s)",command,name);
			}
	}
}
*/

/*****************************************************************************
;	
******************************************************************************/
#ifdef SGDK
#else
}
#endif



