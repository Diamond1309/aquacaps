#include "comondef.h"
#define GetPointer(addr,pointer,size) if((Addr>=addr)&&(Addr<(size/2 + addr))) return *(((uint16_t*)(pointer)) + (Addr - addr))
#define SetPointer(addr,pointer,size) if((Addr>=addr)&&(Addr<(size/2 + addr))) *(((uint16_t*)(pointer)) + (Addr - addr)) = Data
void MBS_OnResponseStart(){};
void MBS_OnResponseEnd(){};
extern u16 *ms_waits, *ms_indexes;
u16 tmpmbbuf[40];
/// дебаги
extern unsigned int TxLen1,TxLen2,TxLen3;
extern u8 step_write_step1, step_write_start1;
extern u16 Alg_Events;
u16 i2c_buf[128];
///--------------------------------------
unsigned int MBS_GetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr)
{
    switch (Addr)
    {
        case 0x0000: return VERSION;
        case 0x0006: return Periph_BKPReadLong(BKP_SVN);
        case 0x0007: return (Periph_BKPReadLong(BKP_SVN)>>16);
        case 0x0008: return Periph_BKPRead(BKP_SKN);
        default:
        	GetPointer(0x0001,&GlobalTime,4);
        	//GetPointer(0x0008,&BLStart,4);
        	GetMassiv(AddrMyData_Begin,MyData);
        	GetMassiv(AddrTmpData_Begin,TmpData);
        	GetMassiv(0x200,tmpmbbuf);
        	GetMassiv(AddrConfig_Begin,FlashConf);
        	GetMassiv(0x1000,i2c_buf);
        	return 0xAAAA;
    }
}

void MBS_SetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr,unsigned short int  Data)
{
    switch (Addr)
    {
        case 0x0000: break;
        case 0x0004: CmdFlags |= Data; break;
        case 0x0005: CmdFlags |= ((u32)Data)<<16; break;
        case 0x0006: Periph_BKPWrite(0,Data); break;
        case 0x0007: Periph_BKPWrite(1,Data); break;
        case 0x0FFE: I2C_EE_BufferRead((uint8_t*)i2c_buf,((u32)Data)<<4,8); break;
        case 0x0FFF: I2C_EE_BufferWrite((uint8_t*)i2c_buf,((u32)Data)<<4,8); break;
        default:
        	SetPointer(0x0001,&GlobalTime,4);
        	SetMassiv(AddrMyData_Begin,MyData);}
            SetMassiv(AddrConfig_Begin,FlashConf);
            		flashconfwrite_timer = 200;}
			if(Addr == TmpData.MBaddr_HoldeZone){
        		Alg_HoldZone2();
        	}
			if((Addr>=TmpData.MBaddr_PressStart)&&(Addr<=TmpData.MBaddr_PressEnd))
				Alg_FlagChaneZoneParam();
			if((Addr>=TmpData.MBaddr_KlapanStart)&&(Addr<=TmpData.MBaddr_KlapanEnd))
				Alg_FlagChaneZoneParam();
			SetMassiv(0x1000,i2c_buf);}
        break;
    }
}

void MBS_SetBit(unsigned char UART_n, unsigned short int RTU, unsigned short int Addr,unsigned short int  Data)
{
	/*unsigned int TmpInt, TmpInt2;
    TmpInt=MBS_GetReg((Addr>>4));
    TmpInt2 = 1 << (Addr&0x0F);
    if (Data)
        TmpInt |= TmpInt2;
    else
        TmpInt &=~ TmpInt2;
    MBS_SetReg((Addr>>4),TmpInt);*/
}

unsigned char MBS_UserFunction(unsigned char UART_n, unsigned char *Data, unsigned char DataLen, unsigned char Address, unsigned char da){
	//
	// 	Обработка пользовательских функций ModBus
	//	CRC16 пакета и MB адрес уже проверен.
	//	Функция должна:
	//	- обработать пакет Data[DataLen]
	//	- собрать ответ в буфер *Data
	//	- вернуть кол-во байт в ответе
	//	- если функция не отправляет ответа, либо пакет не подходит для обработки, то должна вернуть 0
	if(Data[1]==0x54){

	}
	if(Data[1]==0x51){
		if (DataLen <= 8) return 0;
		u32 StartAddr = *((uint32_t*)(Data+2+da));
		u16 PointNo  = Data[6+da];                             //количество передааемых байт
		if(Data[7+da] != 0){
			FlashFirm_Write(StartAddr, (uint32_t*)(Data+8+da), PointNo);
		}else{
			u32 progsize = PointNo;
			while(progsize%4)progsize++; // выравнивание к кратности 4м
			FlashFirm_Write(StartAddr, (uint32_t*)(Data+8+da), progsize);
			if(!Get_Firmware_CRC(StartAddr + PointNo))firm_timer=10000;
		}
		return 6+da;
	}
	return 0;
}

