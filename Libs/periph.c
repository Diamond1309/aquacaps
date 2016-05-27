#include "periph.h"
uint8_t DS18b20_addr[2][8];
#define DS18B20_READ_PAUSE 1000
u32 ds18b20_read_pause;
u32 blink_timer[8];
u8 LedValue;
#define BKP_DR_NUMBER              42
uint16_t BKPDataReg[BKP_DR_NUMBER] = { BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4,
        BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8, BKP_DR9, BKP_DR10, BKP_DR11,
        BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16, BKP_DR17, BKP_DR18,
        BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24, BKP_DR25,
        BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
        BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39,
        BKP_DR40, BKP_DR41, BKP_DR42 };

void Init_Leds(){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_13 |GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void Init_DI(){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
int Init_RTC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//разрешить тактирование модулей управления питанием и управлением резервной областью
	  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	  //разрешить доступ к области резервных данных
	  PWR->CR |= PWR_CR_DBP;
	  //если часы выключены - инициализировать их
	  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
	  //if(1)
	  {
	    //выполнить сброс области резервных данных
	    RCC->BDCR |=  RCC_BDCR_BDRST;
	    RCC->BDCR &= ~RCC_BDCR_BDRST;

	    //выбрать источником тактовых импульсов внешний кварц 32768 и подать тактирование
	    RCC->BDCR |=  RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;

	    RTC->CRL  |=  RTC_CRL_CNF;   //разрешить конфигурирование регистров RTC
	    RTC->PRLL  = 0x7FFF;         //регистр деления на 32768
	    RTC->CRH  =  RTC_CRH_SECIE;  //разрешить прерывание от секундных импульсов
	    RTC->CRL  &=  ~RTC_CRL_CNF;  //выйти из режима конфигурирования

	    //установить бит разрешения работы и дождаться установки бита готовности
	    RCC->BDCR |= RCC_BDCR_LSEON;
	    while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){}

	    RTC->CRL &= (uint16_t)~RTC_CRL_RSF;
	    while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}

	    return 1;
	  }
	  RTC->CRL |=  RTC_CRL_CNF;  //разрешить конфигурирование регистров RTC
	  RTC->CRH  =  RTC_CRH_SECIE;//разрешить прерывание от секундных импульсов
	  RTC->CRL &= ~RTC_CRL_CNF;  //выйти из режима конфигурирования
	  NVIC_EnableIRQ (RTC_IRQn);
	  return 0;

}
void Init_RCC(void)
{
	ErrorStatus HSEStartUpStatus;
    /*RCC system reset(for debug purpose) */
    RCC_DeInit();
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);
    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        /* PCLK2 = HCLK*/
        RCC_PCLK2Config(RCC_HCLK_Div1);
        /* PCLK1 = HCLK*/
        RCC_PCLK1Config(RCC_HCLK_Div1);
        //ADC CLK
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);
        /* PLLCLK = 8MHz * 3 = 24 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        /* Enable PLL */
        RCC_PLLCmd(ENABLE);
        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }

    /*Then need to enable peripheral clocks ----------------------------------------------*/
}
void SetSysClockTo72(void)
{

	ErrorStatus HSEStartUpStatus;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();
    /* Enable HSE */
    RCC_HSEConfig( RCC_HSE_ON);
    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);
        /* Flash 2 wait state */
        FLASH_SetLatency( FLASH_Latency_2);
        /* HCLK = SYSCLK */
        RCC_HCLKConfig( RCC_SYSCLK_Div1);
        /* PCLK2 = HCLK */
        RCC_PCLK2Config( RCC_HCLK_Div1);
        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config( RCC_HCLK_Div2);
        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(0x00010000, RCC_PLLMul_9);
        /* Enable PLL */
        RCC_PLLCmd( ENABLE);
        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }
        /* Select PLL as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);
        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock configuration.
     User can add here some code to deal with this error */
        /* Go to infinite loop */
        while (1)
        {
        }
    }
}
void Set_Led(u8 led, u8 state){
	if(state)
		Set_Leds(LedValue | (1<<led));
	else Set_Leds(LedValue & (~(1<<led)));
}
void Set_Leds(u16 value){
	LedValue = value;
	if(value & 0x0001) GPIOC->ODR |= GPIO_Pin_6; 	else GPIOC->ODR &= ~ GPIO_Pin_6;
	if(value & 0x0002) GPIOB->ODR |= GPIO_Pin_15; 	else GPIOB->ODR &= ~ GPIO_Pin_15;
	if(value & 0x0004) GPIOB->ODR |= GPIO_Pin_14; 	else GPIOB->ODR &= ~ GPIO_Pin_14;
	if(value & 0x0008) GPIOB->ODR |= GPIO_Pin_13; 	else GPIOB->ODR &= ~ GPIO_Pin_13;
	if(value & 0x0010) GPIOB->ODR |= GPIO_Pin_5; 	else GPIOB->ODR &= ~ GPIO_Pin_5;
	if(value & 0x0020) GPIOB->ODR |= GPIO_Pin_2; 	else GPIOB->ODR &= ~ GPIO_Pin_2;
	if(value & 0x0040) GPIOB->ODR |= GPIO_Pin_1; 	else GPIOB->ODR &= ~ GPIO_Pin_1;
	if(value & 0x0080) GPIOB->ODR |= GPIO_Pin_0; 	else GPIOB->ODR &= ~ GPIO_Pin_0;
}
 u16 GetDI(){
	 u16 di = 0;
	 if(!(GPIOC->IDR & GPIO_Pin_9)) di |= 0x0001;
	 if(!(GPIOA->IDR & GPIO_Pin_8)) di |= 0x0002;
	 if(!(GPIOC->IDR & GPIO_Pin_7)) di |= 0x0004;
	 if(!(GPIOC->IDR & GPIO_Pin_8)) di |= 0x0008;
	 return di;
 }
