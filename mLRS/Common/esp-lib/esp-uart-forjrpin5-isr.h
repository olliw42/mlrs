//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// ESP UART
//********************************************************
// For ESP32:
// usefull resource
// - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
//********************************************************
#ifndef ESPLIB_UART_FORJRIN5_H
#define ESPLIB_UART_FORJRIN5_H

#ifdef ESP32
#include "driver/uart.h"
#include "hal/uart_ll.h"
#endif


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

#ifdef UART_USE_SERIAL
  #ifdef ESP32
    #define UART_SERIAL_NO     UART_NUM_0
    #define UART_UARTX         UART0
  #elif defined ESP8266
    #define UART_SERIAL_NO     Serial
  #endif
#elif defined UART_USE_SERIAL1
  #ifdef ESP32
    #define UART_SERIAL_NO     UART_NUM_1
    #define UART_UARTX         UART1
  #elif defined ESP8266
    #define UART_SERIAL_NO     Serial1
  #endif
#elif defined UART_USE_SERIAL2
  #ifdef ESP32
    #define UART_SERIAL_NO     UART_NUM_2
    #define UART_UARTX         UART2
  #else
    #error ESP8266 has no SERIAL2!
  #endif
#else
  #error UART_SERIAL_NO must be defined!
#endif

#ifndef UART_TXBUFSIZE
  #define UART_TXBUFSIZE       256 // MUST be 2^N
#endif
#ifndef UART_RXBUFSIZE
  #define UART_RXBUFSIZE       256 // MUST be 2^N
#endif

#ifdef ESP32
/*
  #if (UART_TXBUFSIZE > 0) && (UART_TXBUFSIZE < 256) 
    #error UART_TXBUFSIZE must be 0 or >= 256
  #endif
  #if (UART_RXBUFSIZE < 256) 
    #error UART_RXBUFSIZE must be >= 256
  #endif
*/

#if 1 //def UART_USE_TX_ISR
  #ifndef UART_TXBUFSIZE
    #define UART_TXBUFSIZE      256 // MUST be 2^N
  #endif
  #if UART_TXBUFSIZE < 2
    #error UART_TXBUFSIZE must be larger than 1 !
  #elif ((UART_TXBUFSIZE & (UART_TXBUFSIZE-1)) != 0)
    #error UART_TXBUFSIZE must be a power of 2 !
  #endif

  #define UART_TXBUFSIZEMASK  (UART_TXBUFSIZE-1)

  volatile char uart_txbuf[UART_TXBUFSIZE];
  volatile uint16_t uart_txwritepos; // pos at which the last byte was stored
  volatile uint16_t uart_txreadpos; // pos at which the next byte is to be fetched
#endif


#if 1 //def UART_USE_RX
  #ifndef UART_RXBUFSIZE
    #define UART_RXBUFSIZE      256 //128 //MUST be 2^N
  #endif
  #if UART_RXBUFSIZE < 2
    #error UART_RXBUFSIZE must be larger than 1 !
  #elif ((UART_RXBUFSIZE & (UART_RXBUFSIZE-1)) != 0)
    #error UART_RXBUFSIZE must be a power of 2 !
  #endif

  #define UART_RXBUFSIZEMASK  (UART_RXBUFSIZE-1)

  volatile char uart_rxbuf[UART_RXBUFSIZE];
  volatile uint16_t uart_rxwritepos; // pos at which the last byte was stored
  volatile uint16_t uart_rxreadpos; // pos at which the next byte is to be fetched
#endif

#endif // ESP32


//-------------------------------------------------------
// ISR routine
//-------------------------------------------------------

