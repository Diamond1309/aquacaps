#ifndef _I2C_EEPROM_H_
#define _I2C_EEPROM_H_
#include "comondef.h"
#include "stm32f10x_i2c.h"

#define I2C_EE             I2C1
#define I2C_EE_CLK         RCC_APB1Periph_I2C1
#define I2C_EE_GPIO        GPIOB
#define I2C_EE_GPIO_CLK    RCC_APB2Periph_GPIOB
#define I2C_EE_SCL         GPIO_Pin_6
#define I2C_EE_SDA         GPIO_Pin_7

#define I2C_Speed              1000
#define I2C_SLAVE_ADDRESS7     0xA0
#define I2C_FLASH_PAGESIZE     32
#define EEPROM_HW_ADDRESS      0xA0   /* E0 = E1 = E2 = 0 */
#define I2C_EE_ADDRESS_MASK		0x003FFFF

void I2C_EE_Init(void);
void I2C_EE_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint8_t NumByteToWrite);
void I2C_EE_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void I2C_EE_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t I2C_EE_GetArchIndex();
void I2C_EE_SetArchIndex(uint32_t v);
#endif