void Blink(u8 led){
	if(led>=8) return;
	Set_Led(led,1);
	blink_timer[led]=50;
}
void BlinkLong(u8 led){
	if(led>=8) return;
	Set_Led(led,1);
	blink_timer[led]=100;
}

void TIM2_IRQHandler(void)
{
	static u32 tick;
	u8 k;
	// 1мс
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
	{
	   TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	   tick++;
	   if(tick==1000){
		   EventFlags |= EVENT_F1S;
		   tick=0;
	   }
	   for(k=0;k<8;k++)
		   if(blink_timer[k])if(!(--blink_timer[k]))Set_Led(k,0);
	   UART_Timers_Execute();
	   MultiScanner_Timers();
	   Alg_Timer1ms();
	   if(firm_timer){
		   if(!(--firm_timer)){
			   BLStart = 0x12345678;
			   NVIC_SystemReset();
		   }
	   }
	   if(!(--flashconfwrite_timer)){
		   EventFlags |= EVENT_CHECKCONF;
		   flashconfwrite_timer = FLACHCONFCHECK_TIMER;
	   }
	   if(ds18b20_read_pause){
		   ds18b20_read_pause--;
	   }
	}
}
void RTC_IRQHandler(void)
{
	uint32_t Time=0;
    //причина прерывания - переполнение входного делителя (новая секунда)
   if(RTC->CRL & RTC_CRL_SECF)
     {
        RTC->CRL &= ~RTC_CRL_SECF;    //сбросить флаг (обязательно!!!)
        //выполняем какие-то действия
        //GlobalTime=RTC_GetCounter();
        GlobalTime=RTC_GetCounter();
     }
}
void InitTIM2(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//setting timer 2 interrupt to 200Hz ((72000000/720/500)s
	//setting timer 2 interrupt to 0,5Hz ((72000000/7200/20000)s
	//setting timer 2 interrupt to 1hz ((24000000/24000)*1000)s
	//setting timer 2 interrupt to 100hz ((72000000/720)*1000)s
	//setting timer 2 interrupt to 1000hz ((72000000/720)*100)s
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 720;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 100-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}
void InitIWDG (void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(0x0AFF);
    IWDG_ReloadCounter();
    IWDG_Enable();

}
void InitBKP(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	BKP_ClearFlag();
	if(BKP_ReadBackupRegister(BKPDataReg[0])!=0x1234){
		u8 k;
		for(k=0;k<BKP_DR_NUMBER;k++)BKP_WriteBackupRegister(BKPDataReg[k],0);
		BKP_WriteBackupRegister(BKPDataReg[0],0x1234);
	}
}
void Periph_BKPWrite(u8 addr, u16 value){
	BKP_WriteBackupRegister(BKPDataReg[addr+1],value);
}
u16 Periph_BKPRead(u8 addr){
	return BKP_ReadBackupRegister(BKPDataReg[addr+1]);
}
void Periph_BKPWriteLong(u8 addr, u32 value){
	BKP_WriteBackupRegister(BKPDataReg[addr+1],value & 0xFFFF);
	BKP_WriteBackupRegister(BKPDataReg[addr+2],value >>16);
}
u32 Periph_BKPReadLong(u8 addr){
	u32 res = BKP_ReadBackupRegister(BKPDataReg[addr+1]);
	res |= ((u32)BKP_ReadBackupRegister(BKPDataReg[addr+2]))<<16;
	return res;
}

void Init_Periph(){
	SetSysClockTo72();
	InitIWDG();
	Init_RTC();
	InitBKP();
	InitTIM2();
	Init_Leds();
	Init_DI();
	DS18b20_Init(2,DS18b20_addr[0]);
	DS18b20_Convert_T();
	ds18b20_read_pause = DS18B20_READ_PAUSE;
}
void ResetIWDG(){
	IWDG_ReloadCounter();
}
void PeriphExecute(){
	if(!ds18b20_read_pause){
		MyData.Term1 = DS18b20_Read_T(DS18b20_addr[0]);
		MyData.Term2 = DS18b20_Read_T(DS18b20_addr[1]);
		DS18b20_Convert_T();
		ds18b20_read_pause = DS18B20_READ_PAUSE;
	}
}
