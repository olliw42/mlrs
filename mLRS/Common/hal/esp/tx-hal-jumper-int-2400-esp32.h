//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// hal
//*******************************************************

//-------------------------------------------------------
// ESP32, Jumper Tx Internal ELRS, good for T20, T20 V2, T15, T14, T-Pro S
// Backpack: Generic ESP8266 Module, define LED_IO 16
//-------------------------------------------------------

// https://github.com/ExpressLRS/targets/blob/master/TX/Jumper%20T-20%202400.json
// Added fan on GPIO 33

#define DEVICE_HAS_JRPIN5
#define DEVICE_HAS_NO_LED
#define DEVICE_HAS_NO_COM
#define DEVICE_HAS_NO_DEBUG
#define DEVICE_HAS_FAN_ONOFF
#define DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL


//-- UARTS
// UARTB = serial port BT/ESP port
// UARTC = COM (CLI)
// UARTD = serial2
// UART  = JR bay pin5, full duplex CRSF serial connection to radio - code still calls it JR bay pin5
// UARTE = in port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_SERIAL // serial, is on P12/P13
#define UARTB_BAUD                TX_SERIAL_BAUDRATE
#define UARTB_USE_TX_IO           IO_P12
#define UARTB_USE_RX_IO           IO_P13
#define UARTB_TXBUFSIZE           TX_SERIAL_TXBUFSIZE // 512
#define UARTB_RXBUFSIZE           TX_SERIAL_RXBUFSIZE // 2048

#define UART_USE_SERIAL1 // full duplex CRSF/MBridge (JR pin5)
#define UART_BAUD                 400000
#define UART_USE_TX_IO            IO_P1
#define UART_USE_RX_IO            IO_P3
#define UART_TXBUFSIZE            0 // TX_SERIAL_TXBUFSIZE  // 512
#define UART_RXBUFSIZE            0 // TX_SERIAL_RXBUFSIZE  // 2048

#define JR_PIN5_FULL_DUPLEX       // Internal module

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
#define PWR_EN                    IO_P25

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(PWR_EN, IO_MODE_OUTPUT_PP_HIGH);  // front-end enable
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

#define BUTTON                    -1

void button_init(void) {}

IRAM_ATTR bool button_pressed(void) { return false; }


//-- LEDs
//--


//-- Cooling Fan

#define FAN_IO                    IO_P33

void fan_init(void)
{
    gpio_init(FAN_IO, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR void fan_set_power(int8_t power_dbm)
{
    if (power_dbm >= POWER_23_DBM) {
        gpio_high(FAN_IO);
    } else {
        gpio_low(FAN_IO);
    }
}


//-- ESP32 Wifi Bridge

#ifdef DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL

#define ESP_RESET                 IO_P15 // backpack_en
#define ESP_GPIO0                 IO_P2  // backpack_boot inverted?
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
#define POWER_GAIN_DBM            28 // gain of a PA stage if present
#define POWER_SX1280_MAX_DBM      SX1280_POWER_3_DBM  // maximum allowed sx power
#define POWER_USE_DEFAULT_RFPOWER_CALC

#define RFPOWER_DEFAULT           0 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_17_DBM, .mW = 50 },
    { .dbm = POWER_20_DBM, .mW = 100 },
    { .dbm = POWER_24_DBM, .mW = 250 },
    { .dbm = POWER_27_DBM, .mW = 500 },
    { .dbm = POWER_30_DBM, .mW = 1000 },
};

