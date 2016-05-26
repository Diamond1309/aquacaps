#ifndef _UARTS_H_
#define _UARTS_H_
#ifndef _COMONDEF_H
#include "comondef.h"
#endif
#define RS485_UART 	1
#define RS232_UART 	4


void UART1_Init(unsigned long baudrate);
void UART2_Init(unsigned long baudrate);
void UART3_Init(unsigned long baudrate);
void UART4_Init(unsigned long baudrate);

void UARTBufSend(char UART_num,unsigned char *tx_buffer,unsigned int TxLen); // отправка данных
void UARTSend(char UART_num,unsigned char *tx_buffer);						// отправка текста

void UART_Timers_Execute(void); // функция обработки таймеров по приему данных по USART
void UART_Execute(void);
extern uint32_t Baud[];

extern char UART_Rx_Flags;
#define UART1_timeout 10
#define UART2_timeout 10
#define UART3_timeout 10
#define UART4_timeout 100
#define UART5_timeout 10

extern unsigned int UART_break[];
#define RX_BUFFER_SIZE_1 1024
#define RX_BUFFER_SIZE_2 256
#define RX_BUFFER_SIZE_3 256
#define RX_BUFFER_SIZE_4 1024
extern unsigned char rx_buffer1[],rx_buffer2[],rx_buffer3[],rx_buffer4[];
extern unsigned int rx_len1,rx_len2,rx_len3,rx_len4;
extern unsigned int rx_index1,rx_index2,rx_index3,rx_index4;
extern char UART_PacketFlags;

#define FLAG1 0x01
#define FLAG2 0x02
#define FLAG3 0x04
#define FLAG4 0x08
#define UN_FLAG1 0xFE
#define UN_FLAG2 0xFD
#define UN_FLAG3 0xFB
#define UN_FLAG4 0xF7

#endif
