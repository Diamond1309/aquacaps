#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "comondef.h"
#pragma pack(1)
typedef struct ArchiveStruct_t{
	uint8_t code;
	uint8_t number;
	uint16_t value;
	uint32_t time;
}ArchiveStruct_t;
#pragma pack(0)

void Archive_Add(uint8_t code, uint8_t number, uint16_t value);
// � ��� ���� ������� ��������� �������
void Archive_TmpFunc();
// ��� ����
void Archive_TmpFunc2();
#endif
