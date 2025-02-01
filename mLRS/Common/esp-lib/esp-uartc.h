//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// ESP UARTC
//********************************************************
// For ESP32:
// usefull resource
// - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
//********************************************************
#ifndef ESPLIB_UARTC_H
#define ESPLIB_UARTC_H


//-------------------------------------------------------
// Enums
//-------------------------------------------------------
#ifndef ESPLIB_UART_ENUMS
#define ESPLIB_UART_ENUMS

typedef enum {
    XUART_PARITY_NO = 0,
    XUART_PARITY_EVEN,
    XUART_PARITY_ODD,
} UARTPARITYENUM;

typedef enum {
//    UART_STOPBIT_0_5 = 0, // not supported by ESP
    UART_STOPBIT_1 = 0,
    UART_STOPBIT_2,
} UARTSTOPBITENUM;

#endif


//-------------------------------------------------------
// Defines
//-------------------------------------------------------

#ifdef UARTC_USE_SERIAL
  #define UARTC_SERIAL_NO       Serial
#elif defined UARTC_USE_SERIAL1
  #define UARTC_SERIAL_NO       Serial1
#elif defined UARTC_USE_SERIAL2
  #define UARTC_SERIAL_NO       Serial2
#else
  #error UARTC_SERIAL_NO must be defined!
#endif

#ifndef UARTC_TXBUFSIZE
  #define UARTC_TXBUFSIZE       256 // MUST be 2^N
#endif
#ifndef UARTC_RXBUFSIZE
  #define UARTC_RXBUFSIZE       256 // MUST be 2^N
#endif

#ifdef ESP32
  #if (UARTC_TXBUFSIZE > 0) && (UARTC_TXBUFSIZE < 256)
    #error UARTC_TXBUFSIZE must be 0 or >= 256
  #endif
  #if (UARTC_RXBUFSIZE > 0) && (UARTC_RXBUFSIZE < 256)
    #error UARTC_TXBUFSIZE must be 0 or >= 256
  #endif
#endif


//-------------------------------------------------------
// TX routines
//-------------------------------------------------------

IRAM_ATTR void uartc_putbuf(uint8_t* buf, uint16_t len)
{
    UARTC_SERIAL_NO.write((uint8_t*)buf, len);
}


IRAM_ATTR uint16_t uartc_tx_notfull(void)
{
    return 1; // fifo not full
}


IRAM_ATTR void uartc_tx_flush(void)
{
#ifdef ESP32
    // flush of tx buffer not available
#elif defined ESP8266
    UARTC_SERIAL_NO.flush();
#endif
}


//-------------------------------------------------------
// RX routines
//-------------------------------------------------------

IRAM_ATTR char uartc_getc(void)
{
    return (char)UARTC_SERIAL_NO.read();
}


IRAM_ATTR void uartc_rx_flush(void)
{
    while (UARTC_SERIAL_NO.available() > 0) UARTC_SERIAL_NO.read();
}


IRAM_ATTR uint16_t uartc_rx_bytesavailable(void)
{
    return (UARTC_SERIAL_NO.available() > 0) ? UARTC_SERIAL_NO.available() : 0;
}


IRAM_ATTR uint16_t uartc_rx_available(void)
{
    return (UARTC_SERIAL_NO.available() > 0) ? 1 : 0;
}


//-------------------------------------------------------
// INIT routines
//-------------------------------------------------------
// Note: ESP32 has a hardware fifo for tx, which is 128 bytes in size. However, MAVLink messages
// can be larger than this, and data would thus be lost when put only into the fifo. It is therefore
// crucial to set a Tx buffer size of sufficient size. setTxBufferSize() is not available for ESP82xx.

void _uartc_initit(uint32_t baud, UARTPARITYENUM parity, UARTSTOPBITENUM stopbits)
{
#ifdef ESP32
    UARTC_SERIAL_NO.setTxBufferSize(UARTC_TXBUFSIZE);
    UARTC_SERIAL_NO.setRxBufferSize(UARTC_RXBUFSIZE);

    uint32_t config = SERIAL_8N1;
    switch (parity) {
        case XUART_PARITY_NO:
            switch (stopbits) {
                case UART_STOPBIT_1: config = SERIAL_8N1; break;
                case UART_STOPBIT_2: config = SERIAL_8N2; break;
            }
            break;
        case XUART_PARITY_EVEN:
            switch (stopbits) {
                case UART_STOPBIT_1: config = SERIAL_8E1; break;
                case UART_STOPBIT_2: config = SERIAL_8E2; break;
            }
            break;
        case XUART_PARITY_ODD:
            switch (stopbits) {
                case UART_STOPBIT_1: config = SERIAL_8O1; break;
                case UART_STOPBIT_2: config = SERIAL_8O2; break;
            }
            break;
    }
#if defined UARTC_USE_TX_IO || defined UARTC_USE_RX_IO // both need to be defined
    UARTC_SERIAL_NO.begin(baud, config, UARTC_USE_RX_IO, UARTC_USE_TX_IO);
#else
    UARTC_SERIAL_NO.begin(baud, config);
#endif

    UARTC_SERIAL_NO.setRxFIFOFull(8);  // > 57600 baud sets to 120 which is too much, buffer only 128 bytes
    UARTC_SERIAL_NO.setRxTimeout(1);   // wait for 1 symbol (~11 bits) to trigger Rx ISR, default 2

#elif defined ESP8266
    UARTC_SERIAL_NO.setRxBufferSize(UARTC_RXBUFSIZE);
    UARTC_SERIAL_NO.begin(baud);
#endif
}


void uartc_setbaudrate(uint32_t baud)
{
    UARTC_SERIAL_NO.end();
    _uartc_initit(baud, XUART_PARITY_NO, UART_STOPBIT_1);
}


void uartc_setprotocol(uint32_t baud, UARTPARITYENUM parity, UARTSTOPBITENUM stopbits)
{
    UARTC_SERIAL_NO.end();
    _uartc_initit(baud, parity, stopbits);
}


void uartc_init_isroff(void)
{
    UARTC_SERIAL_NO.end();
    _uartc_initit(UARTC_BAUD, XUART_PARITY_NO, UART_STOPBIT_1);
}


void uartc_init(void)
{
    uartc_init_isroff();
    // isr is enabled !
}


//-------------------------------------------------------
// System bootloader
//-------------------------------------------------------
// ESP8266, ESP32 can't reboot into system bootloader

IRAM_ATTR uint8_t uartc_has_systemboot(void)
{
    return 0;
}


#endif // ESPLIB_UARTC_H
