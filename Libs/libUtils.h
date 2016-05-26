#ifndef UTILS_H
#define UTILS_H
#include "misc.h"
/*
 * Инициализация PWR, BKP, RTC (проверка на флаг перепрошивки)
 * Функция Init_Utils() должна запускаться первым
 * если проект использует USB, то запустить еще раз, после инициализации USB
 */
extern void Init_Utils();

// функции
extern unsigned short int GetCRC16(unsigned char *Data, unsigned short int DataLen);
extern unsigned char GetCRC16_OkLen(unsigned char *Data, int DataLen, unsigned short int *ResultLen);
extern void SetCheckSum(unsigned char *cmd, unsigned short len);
extern unsigned char CheckSumOk(unsigned char *cmd, unsigned short len); // контрольная сумма DCON
extern char* IntToHexStr(uint32_t source, char len); // число в ASCII, len - кол-во символов ASCII
extern void _IntToHexStr(char* dest,uint32_t source, char len); // число в ASCII, len - кол-во символов ASCII
extern void ArrayToHexStr(char* source, char* dest, char len); // массив чисел в строку ASCII [16,33,44] = "10212C"
extern uint32_t HexStrToInt(unsigned char *s,unsigned char len); // ASCII в число, len - кол-во байт выходного числа
extern char IntToDecStr(unsigned char *dest,uint32_t source); // число в строку 10 = "10"
extern void _IntToDecStr(unsigned char *dest,uint32_t source,unsigned char len);
extern uint32_t DecStrToInt(unsigned char *s,unsigned char len);
extern uint16_t mystrlen(char *str); // длина строки, признак конца - символ \0
extern void mystrcpy(uint8_t *dest,uint8_t *source);
extern void Delay_ms(uint32_t ms); // задержка в мс


// Функции для работы с конфигурацией во флеш
extern void FlashConf_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashConf_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned short int FlashConf_CRC(unsigned int len);
/*extern void FlashConf_Check(); // выставить флаг для проверки изменений и записи, если они есть
extern void _FlashConf_Check(); // немедленная проверка и запись изменений
extern void FlashConf_Pool(); // запускать периодически в main
extern void Init_FlashConf(); // инициализация конфигурации
extern void FlashConf_CheckTimer(); // запускать каждую секунду
extern uint16_t FlashWriteStatus;
extern uint16_t ConfCRC;
extern uint8_t	confcrc_check;*/

/* Функции прошивки
 * при записи кусков прошивки длина этих кусков должна быть кратна 4м
 */
extern uint32_t FlashFirm_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashFirm_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned int Get_Firmware_CRC(unsigned int size); // проверить CRC загруженной прошивки
extern unsigned long int Find_Firmware_CRC(uint32_t StartOffset, uint32_t EndOffset); // найти сдвиг, когда CRC == 0
extern void FlashFirm_UpdateFirmware(); // перезагрузить и начать прошивку

extern uint32_t FlashProg_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashProg_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned int Get_Prog_CRC(unsigned int size);


// Функции записи в область BackupSRAM - 4kB - 64B системных
extern void BKP_Write(uint32_t StartAddr, unsigned char *source, uint32_t Len);
extern void BKP_Read(uint32_t StartAddr, unsigned char *dest, uint32_t Len);

/* Функции для работы с RTC
 * При инициализации время с RTC записывается во внешнюю переменнуюю GlobalTime
 */
extern void Utils_SetTime(uint32_t settime); // записать время
extern void Utils_SetTimeL(uint16_t Ltime); // записать младшее слово времени
extern void Utils_SetTimeH(uint16_t Htime); // записать старшее слово времени
extern uint32_t Utils_GetTime();
extern uint32_t Utils_GetDime();
/*
typedef struct FlashConf_t{
	uint16_t SN;
	uint16_t SlaveAddr;
	uint16_t tmp1,tmp2;
}FlashConf_t;
FlashConf_t FlashConf;
// это все нужно раскоментить
void Init_FlashConf(){
	if(FlashConf.SN==0xFFFF){
		// TODO: Записать значения по умолчанию
		FlashConf.SN=140724;
		FlashConf.SlaveAddr = 21;
		FlashConf_Check();
	}
}
// Проверка на изменения и запись если есть
void _FlashConf_Check(){
	uint16_t TmpCRC=0;
    TmpCRC = GetCRC16((unsigned char*)&FlashConf,sizeof(FlashConf));
    if(ConfCRC != TmpCRC){
    	FlashConf_Write((uint32_t*)&FlashConf,sizeof(FlashConf));
    	FlashConf_Read((uint32_t*)&FlashConf,sizeof(FlashConf));
    	ConfCRC = TmpCRC;
    }
}
*/
#endif
