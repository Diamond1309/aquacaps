/**************************************************
	���������� "ModBusSlave" ��� stm32F405R
	��� ��� "�������-����������"
	������� ����� ���������
	14 ��� 2014
		
**************************************************/
#define GetMassiv(addr,massiv) if((Addr>=addr)&&(Addr<(sizeof(massiv)/2 + addr))) return *(((uint16_t*)(&massiv)) + (Addr - addr))
#define SetMassiv(addr,massiv) if((Addr>=addr)&&(Addr<(sizeof(massiv)/2 + addr))) {*(((uint16_t*)(&massiv)) + (Addr - addr)) = Data

extern unsigned int MBS_GetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr);
extern void MBS_SetBit(unsigned char UART_n, unsigned short int RTU, unsigned short int Addr,unsigned short int  Data);
extern void MBS_SetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr,unsigned short int  Data);

char MBS_Process(unsigned char UART_n, unsigned char *Data, unsigned char DataLen, unsigned short Address); // - �������� ��� ������ ������ �� UART
unsigned char MBS_GetAnswer(unsigned char UART_n, unsigned char *Data, unsigned char DataLen, unsigned short Address); // �������� ����� � ����� �� ���������
/*
unsigned int MBS_GetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr)
{
    switch (Addr)
    {
        //case 0x0000: return VERSION;
        default:
        	//GetMassiv(0x0020,ci_timer);
        	//GetMassiv(AddrMyData_Begin,MyData);
        	//GetMassiv(AddrConfig_Begin,FlashConf);
        	return 0xAAAA;
    }
}

void MBS_SetReg(unsigned char UART_n, unsigned short int RTU, unsigned short int  Addr,unsigned short int  Data)
{
    switch (Addr)
    {
        case 0x0000: break;
        default:
        	//SetMassiv(AddrMyData_Begin,MyData);}
            //SetMassiv(AddrConfig_Begin,FlashConf);
            //		flashconfwrite_timer = 200;}

        break;
    }
}

void MBS_SetBit(unsigned char UART_n, unsigned short int RTU, unsigned short int Addr,unsigned short int  Data)
{
	unsigned int TmpInt, TmpInt2;
    TmpInt=MBS_GetReg((Addr>>4));
    TmpInt2 = 1 << (Addr&0x0F);
    if (Data)
        TmpInt |= TmpInt2;
    else
        TmpInt &=~ TmpInt2;
    MBS_SetReg((Addr>>4),TmpInt);
}

unsigned char MBS_UserFunction(unsigned char UART_n, unsigned char *Data, unsigned char DataLen, unsigned char Address, unsigned char da){
	// 
	// 	��������� ���������������� ������� ModBus
	//	CRC16 ������ � MB ����� ��� ��������.
	//	������� ������:
	//	- ���������� ����� Data[DataLen] 
	//	- ������� ����� � ����� *Data
	//	- ������� ���-�� ���� � ������
	//	- ���� ������� �� ���������� ������, ���� ����� �� �������� ��� ���������, �� ������ ������� 0
	return 0;
}

*/
