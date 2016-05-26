#include "uarts.h"
#include "stm32f10x_usart.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

uint32_t Baud[] = {0,0,0,0,1200,2400,4800,9600,19200,38400,57600,115200};

char UART_Rx_Flags=0;
char UART_PacketFlags=0;
char UART_TextSendFlag=0;
unsigned int UART_break[5]={0,0,0,0,0};
unsigned char rx_buffer1[RX_BUFFER_SIZE_1] = {'\0'};
unsigned char rx_buffer2[RX_BUFFER_SIZE_2] = {'\0'};
unsigned char rx_buffer3[RX_BUFFER_SIZE_3] = {'\0'};
unsigned char rx_buffer4[RX_BUFFER_SIZE_4] = {'\0'};
unsigned int rx_len1=0,rx_len2=0,rx_len3=0,rx_len4=0;
unsigned int rx_index1=0,rx_index2=0,rx_index3=0,rx_index4=0;
unsigned int TxLen1=0;
unsigned int tx_index1=0;
unsigned char *tx_buffer1;
unsigned int TxLen2=0;
unsigned int tx_index2=0;
unsigned char *tx_buffer2;
unsigned int TxLen3=0;
unsigned int tx_index3=0;
unsigned char *tx_buffer3;
unsigned int TxLen4=0;
unsigned int tx_index4=0;
unsigned char *tx_buffer4;

#define TRANSFER2 		GPIOA->ODR|= GPIO_Pin_1
#define RECIEVE2 		GPIOA->ODR &= ~(GPIO_Pin_1)
#define CHECK_TRANSFER2	GPIOA->ODR & GPIO_Pin_1
#define TRANSFER3 		GPIOB->ODR |= GPIO_Pin_12
#define RECIEVE3 		GPIOB->ODR &= ~(GPIO_Pin_12)
#define CHECK_TRANSFER3	GPIOB->ODR & GPIO_Pin_12

unsigned int UART_IntEnabled = 0;
#define UART1_IE 0x0001
#define UART2_IE 0x0002
#define UART3_IE 0x0004
#define UART4_IE 0x0008


