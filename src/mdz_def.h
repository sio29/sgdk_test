/*-----------------------------------------------------------------------------
;	mdz_def.h
-----------------------------------------------------------------------------*/
#ifndef _MDZ_DEF_H_
#define _MDZ_DEF_H_

/*****************************************************************************
;	おしまい
******************************************************************************/
#define MDZS_VER			"0.05"			//MDZバージョン
#define _MDZS_VER			1				//ｺﾝﾊﾟｲﾗ処理可能バージョン
#define BGM_CNL_MAX			26				//BGMチャネル数
#define PZI_BANK_MAX		4				//バンク数
//MDZヘッダー
#define MDZH_STRING			0x04		//4byte 
#define MDZH_VER			0x09		//2byte 
#define MDZH_EX_FLG			0x10		//2byte 拡張フラグ
#define MDZH_QFLG			0x12		//2byte ゲートタイムの種類
#define MDZH_FM_OTO			0x14		//2byte FM音色
#define MDZH_SSG_OTO		0x16		//2byte SSG音色
#define MDZH_ADPCM_OTO		0x18		//2byte ADPCM音色
#define MDZH_PCM2_NAME		0x1A		//2byte PCM2のファイル名
#define MDZH_COMMENT		0x1C		//2byte コメントアドレス
#define MDZH_FM_VOL_FLG		0x1E		//2byte FMボリュームフラグ
#define MDZH_PRIORYTY_FLG	0x20		//2byte 
#define MDZH_BGM_DATA2_TOP	0x22		//2byte 
#define MDZH_CNL_NUM		0x24		//2byte チャネル数
#define MDZH_PCM1_NAME		0x26		//2byte PCM1のファイル名
#define MDZH_PPZ8_OTO		0x28		//2byte 
#define MDZH_PCM1_CATE		0x2A		//2byte PCM1のファイルの種類
#define MDZH_BASE			0x2E		//2byte ベースカウント
#define MDZ_HEADER_SIZE		0x30		//ヘッダーサイズ
//チャネルの種別
#define _FM_F				0x00		//FM
#define _SSG_F				0x01		//SSG
#define _RITHM_F			0x02		//リズム
#define _ADPCM_F			0x03		//ADPCM
#define _PCM68_F			0x04		//68ADPCM
#define _PCM86_F			0x05		//86PCM
#define _PPZ_F				0x06		//PPZ CHANEL
#define _PPZ86_F			0x07		//PPZ86 CHANEL
#define _PPZ8_F				0x08		//PPZ8 CHANEL
#define _MIDI_F				0x09		//MIDI
#define _OPX_F				0x0A		//OPX
#define _OPL4_FM_F			0x0B		//OPL4_FM
#define _OPL4_RITHM_F		0x0C		//OPL4_RITHM
#define _OPL4_PCM_F			0x0D		//OPL4_PCM
	//コマンド
#define _REST_COM			0x80		//休符
#define _COM_TOP			0x81		//コマンドの先頭
#define _STOP_COM			0xFF		//停止コマンド

//#define BGM_CNL_MAX			16			//BGMチャネル数
#define PZI_BANK_MAX		4			//バンク数
//#define X_N0				0x80		//Xn の初期値
//#define DELTA_N0			127			//DELTA_N の初期値
//#define DEF_PBUFF_VOL		0x400		//合成バッファの大きさ
#define DEF_TEMPO			120			//デフォルトテンポ
#define TEMPO_MIN			30
#define TEMPO_MAX			1024
#define CNL_LOOPSTACK_MAX	20			//ループスタック数

#define ADPCM_VOL_SCALE		150
#define SSG_VOL_SCALE		85
//#define RITHM_VOL_SCALE		110
#define RITHM_VOL_SCALE		50
#define SSGPPZ8_VOL_SCALE	150

/*****************************************************************************
	おしまい
******************************************************************************/
#endif
