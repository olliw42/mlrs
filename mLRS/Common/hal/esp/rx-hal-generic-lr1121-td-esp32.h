//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// hal
//********************************************************

//-------------------------------------------------------
// ESP32, ELRS GENERIC LR1121 True Diversity Receiver
//-------------------------------------------------------

#define DEVICE_HAS_SINGLE_LED_RGB
#define DEVICE_HAS_DIVERSITY_SINGLE_SPI // must be set, doesn't work without it
#define DEVICE_HAS_NO_DEBUG
//#define DEVICE_HAS_SERIAL_OR_DEBUG


//-- UARTS
// UARTB = serial port
// UART = output port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_SERIAL
#define UARTB_BAUD                RX_SERIAL_BAUDRATE
#define UARTB_TXBUFSIZE           RX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           RX_SERIAL_RXBUFSIZE

#define UARTF_USE_SERIAL
#define UARTF_BAUD                115200


//-- SX1: LR11xx & SPI

#define SPI_CS_IO                 IO_P27
#define SPI_MISO                  IO_P33
#define SPI_MOSI                  IO_P32
#define SPI_SCK                   IO_P25
#define SPI_FREQUENCY             16000000L  // 16 MHz max per datasheet
#define SX_BUSY                   IO_P36
#define SX_DIO1                   IO_P37
#define SX_RESET                  IO_P26

#define SX_USE_REGULATOR_MODE_DCDC

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_BUSY, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

IRAM_ATTR void sx_amp_transmit(void) {}

IRAM_ATTR void sx_amp_receive(void) {}

void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }

void sx_dio_enable_exti_isr(void) { attachInterrupt(SX_DIO1, SX_DIO_EXTI_IRQHandler, RISING); }

IRAM_ATTR void sx_dio_exti_isr_clearflag(void) {}


//-- SX2: LR11xx & SPI

#define SX2_CS_IO                 IO_P13
#define SX2_BUSY                  IO_P39
#define SX2_DIO1                  IO_P34
#define SX2_RESET                 IO_P21

#define SX2_USE_REGULATOR_MODE_DCDC

IRQHANDLER(void SX2_DIO_EXTI_IRQHandler(void);)

void sx2_init_gpio(void)
{
    gpio_init(SX2_CS_IO, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(SX2_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX2_BUSY, IO_MODE_INPUT_ANALOG);
    gpio_init(SX2_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR void spib_select(void) {  gpio_low(SX2_CS_IO); }

IRAM_ATTR void spib_deselect(void) { gpio_high(SX2_CS_IO); }

IRAM_ATTR bool sx2_busy_read(void) { return (gpio_read_activehigh(SX2_BUSY)) ? true : false; }

IRAM_ATTR void sx2_amp_transmit(void) {}

IRAM_ATTR void sx2_amp_receive(void) {}

void sx2_dio_init_exti_isroff(void) { detachInterrupt(SX2_DIO1); }

void sx2_dio_enable_exti_isr(void) { attachInterrupt(SX2_DIO1, SX2_DIO_EXTI_IRQHandler, RISING); }

void sx2_dio_exti_isr_clearflag(void) {}


//-- Button

#define BUTTON                    IO_P0

void button_init(void) { gpio_init(BUTTON, IO_MODE_INPUT_PU); }

IRAM_ATTR bool button_pressed(void) { return gpio_read_activelow(BUTTON) ? true : false; }


//-- LEDs

#include "../../esp-lib/esp-rgb.h"

#define LED_PIN                    IO_P22
#define LED_COUNT                  1

static ESP32LedDriver *ledRGB;

void leds_init(void) { ledRGB = new ESP32LedDriver(LED_COUNT, LED_PIN); }

IRAM_ATTR void led_red_off(void) { ledRGB->redOff(0,0); }
IRAM_ATTR void led_red_on(void) { ledRGB->redOn(0,0); }
IRAM_ATTR void led_red_toggle(void) { ledRGB->redToggle(0,0); }

IRAM_ATTR void led_green_off(void) { ledRGB->greenOff(0,0); }
IRAM_ATTR void led_green_on(void) { ledRGB->greenOn(0,0); }
IRAM_ATTR void led_green_toggle(void) { ledRGB->greenToggle(0,0); }

IRAM_ATTR void led_blue_off(void) { ledRGB->blueOff(0,0); }
IRAM_ATTR void led_blue_on(void) { ledRGB->blueOn(0,0); }
IRAM_ATTR void led_blue_toggle(void) { ledRGB->blueToggle(0,0); }


//-- POWER
#ifndef POWER_OVERLAY

#define POWER_GAIN_DBM            -2 // gain of a PA stage if present
#define POWER_LR11XX_MAX_DBM      LR11XX_POWER_MAX // maximum allowed sx power
#define POWER_USE_DEFAULT_RFPOWER_CALC

#define RFPOWER_DEFAULT           0 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_0_DBM, .mW = 1 },
    { .dbm = POWER_10_DBM, .mW = 10 },
    { .dbm = POWER_14_DBM, .mW = 25 },
    { .dbm = POWER_20_DBM, .mW = 100 },
};

#endif // !POWER_OVERLAY
