//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// JR Pin5 Interface Header for Full Duplex
//********************************************************
#ifndef JRPIN5_INTERFACE_FULL_DUPLEX_H
#define JRPIN5_INTERFACE_FULL_DUPLEX_H


#if !(defined ESP8266 || defined ESP32)
  #error JrPin5 full duplex interface only for ESP currently!
#endif

#include "../Common/esp-lib/esp-uart.h"

//-------------------------------------------------------
// Pin5BridgeBase class

class tPin5BridgeBase
{
  public:
    void Init(void);

    // telemetry handling
    bool telemetry_start_next_tick;
    uint16_t telemetry_state;

    void TelemetryStart(void);

    // interface to the uart hardware peripheral used for the bridge, may be called in isr context
    void pin5_init(void);
    void pin5_tx_start(void) {}
    void pin5_putbuf(uint8_t* const buf, uint16_t len) { uart_putbuf(buf, len); }

    // for in-isr processing
    void pin5_tx_enable(bool enable_flag);
    virtual void parse_nextchar(uint8_t c) = 0;
    virtual bool transmit_start(void) = 0; // returns true if transmission should be started

    // actual isr functions
    void pin5_rx_callback(uint8_t c);
    void pin5_tc_callback(void);

    // parser
    typedef enum {
        STATE_IDLE = 0,

        // mBridge receive states
        STATE_RECEIVE_MBRIDGE_STX2,
        STATE_RECEIVE_MBRIDGE_LEN,
        STATE_RECEIVE_MBRIDGE_SERIALPACKET,
        STATE_RECEIVE_MBRIDGE_CHANNELPACKET,
        STATE_RECEIVE_MBRIDGE_COMMANDPACKET,

        // CRSF receive states
        STATE_RECEIVE_CRSF_LEN,
        STATE_RECEIVE_CRSF_PAYLOAD,
        STATE_RECEIVE_CRSF_CRC,

        // transmit states, used by all
        STATE_TRANSMIT_START,
        STATE_TRANSMITING,
    } STATE_ENUM;

    // not used in this class, but required by the children, so just add them here
    // no need for volatile since used only in isr context
    uint8_t state;
    uint8_t len;
    uint8_t cnt;
    uint16_t tlast_us;
    uint16_t discarded;

    // check and rescue
    void CheckAndRescue(void);
};


void tPin5BridgeBase::Init(void)
{
    state = STATE_IDLE;
    len = 0;
    cnt = 0;
    tlast_us = 0;
    discarded = 0;

    telemetry_start_next_tick = false;
    telemetry_state = 0;

    pin5_init();

    // A small value (like 4) for RxFIFOFull has a higher impact on main loop time,
    // but reduces the delay between JRpin5 receive and transmit.  Choose large for now.
    UART_SERIAL_NO.setRxFIFOFull(64);
    UART_SERIAL_NO.onReceive((void (*)(void)) uart_rx_callback_ptr, false);
    
#ifdef JR_PIN5_FULL_DUPLEX

#ifdef UART_USE_SERIAL
  #define UART_SERIAL_NO       Serial
#elif defined UART_USE_SERIAL1
  #define UART_SERIAL_NO       Serial1
#elif defined UART_USE_SERIAL2
  #define UART_SERIAL_NO       Serial2
#else
  #error UART_SERIAL_NO must be defined!
#endif
    UART_SERIAL_NO.setMode(MODE_RS485_HALF_DUPLEX);

    gpio_matrix_in((gpio_num_t)UART_USE_TX_IO, U1RXD_IN_IDX, true);
    gpio_pulldown_en((gpio_num_t)UART_USE_TX_IO);
    gpio_pullup_dis((gpio_num_t)UART_USE_TX_IO);

    gpio_set_level((gpio_num_t)UART_USE_TX_IO, 0);
    gpio_set_direction((gpio_num_t)UART_USE_TX_IO, GPIO_MODE_INPUT_OUTPUT);
    gpio_matrix_out((gpio_num_t)UART_USE_TX_IO, U1TXD_OUT_IDX, true, false);
    // We really want inverted open-drain, but apparently it is not possible.
    // Because of the 51 Ohm resistor tested radios seem to utilize on their output,
    // this reduces the pull enough to maintain a usable signal when the handset is sending.
    gpio_set_drive_capability((gpio_num_t)UART_USE_TX_IO, GPIO_DRIVE_CAP_0);
#endif
}


void tPin5BridgeBase::TelemetryStart(void)
{
    telemetry_start_next_tick = true;
}


//-------------------------------------------------------
// Interface to the uart hardware peripheral used for the bridge
// except pin5_init() called in isr context

void tPin5BridgeBase::pin5_init(void)
{
    uart_init();
}


void tPin5BridgeBase::pin5_tx_enable(bool enable_flag)
{
    // not used
}


void tPin5BridgeBase::pin5_rx_callback(uint8_t c)
{
    // poll uart
    while (uart_rx_available() && state != STATE_TRANSMIT_START) { // read at most 1 message
        parse_nextchar(uart_getc());
    }

    // send telemetry after every received message
    if (state == STATE_TRANSMIT_START) { // time to send telemetry
        transmit_start();
        state = STATE_IDLE;
    }
}


void tPin5BridgeBase::pin5_tc_callback(void)
{
    // not used
}


//-------------------------------------------------------
// Check and rescue

void tPin5BridgeBase::CheckAndRescue(void)
{
    // not needed
}


//-------------------------------------------------------
// Pin5 Serial class

class tJrPin5SerialPort : public tSerialBase
{
  public:
    // void Init(void) override { uart_init(); }
    void SetBaudRate(uint32_t baud) override { uart_setprotocol(baud, XUART_PARITY_NO, UART_STOPBIT_1); }
    bool full(void) { return !uart_tx_notfull(); }
    void putbuf(uint8_t* const buf, uint16_t len) override { uart_putbuf(buf, len); }
    bool available(void) override { return uart_rx_available(); }
    char getc(void) override { return uart_getc(); }
    void flush(void) override { uart_rx_flush(); uart_tx_flush(); }
    uint16_t bytes_available(void) override { return uart_rx_bytesavailable(); }
    // bool has_systemboot(void) override { return uart_has_systemboot(); }
};

tJrPin5SerialPort jrpin5serial;


#endif // JRPIN5_INTERFACE_FULL_DUPLEX_H
