/*-----------------------------------------------------------------------------
;	mdz_def.h
-----------------------------------------------------------------------------*/
#ifndef _MDZ_DEF_H_
#define _MDZ_DEF_H_

/*****************************************************************************
;	�����܂�
******************************************************************************/
#define MDZS_VER			"0.05"			//MDZ�o�[�W����
#define _MDZS_VER			1				//���߲׏����\�o�[�W����
#define BGM_CNL_MAX			26				//BGM�`���l����
#define PZI_BANK_MAX		4				//�o���N��
//MDZ�w�b�_�[
#define MDZH_STRING			0x04		//4byte 
#define MDZH_VER			0x09		//2byte 
#define MDZH_EX_FLG			0x10		//2byte �g���t���O
#define MDZH_QFLG			0x12		//2byte �Q�[�g�^�C���̎��
#define MDZH_FM_OTO			0x14		//2byte FM���F
#define MDZH_SSG_OTO		0x16		//2byte SSG���F
#define MDZH_ADPCM_OTO		0x18		//2byte ADPCM���F
#define MDZH_PCM2_NAME		0x1A		//2byte PCM2�̃t�@�C����
#define MDZH_COMMENT		0x1C		//2byte �R�����g�A�h���X
#define MDZH_FM_VOL_FLG		0x1E		//2byte FM�{�����[���t���O
#define MDZH_PRIORYTY_FLG	0x20		//2byte 
#define MDZH_BGM_DATA2_TOP	0x22		//2byte 
#define MDZH_CNL_NUM		0x24		//2byte �`���l����
#define MDZH_PCM1_NAME		0x26		//2byte PCM1�̃t�@�C����
#define MDZH_PPZ8_OTO		0x28		//2byte 
#define MDZH_PCM1_CATE		0x2A		//2byte PCM1�̃t�@�C���̎��
#define MDZH_BASE			0x2E		//2byte �x�[�X�J�E���g
#define MDZ_HEADER_SIZE		0x30		//�w�b�_�[�T�C�Y
//�`���l���̎��
#define _FM_F				0x00		//FM
#define _SSG_F				0x01		//SSG
#define _RITHM_F			0x02		//���Y��
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
	//�R�}���h
#define _REST_COM			0x80		//�x��
#define _COM_TOP			0x81		//�R�}���h�̐擪
#define _STOP_COM			0xFF		//��~�R�}���h

//#define BGM_CNL_MAX			16			//BGM�`���l����
#define PZI_BANK_MAX		4			//�o���N��
//#define X_N0				0x80		//Xn �̏����l
//#define DELTA_N0			127			//DELTA_N �̏����l
//#define DEF_PBUFF_VOL		0x400		//�����o�b�t�@�̑傫��
#define DEF_TEMPO			120			//�f�t�H���g�e���|
#define TEMPO_MIN			30
#define TEMPO_MAX			1024
#define CNL_LOOPSTACK_MAX	20			//���[�v�X�^�b�N��

#define ADPCM_VOL_SCALE		150
#define SSG_VOL_SCALE		85
//#define RITHM_VOL_SCALE		110
#define RITHM_VOL_SCALE		50
#define SSGPPZ8_VOL_SCALE	150

/*****************************************************************************
	�����܂�
******************************************************************************/
#endif
