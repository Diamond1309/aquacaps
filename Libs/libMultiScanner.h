#ifndef MULTISCANNER_H
#define MULTISCANNER_H

#define MULTISCANNER_SEND		0x0001
#define MULTISCANNER_ONCESEND	0x0002
#define MULTISCANNER_PACKCHANGE	0x0004
#define MULTISCANNER_ANS_EXIST	0x8000

#define MULTISCANNER_CHK_NONE	0
#define MULTISCANNER_CHK_DCON	1
#define MULTISCANNER_CHK_CRC16	2
#pragma pack(1);
typedef struct MultiScanner_t{
	u8	port;
	u32 su_mask;
	u16 packet_id;
	u32 baudrate;
	u8	*packet;
	u8	packet_len;
	u8	chksum_type; // 0-нет, 1-DCON, 2-CRC16
	u8	errors;		// = 0
	u16	flags;
	u16	waittime;
	u16 pause;
	u16 portconfbits;
}MultiScanner_t;
#pragma pack();
void MultiScanner_Init(MultiScanner_t *mscanner, u8 count, u32 *SU_state, u32 *SU_conf);
void MultiScanner_Timers(void);
void MultiScanner_Execute(void);
void MultiScanner_RxProcess(u8 port, u8* buf, u8 len);
extern void MultiSCanner_OnAnswer(u8 port, u16 packet_id, u8 *buf, u8 len);
extern void MultiScanner_OnFlaggedSending(u16 packet_id, u16 index);

extern u32 UART_GetBaudrate(u8 UART_n); // считать настроенный baudrate
extern void UARTn_Init(u8 UARTn, u32 baudrate, u16 confbits);
extern void UARTBufSend(char UART_num,unsigned char *tx_buffer,unsigned int TxLen);
extern unsigned short int GetCRC16(unsigned char *Data, unsigned short int DataLen);
extern void SetCheckSum(unsigned char *cmd, unsigned short len);
extern unsigned char CheckSumOk(unsigned char *cmd, unsigned short len); // контрольная сумма DCON

u16 *MS_waits();
u16 *MS_indexes();
#endif
