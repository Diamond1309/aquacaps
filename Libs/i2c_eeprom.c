#include "i2c_eeprom.h"
void delay(uint32_t value){
	for(;value;value--);
}
void delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
        RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq (&RCC_Clocks);

        nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
        for (; nCount!=0; nCount--)IWDG_ReloadCounter();
}
void I2C_EE_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(I2C_EE_CLK,ENABLE);
    RCC_APB2PeriphClockCmd(I2C_EE_GPIO_CLK,ENABLE);
    /* Configure I2C_EE pins: SCL and SDA */
    GPIO_InitStructure.GPIO_Pin =  I2C_EE_SCL | I2C_EE_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(I2C_EE_GPIO, &GPIO_InitStructure);


	I2C_InitTypeDef  I2C_InitStructure;

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;

    I2C_DeInit(I2C_EE);
    I2C_Cmd(I2C_EE, ENABLE);
    I2C_Init(I2C_EE, &I2C_InitStructure);
}
void TestI2CAccess()
{
   if(!(I2C_GetFlagStatus(I2C_EE, I2C_FLAG_BUSY))) // провер€ю, зан€та ли шина I2C (взведЄн ли флаг BUSY)
      return; // если нет - выходим
   // а если взведЄн - рулю ситуацию
   GPIOB->BSRR |= I2C_EE_SDA | I2C_EE_SCL; // выставл€ю SDA и SCL в 1
   GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin =  I2C_EE_SDA;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(I2C_EE_GPIO, &GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Pin =  I2C_EE_SCL;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(I2C_EE_GPIO, &GPIO_InitStructure);

   while(1) // здесь мы можем быть максимум 9 раз (крайний случай, смотри в pdf)
   {// дЄргаю SCL (тактирую Slave)
      GPIOB->BRR |= I2C_EE_SCL; // SCL = 0
      delay_ms(1);
      GPIOB->BSRR |= I2C_EE_SCL; // SCL = 1
      delay_ms(1);
      if(GPIOB->IDR & I2C_EE_SDA) // смотрю, отпустил ли Slave SDA (SDA == 1 ?)
      {// если да - настраиваю выводы на выход и делаю Stop состо€ние
         GPIOB->BRR |= I2C_EE_SCL; // SCL = 0
         delay_ms(1);
         GPIOB->BRR |= I2C_EE_SDA; // SDA = 0
         delay_ms(1);

         GPIO_InitStructure.GPIO_Pin =  I2C_EE_SDA;
         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
         GPIO_Init(I2C_EE_GPIO, &GPIO_InitStructure);

         GPIOB->BSRR |= I2C_EE_SCL; // SCL = 1
         delay_ms(1);
         GPIOB->BSRR |= I2C_EE_SDA; // SDA = 1
         break; // выходим из цикла
      }
   }
   // возвращаю настройки порта (аппаратный I2C)
   I2C_EE_Init();
   delay(1);
   // после этого шина I2C свободна и готова к работе
}
/* ќжидание StandbyState
 */
void I2C_EE_WaitEepromStandbyState(void)
{
    __IO uint16_t SR1_Tmp = 0;

    do
    {
        /* Send START condition */
        I2C_GenerateSTART(I2C_EE, ENABLE);

        /* Read I2C_EE SR1 register to clear pending flags */
        SR1_Tmp = I2C_ReadRegister(I2C_EE, I2C_Register_SR1);

        /* Send EEPROM address for write */
        I2C_Send7bitAddress(I2C_EE, I2C_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
    }while(!(I2C_ReadRegister(I2C_EE, I2C_Register_SR1) & 0x0002));

    /* Clear AF flag */
    I2C_ClearFlag(I2C_EE, I2C_FLAG_AF);

    /* STOP condition */
    I2C_GenerateSTOP(I2C_EE, ENABLE);
}

/* «апись буфера в EEPROM, при этом кол-во данных должно быть меньше
 * размера страницы и не должно быть перехода на другую страницу
 */
void I2C_EE_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint8_t NumByteToWrite)
{
	WriteAddr &= I2C_EE_ADDRESS_MASK;
	TestI2CAccess();
	while(I2C_GetFlagStatus(I2C_EE, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C_EE, ENABLE);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C_EE, I2C_SLAVE_ADDRESS7 |((WriteAddr&0xFF0000)>>15), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C_EE, (uint8_t)((WriteAddr & 0xFF00) >> 8));
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C_EE, (uint8_t)(WriteAddr & 0x00FF));
    while(! I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    while(NumByteToWrite--){
        I2C_SendData(I2C_EE, *pBuffer);
        pBuffer++;
        while (!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    I2C_GenerateSTOP(I2C_EE, ENABLE);
}
/* «апись буфера в EEPROM.
 * функци€ сама разбирает по страницам
 */
void I2C_EE_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, count = 0;
    uint16_t Addr = 0;
    WriteAddr &= I2C_EE_ADDRESS_MASK;
    TestI2CAccess();
    Addr = WriteAddr % I2C_FLASH_PAGESIZE;
    count = I2C_FLASH_PAGESIZE - Addr;
    NumOfPage =  NumByteToWrite / I2C_FLASH_PAGESIZE;
    NumOfSingle = NumByteToWrite % I2C_FLASH_PAGESIZE;

    /* если выровнено по страницам  */
    if(Addr == 0)
    {
        /* если данные умещаютс€ в одну страницу */
        if(NumOfPage == 0)
        {
            I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            I2C_EE_WaitEepromStandbyState();
        }
        /* если данные не умещаютс€ в одной странице */
        else
        {
            while(NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_FLASH_PAGESIZE);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr +=  I2C_FLASH_PAGESIZE;
                pBuffer += I2C_FLASH_PAGESIZE;
            }

            if(NumOfSingle!=0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }
    /* если не выровнено по страницам  */
    else
    {
        /* если данные умещаютс€ на одной странце */
        if(NumOfPage== 0)
        {
            /* ≈сли данные переход€т на другую страницу */
            if (NumByteToWrite > count)
            {
                /* пишем в текущую страницу */
                I2C_EE_PageWrite(pBuffer, WriteAddr, count);
                I2C_EE_WaitEepromStandbyState();

                /* оставшиес€с данные пишем на следующую страницу */
                I2C_EE_PageWrite((uint8_t*)(pBuffer + count), (WriteAddr + count), (NumByteToWrite - count));
                I2C_EE_WaitEepromStandbyState();
            }
            /* если данные пишутс€ только в эту страницу */
            else
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
        /* если данные не умещаютс€ в одну страницу */
        else
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / I2C_FLASH_PAGESIZE;
            NumOfSingle = NumByteToWrite % I2C_FLASH_PAGESIZE;
            /* пишем данные на первую страницу */
            if(count != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, count);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr += count;
                pBuffer += count;
            }
            /* пишем данные на следющие страницы с заполнением до конца */
            while(NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_FLASH_PAGESIZE);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr +=  I2C_FLASH_PAGESIZE;
                pBuffer += I2C_FLASH_PAGESIZE;
            }
            /* пишем в последнюю страницу */
            if(NumOfSingle != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }
}
void I2C_EE_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	ReadAddr &= I2C_EE_ADDRESS_MASK;
	TestI2CAccess();
	while(I2C_GetFlagStatus(I2C_EE, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C_EE, ENABLE);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C_EE, I2C_SLAVE_ADDRESS7|((ReadAddr&0xFF0000)>>15), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C_EE, (uint8_t)((ReadAddr & 0xFF00) >> 8));
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C_EE, (uint8_t)(ReadAddr & 0x00FF));
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C_EE, ENABLE);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C_EE, I2C_SLAVE_ADDRESS7|((ReadAddr&0xFF0000)>>15), I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while(NumByteToRead){
        if(NumByteToRead == 1){
            I2C_AcknowledgeConfig(I2C_EE, DISABLE);
            I2C_GenerateSTOP(I2C_EE, ENABLE);
        }

        if(I2C_CheckEvent(I2C_EE, I2C_EVENT_MASTER_BYTE_RECEIVED)){
            *pBuffer = I2C_ReceiveData(I2C_EE);
            pBuffer++;
            NumByteToRead--;
        }
    }
    I2C_AcknowledgeConfig(I2C_EE, ENABLE);
}

uint32_t I2C_EE_GetArchIndex(){
	uint32_t v;
	v = BKP_ReadWord(2);
	v <<=16;
	v |= BKP_ReadWord(1);
	return v;
}
void I2C_EE_SetArchIndex(uint32_t v){
	BKP_WriteWord(2, v>>16);
	BKP_WriteWord(1, v);
}
