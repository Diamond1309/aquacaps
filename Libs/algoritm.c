#include "algoritm.h"
u8 step_write_step1, step_write_step2; // шаги алгоритма отправки пакетов для DIDO_Steper
u8 step_write_start1, step_write_start2; // флаги отправки последнего пакета "Запустить задачу"
u8 step_write_func1, step_write_func2; // идентификатор алгоритма отправки пакетов для DIDO_Steper
u8 step_rule_timer1, step_rule_timer2; // таймер алгоритма отправки пакетов для DIDO_Steper
u32 alg_step_init_pause;	// пауза перед выполнением следующего шага алгоритма отправки пакетов для DIDO_Steper
u32 alarm_door_timer;		// фильтр для сигнала "крышка закрыта"
#define STEP_WRITE_DONE		0xDD

#define ALG_EVENT_START				0x0001
#define ALG_EVENT_STARTING			0x0002
#define ALG_EVENT_WORK				0x0004
#define ALG_EVENT_STOP				0x0008
#define ALG_EVENT_STOPING			0x0010
#define ALG_EVENT_OFF				0x0020
#define ALG_EVENT_POSS				0x0040
#define ALG_EVENT_ZONEPARAM			0x0080
#define ALG_EVENT_START_AFTER_INIT	0x0100
u16 Alg_Events = 0;

#define ALG_STEP_INIT_PASUE	1000
#define ALARM_DOOR_TIMER	500
#define STEP_RULE_TIMER		10

