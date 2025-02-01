//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// hal
//*******************************************************

/*
  Flashing ESP8285:
  - change ser dest to serial2
  - change ser baudrate to 115200
  - put Tx module into FLASH_ESP mode from tools
  - Board: Generic ESP8266 Module
  - Upload Speed: 115200
  - Reset Method: no dtr (aka ck)
*/

//-------------------------------------------------------
// ESP32, ELRS RADIOMASTER BANDIT MICRO 900 TX
//-------------------------------------------------------
/*
  "serial_rx": 13,
  "serial_tx": 13,
  "radio_dio0": 22,
  "radio_dio1": 21,
  "radio_miso": 19,
  "radio_mosi": 23,
  "radio_nss": 4,
  "radio_rst": 5,
  "radio_sck": 18,
  "radio_dcdc": true,       ???
  "radio_rfo_hf": true,
  "power_txen": 33,
  "power_apc2": 26,
  "power_min": 3,
  "power_high": 6,
  "power_max": 6,
  "power_default": 3,
  "power_control": 3,
  "power_values": [168,148,128,90],
  "power_values2": [2,6,9,12],
  "use_backpack": true,
  "debug_backpack_baud": 460800,
  "debug_backpack_rx": 16,
  "debug_backpack_tx": 17,
  "backpack_boot": 32,
  "backpack_en": 25,
  "passthrough_baud": 230400,
  "led_red": 15,
  "led_red_invert": true,
  "misc_fan_en": 2,
  "screen_type": 1,
  "screen_sck": 12,
  "screen_sda": 14,
  "screen_reversed": true,
  "joystick": 39,
  "joystick_values": [3227,0,1961,2668,1290,4095]
*/


#define DEVICE_HAS_JRPIN5
//#define DEVICE_HAS_IN
//#define DEVICE_HAS_SERIAL_OR_COM // board has UART which is shared between Serial or Com, selected by e.g. a switch
#define DEVICE_HAS_NO_SERIAL
#define DEVICE_HAS_NO_COM
//#define DEVICE_HAS_NO_DEBUG

#ifdef TX_ELRS_RADIOMASTER_BANDIT_900_ESP32
#define DEVICE_HAS_I2C_DISPLAY
#else
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_I2C_DISPLAY_ROT180
#endif

#define DEVICE_HAS_FAN_ONOFF // board has a Fan, which can be set on or off
#define DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL2 // board has an ESP8285 backpack with GPIO,RST, but no CONFIGURE for now


//-- UARTS
// UARTB = serial port
// UARTC or USB = COM (CLI)
// UARTD = serial2 BT/ESP port
// UART  = JR bay pin5
// UARTE = in port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_SERIAL // serial
#define UARTB_BAUD                TX_SERIAL_BAUDRATE
//#define UARTB_USE_TX_IO           IO_P17
//#define UARTB_USE_RX_IO           IO_P16
#define UARTB_TXBUFSIZE           1024 // TX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           TX_SERIAL_RXBUFSIZE

#define UARTC_USE_SERIAL // com USB/CLI
#define UARTC_BAUD                115200
#define UARTC_TXBUFSIZE           0 // ?? // TX_COM_TXBUFSIZE
#define UARTC_RXBUFSIZE           TX_COM_RXBUFSIZE

#define UARTD_USE_SERIAL2 // serial2 BT/ESP
#define UARTD_BAUD                115200
#define UARTD_USE_TX_IO           IO_P17
#define UARTD_USE_RX_IO           IO_P16
#define UARTD_TXBUFSIZE           1024 // TX_SERIAL_TXBUFSIZE
#define UARTD_RXBUFSIZE           TX_SERIAL_RXBUFSIZE

#define UART_USE_SERIAL1 // full duplex CRSF/MBridge (JR pin5)
#define UART_BAUD                 400000
#define UART_USE_TX_IO            IO_P13
#define UART_USE_RX_IO            IO_P13
#define UART_TXBUFSIZE            0 // 512 // 128 fifo should be sufficient
#define UART_RXBUFSIZE            0 // 512

#define UARTF_USE_SERIAL // debug is on RT
#define UARTF_BAUD                115200
#define UARTF_USE_TX_IO           IO_P1
#define UARTF_USE_RX_IO           -1
#define UARTF_TXBUFSIZE           512 // ?? // 512


//-- SX1: SX12xx & SPI

#define SPI_CS_IO                 IO_P4
#define SPI_MISO                  IO_P19
#define SPI_MOSI                  IO_P23
#define SPI_SCK                   IO_P18
#define SPI_FREQUENCY             10000000L
#define SX_RESET                  IO_P5
#define SX_DIO0                   IO_P22
#define SX_TX_EN                  IO_P33