static void IRAM_ATTR uart_intr_handle(void *arg)
{
    uint32_t intr_status = UART_UARTX.int_st.val;

    if (intr_status & UART_INTR_RXFIFO_FULL || intr_status & UART_INTR_RXFIFO_TOUT) {
        // the interrupt was from one of the Rx interrupts
        uint32_t rx_fifo_len = UART_UARTX.status.rxfifo_cnt; // read number of bytes in UART buffer
        while (rx_fifo_len) {
            uint8_t usart_dr = UART_UARTX.fifo.rw_byte; // read a byte
#ifdef UART_RX_CALLBACK_FULLXX
            UART_RX_CALLBACK_FULL(usart_dr);
#else /*
            uint16_t next = (uart_rxwritepos + 1) & UART_RXBUFSIZEMASK;
            if (uart_rxreadpos != next) { // fifo not full
                uart_rxbuf[next] = usart_dr;
                uart_rxwritepos = next;
            } */
#endif            
            rx_fifo_len--;
        }
        // after reading bytes from buffer clear UART interrupt status
        uart_clear_intr_status(UART_SERIAL_NO, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
    }
}


//-------------------------------------------------------
// TX routines
//-------------------------------------------------------

IRAM_ATTR void uart_putbuf(uint8_t* buf, uint16_t len)
{
#ifdef ESP32
//    uart_write_bytes(UART_SERIAL_NO, (uint8_t*)buf, len);
#elif defined ESP8266
    UART_SERIAL_NO.write((uint8_t*)buf, len);
#endif
}


IRAM_ATTR void uart_tx_flush(void)
{
#ifdef ESP32
    // flush of tx buffer not available

    uart_txwritepos = uart_txreadpos = 0;

#elif defined ESP8266
    UART_SERIAL_NO.flush();
#endif
}


//-------------------------------------------------------
// RX routines
//-------------------------------------------------------

IRAM_ATTR char uart_getc(void)
{
#ifdef ESP32
/*
    uint8_t c = 0;
    uart_read_bytes(UART_SERIAL_NO, &c, 1, 0);
    return (char)c;
*/    

    while (uart_rxwritepos == uart_rxreadpos) {};
    uart_rxreadpos = (uart_rxreadpos + 1) & UART_RXBUFSIZEMASK;
    return uart_rxbuf[uart_rxreadpos];

#elif defined ESP8266
    return (char)UART_SERIAL_NO.read();
#endif
}


IRAM_ATTR void uart_rx_flush(void)
{
#ifdef ESP32
    uart_flush(UART_SERIAL_NO);

    uart_rxwritepos = uart_rxreadpos = 0;

#elif defined ESP8266
    while (UART_SERIAL_NO.available() > 0) UART_SERIAL_NO.read();
#endif
}


IRAM_ATTR uint16_t uart_rx_bytesavailable(void)
{
#ifdef ESP32
/*
    uint32_t available = 0;
    uart_get_buffered_data_len(UART_SERIAL_NO, &available);
    return (uint16_t)available;
*/    
int16_t d;

    d = (int16_t)uart_rxwritepos - (int16_t)uart_rxreadpos;
    return (d < 0) ? d + (UART_RXBUFSIZEMASK + 1) : d;

#elif defined ESP8266
    return (UART_SERIAL_NO.available() > 0) ? UART_SERIAL_NO.available() : 0;
#endif
}


IRAM_ATTR uint16_t uart_rx_available(void)
{
#ifdef ESP32
/*
    uint32_t available = 0;
    uart_get_buffered_data_len(UART_SERIAL_NO, &available);
    return (available > 0) ? 1 : 0;
*/

    if (uart_rxwritepos == uart_rxreadpos) return 0; // fifo empty
    return 1;

#elif defined ESP8266
    return (UART_SERIAL_NO.available() > 0) ? 1 : 0;
#endif
}


//-------------------------------------------------------
// INIT routines
//-------------------------------------------------------
// Note: ESP32 has a hardware fifo for tx, which is 128 bytes in size. However, MAVLink messages
// can be larger than this, and data would thus be lost when put only into the fifo. It is therefore
// crucial to set a Tx buffer size of sufficient size. setTxBufferSize() is not available for ESP82xx.

void _uart_initit(uint32_t baud, UARTPARITYENUM parity, UARTSTOPBITENUM stopbits)
{
#ifdef ESP32

    uart_parity_t _parity = UART_PARITY_DISABLE;
    switch (parity) {
        case XUART_PARITY_NO:
            _parity = UART_PARITY_DISABLE; break;
        case XUART_PARITY_EVEN:
            _parity = UART_PARITY_EVEN; break;
        case XUART_PARITY_ODD:
            _parity = UART_PARITY_ODD; break;
    }

    uart_stop_bits_t _stopbits = UART_STOP_BITS_1;
    switch (stopbits) {
        case UART_STOPBIT_1:
            _stopbits = UART_STOP_BITS_1; break;
        case UART_STOPBIT_2:
            _stopbits = UART_STOP_BITS_2; break;
    }

    uart_config_t config = {
        .baud_rate  = (int)baud,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = _parity,
        .stop_bits  = _stopbits,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_SERIAL_NO, &config));

#if defined UART_USE_TX_IO || defined UART_USE_RX_IO // both need to be defined
    ESP_ERROR_CHECK(uart_set_pin(UART_SERIAL_NO, UART_USE_TX_IO, UART_USE_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
#else
    ESP_ERROR_CHECK(uart_set_pin(UART_SERIAL_NO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
#endif

//    ESP_ERROR_CHECK(uart_driver_install(UART_SERIAL_NO, UART_RXBUFSIZE, UART_TXBUFSIZE, 0, NULL, 0)); // rx buf size needs to be > 128
//    ESP_ERROR_CHECK(uart_set_rx_full_threshold(UART_SERIAL_NO, 8)); // default is 120 which is too much, buffer only 128 bytes
//    ESP_ERROR_CHECK(uart_set_rx_timeout(UART_SERIAL_NO, 1)); // wait for 1 symbol (~11 bits) to trigger Rx ISR, default 2


    ESP_ERROR_CHECK(uart_driver_install(UART_SERIAL_NO, 256, 0, 0, NULL, 0)); // rx buf size needs to be > 128

    // diasble the 'built-in' ISR
    ESP_ERROR_CHECK(uart_isr_free(UART_SERIAL_NO));

    // register our ISR
    ESP_ERROR_CHECK(uart_isr_register(UART_SERIAL_NO, uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, NULL));

    // configure the ISR conditions
    uart_intr_config_t intr_config = {
        .intr_enable_mask = UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT,
        .rx_timeout_thresh = 1,             // 1 symbols ~ 11 bits
        .txfifo_empty_intr_thresh = 10,     // we don't use - doesn't matter
        .rxfifo_full_thresh = 1,            // interrupt every byte
    };
    ESP_ERROR_CHECK(uart_intr_config(UART_SERIAL_NO, &intr_config));


uart_set_line_inverse(UART_SERIAL_NO, UART_SIGNAL_RXD_INV | UART_SIGNAL_TXD_INV);


#elif defined ESP8266
    UART_SERIAL_NO.setRxBufferSize(UART_RXBUFSIZE);
    UART_SERIAL_NO.begin(baud);
#endif
}

#if 0
void uart_setbaudrate(uint32_t baud)
{
#ifdef ESP32
//    ESP_ERROR_CHECK(uart_driver_delete(UART_SERIAL_NO));
// reconfiguration doesn't work
#elif defined ESP8266
    UART_SERIAL_NO.end();
    _uart_initit(baud, XUART_PARITY_NO, UART_STOPBIT_1);
#endif
}


void uart_setprotocol(uint32_t baud, UARTPARITYENUM parity, UARTSTOPBITENUM stopbits)
{
#ifdef ESP32
//    ESP_ERROR_CHECK(uart_driver_delete(UART_SERIAL_NO));
// reconfiguration doesn't work
#elif defined ESP8266
    UART_SERIAL_NO.end();
    _uart_initit(baud, parity, stopbits);
#endif
}


void uart_tx_enablepin(FunctionalState flag) {} // not supported


void uart_rx_enableisr(FunctionalState flag) 
{
#ifdef ESP32
    if (flag == ENABLE) {
//        ESP_ERROR_CHECK(uart_enable_rx_intr(UART_SERIAL_NO));    
    } else {
//        ESP_ERROR_CHECK(uart_disable_rx_intr(UART_SERIAL_NO));    
    }
#endif    
}

#else
void uart_rx_enableisr(FunctionalState flag) {}
#endif


void uart_init_isroff(void)
{
#ifdef ESP32
//    ESP_ERROR_CHECK(uart_driver_delete(UART_SERIAL_NO));
#elif defined ESP8266
    UART_SERIAL_NO.end();
#endif
    _uart_initit(UART_BAUD, XUART_PARITY_NO, UART_STOPBIT_1);
}


void uart_init(void)
{
    uart_init_isroff();
    uart_rx_enableisr(ENABLE);
}


//-------------------------------------------------------
// System bootloader
//-------------------------------------------------------
// ESP8266, ESP32 can't reboot into system bootloader
  
IRAM_ATTR uint8_t uart_has_systemboot(void)
{
    return 0;
}


#endif // ESPLIB_UART_FORJRIN5_H
