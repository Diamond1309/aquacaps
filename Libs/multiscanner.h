#ifndef MS_H
#define MS_H
#ifndef _COMONDEF_H
	#include "comondef.h"
#endif

#define MS_COUNT	12
extern MultiScanner_t ms[MS_COUNT];
#define MS_DIDO_WAIT	200
#define MS_DIDO_PAUSE	100
#define MS_MR_WAIT		300
#define MS_MR_PAUSE		100
#define MS_PCH_WAIT		300
#define MS_PCH_PAUSE	100

#define SU_DIDO		0x0001
#define SU_MR1		0x0002
#define SU_MR2		0x0004
#define SU_PCH		0x0008

#define MS_DIDO_PORT	1
#define MS_MR1_PORT		2
#define MS_MR2_PORT		2
#define MS_PCH_PORT		2

#define piDIDO_r	100
#define piDIDO_w	101
#define piStepMove1	102
#define piStepCmd1	103
#define piStepMode1	104
#define piStepMode2	105
#define piStepFreq	106
#define piMR1		200
#define piMR2		300
#define piPCHGetStat	400
#define piPCHGetSens	401
#define piPCHSet		402
#define piPCHCmd		403


#define MB_READ_START_REG	0x0006
#define MB_STEP_DI			(0x0006 - MB_READ_START_REG)
#define MB_STEP_MOVE1		(0x0008 - MB_READ_START_REG)
#define MB_STEP_MODE1		(0x0010 - MB_READ_START_REG)
#define MB_STEP_MODE2		(0x0017 - MB_READ_START_REG)
#define MB_STEP_STAT1		(0x0011 - MB_READ_START_REG)
#define MB_STEP_STAT2		(0x0018 - MB_READ_START_REG)
#define MB_STEP_CMD1		(0x0012 - MB_READ_START_REG)
#define MB_STEP_CMD2		(0x001A - MB_READ_START_REG)
#define MB_STEP_LEN			(0x0015 - MB_READ_START_REG)
#define MB_STEP_CUR			(0x001E - MB_READ_START_REG)
#define MB_STEP_POS1		(0x000C - MB_READ_START_REG)
#define MB_STEP_POS2		(0x000E - MB_READ_START_REG)
#define MB_STEP_KLFREQ		(0x0019 - MB_READ_START_REG)

#define STEP_MODE1_OFF			0
#define STEP_MODE1_FINDSTART	1
#define STEP_MODE1_FINDEND		2
#define STEP_MODE1_MOVE			3
#define STEP_MODE1_WORK			4
#define STEP_MODE2_OFF			0
#define STEP_MODE2_KL1			1
#define STEP_MODE2_KL2			2
#define STEP_MODE2_KL3			3
#define STEP_MODE2_FREQ			4
#define STEP_CMD_START		0x0001
#define STEP_CMD_STOP		0x2000
#define STEP_STATE_BUSY		0x4000
#define STEP_STATE_DONE		0x8000
#define STEP_ALARM_FLAGS1	0x0F00
#define STEP_ALARM_FLAGS2	0x0F00

#define PCH_STATE_ALARM		0x0008
#define PCH_STATE_WARN		0xE080
#define PCH_CMD_STOP		0x0474
#define PCH_CMD_START		0x047C

void Init_MS();
void MS_SendFlaggedPacket(u16 packet_id);
#endif
