#include "archive.h"
void Archive_Add(uint8_t code, uint8_t number, uint16_t value){
	ArchiveStruct_t as;
	as.code = code;
	as.number = number;
	as.value = value;
	as.time = GlobalTime;
	I2C_EE_BufferWrite(&as,MyData.ArchPoint,sizeof(ArchiveStruct_t));
	MyData.ArchPoint+=sizeof(ArchiveStruct_t);
	Periph_BKPWriteLong(BKP_ARCH,MyData.ArchPoint);
}

uint16_t Archive_ModbusResponse(uint8_t *Data,uint16_t len){
	if(len<8)return 0;
	uint32_t ArcAddr  = ((uint32_t)(Data[5])<<16) | ((uint32_t)(Data[4])<<8) | (uint32_t)(Data[3]);
	uint32_t k, Count = Data[2]; // nbw (���-�� ���������)
	if(Count>124)Count = 124;
	Count*=2; // nbb = nbw*2 (���-�� ����)

	ArcAddr &= I2C_EE_ADDRESS_MASK;

	I2C_EE_BufferRead(&Data[5],ArcAddr,Count); // ��������� ���������� ������

	Data[3] = 0x3;
	for(k=0;k<Count;k++){ // �������� �� ������� ����� ������
		if(((ArcAddr+k) & I2C_EE_ADDRESS_MASK) == (MyData.ArchPoint & I2C_EE_ADDRESS_MASK)){
			Data[3]&=~0x02; // ������������� ��� "����� ������" - �� �����!
			Count = k; // ������� ����� ������ �� ��������� �������� ������ !!
			break;
		}
	}
	Data[4] = Count; // �������� � ����� nbb
	//uint16_t TmpCRC = GetCRC16(Data,Count+5); // �������� CRC ����� ������
	//Data[Count+5] = TmpCRC;
	//Data[Count+6] = TmpCRC>>8;
	return Count+5; // ���������� ����� ����� ������
}
