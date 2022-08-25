/*-----------------------------------------------------------------------------
;	mdzdrv_c.h
-----------------------------------------------------------------------------*/
#ifndef _MDZDRV_C_H_
#define _MDZDRV_C_H_

//#define SGDK
#include "mdzdrv_c_def.h"

//===============================
#ifdef SGDK
#else
extern "C" {
#endif


/*****************************************************************************
;	struct
******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS

#ifndef _CRT_SECURE_NO_DEPRECATE
#define	_CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef SGDK
//===============================
#include "genesis.h"

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef long int32_t;
typedef unsigned long uint32_t;

typedef uint32_t size_t;
typedef uint32_t uintptr_t;

typedef unsigned short bool;
//typedef uint8_t  bool;
typedef bool BOOL;


#define null	((void*)0)
#define true	((bool)1)
#define false	((bool)0)
#ifndef NULL
#define NULL	((void*)0)
#endif
#ifndef TRUE
#define TRUE	((BOOL)1)
#endif
#ifndef FALSE
#define FALSE	((BOOL)0)
#endif


/*
static void printf(const char *m,...){
	
}
static void *malloc(size_t size){
	return MEM_alloc((u16)size);
}
static void free(void *adr){
	MEM_free(adr);
}
*/

//===============================
#else

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

#endif
//===============================

typedef uint8_t  Uint8;
typedef int8_t   Sint8;
typedef uint16_t Uint16;
typedef int16_t  Sint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;

//===============================
#include "mdz_def.h"
/*****************************************************************************
;	struct
******************************************************************************/
//バイナリデータ
typedef struct CMDZ_BinData{
	void *buff;
	size_t buff_size;
	bool alloc_flg;
}CMDZ_BinData;
//チャネル情報
typedef struct CMDZ_BgmDataCnl{
	int type;
	int index;
	void *offset;
}CMDZ_BgmDataCnl;
//PCMファイル設定
typedef struct CMDZ_BgmPcmFile{
	char file[256];
	int cate;
}CMDZ_BgmPcmFile;
//コメント
typedef struct CMDZ_BgmComment{
	char comment[256];
}CMDZ_BgmComment;
//ADPCM音色
typedef struct CMDZ_BgmAdpcmNeiro{
	uint16_t start;
	uint16_t end;
	uint16_t rate;
}CMDZ_BgmAdpcmNeiro;
//FM音色
typedef struct CMDZ_BgmFmNeiro{
	uint8_t data[25];
}CMDZ_BgmFmNeiro;
//PSG音色
typedef struct CMDZ_BgmSsgNeiro{
	uint8_t data[6];
}CMDZ_BgmSsgNeiro;

//PZIDATA 
typedef struct CMDZ_PcmBankTbl{
	
}CMDZ_PcmBankTbl;
//PZIDATA 
typedef struct CMDZ_PcmBank{
	
}CMDZ_PcmBank;
//OffsetTbl
typedef struct CMDZ_OffsetTblOne{
	int off;
	char name[256];
}CMDZ_OffsetTblOne;
//OffsetTbl
typedef struct CMDZ_OffsetTbl{
	CMDZ_OffsetTblOne tbl[64];
	int tbl_num;
}CMDZ_OffsetTbl;


typedef struct CMDZ_BgmData{
	CMDZ_BinData bindata;
	CMDZ_BgmDataCnl cnltbl[BGM_CNL_MAX];
	CMDZ_BgmPcmFile pcm_file[PZI_BANK_MAX];
	CMDZ_BgmComment comment;
	int ex_flg;
	int qflg;
	int fm_oto;
	int ssg_oto;
	int adpcm_oto;
	int ppz8_oto;
	int fm_oto_num;
	int ssg_oto_num;
	int adpcm_oto_num;
	int ppz8_oto_num;
	int cnl_num;
	int base_cnt;
	int pri;
	int data_top2;
	CMDZ_PcmBank *pzi_tbl[PZI_BANK_MAX];
	CMDZ_OffsetTbl offtbl;
}CMDZ_BgmData;
/*****************************************************************************
;	struct
******************************************************************************/
//PCM作成のパラメータ
typedef struct CMDZ_MakePara{
	int loopadr;			//ループアドレス
	int endadr;				//終了アドレス
	int nowsize;			//現在のサイズ
	bool play_flg;		//再生終了フラグ
}CMDZ_MakePara;


typedef struct CMDZ_PcmInfo{
	int cnl;
	int bit;
	int rate;
	int sample_num;
	bool loop_flg;
	int loop_start;
	int loop_end;
}CMDZ_PcmInfo;


//
typedef struct CMDZ_WAVEOCT{
	int wave;
	int oct;
}CMDZ_WAVEOCT;
//ループテーブル
typedef struct CMDZ_PZILT{
	int loop_start;				//
	int loop_end;				//
}CMDZ_PZILT;
//
typedef struct CMDZ_PZIL{
	CMDZ_PcmBank *pcmbank;
	CMDZ_PZILT *tbl;				//128
	int tbl_num;				//
}CMDZ_PZIL;
//キーオン時PCM登録用ワーク
typedef struct CMDZ_PcmTone{
	int start;			//スタート
	int end;			//エンド
	int loop;			//ループ
	int loop_flg;		//ループフラグ
	int rate;			//レート
	size_t size;		//
	CMDZ_PcmBankTbl *pzit;
}CMDZ_PcmTone;
//チャンネルフラグ
typedef struct CMDZ_CNLFLG{
	uint32_t tai:1;			//タイ
	uint32_t tai2:1;		//タイ2
	uint32_t stop:1;		//停止
	uint32_t rest:1;		//休符
	uint32_t kon:1;			//キーオン
	uint32_t koff:1;		//キーオフ
	uint32_t start:1;		//スタート
	uint32_t vend:1;		//ベンド
	uint32_t lfo:3; 		//P,Q,R LFO
	uint32_t alfo:1;		//A_LFO
	uint32_t hlfo:1;		//H_LFO
	uint32_t kon_r:1;		//キーオン
	uint32_t sura:1;		//スラー
	uint32_t sura2:1;		//スラー２
	uint32_t apan:1;		//APAN
	uint32_t rest_off:1;	//REST_OFF
	uint32_t oneloop:1;		//1周した
	uint32_t dummy:32-19;	//
}CMDZ_CNLFLG;
//LFO
typedef struct CMDZ_LFO{
	int md;				//LFOデェレイ
	int md_cnt;			//LFOデェレイカウンタ
	int speed;			//LFOスピード
	int speed_cnt;		//LFOスピードカウンタ
	int rate;			//LFO大きさ
	int rate_sub;		//LFO大きさサブ
	int depth;			//LFO深さ
	int depth_cnt;		//LFO深さカウンタ
	int wave_num;		//LFOの波形番号
}CMDZ_LFO;
//ALFO
typedef struct CMDZ_ALFO{
}CMDZ_ALFO;
//HLFO
typedef struct CMDZ_HLFO{
}CMDZ_HLFO;
//VEND
typedef struct CMDZ_VEND{
	int md_cnt;			//ピッチベンドのデュレイ
	int speed;			//ピッチベンドのスピード
	int speed_cnt;		//ピッチベンドのスピードカウンタ
	int ontei;			//ピッチベンドの音程
	int rate;			//ピッチベンドの大きさ
	int wave;			//ピッチベンドの目標周波数
	int oct_wk;			//
}CMDZ_VEND;
//ENV
typedef struct CMDZ_ENVFLG{
	uint8_t ar:1;			//AR
	uint8_t dr:1;			//DR
	uint8_t sr:1;			//SR
	uint8_t dummy:8-2;
}CMDZ_ENVFLG;
typedef struct CMDZ_ENV{
	CMDZ_ENVFLG flg;
	int vol2;			//エンベロープ用ボリューム
	int sv;				//エンベロープ用スタートボリュー
	int ar;				//アタックレート
	int dr;				//デュケイレート
	int sl;				//サスティンレベル
	int sr;				//サスティンレート
	int rr;				//リリースレート
}CMDZ_ENV;
//オートPAN
typedef struct CMDZ_APAN{
	int md;				//デュレイ
	int md_cnt;			//デュレイ
	int speed;			//スピード
	int speed_cnt;		//スピードカウンタ
	int sorc;			//PANソース
	int dist;			//PANディスト
	int sorc_w;			//PANソース
	int dist_w;			//PANディスト
	int num;			//現在のPAN
	int add;			//PAN加算方向
	int type;			//PANの種類
}CMDZ_APAN;
//
typedef struct CMDZ_FMChip{
	bool (*hasPort)(int port);
	void (*outReg0)(int reg,int data);
	void (*outReg)(int port,int reg,int data);
}CMDZ_FMChip;
//
typedef struct CMDZ_PcmDriver{
	void (*startPCMBuffer)();
	void (*setTimer)(int tempo,int base_cnt);
	int (*getNowMakePCMSize)(void);
	int (*getPCMBufferAdr)(void);
	void (*allocCnl)(int cnl);
	void (*initCnl)(int cnl);
	void (*stopCnl)(int cnl);
//	void (*keyOnCnl)(int cnl,CMDZ_PcmBankTbl *pcmbanktbl,int vol,int _note,int _pan);
	void (*keyOnCnl)(int cnl,CMDZ_PcmBankTbl *pcmbanktbl,int bank,int oto,int vol,int _note,int _pan);
	void (*setPanCnl)(int cnl,int pan);
	void (*setNoteCnl)(int cnl,int wave);
	void (*setVolumeCnl)(int cnl,int vol);
}CMDZ_PcmDriver;

//
//チャネルワーク
typedef struct CMDZ_CNL{
	//基本
	int loop_stack[CNL_LOOPSTACK_MAX];	//ループスタック
	int loop_adr;						//ループスタックのアドレス
	void *data_adr_top;					//データアドレス
	void *data_adr;						//データアドレス
	CMDZ_CNLFLG state;					//ステータス
	int def_len;						//デフォルトの音長
	int len_wk;							//音長のカウンタ
	int wave;							//現在の周波数
	int oct_wk;							//現在の周波数
	int before_wave;					//前の周波数
	int quota_wk;						//ゲートタイムカウンタ
	int detune;							//ディチューン
	int cnl_index;						//チャネルindex
	int cnl_cate;						//チャネル種類
	int cnl_number;						//チャネル番号
	int cnl_port_num;					//チャネルポート番号
	int quota;							//ゲートタイム
	int now_ontei;						//現在の音程
	int before_ontei;					//前の音程
	int vol;							//ボリューム
	int before_vol;						//前の音量
	int soutai_icho;					//相対移調の値
	int pan;							//PAN
	int oto_num;						//音色番号
	int now_cmd;
	int now_len;
	//拡張
	CMDZ_LFO lfo[3];						//LFO
	CMDZ_VEND vend;						//ピッチベンド
	//FM
	int fm_cnl_number;					//
	//int fm_now_oto;					//FM音源の現在の音色アドレス
	int fm_now_oto_tl[4];				//
	int fm_alg;							//アルゴリズム
	//SSG/PPZ8
	CMDZ_ENV env;						//エンベロープ
	//リズム
	int rt_vol[6];						//リズムのボリューム
	//PPZ8
	CMDZ_APAN apan;						//オートパン
	CMDZ_PcmTone pcmtone;				//PCM音色
	int oto_bank;						//バンク番号
	int pcm_work;						//使用するPCMチャネル
}CMDZ_CNL;
//
typedef struct CMDZ_WORK{
	CMDZ_BgmData *bgmr;			//BGMデータ
	CMDZ_PZIL *pzi_lp[PZI_BANK_MAX];		//PCMバンクテーブル
	uint8_t keyon_tbl[BGM_CNL_MAX];
	CMDZ_CNL *cnlwork[BGM_CNL_MAX];
	int cnl_num;						//
	int soutai_tempo;					//相対テンポ
	int fade_cnt;						//フェードカウンタ
	int fade_cnt2;						//
	int fade_vol;						//
	int src_tempo;						//元のテンポ
	int now_tempo;						//実際のテンポ
	int init_flg;						//初期化された
	int bgm_state;						//BGM停止
	int loop_is_flg;					//ループクリア
	int loop_is_adr;					//ループアドレス
	int end_is_adr;						//演奏終了アドレス
	int pause_flg;						//ポーズフラグ
	int one_loop_flg;					//
	int sent_data;						//外部出力データ
	int key_check_flg;					//キーチェックフラグ
	int ssg_noise;						//
	int ssg_mixer;						//
	int before_noise;					//
	int before_mixer;					//
	int fm_vol_flg;						//
	bool p8_data;						//
	int pcm_cnl_cnt;
	//
	CMDZ_FMChip *fmchip;
	CMDZ_PcmDriver *pcmdriver;
	CMDZ_PcmBank *rithm_pcmbank;
	int old_tempo;
	int old_base_cnt;
}CMDZ_WORK;

/*****************************************************************************
;	
******************************************************************************/
extern bool CMDZ_BgmData_initSet(CMDZ_BgmData *bgmr,const void *_data,size_t _size,bool releaseble_flg);
extern CMDZ_BgmPcmFile *CMDZ_BgmData_getPcmFile(CMDZ_BgmData *bgmr,int bank);
extern void CMDZ_BgmData_setPcmBank(CMDZ_BgmData *bgmr,int bank,CMDZ_PcmBank *pcm_bank);
extern const CMDZ_BgmComment *CMDZ_BgmData_getComment(CMDZ_BgmData *bgmr);
extern CMDZ_BgmPcmFile *CMDZ_BgmData_getPcmFile(CMDZ_BgmData *bgmr,int i);
//
extern void CMDZ_WORK_init(CMDZ_WORK *work);
extern void CMDZ_WORK_delete(CMDZ_WORK *work);
extern bool CMDZ_WORK_playBgm(CMDZ_WORK *work,CMDZ_BgmData *_bgmdata);
extern void CMDZ_WORK_playMain(CMDZ_WORK *work);
extern int CMDZ_WORK_getCnlNum(CMDZ_WORK *work);
extern bool CMDZ_WORK_getCnlInfoMess(CMDZ_WORK *work,int i,char *m);
extern void CMDZ_WORK_setFMChip(CMDZ_WORK *work,CMDZ_FMChip *_fmchip);
extern void CMDZ_WORK_setPcmDriver(CMDZ_WORK *work,CMDZ_PcmDriver *_pcmdriver);
extern void CMDZ_WORK_fadeBgm(CMDZ_WORK *work,int speed);
extern bool CMDZ_WORK_stopBgm(CMDZ_WORK *work);
extern void CMDZ_BgmData_delete(CMDZ_BgmData *bgmr);


/*****************************************************************************
;	
******************************************************************************/
#ifdef SGDK
#else
}
#endif
#endif
