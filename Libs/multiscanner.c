#include "multiscanner.h"
MultiScanner_t _ms[MS_COUNT];
u8 ms_packets[MS_COUNT][20];
extern u16 tmpmbbuf[40];
u8 MS_DIDO_updated = 0;
void MS_setpacket(u8 index, u8 port, u32 su_mask, u16 packet_id, u8 *packet, u8 packet_len, u16 flags, u16 waittime, u16 pause){

	_ms[index].port = port;
	_ms[index].su_mask = su_mask;
	_ms[index].packet_id = packet_id;
	_ms[index].baudrate = 115200;
	u8 k;
	for(k=0;k<16;k++) ms_packets[index][k] = ' ';
	for(k=0;k<(packet_len+3);k++) ms_packets[index][k] = packet[k];
	_ms[index].packet = ms_packets[index];
	_ms[index].packet_len = packet_len;
	_ms[index].chksum_type = MULTISCANNER_CHK_CRC16;//chksum_type;
	_ms[index].errors = 0;
	_ms[index].flags = flags;
	_ms[index].waittime = waittime;
	_ms[index].pause = pause;
}
u16 *ms_waits, *ms_indexes;
void Init_MS(){
	u8 index = 0;
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piDIDO_r,		"\x2A\x03\x00\x06\x00\x1A..", 	6, 	MULTISCANNER_SEND, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_MR1_PORT,	SU_MR1, 	piMR1,			"\x10\x06\x00\x10\x00\x00..", 	6, 	MULTISCANNER_SEND|MULTISCANNER_PACKCHANGE, MS_MR_WAIT,MS_MR_PAUSE);
	MS_setpacket(index++,MS_MR2_PORT,	SU_MR2, 	piMR2,			"\x11\x06\x00\x10\x00\x00..", 	6, 	MULTISCANNER_SEND|MULTISCANNER_PACKCHANGE, MS_MR_WAIT,MS_MR_PAUSE);
	MS_setpacket(index++,MS_PCH_PORT,	SU_PCH, 	piPCHGetStat,	"\x01\x03\xC4\x17\x00\x01..", 	6, 	MULTISCANNER_SEND|MULTISCANNER_PACKCHANGE, MS_PCH_WAIT,MS_PCH_PAUSE);
	MS_setpacket(index++,MS_PCH_PORT,	SU_PCH, 	piPCHGetSens,	"\x01\x03\xC4\x17\x00\x01..", 	6, 	MULTISCANNER_SEND|MULTISCANNER_PACKCHANGE, MS_PCH_WAIT,MS_PCH_PAUSE);
	// отправка по флагам
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piStepCmd1,		"\x2A\x06\x00\x00\**..", 		6,	MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piStepMode1,	"\x2A\x10\x00\xFF\x00\x05\x0A*234567890..", 17, MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piStepMove1,	"\x2A\x10\x00\xFF\x00\x02\x04*234..", 		11, MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piStepMode2,	"\x2A\x10\x00\xFF\x00\x04\x08*2345678..", 	15, MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_DIDO_PORT,	SU_DIDO,	piStepFreq,		"\x2A\x06\x00\x00\**..", 		6,	MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_DIDO_WAIT,MS_DIDO_PAUSE);
	MS_setpacket(index++,MS_PCH_PORT,	SU_PCH,		piPCHCmd,		"\x01\x06\xC3\x4F\**..", 		6,	MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_PCH_WAIT,MS_PCH_PAUSE);
	MS_setpacket(index++,MS_PCH_PORT,	SU_PCH,		piPCHSet,		"\x01\x06\x0C\x1B\**..", 		6,	MULTISCANNER_ONCESEND|MULTISCANNER_PACKCHANGE, MS_PCH_WAIT,MS_PCH_PAUSE);
	MultiScanner_Init(&_ms,index,&MyData.SU_State,&FlashConf.SU_Conf);
	ms_indexes = MS_indexes();
	ms_waits = MS_waits();

	// подготовка адресов
	u8 k;
	for(k=0;k<MS_COUNT;k++)
	switch (_ms[k].packet_id) {
		case piDIDO_r:		ms_packets[k][3] = MB_READ_START_REG; break;
		case piStepCmd1: 	ms_packets[k][3] = MB_STEP_CMD1 + MB_READ_START_REG; break;
		case piStepMode1: 	ms_packets[k][3] = MB_STEP_POS1 + MB_READ_START_REG; break;
		case piStepMove1:	ms_packets[k][3] = MB_STEP_MOVE1 + MB_READ_START_REG; break;
		case piStepMode2: 	ms_packets[k][3] = MB_STEP_MODE2 + MB_READ_START_REG; break;
		case piStepFreq: 	ms_packets[k][3] = MB_STEP_KLFREQ + MB_READ_START_REG; break;

		default: break;
	}
}
#define GETREG(buf,reg)	((((u16)(buf[3+(reg)*2]))<<8) | buf[4+(reg)*2])
u16 LastZone;
void MultiSCanner_OnAnswer(u8 port, u16 packet_id, u8 *buf, u8 len){
	u32 tmp32;
	u8 k, found;;
	switch (packet_id) {
		case piDIDO_r:
			for(k=0;k<0x1A;k++)tmpmbbuf[k] = GETREG(buf,k);
			for(k=0x1A;k<40;k++)tmpmbbuf[k] = 0xBBBB;
			tmp32 = GETREG(buf,MB_STEP_DI); tmp32<<=16;
			MyData.DI = (MyData.DI & 0x0000FFFF) | tmp32;
			TmpData.StepMode1 = GETREG(buf,MB_STEP_MODE1);
			TmpData.StepStat1 = GETREG(buf,MB_STEP_STAT1);
			TmpData.StepMode2 = GETREG(buf,MB_STEP_MODE2);
			TmpData.StepStat2 = GETREG(buf,MB_STEP_STAT2);
			TmpData.StepBusy1  = GETREG(buf,MB_STEP_CMD1);
			TmpData.StepBusy2 = GETREG(buf,MB_STEP_CMD2);
			tmp32 = GETREG(buf,MB_STEP_LEN+1); tmp32 <<= 16;
			TmpData.StepLen  = tmp32 | GETREG(buf,MB_STEP_LEN);
			tmp32 = GETREG(buf,MB_STEP_CUR+1); tmp32 <<= 16;
			TmpData.StepCur  = tmp32 | GETREG(buf,MB_STEP_CUR);
			found = 0;
			for(k=1;k<=ZONE_COUNT;k++){
				if(TmpData.StepCur==TmpData.ScaledZones[k-1]){found = 1; break;}
				if(TmpData.StepCur<TmpData.ScaledZones[k]){ MyData.CurZone = k-1; found = 1; break; }
			}
			if(TmpData.StepCur==TmpData.ScaledZones[ZONE_COUNT]){ MyData.CurZone = ZONE_COUNT-1; found = 1; }
			if(TmpData.StepCur==TmpData.ScaledZones[0]){ MyData.CurZone = 0; found = 1; }
			if(!found)MyData.CurZone = 0xFF; else {
				if(LastZone!=MyData.CurZone){
					LastZone = MyData.CurZone;
					Alg_StepChangeZone();
				}
			}
			MS_DIDO_updated = 1;
			Blink(LED_DIDO_LINK);
			break;
		case piStepCmd1:
		case piStepMode1:
		case piStepMove1:
			Alg_StepWriteNext1();
			break;
		case piStepMode2:
			Alg_StepWriteNext2();
			break;
		case piPCHGetSens:
			MyData.PressCur = GETREG(buf,0);
			break;
		case piPCHGetStat:
			TmpData.PCHStat = GETREG(buf,0);
			break;
		case piMR1:

			break;
		case piMR2:
			Blink(LED_MR2_LINK);
			break;
		break;
	}
}
void MultiScanner_OnFlaggedSending(u16 packet_id, u16 index){
	u8 k;
	u16 v;
	switch (packet_id) {
	case piStepCmd1:
		v = TmpData.StepSend_Cmd1;// | (MyData.StepBusy1 & (STEP_STATE_BUSY));
		ms_packets[index][4] = v>>8;
		ms_packets[index][5] = v>>0;
		TmpData.StepSend_Cmd1 = 0;
		break;
	case piStepMove1:
		k=7;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>8;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>0;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>24;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>16;
		break;
	case piStepMode1:
		k=7;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>8;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>0;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>24;
		ms_packets[index][k++] = TmpData.StepSend_Pos1>>16;
		ms_packets[index][k++] = TmpData.StepSend_Pos2>>8;
		ms_packets[index][k++] = TmpData.StepSend_Pos2>>0;
		ms_packets[index][k++] = TmpData.StepSend_Pos2>>24;
		ms_packets[index][k++] = TmpData.StepSend_Pos2>>16;
		ms_packets[index][k++] = TmpData.StepSend_Mode1>>8;
		ms_packets[index][k++] = TmpData.StepSend_Mode1>>0;
		break;
	case piStepMode2:
		k=7;
		ms_packets[index][k++] = TmpData.StepSend_Mode2>>8;
		ms_packets[index][k++] = TmpData.StepSend_Mode2>>0;
		ms_packets[index][k++] = TmpData.StepStat2>>8;
		ms_packets[index][k++] = TmpData.StepStat2>>0;
		ms_packets[index][k++] = TmpData.StepSend_Freq>>8;
		ms_packets[index][k++] = TmpData.StepSend_Freq>>0;
		ms_packets[index][k++] = TmpData.StepSend_Cmd2>>8;
		ms_packets[index][k++] = TmpData.StepSend_Cmd2>>0;
		break;
	case piStepFreq:
		ms_packets[index][4] = TmpData.StepSend_Freq>>8;
		ms_packets[index][5] = TmpData.StepSend_Freq>>0;
		break;
	case piMR1:
		ms_packets[index][4] = 0;
		ms_packets[index][5] = MyData.DO;
		Blink(LED_MR1_LINK);
		break;
	case piMR2:
		ms_packets[index][4] = 0;
		ms_packets[index][5] = MyData.DO>>8;
		break;
	case piPCHSet:
		ms_packets[index][4] = TmpData.SendPCH_Set>>8;
		ms_packets[index][5] = TmpData.SendPCH_Set;
		break;
	case piPCHCmd:
		ms_packets[index][4] = TmpData.SendPCH_Cmd>>8;
		ms_packets[index][5] = TmpData.SendPCH_Cmd;
		break;
	default:
		break;
	}
}
void MS_SendFlaggedPacket(u16 packet_id){
	u8 k;
	for(k=0;k<MS_COUNT;k++)
		if(_ms[k].packet_id==packet_id)
			_ms[k].flags |= MULTISCANNER_SEND;
}
u32 UART_GetBaudrate(u8 UART_n){
	return 115200;
}
void UARTn_Init(u8 UARTn, u32 baudrate, u16 confbits){};
