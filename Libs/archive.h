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
#define ARCH_CODE_EVENT		1
#define ARCH_CODE_WARN		2
#define ARCH_CODE_ALARM		3
#define ARCH_CODE_SU		4

#define ARCH_NUMBER_EVENT_POWERON		1
#define ARCH_NUMBER_EVENT_STARTING		2
#define ARCH_NUMBER_EVENT_STARTED		3
#define ARCH_NUMBER_EVENT_WORKDONE		4
#define ARCH_NUMBER_EVENT_WORKABORT		5

// добавление архива
void Archive_Add(uint8_t code, uint8_t number, uint16_t value);
// чтение архива
uint16_t Archive_ModbusResponse(uint8_t *Data,uint16_t len);

#endif
