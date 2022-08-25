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
//�o�C�i���f�[�^
typedef struct CMDZ_BinData{
	void *buff;
	size_t buff_size;
	bool alloc_flg;
}CMDZ_BinData;
//�`���l�����
typedef struct CMDZ_BgmDataCnl{
	int type;
	int index;
	void *offset;
}CMDZ_BgmDataCnl;
//PCM�t�@�C���ݒ�
typedef struct CMDZ_BgmPcmFile{
	char file[256];
	int cate;
}CMDZ_BgmPcmFile;
//�R�����g
typedef struct CMDZ_BgmComment{
	char comment[256];
}CMDZ_BgmComment;
//ADPCM���F
typedef struct CMDZ_BgmAdpcmNeiro{
	uint16_t start;
	uint16_t end;
	uint16_t rate;
}CMDZ_BgmAdpcmNeiro;
//FM���F
typedef struct CMDZ_BgmFmNeiro{
	uint8_t data[25];
}CMDZ_BgmFmNeiro;
//PSG���F
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
//PCM�쐬�̃p�����[�^
typedef struct CMDZ_MakePara{
	int loopadr;			//���[�v�A�h���X
	int endadr;				//�I���A�h���X
	int nowsize;			//���݂̃T�C�Y
	bool play_flg;		//�Đ��I���t���O
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
//���[�v�e�[�u��
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
//�L�[�I����PCM�o�^�p���[�N
typedef struct CMDZ_PcmTone{
	int start;			//�X�^�[�g
	int end;			//�G���h
	int loop;			//���[�v
	int loop_flg;		//���[�v�t���O
	int rate;			//���[�g
	size_t size;		//
	CMDZ_PcmBankTbl *pzit;
}CMDZ_PcmTone;
//�`�����l���t���O
typedef struct CMDZ_CNLFLG{
	uint32_t tai:1;			//�^�C
	uint32_t tai2:1;		//�^�C2
	uint32_t stop:1;		//��~
	uint32_t rest:1;		//�x��
	uint32_t kon:1;			//�L�[�I��
	uint32_t koff:1;		//�L�[�I�t
	uint32_t start:1;		//�X�^�[�g
	uint32_t vend:1;		//�x���h
	uint32_t lfo:3; 		//P,Q,R LFO
	uint32_t alfo:1;		//A_LFO
	uint32_t hlfo:1;		//H_LFO
	uint32_t kon_r:1;		//�L�[�I��
	uint32_t sura:1;		//�X���[
	uint32_t sura2:1;		//�X���[�Q
	uint32_t apan:1;		//APAN
	uint32_t rest_off:1;	//REST_OFF
	uint32_t oneloop:1;		//1������
	uint32_t dummy:32-19;	//
}CMDZ_CNLFLG;
//LFO
typedef struct CMDZ_LFO{
	int md;				//LFO�f�F���C
	int md_cnt;			//LFO�f�F���C�J�E���^
	int speed;			//LFO�X�s�[�h
	int speed_cnt;		//LFO�X�s�[�h�J�E���^
	int rate;			//LFO�傫��
	int rate_sub;		//LFO�傫���T�u
	int depth;			//LFO�[��
	int depth_cnt;		//LFO�[���J�E���^
	int wave_num;		//LFO�̔g�`�ԍ�
}CMDZ_LFO;
//ALFO
typedef struct CMDZ_ALFO{
}CMDZ_ALFO;
//HLFO
typedef struct CMDZ_HLFO{
}CMDZ_HLFO;
//VEND
typedef struct CMDZ_VEND{
	int md_cnt;			//�s�b�`�x���h�̃f�����C
	int speed;			//�s�b�`�x���h�̃X�s�[�h
	int speed_cnt;		//�s�b�`�x���h�̃X�s�[�h�J�E���^
	int ontei;			//�s�b�`�x���h�̉���
	int rate;			//�s�b�`�x���h�̑傫��
	int wave;			//�s�b�`�x���h�̖ڕW���g��
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
	int vol2;			//�G���x���[�v�p�{�����[��
	int sv;				//�G���x���[�v�p�X�^�[�g�{�����[
	int ar;				//�A�^�b�N���[�g
	int dr;				//�f���P�C���[�g
	int sl;				//�T�X�e�B�����x��
	int sr;				//�T�X�e�B�����[�g
	int rr;				//�����[�X���[�g
}CMDZ_ENV;
//�I�[�gPAN
typedef struct CMDZ_APAN{
	int md;				//�f�����C
	int md_cnt;			//�f�����C
	int speed;			//�X�s�[�h
	int speed_cnt;		//�X�s�[�h�J�E���^
	int sorc;			//PAN�\�[�X
	int dist;			//PAN�f�B�X�g
	int sorc_w;			//PAN�\�[�X
	int dist_w;			//PAN�f�B�X�g
	int num;			//���݂�PAN
	int add;			//PAN���Z����
	int type;			//PAN�̎��
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
//�`���l�����[�N
typedef struct CMDZ_CNL{
	//��{
	int loop_stack[CNL_LOOPSTACK_MAX];	//���[�v�X�^�b�N
	int loop_adr;						//���[�v�X�^�b�N�̃A�h���X
	void *data_adr_top;					//�f�[�^�A�h���X
	void *data_adr;						//�f�[�^�A�h���X
	CMDZ_CNLFLG state;					//�X�e�[�^�X
	int def_len;						//�f�t�H���g�̉���
	int len_wk;							//�����̃J�E���^
	int wave;							//���݂̎��g��
	int oct_wk;							//���݂̎��g��
	int before_wave;					//�O�̎��g��
	int quota_wk;						//�Q�[�g�^�C���J�E���^
	int detune;							//�f�B�`���[��
	int cnl_index;						//�`���l��index
	int cnl_cate;						//�`���l�����
	int cnl_number;						//�`���l���ԍ�
	int cnl_port_num;					//�`���l���|�[�g�ԍ�
	int quota;							//�Q�[�g�^�C��
	int now_ontei;						//���݂̉���
	int before_ontei;					//�O�̉���
	int vol;							//�{�����[��
	int before_vol;						//�O�̉���
	int soutai_icho;					//���Έڒ��̒l
	int pan;							//PAN
	int oto_num;						//���F�ԍ�
	int now_cmd;
	int now_len;
	//�g��
	CMDZ_LFO lfo[3];						//LFO
	CMDZ_VEND vend;						//�s�b�`�x���h
	//FM
	int fm_cnl_number;					//
	//int fm_now_oto;					//FM�����̌��݂̉��F�A�h���X
	int fm_now_oto_tl[4];				//
	int fm_alg;							//�A���S���Y��
	//SSG/PPZ8
	CMDZ_ENV env;						//�G���x���[�v
	//���Y��
	int rt_vol[6];						//���Y���̃{�����[��
	//PPZ8
	CMDZ_APAN apan;						//�I�[�g�p��
	CMDZ_PcmTone pcmtone;				//PCM���F
	int oto_bank;						//�o���N�ԍ�
	int pcm_work;						//�g�p����PCM�`���l��
}CMDZ_CNL;
//
typedef struct CMDZ_WORK{
	CMDZ_BgmData *bgmr;			//BGM�f�[�^
	CMDZ_PZIL *pzi_lp[PZI_BANK_MAX];		//PCM�o���N�e�[�u��
	uint8_t keyon_tbl[BGM_CNL_MAX];
	CMDZ_CNL *cnlwork[BGM_CNL_MAX];
	int cnl_num;						//
	int soutai_tempo;					//���΃e���|
	int fade_cnt;						//�t�F�[�h�J�E���^
	int fade_cnt2;						//
	int fade_vol;						//
	int src_tempo;						//���̃e���|
	int now_tempo;						//���ۂ̃e���|
	int init_flg;						//���������ꂽ
	int bgm_state;						//BGM��~
	int loop_is_flg;					//���[�v�N���A
	int loop_is_adr;					//���[�v�A�h���X
	int end_is_adr;						//���t�I���A�h���X
	int pause_flg;						//�|�[�Y�t���O
	int one_loop_flg;					//
	int sent_data;						//�O���o�̓f�[�^
	int key_check_flg;					//�L�[�`�F�b�N�t���O
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
