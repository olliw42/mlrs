//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// hal
//*******************************************************

/*
  Flashing ESP8285 backpack WiFi bridge:
  - Board: Generic ESP8266 Module, define LED_IO 16
  - First flash can be via web browser/ELRS WiFi
  - Need ELRS version of esptool from https://github.com/ExpressLRS/Backpack
  - Power up radio, plug in USB, select VCP
  - On Linux, run something like "Backpack/python/external/esptool/esptool.py --passthrough --port /dev/ttyACM0 --baud 115200 --before etx --after hard_reset write_flash 0x0000 ~/Arduino/build/mlrs-wireless-bridge-esp8266.ino.bin

  Flashing ESP32 module:
  - Need ELRS python folder from https://github.com/ExpressLRS/ExpressLRS
  - Power up radio, plug in USB, select VCP
  - On Linux, run something like "python ExpressLRS/src/python/ETXinitPassthrough.py"
  - Use Visual Studio Code or esptool (or maybe web flasher?) to flash via ACM serial port.

  Set the Internal RF to CRSF 400k baud in SYS->HARDWARE on EdgeTx radio
*/

//-------------------------------------------------------
// ESP32, Radiomaster Tx Internal ELRS 2.4GHz, good for TX16S, TX12, MT12, Zorro, Pocket
//-------------------------------------------------------
/*
    Pin Defs
    "serial_rx": 3,
    "serial_tx": 1,
    "radio_busy": 21,
    "radio_dio1": 4,
    "radio_dio2": 22,
    "radio_miso": 19,
    "radio_mosi": 23,
    "radio_nss": 5,
    "radio_rst": 14,
    "radio_sck": 18,
    "radio_dcdc": true,
    "power_rxen": 27,
    "power_txen": 26,
    "power_lna_gain": 12,
    "power_min": 0,
    "power_high": 4,
    "power_max": 4,
    "power_default": 2,
    "power_pdet": 35,
    "power_pdet_intercept": 1.5,
    "power_pdet_slope": 0.032,
    "power_control": 0,
    "power_values": [-17,-13,-9,-6,-2],
    "use_backpack": true,
    "debug_backpack_baud": 460800,
    "debug_backpack_rx": 16,
    "debug_backpack_tx": 17,
    "backpack_boot": 15,
    "backpack_en": 25
*/


#define DEVICE_HAS_JRPIN5
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_NO_COM
//#define DEVICE_HAS_NO_DEBUG
#define DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL


//-- UARTS
// UARTB = serial port BT/ESP port
// UARTC = COM (CLI)
// UARTD = serial2
// UART  = JR bay pin5, full duplex CRSF serial connection to radio - code still calls it JR bay pin5
// UARTE = in port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_SERIAL // serial, is on P16/P17
#define UARTB_BAUD                TX_SERIAL_BAUDRATE
#define UARTB_USE_TX_IO           IO_P17
#define UARTB_USE_RX_IO           IO_P16
#define UARTB_TXBUFSIZE           TX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           TX_SERIAL_RXBUFSIZE

#define UART_USE_SERIAL1 // full duplex CRSF/MBridge (JR pin5)
#define UART_BAUD                 400000
#define UART_USE_TX_IO            IO_P1
#define UART_USE_RX_IO            IO_P3
#define UART_TXBUFSIZE            0 // TX_SERIAL_TXBUFSIZE
#define UART_RXBUFSIZE            0 // TX_SERIAL_RXBUFSIZE // 512

#define JR_PIN5_FULL_DUPLEX       // Internal module

#define UARTF_USE_SERIAL2 // debug
#define UARTF_BAUD                115200
#define UARTF_USE_TX_IO           IO_P33 // Or choose another unused pin/test pad
#define UARTF_USE_RX_IO           -1
#define UARTF_TXBUFSIZE           512


//-- SX1: SX12xx & SPI

#define SPI_CS_IO                 IO_P5
#define SPI_MISO                  IO_P19
#define SPI_MOSI                  IO_P23
#define SPI_SCK                   IO_P18
#define SPI_FREQUENCY             18000000L
#define SX_RESET                  IO_P14
#define SX_BUSY                   IO_P21
#define SX_DIO1                   IO_P4
#define SX_TX_EN                  IO_P26
#define SX_RX_EN                  IO_P27

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
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


//-- Button
// Not normally used since this is inside the radio

//#define BUTTON                    IO_P0

void button_init(void)
{
    //gpio_init(BUTTON, IO_MODE_INPUT_PU);
}

IRAM_ATTR bool button_pressed(void)
{
    //return gpio_read_activelow(BUTTON) ? true : false;
    return false;
}

//-- LEDs

#define LED_RED                   IO_P13

void leds_init(void)
{
    gpio_init(LED_RED, IO_MODE_OUTPUT_PP_HIGH);
}

IRAM_ATTR void led_red_off(void) { gpio_low(LED_RED); }
IRAM_ATTR void led_red_on(void) { gpio_high(LED_RED); }
IRAM_ATTR void led_red_toggle(void) { gpio_toggle(LED_RED); }


//-- ESP32 Wifi Bridge

#ifdef DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL

#define ESP_RESET                 IO_P25 // backpack_en
#define ESP_GPIO0                 IO_P15 // backpack_boot inverted?
#define ESP_BOOT0                 IO_P0 // Will always be IO_P0

uint8_t esp_boot0()
{
    return gpio_read_activelow(ESP_BOOT0);
}


void esp_init(void)
{
    // No need to configure ESP_BOOT0 which will always be IO_P0 and is pull-up by default
    gpio_init(ESP_GPIO0, IO_MODE_OUTPUT_PP_LOW); // high -> esp will start in bootloader mode
    gpio_init(ESP_RESET, IO_MODE_OUTPUT_PP_LOW); // low -> esp is in reset
}

IRAM_ATTR void esp_reset_high(void) { gpio_high(ESP_RESET); }
IRAM_ATTR void esp_reset_low(void) { gpio_low(ESP_RESET); }

IRAM_ATTR void esp_gpio0_high(void) { gpio_low(ESP_GPIO0); }
IRAM_ATTR void esp_gpio0_low(void) { gpio_high(ESP_GPIO0); }
#endif

//-- POWER
// Need to confirm all of this!
#define POWER_GAIN_DBM            26 // gain of a PA stage if present
#define POWER_SX1280_MAX_DBM      SX1280_POWER_0_DBM  // maximum allowed sx power
#define POWER_USE_DEFAULT_RFPOWER_CALC

#define RFPOWER_DEFAULT           0 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_10_DBM, .mW = 10 },
    { .dbm = POWER_17_DBM, .mW = 50 },
    { .dbm = POWER_20_DBM, .mW = 100 },
    { .dbm = POWER_24_DBM, .mW = 250 },
};

