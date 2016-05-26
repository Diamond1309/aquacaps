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
