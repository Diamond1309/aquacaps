#ifndef UTILS_H
#define UTILS_H
#include "misc.h"
/*
 * ������������� PWR, BKP, RTC (�������� �� ���� ������������)
 * ������� Init_Utils() ������ ����������� ������
 * ���� ������ ���������� USB, �� ��������� ��� ���, ����� ������������� USB
 */
extern void Init_Utils();

// �������
extern unsigned short int GetCRC16(unsigned char *Data, unsigned short int DataLen);
extern unsigned char GetCRC16_OkLen(unsigned char *Data, int DataLen, unsigned short int *ResultLen);
extern void SetCheckSum(unsigned char *cmd, unsigned short len);
extern unsigned char CheckSumOk(unsigned char *cmd, unsigned short len); // ����������� ����� DCON
extern char* IntToHexStr(uint32_t source, char len); // ����� � ASCII, len - ���-�� �������� ASCII
extern void _IntToHexStr(char* dest,uint32_t source, char len); // ����� � ASCII, len - ���-�� �������� ASCII
extern void ArrayToHexStr(char* source, char* dest, char len); // ������ ����� � ������ ASCII [16,33,44] = "10212C"
extern uint32_t HexStrToInt(unsigned char *s,unsigned char len); // ASCII � �����, len - ���-�� ���� ��������� �����
extern char IntToDecStr(unsigned char *dest,uint32_t source); // ����� � ������ 10 = "10"
extern void _IntToDecStr(unsigned char *dest,uint32_t source,unsigned char len);
extern uint32_t DecStrToInt(unsigned char *s,unsigned char len);
extern uint16_t mystrlen(char *str); // ����� ������, ������� ����� - ������ \0
extern void mystrcpy(uint8_t *dest,uint8_t *source);
extern void Delay_ms(uint32_t ms); // �������� � ��


// ������� ��� ������ � ������������� �� ����
extern void FlashConf_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashConf_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned short int FlashConf_CRC(unsigned int len);
/*extern void FlashConf_Check(); // ��������� ���� ��� �������� ��������� � ������, ���� ��� ����
extern void _FlashConf_Check(); // ����������� �������� � ������ ���������
extern void FlashConf_Pool(); // ��������� ������������ � main
extern void Init_FlashConf(); // ������������� ������������
extern void FlashConf_CheckTimer(); // ��������� ������ �������
extern uint16_t FlashWriteStatus;
extern uint16_t ConfCRC;
extern uint8_t	confcrc_check;*/

/* ������� ��������
 * ��� ������ ������ �������� ����� ���� ������ ������ ���� ������ 4�
 */
extern uint32_t FlashFirm_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashFirm_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned int Get_Firmware_CRC(unsigned int size); // ��������� CRC ����������� ��������
extern unsigned long int Find_Firmware_CRC(uint32_t StartOffset, uint32_t EndOffset); // ����� �����, ����� CRC == 0
extern void FlashFirm_UpdateFirmware(); // ������������� � ������ ��������

extern uint32_t FlashProg_Write(uint32_t addr, uint32_t *source, uint32_t len);
extern void FlashProg_Read(uint32_t addr, uint32_t *dest, uint32_t len);
extern unsigned int Get_Prog_CRC(unsigned int size);


// ������� ������ � ������� BackupSRAM - 4kB - 64B ���������
extern void BKP_Write(uint32_t StartAddr, unsigned char *source, uint32_t Len);
extern void BKP_Read(uint32_t StartAddr, unsigned char *dest, uint32_t Len);

/* ������� ��� ������ � RTC
 * ��� ������������� ����� � RTC ������������ �� ������� ����������� GlobalTime
 */
extern void Utils_SetTime(uint32_t settime); // �������� �����
extern void Utils_SetTimeL(uint16_t Ltime); // �������� ������� ����� �������
extern void Utils_SetTimeH(uint16_t Htime); // �������� ������� ����� �������
extern uint32_t Utils_GetTime();
extern uint32_t Utils_GetDime();
/*
typedef struct FlashConf_t{
	uint16_t SN;
	uint16_t SlaveAddr;
	uint16_t tmp1,tmp2;
}FlashConf_t;
FlashConf_t FlashConf;
// ��� ��� ����� ������������
void Init_FlashConf(){
	if(FlashConf.SN==0xFFFF){
		// TODO: �������� �������� �� ���������
		FlashConf.SN=140724;
		FlashConf.SlaveAddr = 21;
		FlashConf_Check();
	}
}
// �������� �� ��������� � ������ ���� ����
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
