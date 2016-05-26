#ifndef ALGORITM_H
#define ALGORITM_H
#ifndef _COMONDEF_H
	#include "comondef.h"
#endif
//MyData.AlgStepState
/*#define ALG_STEP_STATE_NOTINIT		0
#define ALG_STEP_STATE_INIT			1
#define ALG_STEP_STATE_BUSY			2
#define ALG_STEP_STATE_WORK			3
#define ALG_STEP_STATE_READY		4*/

#define STEP_WRITE_FUNC_MODE1	1
#define STEP_WRITE_FUNC_MOVE1	2
#define STEP_WRITE_FUNC_POSS	3
#define STEP_WRITE_FUNC_MODE2	10
#define STEP_WRITE_FUNC_FREQ	10

void Alg_Init();
void Alg_Execute();
void Alg_Timer1s();
void Alg_Timer1ms();

void Alg_StepWrite_Mode1(u16 mode, u8 isStart); // запуск выполнения задачи для шаговика
void Alg_AktOpen();
void Alg_AktClose();
void Alg_InitStep();
void Alg_Start();
void Alg_Stop();
void Alg_StepChangeZone();
void Alg_StepWriteNext1();
void Alg_FlagChaneZoneParam();
void Alg_StepChangeZone();

#endif
