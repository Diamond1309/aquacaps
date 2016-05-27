#ifndef PERIPH_H
#define PERIPH_H
#include "comondef.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_flash.h"
#include "onewire.h"

void Init_Periph();
void ResetIWDG();
void Init_Leds();
void Set_Leds(u16 value);
u16 GetDI();
void PeriphExecute();
void Blink(u8 led);
void BlinkLong(u8 led);
void Periph_BKPWrite(u8 addr, u16 value);
u16 Periph_BKPRead(u8 addr);
void Periph_BKPWriteLong(u8 addr, u32 value);
u32 Periph_BKPReadLong(u8 addr);

#define LED_DIDO_LINK	3
#define LED_MR1_LINK	1
#define LED_MR2_LINK	5
#define LED_1S			7

#endif
