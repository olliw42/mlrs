//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// JR Pin5 Interface Header
//********************************************************
//
// Documentation
//
// 1. Methods which require external hardware, like xor and buffer chips
//   these are essentially used for F103 chips
//
//   #define JRPIN5_TX_XOR  : a xor is in the Tx line to invert the Tx signal
//
//   #define JRPIN5_RX_XOR  : a xor is in the Rx line to invert the Rx signal
//
//   #define JRPIN5_TX_OE   : Tx & Rx inverter with Tx buffer method
//
// 2. Methods which use the swap and invert capability of more modern UART peripherals
//   but need an external Schottky diode
//
//   #define JRPIN5_RX_TX_INVERT_INTERNAL
//     internal peripheral inverter method, needs a diode from Tx to Rx
//     the jrpin5 signal is on the Rx pin
//
//   #define JRPIN5_RX_TX_INVERT_SWAP_INTERNAL
//     internal peripheral inverter method with Tx<->Rx swap, needs a diode from Rx to Tx
//     the jrpin5 signal is on the Tx pin
//
// 3. Methods which use the swap and invert capability of more modern UART peripherals
//   but avoid the need of an external Schottky diode
//   they essentially supersede the other options
//
//   #define JRPIN5_FULL_INTERNAL_ON_TX     : jrpin5 signal is on the Tx pin
//
//   #define JRPIN5_FULL_INTERNAL_ON_RX     : jrpin5 signal is on the Rx pin
//
//   #define JRPIN5_FULL_INTERNAL_ON_RX_TX  : in cases there Rx & Tx pins are electrically connected
//
#ifndef JRPIN5_INTERFACE_H
#define JRPIN5_INTERFACE_H
#pragma once


#include "../Common/hal/hal.h" // not needed but helps editor to get defines correct LOL


extern volatile uint32_t millis32(void);


//-------------------------------------------------------
// Interface Implementation
// the uart used for JR pin5 must be UART_UARTx

void uart_rx_callback_dummy(uint8_t c) {}
void uart_tc_callback_dummy(void) {}

void (*uart_rx_callback_ptr)(uint8_t) = &uart_rx_callback_dummy;
void (*uart_tc_callback_ptr)(void) = &uart_tc_callback_dummy;

#define UART_RX_CALLBACK_FULL(c)    (*uart_rx_callback_ptr)(c)
#define UART_TC_CALLBACK()          (*uart_tc_callback_ptr)()

#if defined ESP8266 || defined ESP32
#include "jr_pin5_interface_esp.h"
#else
#include "../modules/stm32ll-lib/src/stdstm32-uart.h"

// not available in stdstm32-uart.h, used for half-duplex mode
void uart_tx_putc_totxbuf(char c)
{
    uint16_t next = (uart_txwritepos + 1) & UART_TXBUFSIZEMASK;
    if (uart_txreadpos != next) { // fifo not full //this is isr safe, works also if readpos has changed in the meanwhile
        uart_txbuf[next] = c;
        uart_txwritepos = next;
    }
}

// not available in stdstm32-uart.h, used for half-duplex mode
void uart_tx_start(void)
{
    LL_USART_EnableIT_TXE(UART_UARTx); // initiates transmitting
}

// not available in stdstm32-uart.h, used for IN on JrPin5 mode
void uart_rx_putc_torxbuf(uint8_t c)
{
    uint16_t next = (uart_rxwritepos + 1) & UART_RXBUFSIZEMASK;
    if (uart_rxreadpos != next) { // fifo not full
        uart_rxbuf[next] = c;
        uart_rxwritepos = next;
    }
}


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
    void pin5_tx_start(void) { uart_tx_start(); }
    void pin5_putbuf(uint8_t* const buf, uint16_t len) { for (uint16_t i = 0; i < len; i++) uart_tx_putc_totxbuf(buf[i]); }

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
    // the FRM303 can get stuck, whatever we tried, so brutal rescue
    // can't hurt generally as safety net
    uint32_t nottransmiting_tlast_ms;
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

    nottransmiting_tlast_ms = 0;

    pin5_init();
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
// TX & RX XOR method, F103
#if defined JRPIN5_TX_XOR && defined JRPIN5_RX_XOR
    gpio_init(JRPIN5_TX_XOR, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_VERYFAST);
    gpio_init(JRPIN5_RX_XOR, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_VERYFAST);
    JRPIN5_TX_SET_INVERTED;
    JRPIN5_RX_SET_INVERTED;
#endif

