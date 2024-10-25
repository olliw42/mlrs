//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// glue
//*******************************************************


// for as long as issues with gcc12 are not sorted
#if __GNUC__ > 11
  #error Must be gnu gcc 11 or lower!
#endif


//-------------------------------------------------------
// what we'll use anyway
//-------------------------------------------------------

#include <inttypes.h>
#include <string.h>


//-------------------------------------------------------
// STm32Cube LL & HAL
//-------------------------------------------------------
// defines like STM32F1 etc are not yet defined, so we need to be explicit

#ifdef STM32F103xB

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#endif
#if defined STM32F303xC || defined STM32F373xC

#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_rcc.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_tim.h"
#include "stm32f3xx_ll_usart.h"
#include "stm32f3xx_ll_spi.h"
#include "stm32f3xx_ll_system.h"
#include "stm32f3xx_ll_exti.h"
#include "stm32f3xx_ll_dac.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_flash.h"
#include "stm32f3xx_hal_flash_ex.h"

#endif
#if defined STM32G431xx ||defined STM32G441xx || defined STM32G491xx || defined STM32G474xx

#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_lpuart.h"
#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_flash.h"
#include "stm32g4xx_hal_flash_ex.h"

#endif
#ifdef STM32L433xx

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_adc.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"

#endif
#ifdef STM32WLE5xx

#include "stm32wlxx_ll_cortex.h"
#include "stm32wlxx_ll_bus.h"
#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_gpio.h"
#include "stm32wlxx_ll_tim.h"
#include "stm32wlxx_ll_usart.h"
#include "stm32wlxx_ll_spi.h"
#include "stm32wlxx_ll_system.h"
#include "stm32wlxx_ll_exti.h"
#include "stm32wlxx_ll_lpuart.h"
#include "stm32wlxx_ll_adc.h"
#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_flash.h"
#include "stm32wlxx_hal_flash_ex.h"

#endif
#if defined STM32F070xB || defined  STM32F072xB

#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_spi.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash.h"
#include "stm32f0xx_hal_flash_ex.h"

#endif


extern "C" {
void i2c_spin(uint16_t chunksize) {}
}


// setup(), loop() streamlining between Arduino/STM code
uint8_t restart_controller = 0;
void main_loop(void);
int main_main(void) { while(1) main_loop(); }

#define INITCONTROLLER_ONCE \
    if(restart_controller <= 1){ \
    if(restart_controller == 0){
#define RESTARTCONTROLLER \
    }
#define INITCONTROLLER_END \
    restart_controller = UINT8_MAX; \
    }
#define GOTO_RESTARTCONTROLLER \
    restart_controller = 1; \
    return;
