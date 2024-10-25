//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// hal
//********************************************************

//-------------------------------------------------------
// ESP32, ELRS GENERIC 2400 True Diversity PA Receiver as TX, can only do SiK
//-------------------------------------------------------

#define DEVICE_HAS_SINGLE_LED_RGB
#define DEVICE_HAS_DIVERSITY_SINGLE_SPI // must be set, doesn't work without it
//#define DEVICE_HAS_JRPIN5
//#define DEVICE_HAS_IN
//#define DEVICE_HAS_NO_COM
#define DEVICE_HAS_NO_DEBUG
//#define DEVICE_HAS_NO_SERIAL

// #define USE_FEATURE_MAVLINK_PARAMS // has no CLI, no Lua, hence needs this


//-- UARTS
// UARTB = serial port
// UARTC or USB = COM (CLI)
// UARTD = -
// UART  = JR bay pin5
// UARTE = in port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_SERIAL
#define UARTB_BAUD                TX_SERIAL_BAUDRATE
#define UARTB_TXBUFSIZE           1024 // TX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           TX_SERIAL_RXBUFSIZE

#define UARTC_USE_SERIAL1
#define UARTC_BAUD                115200
#define UARTC_USE_TX_IO           18 
#define UARTC_USE_RX_IO           5
#define UARTC_TXBUFSIZE           0 // ?? // TX_COM_TXBUFSIZE
#define UARTC_RXBUFSIZE           TX_COM_RXBUFSIZE

#define UARTF_USE_SERIAL1
#define UARTF_BAUD                115200
#define UARTF_USE_TX_IO           18 
#define UARTF_USE_RX_IO           -1
#define UARTF_TXBUFSIZE           0 // ?? // 512


//-- SX1: SX12xx & SPI

#define SPI_CS_IO                 IO_P27
#define SPI_MISO                  IO_P33
#define SPI_MOSI                  IO_P32
#define SPI_SCK                   IO_P25
#define SPI_FREQUENCY             18000000L
#define SX_BUSY                   IO_P36
#define SX_DIO1                   IO_P37
#define SX_RESET                  IO_P26
#define SX_RX_EN                  IO_P10
#define SX_TX_EN                  IO_P14

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR bool sx_busy_read(void)
{
    return (gpio_read_activehigh(SX_BUSY)) ? true : false;
}

IRAM_ATTR void sx_amp_transmit(void)
{
    gpio_low(SX_RX_EN);
    gpio_high(SX_TX_EN);
}

IRAM_ATTR void sx_amp_receive(void)
{
    gpio_low(SX_TX_EN);
    gpio_high(SX_RX_EN);
}

void sx_dio_enable_exti_isr(void)
{
    attachInterrupt(SX_DIO1, SX_DIO_EXTI_IRQHandler, RISING);
}

void sx_dio_init_exti_isroff(void)
{
    detachInterrupt(SX_DIO1);
}

void sx_dio_exti_isr_clearflag(void) {}


//-- SX2: SX12xx & SPI

#define SX2_CS_IO                 IO_P13
#define SX2_BUSY                  IO_P39
#define SX2_DIO1                  IO_P34
#define SX2_RESET                 IO_P21
#define SX2_RX_EN                 IO_P9
#define SX2_TX_EN                 IO_P15

IRQHANDLER(void SX2_DIO_EXTI_IRQHandler(void);)

void sx2_init_gpio(void)
{
    gpio_init(SX2_CS_IO, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(SX2_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX2_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX2_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX2_RX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX2_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR void spib_select(void)
{
    gpio_low(SX2_CS_IO);
}

IRAM_ATTR void spib_deselect(void)
{
    gpio_high(SX2_CS_IO);
}

IRAM_ATTR bool sx2_busy_read(void)
{
    return (gpio_read_activehigh(SX2_BUSY)) ? true : false;
}

IRAM_ATTR void sx2_amp_transmit(void)
{
    gpio_low(SX2_RX_EN);
    gpio_high(SX2_TX_EN);
}

IRAM_ATTR void sx2_amp_receive(void)
{
    gpio_low(SX2_TX_EN);
    gpio_high(SX2_RX_EN);
}

void sx2_dio_init_exti_isroff(void)
{
    detachInterrupt(SX2_DIO1);
}

void sx2_dio_enable_exti_isr(void)
{
    attachInterrupt(SX2_DIO1, SX2_DIO_EXTI_IRQHandler, RISING);
}

void sx2_dio_exti_isr_clearflag(void) {}


//-- Button

#define BUTTON                    IO_P0

void button_init(void)
{
    gpio_init(BUTTON, IO_MODE_INPUT_PU);
}

IRAM_ATTR bool button_pressed(void)
{
    return gpio_read_activelow(BUTTON) ? true : false;
}


//-- LEDs
#include <NeoPixelBus.h>
#define LED_RED                    IO_P22
bool ledRedState;
bool ledGreenState;
bool ledBlueState;

NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> ledRGB(1, LED_RED);

void leds_init(void)
{
    ledRGB.Begin();
    ledRGB.Show();
}

IRAM_ATTR void led_red_off(void)
{
    if (!ledRedState) return;
    ledRGB.SetPixelColor(0, RgbColor(0, 0, 0));
    ledRGB.Show();
    ledRedState = 0;
}

IRAM_ATTR void led_red_on(void)
{
    if (ledRedState) return;
    ledRGB.SetPixelColor(0, RgbColor(255, 0, 0));
    ledRGB.Show();
    ledRedState = 1;
}

IRAM_ATTR void led_red_toggle(void)
{
    if (ledRedState) { led_red_off(); } else { led_red_on(); }
}

IRAM_ATTR void led_green_off(void)
{
    if (!ledGreenState) return;
    ledRGB.SetPixelColor(0, RgbColor(0, 0, 0));
    ledRGB.Show();
    ledGreenState = 0;
}

IRAM_ATTR void led_green_on(void)
{
    if (ledGreenState) return;
    ledRGB.SetPixelColor(0, RgbColor(0, 255, 0));
    ledRGB.Show();
    ledGreenState = 1;
}

IRAM_ATTR void led_green_toggle(void)
{
    if (ledGreenState) { led_green_off(); } else { led_green_on(); }
}

IRAM_ATTR void led_blue_off(void)
{
    if (!ledBlueState) return;
    ledRGB.SetPixelColor(0, RgbColor(0, 0, 0));
    ledRGB.Show();
    ledBlueState = 0;
}

IRAM_ATTR void led_blue_on(void)
{
    if (ledBlueState) return;
    ledRGB.SetPixelColor(0, RgbColor(0, 0, 255));
    ledRGB.Show();
    ledBlueState = 1;
}

IRAM_ATTR void led_blue_toggle(void)
{
    if (ledBlueState) { led_blue_off(); } else { led_blue_on(); }
}


//-- POWER

#define POWER_GAIN_DBM            18 // gain of a PA stage if present
#define POWER_SX1280_MAX_DBM      SX1280_POWER_3_DBM  // maximum allowed sx power
#define POWER_USE_DEFAULT_RFPOWER_CALC

#define RFPOWER_DEFAULT           1 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_0_DBM, .mW =  1 },
    { .dbm = POWER_10_DBM, .mW = 10 },
    { .dbm = POWER_20_DBM, .mW = 100 },
};