// TX & RX inverter with TX buffer method, F103
#if defined JRPIN5_TX_OE
    gpio_init(JRPIN5_TX_OE, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_VERYFAST);
    JRPIN5_TX_OE_DISABLED;
#endif

    uart_init_isroff();

// internal peripheral inverter method, needs a diode from Tx to Rx
#if defined JRPIN5_RX_TX_INVERT_INTERNAL
    LL_USART_Disable(UART_UARTx);
    LL_USART_SetTXPinLevel(UART_UARTx, LL_USART_TXPIN_LEVEL_INVERTED);
    LL_USART_SetRXPinLevel(UART_UARTx, LL_USART_RXPIN_LEVEL_INVERTED);
    LL_USART_Enable(UART_UARTx);
    gpio_init_af(UART_RX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST);
#endif
// internal peripheral inverter method with Tx<->Rx swap, needs a diode from Rx to Tx
#if defined JRPIN5_RX_TX_INVERT_SWAP_INTERNAL
    LL_USART_Disable(UART_UARTx);
    LL_USART_SetTXPinLevel(UART_UARTx, LL_USART_TXPIN_LEVEL_INVERTED);
    LL_USART_SetRXPinLevel(UART_UARTx, LL_USART_RXPIN_LEVEL_INVERTED);
    LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_SWAPPED);
    LL_USART_Enable(UART_UARTx);
    gpio_init_af(UART_TX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Tx pin is now rx after swap
    gpio_init_af(UART_RX_IO, IO_MODE_OUTPUT_ALTERNATE_PP, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now tx after swap
#endif
// experimental, but seems to work
// first attempt with
//  LL_USART_ConfigHalfDuplexMode(UART_UARTx);
//  LL_USART_SetTransferDirection(UART_UARTx, LL_USART_DIRECTION_NONE);
//  LL_USART_SetTransferDirection(UART_UARTx, LL_USART_DIRECTION_TX);
//  LL_USART_SetTransferDirection(UART_UARTx, LL_USART_DIRECTION_RX);
// did not really work out well
#if defined JRPIN5_FULL_INTERNAL_ON_TX
    LL_USART_Disable(UART_UARTx);
    LL_USART_SetTXPinLevel(UART_UARTx, LL_USART_TXPIN_LEVEL_INVERTED);
    LL_USART_SetRXPinLevel(UART_UARTx, LL_USART_RXPIN_LEVEL_INVERTED);
    LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_SWAPPED);
    LL_USART_Enable(UART_UARTx);
    gpio_init_af(UART_TX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Tx pin is now rx
    gpio_init(UART_RX_IO, IO_MODE_INPUT_PD, IO_SPEED_VERYFAST); // disable Rx pin, seems not really needed but makes sense
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX
    LL_USART_Disable(UART_UARTx);
    LL_USART_SetTXPinLevel(UART_UARTx, LL_USART_TXPIN_LEVEL_INVERTED);
    LL_USART_SetRXPinLevel(UART_UARTx, LL_USART_RXPIN_LEVEL_INVERTED);
    LL_USART_Enable(UART_UARTx);
    gpio_init_af(UART_RX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now rx
    gpio_init(UART_TX_IO, IO_MODE_INPUT_PD, IO_SPEED_VERYFAST); // disable Tx pin, seems not really needed but makes sense
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX_TX
    LL_USART_Disable(UART_UARTx);
    LL_USART_SetTXPinLevel(UART_UARTx, LL_USART_TXPIN_LEVEL_INVERTED);
    LL_USART_SetRXPinLevel(UART_UARTx, LL_USART_RXPIN_LEVEL_INVERTED);
    LL_USART_Enable(UART_UARTx);
    gpio_init_af(UART_RX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now rx
    gpio_init(UART_TX_IO, IO_MODE_INPUT_ANALOG, IO_SPEED_VERYFAST); // disable Tx pin
#endif

    pin5_tx_enable(false); // also enables rx isr

#ifdef TX_FRM303_F072CB
    gpio_init_outpp(IO_PB9);
#endif
#if defined TX_DIY_SXDUAL_MODULE02_G491RE || defined TX_DIY_E28DUAL_MODULE02_G491RE || defined TX_DIY_E22DUAL_MODULE02_G491RE
    gpio_init_outpp(IO_PA0);
#endif
}


void tPin5BridgeBase::pin5_tx_enable(bool enable_flag)
{
    if (enable_flag) {
        uart_rx_enableisr(DISABLE);

#if defined JRPIN5_TX_OE
        JRPIN5_TX_OE_ENABLED;
#endif
#if defined JRPIN5_DISABLE_TX_WHILE_RX
        uart_tx_enablepin(ENABLE);
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_TX
        LL_USART_Disable(UART_UARTx);
        LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_STANDARD);
        LL_USART_Enable(UART_UARTx);
        gpio_change_af(UART_TX_IO, IO_MODE_OUTPUT_ALTERNATE_PP, UART_IO_AF, IO_SPEED_VERYFAST); // Tx pin is now tx
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX
        LL_USART_Disable(UART_UARTx);
        LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_SWAPPED);
        LL_USART_Enable(UART_UARTx);
        gpio_change_af(UART_RX_IO, IO_MODE_OUTPUT_ALTERNATE_PP, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now tx
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX_TX
        gpio_change_af(UART_TX_IO, IO_MODE_OUTPUT_ALTERNATE_PP, UART_IO_AF, IO_SPEED_VERYFAST); // Tx pin is now tx
        gpio_change(UART_RX_IO, IO_MODE_INPUT_ANALOG, IO_SPEED_VERYFAST); // disable Rx pin
#endif

    } else {
#if defined JRPIN5_TX_OE
        JRPIN5_TX_OE_DISABLED;
#endif
#if defined JRPIN5_DISABLE_TX_WHILE_RX
        uart_tx_enablepin(DISABLE);
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_TX
        LL_USART_Disable(UART_UARTx);
        LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_SWAPPED);
        LL_USART_Enable(UART_UARTx);
        gpio_change_af(UART_TX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Tx pin is now rx
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX
        LL_USART_Disable(UART_UARTx);
        LL_USART_SetTXRXSwap(UART_UARTx, LL_USART_TXRX_STANDARD);
        LL_USART_Enable(UART_UARTx);
        gpio_change_af(UART_RX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now rx
#endif
#if defined JRPIN5_FULL_INTERNAL_ON_RX_TX
        gpio_change_af(UART_RX_IO, IO_MODE_INPUT_PD, UART_IO_AF, IO_SPEED_VERYFAST); // Rx pin is now rx
        gpio_change(UART_TX_IO, IO_MODE_INPUT_ANALOG, IO_SPEED_VERYFAST); // disable Tx pin
#endif

        uart_rx_enableisr(ENABLE);
    }
}


// we do not add a delay here before we transmit
// the logic analyzer shows this gives a 30-35 us gap nevertheless, which is perfect

void tPin5BridgeBase::pin5_rx_callback(uint8_t c)
{
    parse_nextchar(c);

    if (state < STATE_TRANSMIT_START) return; // we are in receiving

    if (state != STATE_TRANSMIT_START) { // we are in transmitting, should not happen! (does appear to not happen)
        state = STATE_IDLE;
        return;
    }

    if (transmit_start()) { // check if a transmission waits, put it into buf and return true to start
        pin5_tx_enable(true);
        state = STATE_TRANSMITING;
        pin5_tx_start();
    } else {
        state = STATE_IDLE;
    }
}


void tPin5BridgeBase::pin5_tc_callback(void)
{
    pin5_tx_enable(false); // switches on rx
    state = STATE_IDLE;
}


//-------------------------------------------------------
// Check and rescue
// a good place to call it could be ChannelsUpdated()
// Note: For the FRM303 it was observed that the TC callback may be missed in the uart isr, basically when
// the jrpin5's uart isr priority is too low. This caused the jrpin5 loop to get stuck in STATE_TRANSMITING,
// and not even channel data would be received anymore (= very catastrophic). This code avoids this.
// With proper isr priorities, the issue is mainly gone, but the code remains, as safety net.

void tPin5BridgeBase::CheckAndRescue(void)
{
    uint32_t tnow_ms = millis32();

    if (state < STATE_TRANSMITING) {
        nottransmiting_tlast_ms = tnow_ms;
    } else {
        if (tnow_ms - nottransmiting_tlast_ms > 20) { // we are stuck, so rescue
#ifdef TX_FRM303_F072CB
            gpio_low(IO_PB9);
#endif
#if defined TX_DIY_SXDUAL_MODULE02_G491RE || defined TX_DIY_E28DUAL_MODULE02_G491RE || defined TX_DIY_E22DUAL_MODULE02_G491RE
            gpio_high(IO_PA0);
#endif
            state = STATE_IDLE;
            pin5_tx_enable(false);
            LL_USART_DisableIT_TC(UART_UARTx);
            LL_USART_ClearFlag_TC(UART_UARTx);
        }
    }
}


#endif // !DEVICE_HAS_JRPIN5_NO_TC

#endif // JRPIN5_INTERFACE_H