#define SX_USE_RFO

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO0, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
}

IRAM_ATTR void sx_amp_transmit(void)
{
    gpio_high(SX_TX_EN);
}

IRAM_ATTR void sx_amp_receive(void)
{
    gpio_low(SX_TX_EN);
}

void sx_dio_enable_exti_isr(void)
{
    attachInterrupt(SX_DIO0, SX_DIO_EXTI_IRQHandler, RISING);
}

void sx_dio_init_exti_isroff(void)
{
    detachInterrupt(SX_DIO0);
}

void sx_dio_exti_isr_clearflag(void) {}


//-- Button

void button_init(void) {}
IRAM_ATTR bool button_pressed(void) { return false; }


//-- LEDs

#define LED_RED                   IO_P15

#ifdef TX_ELRS_RADIOMASTER_BANDIT_900_ESP32

#include <NeoPixelBus.h>
bool ledRedState;
bool ledGreenState;
bool ledBlueState;

uint8_t pixelNum = 6;

NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> ledRGB(pixelNum, LED_RED);

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
    ledRGB.SetPixelColor(1, RgbColor(0, 0, 0));
    ledRGB.Show();
    ledGreenState = 0;
}

IRAM_ATTR void led_green_on(void)
{
    if (ledGreenState) return;
    ledRGB.SetPixelColor(1, RgbColor(0, 255, 0));
    ledRGB.Show();
    ledGreenState = 1;
}

IRAM_ATTR void led_green_toggle(void)
{
    if (ledGreenState) { led_green_off(); } else { led_green_on(); }
}

// blue not yet used and not correctly working with red
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

#else

