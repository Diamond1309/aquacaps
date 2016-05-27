#ifndef COMONDEF_H
#define COMONDEF_H
#define VERSION		0x0002

#include "misc.h"
#include "stm32f10x.h"
#include "Libs/libUtils.h"
#include "Libs/uarts.h"
#include "Libs/libModBusSlave.h"
#include "Libs/libMultiScanner.h"
#include "Libs/multiscanner.h"
#include "Libs/periph.h"
#include "Libs/algoritm.h"
#include "Libs/i2c_eeprom.h"
#include "Libs/archive.h"

typedef u32 uint32_t;
typedef u16 uint16_t; 
typedef u8	uint8_t;
/// --------------------------- defines
#define ZONE_COUNT		12

#define EVENT_F1S		0x00000001
#define EVENT_CHECKCONF	0x00000002
#define EVENT_FASTCOOL	0x00000004

#define FLACHCONFCHECK_TIMER	10000

#define BKP_OK			0
#define BKP_ARCH		2
#define BKP_SVN			4
#define BKP_SKN			8
/// --------------------------- Команды
#define CMD_ClrArch		0x0001
#define CMD_AktOpen		0x0002
#define CMD_AktClose	0x0004
#define CMD_InitStep	0x0008
#define CMD_Start		0x0010
#define CMD_Stop		0x0020
#define CMD_HoldZone	0x0040
#define CMD_FastCool	0x0080
#define CMD_TMP1		0x1000
#define CMD_TMP2		0x2000
#define CMD_Reboot		0x4000
#define CMD_DefaultConf	0x8000
#define CMD_Vent1_On	0x00010000
#define CMD_Vent1_Off	0x00020000
#define CMD_Vent2_On	0x00040000
#define CMD_Vent2_Off	0x00080000
#define CMD_VentF_On	0x00100000
#define CMD_VentF_Off	0x00200000
/// --------------------------- Состояния
#define STATE_STOP				0
#define STATE_FIRST_PREPARE		1
#define STATE_WAIT				2
#define STATE_WORK				3
#define STATE_AFTERWORK			4
#define STATE_ALARM				100
#define STATE_ALARM_STOP		200
/// --------------------------- Дискретные входа
#define DI_AktOpen		0x80000001		// актуатор - открыто
#define DI_AktClose		0x40000002		// актуатор - закрыто
#define DI_AlarmStop	0x00000004		// аварийный стоп
#define DI_App			0x08000000		// доступ в аппаратный отсек
#define DI_Closed		0x10000000		// концевик крышки
#define DI_Level		0x20000000		// датички уровня воды
#define DI_Whater		0x80000000		// датичк протечки

/// --------------------------- Дискретные выходы
#define DO_AktOpen		0x00000001		// актуатор крышки - подъем
#define DO_AktClose		0x00000002		// актуатор крышки - закрытие
#define DO_Heater		0x00000004
#define DO_Ozon			0x00000008
#define DO_N2			0x00000010

#define DO_Vent1		0x00000100
#define DO_Vent2		0x00000200
#define DO_VentF		0x00000400
#define DO_VentA		0x00000800

/// --------------------------- Аварии
#define ALARM_LEVEL		0x0001
#define ALARM_AKT_OPEN	0x0002
#define ALARM_AKT_CLOSE	0x0004
#define ALARM_STEP		0x0008
#define ALARM_STEP1		0x0010
#define ALARM_STEP2		0x0020
#define ALARM_PCH		0x0040
#define ALARM_DOOR		0x0080
#define ALARM_SU_STEP	0x0100
#define ALARM_SU_MR1	0x0200
#define ALARM_SU_MR2	0x0400
#define ALARM_SU_PCH	0x0800
#define ALARM_STOP		0x8000

#define WARN_TEMPER_LO	0x0001
#define WARN_TEMPER_HI	0x0002
#define WARN_STEP1_INIT	0x0010
#define WARN_PCH		0x0040
#define WARN_STEP1_PREP	0x0080

/// --------------------------- Глобальные переменные
extern u32 BLStart;
extern u32 EventFlags, CmdFlags, GlobalTime;
extern u32 firm_timer,flashconfwrite_timer;
extern u8 leds;

/// --------------------------- Структуры
#define AddrMyData_Begin	0x0010
#define AddrConfig_Begin	0x0100
#define AddrTmpData_Begin	0x0200
#pragma pack(1)
typedef struct MyData_t{
	u16	State;		// 0010
	u16	Alarms;		// 0011
	u16	Warnings;	// 0012
	u32	SU_State;	// 0013-0014
	u16	Timer;		// 0015
	u32	DI;			// 0016-0017
	u32	DO;			// 0018-0019
	u16	PressCur;	//001A
	u16	Term1;
	u16 Term2;	// 001B, 001C
	u16	AI[3];		 	// 001D,001E,001F
	u16	WorkTime;	 	// 0020 - заданное время процедуры
	u16	Zones[ZONE_COUNT+1];	// 0021-002D зоны
	u16	Press[ZONE_COUNT];	// 002E-0039 давление в зонах
	u16	Klapan[ZONE_COUNT];	// 003A-0045 частота клапана в зонах
	u16	CurZone;	// 46 текущая зона
	u16	HoldZone;	// 47 удерживаемая зона (>12 - не удерживать)
	u32 ArchPoint;
	//----------


	//----------

}MyData_t;
extern MyData_t MyData;
typedef struct TmpData_t{
	u16	StepStat1;	// 0000
	u16	StepStat2;	// 0001
	u16	StepMode1;	// 0002
	u16	StepMode2;	// 0003
	u16	StepBusy1;	// 0004
	u16	StepBusy2;	// 0005
	u32	StepLen;	// 0006-0007
	u32	StepCur;	// 0008-0009
	u32	Pos1;		// 000A-000B
	u32	Pos2;		// 000C-000D
	u16	StepInit_step; 	// 000E
	u16	StepSend_Cmd1; 	// 000F
	u16	StepSend_Mode1;	// 0010
	u32	StepSend_Pos1;	// 0011
	u32	StepSend_Pos2;	// 0013
	u16	StepSend_Freq;	// 0015
	u16	StepSend_Cmd2;	// 0016
	u16	StepSend_Mode2;	// 0017
	u16	SendPCH_Set;	// 0018
	u16 SendPCH_Cmd;	// 0019
	u16	PCHStat;		// 001A
	u16	TimeAktOpen;
	u16	TimeAktClose;
	u32	ScaledZones[ZONE_COUNT+1];
	u16	MBaddr_HoldeZone;
	u16 MBaddr_PressStart,MBaddr_PressEnd;
	u16 MBaddr_KlapanStart, MBaddr_KlapanEnd;
	float	tmpfloat;
	u16	ConfCRC;
}TmpData_t;
extern TmpData_t TmpData;
typedef struct FlashConf_t{
	u32	SN;			// 0100
	u16 SU_Conf;	// 0102
	u16 TimeAkt;	// 0103
	float KStep;	// 0104-0105
	u16 Ton,Toff;	// 0106-0107
	u16 TimePrepare,TimeAfterWork; // 0108, 0109
	u16 DefaultWorkTime;
	u16 MaxFreq;
}FlashConf_t;
extern FlashConf_t FlashConf;

#pragma pack(0)
/// --------------------------- прототипы функций

#endif