extern u8 MS_DIDO_updated;
void Alg_Timer1s(){
	if(TmpData.TimeAktOpen)if(!(--TmpData.TimeAktOpen)){
		MyData.DO &= ~DO_AktOpen;
		MyData.Alarms |= ALARM_AKT_OPEN;
	}
	if(TmpData.TimeAktClose)if(!(--TmpData.TimeAktClose)){
		MyData.DO &= ~DO_AktClose;
		MyData.Alarms |= ALARM_AKT_CLOSE;
	}
	if(MyData.Timer){
		MyData.Timer--;
	}
	if(step_rule_timer1){
		if(!(--step_rule_timer1)){
			MyData.Alarms |= ALARM_STEP;
			step_write_step1 = 0xFF;
			step_write_step2 = 0xFF;
		}
	}
	/// ------------------------------------ Моточасы
	if(MyData.State == STATE_WORK){
		u32 tmp = Periph_BKPReadLong(BKP_SVN);
		Periph_BKPWriteLong(BKP_SVN,++tmp);
	}
}
void Alg_Timer1ms(){
	if(alg_step_init_pause)alg_step_init_pause--;
	if(alarm_door_timer)if(!(--alarm_door_timer))MyData.Alarms |= ALARM_DOOR;
}
void Alg_AlarmStop(){
	MyData.DO = 0;
	MyData.Alarms |= ALARM_STOP;
	MyData.State = STATE_ALARM;
	Alg_StepStop1();
	Alg_StepStop2();
	Alg_PCH_WriteCmd(PCH_CMD_STOP);
}
void Alg_AktOpen(){
	if(!(MyData.DI & DI_AktOpen)){
		TmpData.TimeAktOpen = FlashConf.TimeAkt;
		MyData.DO |= DO_AktOpen;
	}
}
void Alg_AktClose(){
	if(!(MyData.DI & DI_AktClose)){
		TmpData.TimeAktClose = FlashConf.TimeAkt;
		MyData.DO |= DO_AktClose;
	}
}
void Alg_InitStep(){
	TmpData.StepInit_step = 0;
	MS_DIDO_updated = 0;
}
void Alg_Start(){
	if(MyData.Warnings & WARN_STEP1_INIT){
		Alg_InitStep();
		Alg_Events |= ALG_EVENT_START_AFTER_INIT;
		MyData.Warnings |= WARN_STEP1_PREP;
		return;
	}
	u8 k;
	Alg_Events &= ~(ALG_EVENT_START | ALG_EVENT_STARTING | ALG_EVENT_WORK | ALG_EVENT_STOP | ALG_EVENT_STOPING | ALG_EVENT_OFF);
	TmpData.Pos1 = ((float)MyData.Zones[0]) * FlashConf.KStep;
	TmpData.Pos2 = ((float)MyData.Zones[ZONE_COUNT]) * FlashConf.KStep;
	for(k=0;k<=ZONE_COUNT;k++)TmpData.ScaledZones[k] = ((float)MyData.Zones[k]) * FlashConf.KStep;

	MyData.DO |= DO_N2 | DO_Ozon;
	MyData.Timer = MyData.WorkTime;
	MyData.State = STATE_WORK;
	// запускаем шаговик
	Alg_Events |= ALG_EVENT_START;
	// запускаем насос
	Alg_PCH_WriteCmd(PCH_CMD_START);
	Alg_StepWrite_Mode2(STEP_MODE2_FREQ,MyData.Klapan[MyData.CurZone],1);
	Periph_BKPWrite(BKP_SKN, Periph_BKPRead(BKP_SKN)+1);
	MyData.Warnings &= ~WARN_STEP1_PREP;
}
void Alg_HoldZone2(){
	if(MyData.HoldZone<ZONE_COUNT)
		Alg_HoldZone(1);
	else
		Alg_HoldZone(0);
}
void Alg_HoldZone(u8 isHold){
	u8 k;

	if(Alg_Events & ALG_EVENT_WORK){
		for(k=0;k<=ZONE_COUNT;k++)TmpData.ScaledZones[k] = ((float)MyData.Zones[k]) * FlashConf.KStep;
		if(isHold){
			TmpData.Pos1 = ((float)MyData.Zones[MyData.HoldZone]) * FlashConf.KStep;
			TmpData.Pos2 = ((float)MyData.Zones[MyData.HoldZone+1]) * FlashConf.KStep;
			Alg_Events |= ALG_EVENT_START;
		}else{
			TmpData.Pos1 = ((float)MyData.Zones[0]) * FlashConf.KStep;
			TmpData.Pos2 = ((float)MyData.Zones[ZONE_COUNT]) * FlashConf.KStep;
			Alg_Events |= ALG_EVENT_POSS;
		}


	}
}
void Alg_Stop(){
	Alg_Events &= ~(ALG_EVENT_START | ALG_EVENT_STARTING | ALG_EVENT_WORK | ALG_EVENT_STOP | ALG_EVENT_STOPING | ALG_EVENT_OFF);
	Alg_Events |= ALG_EVENT_STOP;
	MyData.State = STATE_AFTERWORK;
	MyData.Timer = FlashConf.TimeAfterWork;
	Alg_PCH_WriteCmd(PCH_CMD_STOP);
	Alg_StepStop2();
}
void Alg_Init(){
	TmpData.StepInit_step = 0;
	//MyData.SU_State = SU_DIDO;
	MS_DIDO_updated = 0;
	TmpData.StepInit_step = 0x11;
	MyData.HoldZone = 0xFF;
	MyData.WorkTime = FlashConf.DefaultWorkTime;
	u8 k;
	for(k=0;k<=ZONE_COUNT;k++)MyData.Zones[k] = k*160 + 50;
	for(k=0;k<=ZONE_COUNT;k++)MyData.Press[k] = 50;
	MyData.State = STATE_FIRST_PREPARE;
	MyData.Timer = FlashConf.TimePrepare;
	MyData.DO |= DO_N2 | DO_Ozon;

}
void Alg_Execute(){
	if(MyData.Alarms & ALARM_STOP) return;
	if(TmpData.TimeAktOpen){
		if(MyData.DI & DI_AktOpen){
			MyData.DO &= ~DO_AktOpen;
			TmpData.TimeAktOpen = 0;
		}
	}
	if(TmpData.TimeAktClose){
		if(MyData.DI & DI_AktClose){
			MyData.DO &= ~DO_AktClose;
			TmpData.TimeAktClose = 0;
		}
	}
	Set_Led(6,MyData.DI & DI_AktOpen);
	Set_Led(7,MyData.DI & DI_AktClose);

	if(!(MyData.SU_State & SU_PCH)){
		if(TmpData.PCHStat & PCH_STATE_WARN)MyData.Warnings |= WARN_PCH; else MyData.Warnings &= ~WARN_PCH;
		if(TmpData.PCHStat & PCH_STATE_ALARM)MyData.Alarms |= ALARM_PCH; else MyData.Alarms &= ~ALARM_PCH;
	}
	if(Alg_Events & ALG_EVENT_ZONEPARAM){
		Alg_Events &= ~ALG_EVENT_ZONEPARAM;
		Alg_StepChangeZone();
	}
	Alg_Step_Exec();

	///------------------------------------ термостатирование воды
	if(MyData.Term1 <= (FlashConf.Ton*100)){
		MyData.DO |= DO_Heater;
	}else if(MyData.Term1 >= (FlashConf.Toff*100)){
		MyData.DO &= ~DO_Heater;
	}
	if(EventFlags & EVENT_FASTCOOL)
		if(MyData.Term1 < (FlashConf.Toff*100)){
			EventFlags &= ~EVENT_FASTCOOL;
			MyData.DO &= ~(DO_Vent1 | DO_Vent2);
		}
	if(((MyData.Term1/100)<FlashConf.Ton)){
		MyData.Warnings |= WARN_TEMPER_LO;
	}else MyData.Warnings &= ~WARN_TEMPER_LO;
	if(((MyData.Term1/100)>FlashConf.Toff)){
			MyData.Warnings |= WARN_TEMPER_HI;
	}else MyData.Warnings &= ~WARN_TEMPER_HI;
	/// ---------------------------------- завершение таймера
	if(!MyData.Timer){
		switch (MyData.State) {
		case STATE_FIRST_PREPARE:
				MyData.DO &= ~(DO_N2 | DO_Ozon);
				MyData.State = STATE_WAIT;
			break;
		case STATE_WORK:
			Alg_Stop();
			break;
		case STATE_AFTERWORK:
			MyData.DO &= ~(DO_N2 | DO_Ozon);
			MyData.State = STATE_WAIT;
		default:
			break;
		}
	}
	/// Авария открытой крышки при работе
	if((MyData.State == STATE_WORK)&&(!(MyData.DI & DI_Closed))){
		if(!alarm_door_timer)alarm_door_timer = ALARM_DOOR_TIMER;
	}else alarm_door_timer = 0;
}