void leds_init(void)
{
    gpio_init(LED_RED, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR void led_red_off(void) { gpio_low(LED_RED); }
IRAM_ATTR void led_red_on(void) { gpio_high(LED_RED); }
IRAM_ATTR void led_red_toggle(void) { gpio_toggle(LED_RED); }

#endif  // TX_ELRS_RADIOMASTER_BANDIT_900_ESP32


//-- Display I2C

#define I2C_SDA_IO                IO_P14
#define I2C_SCL_IO                IO_P12
#define I2C_CLOCKSPEED            1000000L  // fix - rather too much, but helps with LQ, ESP32 max speed
#define I2C_BUFFER_SIZE           1024


//-- 5 Way Switch

#define FIVEWAY_ADC_IO            IO_P39
#define KEY_UP_THRESH             3230
#define KEY_DOWN_THRESH           0
#define KEY_LEFT_THRESH           1890
#define KEY_RIGHT_THRESH          2623
#define KEY_CENTER_THRESH         1205

#if defined DEVICE_HAS_I2C_DISPLAY || defined DEVICE_HAS_I2C_DISPLAY_ROT180 || defined DEVICE_HAS_FIVEWAY

void fiveway_init(void) {} // no init needed to read an analog pin in Arduino

IRAM_ATTR uint16_t fiveway_adc_read(void)
{
    return analogRead(FIVEWAY_ADC_IO);
}

IRAM_ATTR uint8_t fiveway_read(void)
{
    int16_t adc = analogRead(FIVEWAY_ADC_IO);
    if (adc > (KEY_CENTER_THRESH-250) && adc < (KEY_CENTER_THRESH+250)) return (1 << KEY_CENTER);
    if (adc > (KEY_LEFT_THRESH-250) && adc < (KEY_LEFT_THRESH+250)) return (1 << KEY_LEFT); 
    if (adc > (KEY_DOWN_THRESH-250) && adc < (KEY_DOWN_THRESH+250)) return (1 << KEY_DOWN);
    if (adc > (KEY_UP_THRESH-250) && adc < (KEY_UP_THRESH+250)) return (1 << KEY_UP);
    if (adc > (KEY_RIGHT_THRESH-250) && adc < (KEY_RIGHT_THRESH+250)) return (1 << KEY_RIGHT);
    return 0;
}
#endif


//-- Serial or Com Switch
// use com if FIVEWAY is DOWN during power up, else use serial

#ifdef DEVICE_HAS_SERIAL_OR_COM
bool tx_ser_or_com_serial = true; // we use serial as default

void ser_or_com_init(void)
{
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < 16; i++) {
        int16_t adc = analogRead(FIVEWAY_ADC_IO);
        if (adc > (KEY_DOWN_THRESH-200) && adc < (KEY_DOWN_THRESH+200)) cnt++;
    }
    tx_ser_or_com_serial = !(cnt > 8);
}

IRAM_ATTR bool ser_or_com_serial(void)
{
    return tx_ser_or_com_serial;
}

IRAM_ATTR void ser_or_com_set_to_com(void)
{
    tx_ser_or_com_serial = false;
}
#endif


//-- Cooling Fan

#define FAN_IO                    IO_P2

void fan_init(void)
{
    gpio_init(FAN_IO, IO_MODE_OUTPUT_PP_LOW);
    gpio_low(FAN_IO);
}

IRAM_ATTR void fan_set_power(int8_t power_dbm)
{
    if (power_dbm >= POWER_23_DBM) {
        gpio_high(FAN_IO);
    } else {
        gpio_low(FAN_IO);
    }
}


//-- ESP8285 Wifi Bridge

#define ESP_RESET                 IO_P25 // backpack_en
#define ESP_GPIO0                 IO_P32 // backpack_boot, seems to be inverted
//#define ESP_DTR                   IO_PC14 // DTR from USB-TTL adapter -> GPIO
//#define ESP_RTS                   IO_PC3  // RTS from USB-TTL adapter -> RESET

#ifdef DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL2
void esp_init(void)
{
    gpio_init(ESP_GPIO0, IO_MODE_OUTPUT_PP_LOW); // high -> esp will start in bootloader mode
    gpio_init(ESP_RESET, IO_MODE_OUTPUT_PP_LOW); // low -> esp is in reset
}

IRAM_ATTR void esp_reset_high(void) { gpio_high(ESP_RESET); }
IRAM_ATTR void esp_reset_low(void) { gpio_low(ESP_RESET); }

IRAM_ATTR void esp_gpio0_high(void) { gpio_low(ESP_GPIO0); }
IRAM_ATTR void esp_gpio0_low(void) { gpio_high(ESP_GPIO0); }

//IRAM_ATTR uint8_t esp_dtr_rts(void) { return 0; }
#endif


//-- POWER

#define POWER_GAIN_DBM            0 // 13 // gain of a PA stage if present
#define POWER_SX1276_MAX_DBM      SX1276_OUTPUT_POWER_MAX // maximum allowed sx power
//#define POWER_USE_DEFAULT_RFPOWER_CALC

void sx1276_rfpower_calc(const int8_t power_dbm, uint8_t* sx_power, int8_t* actual_power_dbm, const uint8_t GAIN_DBM, const uint8_t SX1276_MAX_DBM)
{
    // jr bay: 
    //   SX1276_MAX_POWER_15_DBM:   dac = 0,   sx_power = 15 => 30.5 dBm
    //                              dac = 0,   sx_power = 0  => 27.4 dBm
    //                              dac = 190, sx_power = 0  => -25 dBm !!!
    //                              dac = 180, sx_power = 0  => 18.8 dBm
    //                              dac = 150, sx_power = 0  => 24.7 dBm
    //                              dac = 100, sx_power = 0  => 27.3 dBm
    //   SX1276_MAX_POWER_10p8_DBM: dac = 0,   sx_power = 15 => 29.4 dBm
    //                              dac = 0,   sx_power = 0  => 21.0 dBm
    //                              dac = 180, sx_power = 0  => 10.7 dBm
    //   SX1276_MAX_POWER_11p4_DBM  dac = 0,   sx_power = 0  => 27.3 dBm
    //                              dac = 100, sx_power = 15 => 29.9 dBm
    //                              dac = 0,   sx_power = 15 => 30.0 dBm
    //                              dac = 180, sx_power = 0  => 10.1 dBm

    uint8_t dac = 100;

    if (power_dbm > 28) { // -> 30
        dac = 0;
        *sx_power = 15; // equals SX1276_OUTPUT_POWER_MAX
        *actual_power_dbm = 30;
    } else if (power_dbm > 25) { // -> 27
        dac = 140;
        *sx_power = 15;
        *actual_power_dbm = 27;
    } else if (power_dbm > 22) { // -> 24
        dac = 150;
        *sx_power = 6;
        *actual_power_dbm = 24;
    } else if (power_dbm > 18) { // -> 20
        dac = 150;
        *sx_power = 0;
        *actual_power_dbm = 20;
    } else {
        dac = 180;
        *sx_power = 0;
        *actual_power_dbm = 10;
    }

    dacWrite(IO_P26, dac);
    dacWrite(IO_P26, dac);
}

#define RFPOWER_DEFAULT           1 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_MIN, .mW = INT8_MIN },
    { .dbm = POWER_20_DBM, .mW = 100 },
    { .dbm = POWER_24_DBM, .mW = 250 },
    { .dbm = POWER_27_DBM, .mW = 500 },
    { .dbm = POWER_30_DBM, .mW = 1000 },
};