void UART_Timers_Execute(void){
	if(UART_Rx_Flags&FLAG1){
		UART_break[1]++;
		//SetLed1(1);
		if(UART_break[1]>UART1_timeout){
			UART_Rx_Flags&=UN_FLAG1;
			UART_PacketFlags|=FLAG1;
			rx_len1 = rx_index1;
			rx_index1 = 0;
			//SetLed1(0);
		}
	}
	if(UART_Rx_Flags&FLAG2){
		UART_break[2]++;
		if(UART_break[2]>UART2_timeout){
			UART_Rx_Flags&=UN_FLAG2;
			UART_PacketFlags|=FLAG2;
			rx_len2 = rx_index2;
			rx_index2 = 0;
		}
	}
	if(UART_Rx_Flags&FLAG3){
		UART_break[3]++;
		if(UART_break[3]>UART3_timeout){
			UART_Rx_Flags&=UN_FLAG3;
			UART_PacketFlags|=FLAG3;
			rx_len3 = rx_index3;
			rx_index3 = 0;
		}
	}
	if(UART_Rx_Flags&FLAG4){
		//SetLed2(1);
		UART_break[4]++;
		if(UART_break[4]>UART4_timeout){
			UART_Rx_Flags&=UN_FLAG4;
			UART_PacketFlags|=FLAG4;
			rx_len4 = rx_index4;
			rx_index4 = 0;
			//SetLed2(0);
		}
	}
}
//----------------- прерывания USART 1 -----------------------------
void USART1_IRQHandler(void)
{
    // прерывание по приему символа
	if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET){
			char data = USART_ReceiveData(USART1);
			if(rx_index1 == RX_BUFFER_SIZE_1){
				rx_buffer1[rx_index1] = data;
                rx_index1 = 0;
			}
			else{
				rx_buffer1[rx_index1++] = data;
			}
			rx_buffer1[rx_index1] = '\0';
			UART_break[1]=1;
			UART_Rx_Flags|=FLAG1;
	}
	// прерывание по завершению отправки символа
	//if(UART_IntEnabled & UART1_IE){
	//	Led2();
	if ((USART1->SR & USART_FLAG_TC) != (u16)RESET){
		USART_ClearFlag(USART1,USART_FLAG_TC);
			if(TxLen1){
				TxLen1--;
				USART1->DR = tx_buffer1[tx_index1++];
			}
		}
	//}
}
//----------------- прерывания USART 2 -----------------------------
void USART2_IRQHandler(void)
{
	// прерывание по приему символа
    if ((USART2->SR & USART_FLAG_RXNE) != (u16)RESET){
			char data = USART_ReceiveData(USART2);
			if(rx_index2 == RX_BUFFER_SIZE_2){
				rx_buffer2[rx_index2] = data;
                rx_index2 = 0;
			}
			else{
				rx_buffer2[rx_index2++] = data;
			}
			rx_buffer2[rx_index2] = '\0';
			UART_break[2]=1;
			UART_Rx_Flags|=FLAG2;
	}
    // прерывание по завершению отправки символа

    if ((USART2->SR & USART_FLAG_TC) != (u16)RESET){
    	USART_ClearFlag(USART2,USART_FLAG_TC);
    	if(TxLen2){
    		TxLen2--;
    		USART2->DR = tx_buffer2[tx_index2++];
    	}else RECIEVE2;
    }
}
//----------------- прерывания USART 3 -----------------------------
void USART3_IRQHandler(void)
{
	// прерывание по приему символа
    if ((USART3->SR & USART_FLAG_RXNE) != (u16)RESET){
			char data = USART_ReceiveData(USART3);
			if(rx_index3 == RX_BUFFER_SIZE_3){
				rx_buffer3[rx_index3] = data;
                rx_index3 = 0;
			}
			else{
				rx_buffer3[rx_index3++] = data;
			}
			rx_buffer3[rx_index3] = '\0';
			UART_break[3]=1;
			UART_Rx_Flags|=FLAG3;
	}
    // прерывание по завершению отправки символа

    if ((USART3->SR & USART_FLAG_TC) != (u16)RESET){
    	USART_ClearFlag(USART3,USART_FLAG_TC);
    	if(TxLen3){
    		TxLen3--;
        	USART3->DR = tx_buffer3[tx_index3++];
    	}
    	else RECIEVE3;
    }
}
//----------------- прерывания USART 4 -----------------------------
void UART4_IRQHandler(void)
{
    // прерывание по приему символа

	if ((UART4->SR & USART_FLAG_RXNE) != (u16)RESET){
			char data = USART_ReceiveData(UART4);
			if(rx_index4 == RX_BUFFER_SIZE_4){
				rx_buffer4[rx_index4] = data;
                rx_index4 = 0;
			}
			else{
				rx_buffer4[rx_index4++] = data;
			}
			rx_buffer4[rx_index4] = '\0';
			UART_break[4]=1;
			UART_Rx_Flags|=FLAG4;
	}
	// прерывание по завершению отправки символа

    if ((UART4->SR & USART_FLAG_TC) != (u16)RESET){
    	USART_ClearFlag(UART4,USART_FLAG_TC);
    	if(TxLen4){
    		TxLen4--;
    		UART4->DR = tx_buffer4[tx_index4++];
    	}
    }
}
// ====================================================================================
/*
 * Отправка данных в UART
 */
void UARTBufSend(char UART_num,unsigned char *tx_buffer,unsigned int TxLen){
	switch(UART_num){
	case 1:
			tx_buffer1=tx_buffer;
			tx_index1=0;
			TxLen1=TxLen;
			if(TxLen1--)
			    USART1->DR = tx_buffer1[tx_index1++];
			break;
	case 2: TRANSFER2;
			tx_buffer2=tx_buffer;
			tx_index2=0;
			TxLen2=TxLen;
			if(TxLen2--)
				USART2->DR = tx_buffer2[tx_index2++];
			break;
	case 3: TRANSFER3;
			tx_buffer3=tx_buffer;
			tx_index3=0;
			TxLen3=TxLen;
			if(TxLen3--)
				USART3->DR = tx_buffer3[tx_index3++];
			break;
	case 4:
			tx_buffer4=tx_buffer;
			tx_index4=0;
			TxLen4=TxLen;
			if(TxLen4--)
				UART4->DR = tx_buffer4[tx_index4++];
			break;
	}
}
/*
 * Отправка текста в UART
 */