void Alg_Step_Exec(){
	if(MyData.SU_State & SU_DIDO)MyData.Alarms |= ALARM_SU_STEP; else MyData.Alarms &= ~ALARM_SU_STEP;
	if(MyData.SU_State & SU_MR1)MyData.Alarms |= ALARM_SU_MR1; else MyData.Alarms &= ~ALARM_SU_MR1;
	if(MyData.SU_State & SU_MR1)MyData.Alarms |= ALARM_SU_MR2; else MyData.Alarms &= ~ALARM_SU_MR2;
	if(MyData.SU_State & SU_PCH)MyData.Alarms |= ALARM_SU_PCH; else MyData.Alarms &= ~ALARM_SU_PCH;
	if(MyData.SU_State & SU_DIDO){
		step_write_step1 = 0xFF;
		step_write_step2 = 0xFF;
		return;
	}
	if(!MS_DIDO_updated) return;
	if(TmpData.StepBusy1 & STEP_ALARM_FLAGS1) MyData.Alarms |= ALARM_STEP1; else MyData.Alarms &= ~ALARM_STEP1;
	if(TmpData.StepBusy2 & STEP_ALARM_FLAGS2) MyData.Alarms |= ALARM_STEP2; else MyData.Alarms &= ~ALARM_STEP2;
	// ---------------------- ИНИЦИАЛИЗАЦИЯ ШАГОВИКА -------------------------
	if(!alg_step_init_pause){
		if(TmpData.StepInit_step > 10){
			if((TmpData.StepStat1 & 0x0003)!=0x0003){ // не выполнен поиск начального положения
				Alg_InitStep();
				MyData.Warnings |= WARN_STEP1_INIT;
				TmpData.StepInit_step = 0x22;
			}
		}else // если нужно выполнить инициализацию шаговика
			if(!(TmpData.StepBusy1 & STEP_STATE_BUSY)){ // если шаговик 1 не занят
				switch (TmpData.StepInit_step) {
				case 0: // поиск начального положения
					Alg_StepWrite_Mode1(STEP_MODE1_FINDSTART, 1);
					MS_DIDO_updated = 0;
					TmpData.StepInit_step++;
					alg_step_init_pause = ALG_STEP_INIT_PASUE;
					break;
				case 1: // поиск конечного положения
					if(step_write_step1==STEP_WRITE_DONE) // если все команды записаны
						if(TmpData.StepBusy1 & STEP_STATE_DONE){ // если задача выполнена
							Alg_StepWrite_Mode1(STEP_MODE1_FINDEND, 1);
							TmpData.StepInit_step++;
							alg_step_init_pause = ALG_STEP_INIT_PASUE;
						}
					break;
				case 2: // переход в начальное положение
					if(step_write_step1==STEP_WRITE_DONE) // если все команды записаны
						if(TmpData.StepBusy1 & STEP_STATE_DONE){ // если задача выполнена
							FlashConf.KStep = ((float)TmpData.StepLen) / 2500;
							Alg_StepWrite_Move1(1000);
							TmpData.StepInit_step++;
							alg_step_init_pause = ALG_STEP_INIT_PASUE;
						}
					break;
				case 3: // инициализация шаговика закончена
					if(step_write_step1==STEP_WRITE_DONE) // если все команды записаны
						if(TmpData.StepBusy1 & STEP_STATE_DONE){ // если задача выполнена
							TmpData.StepInit_step++;
							FlashConf.KStep = ((float)TmpData.StepLen) / 2500;
						}
					break;
				default:
					TmpData.StepInit_step = 0xCC;
					MyData.Warnings &= ~WARN_STEP1_INIT;
					if(Alg_Events & ALG_EVENT_START_AFTER_INIT){
						Alg_Events &= ~ALG_EVENT_START_AFTER_INIT;
						Alg_Start();
					}
					break;
				}
			}
	}
	//------------------------ СОБЫТИЯ ШАГОВИКА ---------------------------
	if((TmpData.StepStat1 & 0x0003)==0x0003){ // если шаговик проинициализирован
		/// команда старта
		if(Alg_Events & ALG_EVENT_START){
			Alg_Events |= ALG_EVENT_START;
			FlashConf.KStep = ((float)TmpData.StepLen) / 2500;
			Alg_StepWrite_Mode1(STEP_MODE1_WORK, 1);
			Alg_Events &= ~ALG_EVENT_START;
			Alg_Events |= ALG_EVENT_STARTING;
		}
		/// выполнение старта
		if(Alg_Events & ALG_EVENT_STARTING){
			if(step_write_step1==STEP_WRITE_DONE)
			if(TmpData.StepBusy1 & STEP_STATE_BUSY){
				Alg_Events &= ~ALG_EVENT_STARTING;
				Alg_Events |= ALG_EVENT_WORK;
			}
		}
		/// шаговик работает
		if(Alg_Events & ALG_EVENT_WORK){

		}
		/// стоп
		if(Alg_Events & ALG_EVENT_STOP){
			Alg_StepWrite_Mode1(STEP_MODE1_WORK, 0);
			Alg_Events &= ~ALG_EVENT_WORK;
			Alg_Events &= ~ALG_EVENT_STOP;
			Alg_Events |= ALG_EVENT_STOPING;
		}
		/// Остановка
		if(Alg_Events & ALG_EVENT_STOPING){
			if(step_write_step1==STEP_WRITE_DONE)
				if(!(TmpData.StepBusy1 & STEP_STATE_BUSY)){
					Alg_Events &= ~ALG_EVENT_STOPING;
					Alg_Events &= ~ALG_EVENT_WORK;
					Alg_Events |= ALG_EVENT_OFF;
				}
		}
		if(Alg_Events & ALG_EVENT_POSS){
			Alg_Events &= ~ALG_EVENT_POSS;
			Alg_StepWrite_Poss();
		}

	}

}
void Alg_FlagChaneZoneParam(){
	Alg_Events |= ALG_EVENT_ZONEPARAM;
}
void Alg_StepChangeZone(){
	if(MyData.State == STATE_WORK){
		if(MyData.CurZone<ZONE_COUNT){
			Alg_StepWrite_Freq(STEP_MODE2_FREQ, MyData.Klapan[MyData.CurZone],1);
			Alg_PCH_WriteSet(MyData.Press[MyData.CurZone]);
		}
	}

}
void Alg_PCH_WriteSet(u16 set){
	TmpData.SendPCH_Set = set*100;
	MS_SendFlaggedPacket(piPCHSet);
}
void Alg_PCH_WriteCmd(u16 cmd){
	TmpData.SendPCH_Cmd = cmd;
	MS_SendFlaggedPacket(piPCHCmd);
}
void Alg_StepWrite_Mode2(u16 mode, u16 freq, u8 isStart){
	step_write_step2 = 0;
	step_rule_timer1 = STEP_RULE_TIMER;
	TmpData.StepSend_Mode2 = mode;
	TmpData.StepSend_Freq = ((float)freq * (float)FlashConf.MaxFreq)/100;
	if(isStart)step_write_start2 = 1; else step_write_start2 = 0;
	//if(isStart)TmpData.StepSend_Cmd2 = (TmpData.StepBusy2 & 0xC000) | STEP_CMD_START; else TmpData.StepSend_Cmd2 = STEP_CMD_STOP;
	step_write_func2 = STEP_WRITE_FUNC_MODE2;
	//MS_SendFlaggedPacket(piStepMode2);
	Alg_StepWriteNext2();
}
void Alg_StepWrite_Freq(u16 mode, u16 freq, u8 isStart){
	step_write_step2 = 0;
	step_rule_timer1 = STEP_RULE_TIMER;
	TmpData.StepSend_Mode2 = mode;
	TmpData.StepSend_Freq = ((float)freq * (float)FlashConf.MaxFreq)/100;
	step_write_func2 = STEP_WRITE_FUNC_FREQ;
	MS_SendFlaggedPacket(piStepFreq);
}
void Alg_StepWrite_Mode1(u16 mode, u8 isStart){
	step_rule_timer1 = STEP_RULE_TIMER;
	step_write_step1 = 0;
	TmpData.StepSend_Mode1 = mode;
	if(isStart)step_write_start1 = 1; else step_write_start1 = 0;
	TmpData.StepSend_Pos1 = TmpData.Pos1;
	TmpData.StepSend_Pos2 = TmpData.Pos2;

	step_write_func1 = STEP_WRITE_FUNC_MODE1;
	Alg_StepWriteNext1();
}
void Alg_StepStop1(){
	TmpData.StepSend_Cmd1 = STEP_CMD_STOP;
	MS_SendFlaggedPacket(piStepCmd1);
}
void Alg_StepStop2(){
	Alg_StepWrite_Mode2(STEP_MODE2_FREQ,0,0);
	return;
	TmpData.StepSend_Cmd2 = STEP_CMD_STOP;
	step_write_step2 = 0;
	step_rule_timer1 = STEP_RULE_TIMER;
	step_write_start2 = 0;
	MS_SendFlaggedPacket(piStepMode2);
}
void Alg_StepWrite_Move1(u16 pos){
	step_rule_timer1 = STEP_RULE_TIMER;
	step_write_step1 = 0;
	step_write_start1 = 1;
	TmpData.StepSend_Mode1 = STEP_MODE1_MOVE;
	TmpData.StepSend_Pos1 = (float)pos * FlashConf.KStep;
	step_write_func1 = STEP_WRITE_FUNC_MOVE1;
	Alg_StepWriteNext1();
}
void Alg_StepWrite_Poss(){
	step_rule_timer1 = STEP_RULE_TIMER;
	step_write_step1 = 0;
	TmpData.StepSend_Mode1 = STEP_MODE1_WORK;
	TmpData.StepSend_Pos1 = TmpData.Pos1;
	TmpData.StepSend_Pos2 = TmpData.Pos2;
	step_write_func1 = STEP_WRITE_FUNC_POSS;
	Alg_StepWriteNext1();
}
void Alg_stepWriteDone1(){
	step_write_step1 = STEP_WRITE_DONE;
	step_rule_timer1 = 0;
}
void Alg_StepWriteNext1(){
	switch(step_write_func1){
	case STEP_WRITE_FUNC_POSS:
		switch(step_write_step1++){
		case 0:
			MS_SendFlaggedPacket(piStepMode1);
			MS_DIDO_updated = 0;
			break;
		case 0xFF: break;
		default:
			Alg_stepWriteDone1();
			break;
		}
		break;
	case STEP_WRITE_FUNC_MODE1:
		switch(step_write_step1++){
		case 0:
			MS_SendFlaggedPacket(piStepMode1);
			MS_DIDO_updated = 0;
			break;
		case 1:
			TmpData.StepSend_Cmd1 = TmpData.StepBusy1 | STEP_CMD_STOP;
			MS_SendFlaggedPacket(piStepCmd1);
			MS_DIDO_updated = 0;
			break;
		case 2:
			if(step_write_start1) {
				TmpData.StepSend_Cmd1 = STEP_CMD_START;
				MS_SendFlaggedPacket(piStepCmd1);
				MS_DIDO_updated = 0;
			}else
				Alg_stepWriteDone1();
			break;
		case 0xFF: break;
		default:
			Alg_stepWriteDone1();
			break;
		}
		break;
	case STEP_WRITE_FUNC_MOVE1:
		switch(step_write_step1++){
		case 0:
			MS_SendFlaggedPacket(piStepMove1);
			MS_DIDO_updated = 0;
			break;
		case 1:
			MS_SendFlaggedPacket(piStepMode1);
			MS_DIDO_updated = 0;
			break;
		case 2:
			TmpData.StepSend_Cmd1 = TmpData.StepBusy1 | STEP_CMD_STOP;
			MS_SendFlaggedPacket(piStepCmd1);
			MS_DIDO_updated = 0;
			break;
		case 3:
			if(step_write_start1) {
				TmpData.StepSend_Cmd1 = STEP_CMD_START;
				MS_SendFlaggedPacket(piStepCmd1);
				MS_DIDO_updated = 0;
			}else
				Alg_stepWriteDone1();
			break;
		case 0xFF: break;
		default:
			Alg_stepWriteDone1();
			break;
		}
		break;
	}


}
void Alg_stepWriteDone2(){
	step_write_step2 = STEP_WRITE_DONE;
	step_rule_timer2 = 0;
}
void Alg_StepWriteNext2(){
	switch(step_write_func2){
	case STEP_WRITE_FUNC_MODE2:
		switch(step_write_step2++){
		case 0:
			TmpData.StepSend_Cmd2 = (TmpData.StepBusy2 & 0xC000) | STEP_CMD_STOP;
			MS_SendFlaggedPacket(piStepMode2);
			MS_DIDO_updated = 0;
			break;
			/*
		case 1:
			TmpData.StepSend_Cmd2 = (TmpData.StepBusy2 & 0xC000) | STEP_CMD_STOP;
			MS_SendFlaggedPacket(piStepMode2);
			MS_DIDO_updated = 0;
			break;
			*/
		case 1:
			if(step_write_start2) {
				TmpData.StepSend_Cmd2 = STEP_CMD_START;
				MS_SendFlaggedPacket(piStepMode2);
				MS_DIDO_updated = 0;
			}else
				Alg_stepWriteDone2();
			break;
		case 0xFF: break;
		default:
			Alg_stepWriteDone2();
			break;
		}
		break;
	default:
		Alg_stepWriteDone2();
		break;
	}
}
