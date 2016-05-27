#include "comondef.h"
u32 BLStart __attribute__ ((section (".noinit")));
u8 leds;
u32 EventFlags, CmdFlags, GlobalTime;
u32 firm_timer = 0, flashconfwrite_timer = FLACHCONFCHECK_TIMER;

MyData_t MyData;
TmpData_t TmpData;
FlashConf_t FlashConf;
  // for git
  // v1.04
void ConfDefaults(){
	FlashConf.SN = 0x12345678;
	FlashConf.SU_Conf = 0xFFFF;
	FlashConf.TimeAkt = 5;
	FlashConf.KStep = 8;
	FlashConf.Ton = 28;
	FlashConf.Toff = 30;
	FlashConf.TimePrepare = 30;//3600;
	FlashConf.TimeAfterWork = 30;//1800;
	FlashConf.DefaultWorkTime = 20;
	FlashConf.MaxFreq = 300;

	FlashConf_Write(0,&FlashConf,sizeof(FlashConf));
}
void FlashConfCheck(){
	u16 crc = GetCRC16(&FlashConf,sizeof(FlashConf));
	if(crc != TmpData.ConfCRC){
		FlashConf_Write(0,&FlashConf,sizeof(FlashConf));
		TmpData.ConfCRC = FlashConf_CRC(sizeof(FlashConf_t));
	}
}
void Init(){
	if(BLStart == 0x12345678){
		BLStart = 0;
		FlashFirm_UpdateFirmware();
	}
	Init_Leds();
	Set_Leds(0);
	Init_Utils();
	Init_Periph();

	FlashConf_Read(0,&FlashConf,sizeof(FlashConf));
	if(FlashConf.SN==0xFFFFFFFF)
		ConfDefaults();
	TmpData.ConfCRC =  GetCRC16(&FlashConf,sizeof(FlashConf));

	UART1_Init(115200);
	UARTSend(1,"Start_1\r\n");
	UART2_Init(9600);
	UARTSend(2,"Start_2\r\n");
	UART3_Init(115200);
	UARTSend(3,"Start_3\r\n");
	Init_MS();
	leds = 0;
	GlobalTime = 0;
	Alg_Init();
	TmpData.MBaddr_HoldeZone = (AddrMyData_Begin + (u16)((u32)&MyData.HoldZone-(u32)&MyData)/2);
	TmpData.MBaddr_PressStart = (AddrMyData_Begin + (u16)((u32)&MyData.Press[0]-(u32)&MyData)/2);
	TmpData.MBaddr_PressEnd = (AddrMyData_Begin + (u16)((u32)&MyData.Press[ZONE_COUNT-1]-(u32)&MyData)/2);
	TmpData.MBaddr_KlapanStart = (AddrMyData_Begin + (u16)((u32)&MyData.Klapan[0]-(u32)&MyData)/2);
	TmpData.MBaddr_KlapanEnd = (AddrMyData_Begin + (u16)((u32)&MyData.Klapan[ZONE_COUNT-1]-(u32)&MyData)/2);
	if(Periph_BKPReadLong(BKP_OK)!=0x12345678){
		Periph_BKPWriteLong(BKP_OK,0x12345678);
		Periph_BKPWriteLong(BKP_ARCH,0);
		Periph_BKPWriteLong(BKP_SVN,0);
		Periph_BKPWrite(BKP_SKN,0);
	}
	I2C_EE_Init();
	MyData.ArchPoint = Periph_BKPReadLong(BKP_ARCH);
	Archive_Add(ARCH_CODE_EVENT,ARCH_NUMBER_EVENT_POWERON,0);
}
void Timer_1s(){
	//GlobalTime++;
	BlinkLong(LED_1S);
	Alg_Timer1s();
}
void EventsAndCmd(){
	u8 k;
	for(k=0;k<32;k++){
		u32 mask = (0x00000001<<k);
		if(EventFlags & mask){
			switch (mask) {
			case EVENT_F1S: 		Timer_1s(); 		break;
			case EVENT_CHECKCONF: 	FlashConfCheck(); 	break;
			}
			EventFlags &= ~mask;
		}
	}
	for(k=0;k<32;k++){
		u32 mask = (0x00000001<<k);
		if(CmdFlags & mask){
			switch (mask) {
			case CMD_TMP1:		Periph_BKPWriteLong(BKP_SVN,44442);
								break;
								//Alg_StepWrite_Mode1(STEP_MODE1_FINDSTART, 1); 		break;
			case CMD_TMP2: 		Alg_StepStop1();break;//Alg_StepWrite_Mode1(STEP_MODE1_FINDEND, 1); 		break;
			case CMD_AktOpen:	Alg_AktOpen();  break;
			case CMD_AktClose:	Alg_AktClose(); break;
			case CMD_InitStep:  Alg_InitStep(); break;
			case CMD_Start:		Alg_Start();	break;
			case CMD_Stop:		Alg_Stop();		break;
			case CMD_HoldZone:	Alg_HoldZone(1);	break;
			case CMD_Vent1_On:  MyData.DO |= DO_Vent1; break;
			case CMD_Vent2_On:  MyData.DO |= DO_Vent2; break;
			case CMD_VentF_On:  MyData.DO |= DO_VentF; break;
			case CMD_Vent1_Off: MyData.DO &= ~DO_Vent1; break;
			case CMD_Vent2_Off: MyData.DO &= ~DO_Vent2; break;
			case CMD_VentF_Off: MyData.DO &= ~DO_VentF; break;
			case CMD_ClrArch:	Periph_BKPWriteLong(BKP_ARCH,0); MyData.ArchPoint = 0; break;
			case CMD_FastCool:
				MyData.DO |= DO_Vent1 | DO_Vent2;
				EventFlags |= EVENT_FASTCOOL;
				break;
			//---
			case CMD_Reboot:	NVIC_SystemReset(); break;
			case CMD_DefaultConf: ConfDefaults(); break;
			}
			CmdFlags &= ~mask;
		}
	}
}
int main(void){
	uint32_t lastAlarm=0,lastWarn=0,lastSU=0;
	uint8_t k;
	Init();
    while(1){
    	ResetIWDG();
    	PeriphExecute();
    	UART_Execute();
    	MyData.DI = (MyData.DI & 0xFFFF0000) | GetDI();
    	if((MyData.DI & DI_AlarmStop) && (MyData.State!=STATE_ALARM_STOP)) Alg_AlarmStop();
    	if((!(MyData.DI & DI_AlarmStop)) && (MyData.State==STATE_ALARM_STOP)) Alg_Init();
    	EventsAndCmd();
    	MultiScanner_Execute();
    	Alg_Execute();
    	// Архивация изменений Alarms, Warn, SU
    	if(lastAlarm ^ MyData.Alarms){
    		u16 mask = lastAlarm^MyData.Alarms;
    		for(k=0;k<16;k++)if(mask & (0x0001<<k))
    			Archive_Add(ARCH_CODE_ALARM,k,(MyData.Alarms & mask)?1:0);
    		lastAlarm = MyData.Alarms;
    	}
    	if(lastWarn ^ MyData.Warnings){
    		u16 mask = lastWarn ^ MyData.Warnings;
    		for(k=0;k<16;k++)if(mask & (0x0001<<k))
    			Archive_Add(ARCH_CODE_WARN, k, (MyData.Warnings & mask)?1:0);
    		lastWarn = MyData.Warnings;
    	}
    	/*if(lastSU ^ MyData.SU_State){
    		u32 mask = lastSU ^ MyData.SU_State;
    		for(k=0;k<32;k++)if(mask & (0x0001<<k))
    			Archive_Add(ARCH_CODE_SU, k, (MyData.SU_State & mask)?1:0);
    		lastSU = MyData.SU_State;
    	}*/

    }
}