void UARTSend(char UART_num,unsigned char *tx_buffer)
{
	unsigned int i = 0;
	while(tx_buffer[i++] != 0){}
	if(i) UARTBufSend(UART_num,tx_buffer,i-1);
	switch(UART_num){
	    case 1: while(TxLen1){;};break;
	    case 2: while(TxLen2){;};break;
	    case 3: while(TxLen3){;};break;
	    case 4: while(TxLen4){;};break;
	}

	return;
	/*
	USART_TypeDef* USART=USART1;
    switch(UART_num){
    case 1: UART_IntEnabled &= ~UART1_IE;break;
    case 2: USART=USART2;TRANSFER;delay(0x1000);break;
    case 3: USART=USART3;break;
    }
    USART_ITConfig(USART, USART_IT_TC, DISABLE);
	while( *tx_buffer != 0x00 ){
		Led1();
        //USART_SendData(USART, (uint16_t) *tx_buffer++);
		USART->DR = *tx_buffer++;
        while(USART_GetFlagStatus(USART, USART_FLAG_TC) == RESET){}
        //USART_ClearFlag(USART2,USART_FLAG_TC);
    }
    //if(UART_num==RS485_UART){delay(0x1000);RECIEVE;}
	*/
}
/*
 * нужно добавить RCC_APB2Periph_USART1 в параметрах функции
 * RCC_APB2PeriphClockCmd(...
 * Пример: 	RCC_APB2PeriphClockCmd(
				RCC_APB2Periph_GPIOA|
				RCC_APB2Periph_GPIOB|
				RCC_APB2Periph_USART1, ENABLE);
 */
void UART1_Init(unsigned long baudrate){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef v;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);

	v.NVIC_IRQChannel = USART1_IRQn;
	v.NVIC_IRQChannelCmd = ENABLE;
	v.NVIC_IRQChannelPreemptionPriority = 3;
	v.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init (&v);

	UART_Rx_Flags&=0xFE; // сброс наличия принятых бит
}
void UART2_Init(unsigned long baudrate){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef v;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_TC, ENABLE);

	v.NVIC_IRQChannel = USART2_IRQn;
	v.NVIC_IRQChannelCmd = ENABLE;
	v.NVIC_IRQChannelPreemptionPriority = 3;
	v.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init (&v);
	UART_Rx_Flags&=0xFD; // сброс наличия принятых бит
	RECIEVE2;
}
void UART3_Init(unsigned long baudrate){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef v;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_Cmd(USART3, ENABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_TC, ENABLE);

	v.NVIC_IRQChannel = USART3_IRQn;
	v.NVIC_IRQChannelCmd = ENABLE;
	v.NVIC_IRQChannelPreemptionPriority = 3;
	v.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init (&v);

	UART_Rx_Flags&=0xFB; // сброс наличия принятых бит
	RECIEVE3;
}
void UART4_Init(unsigned long baudrate){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef v;

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	USART_Cmd(UART4, ENABLE);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART4, USART_IT_TC, ENABLE);

	v.NVIC_IRQChannel = UART4_IRQn;
	v.NVIC_IRQChannelCmd = ENABLE;
	v.NVIC_IRQChannelPreemptionPriority = 3;
	v.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init (&v);

	UART_Rx_Flags&=0xF7; // сброс наличия принятых бит
}

u32 txtmr2,txtmr3;
void UART_Execute(){
	if(UART_PacketFlags&FLAG1){
	    UART_PacketFlags&=UN_FLAG1;
	    MultiScanner_RxProcess(1, rx_buffer1,rx_len1);
	}
	if(UART_PacketFlags&FLAG2){
		UART_PacketFlags&=UN_FLAG2;
		//MBS_Process(2,rx_buffer2,rx_len2,247);
		MultiScanner_RxProcess(2, rx_buffer2,rx_len2);
		//UARTBufSend(2,rx_buffer2,rx_len2);
	}
	if(UART_PacketFlags&FLAG3){
		UART_PacketFlags&=UN_FLAG3;
		MBS_Process(3,rx_buffer3,rx_len3,247);
		//UARTBufSend(3,rx_buffer3,rx_len3);
	}

	if(!TxLen2)if(CHECK_TRANSFER2){
		txtmr2++;
		if(txtmr2>1000){
			RECIEVE2;
			txtmr2=0;
		}
	}else txtmr2=0;
	if(!TxLen3)if(CHECK_TRANSFER3){
		txtmr3++;
		if(txtmr3>1000){
			RECIEVE3;
			txtmr3=0;
		}
	}else txtmr3=0;
	/*
	if(UART_PacketFlags&FLAG4){
		UART_PacketFlags&=UN_FLAG4;

	}
	if(UART_PacketFlags&FLAG2){
		UART_PacketFlags&=UN_FLAG2;
	}*/
}
