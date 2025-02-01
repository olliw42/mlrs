//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// mLRS TX
//*******************************************************


#define DBG_MAIN(x)
#define DBG_MAIN_SLIM(x)
#define DEBUG_ENABLED
#define FAIL_ENABLED


// we set the priorities here to have an overview, SysTick is at 15, I2C is at 15, USB is at 0
#define UART_IRQ_PRIORITY           10 // jrpin5 bridge, this needs to be high, when lower than DIO1, the module could stop sending via the bridge
#define UARTB_IRQ_PRIORITY          11 // serial
#define UARTC_IRQ_PRIORITY          11 // com
#define UARTD_IRQ_PRIORITY          15 // serial2/ESP/BT
#define UARTF_IRQ_PRIORITY          15 // debug
#define SX_DIO_EXTI_IRQ_PRIORITY    13
#define SX2_DIO_EXTI_IRQ_PRIORITY   13 // on single spi diversity systems must be equal to DIO priority
#define SWUART_TIM_IRQ_PRIORITY      9 // debug on swuart
#define BUZZER_TIM_IRQ_PRIORITY     14

#include "../Common/common_conf.h"
#include "../Common/common_types.h"

#if defined ESP8266 || defined ESP32

#include "../Common/hal/esp-glue.h"
#include "../modules/stm32ll-lib/src/stdstm32.h"
#include "../Common/esp-lib/esp-peripherals.h"
#include "../Common/esp-lib/esp-mcu.h"
//xx #include "../Common/esp-lib/esp-adc.h"
#include "../Common/esp-lib/esp-stack.h"
#include "../Common/hal/hal.h"
#include "../Common/esp-lib/esp-delay.h" // these are dependent on hal
#include "../Common/esp-lib/esp-eeprom.h"
#include "../Common/esp-lib/esp-spi.h"
#if defined USE_SERIAL && !defined DEVICE_HAS_SERIAL_ON_USB
#include "../Common/esp-lib/esp-uartb.h"
#endif
#if defined USE_COM && !defined DEVICE_HAS_COM_ON_USB
#include "../Common/esp-lib/esp-uartc.h"
#endif
#ifdef USE_SERIAL2
#include "../Common/esp-lib/esp-uartd.h"
#endif
#ifdef USE_DEBUG
#ifdef DEVICE_HAS_DEBUG_SWUART
#include "../Common/esp-lib/esp-uart-sw.h"
#else
#include "../Common/esp-lib/esp-uartf.h"
#endif
#endif
#ifdef USE_I2C
#include "../Common/esp-lib/esp-i2c.h"
#endif
#include "../Common/hal/esp-timer.h"

#else

#include "../Common/hal/glue.h"
#include "../modules/stm32ll-lib/src/stdstm32.h"
#include "../modules/stm32ll-lib/src/stdstm32-peripherals.h"
#include "../modules/stm32ll-lib/src/stdstm32-mcu.h"
#include "../modules/stm32ll-lib/src/stdstm32-adc.h"
#include "../modules/stm32ll-lib/src/stdstm32-stack.h"
#ifdef STM32WL
#include "../modules/stm32ll-lib/src/stdstm32-subghz.h"
#endif
#include "../Common/hal/hal.h"
#include "../modules/stm32ll-lib/src/stdstm32-delay.h" // these are dependent on hal
#include "../modules/stm32ll-lib/src/stdstm32-eeprom.h"
#include "../modules/stm32ll-lib/src/stdstm32-spi.h"
#ifdef USE_SX2
#include "../modules/stm32ll-lib/src/stdstm32-spib.h"
#endif
#if defined USE_SERIAL && !defined DEVICE_HAS_SERIAL_ON_USB
#include "../modules/stm32ll-lib/src/stdstm32-uartb.h"
#endif
#if defined USE_COM && !defined DEVICE_HAS_COM_ON_USB
#include "../modules/stm32ll-lib/src/stdstm32-uartc.h"
#endif
#ifdef USE_SERIAL2
#include "../modules/stm32ll-lib/src/stdstm32-uartd.h"
#endif
#ifdef USE_USB
#include "../modules/stm32-usb-device/stdstm32-usb-vcp.h"
#endif
#ifdef USE_DEBUG
#ifdef DEVICE_HAS_DEBUG_SWUART
#include "../modules/stm32ll-lib/src/stdstm32-uart-sw.h"
#else
#include "../modules/stm32ll-lib/src/stdstm32-uartf.h"
#endif
#endif
#ifdef USE_I2C
#include "../modules/stm32ll-lib/src/stdstm32-i2c.h"
#endif
#include "../Common/hal/timer.h"

#endif //#if defined ESP8266 || defined ESP32

#include "../Common/sx-drivers/sx12xx.h"
#include "../Common/mavlink/fmav.h"
#include "../Common/setup.h"
#include "../Common/common.h"
#include "../Common/channel_order.h"
#include "../Common/diversity.h"
#include "../Common/arq.h"
#include "../Common/rf_power.h"
//#include "../Common/time_stats.h" // un-comment if you want to use
//#include "../Common/test.h" // un-comment if you want to compile for board test

#include "config_id.h"
#include "cli.h"
#include "mbridge_interface.h" // this includes uart.h as it needs callbacks, declares tMBridge mbridge
#include "crsf_interface_tx.h" // this includes uart.h as it needs callbacks, declares tTxCrsf crsf
#include "in_interface.h" // this includes uarte.h, in.h, declares tIn in


tRDiversity rdiversity;
tTDiversity tdiversity;
tReceiveArq rarq;
tRfPower rfpower;
tChannelOrder channelOrder(tChannelOrder::DIRECTION_TX_TO_MLRS);
tConfigId config_id;
tTxCli cli;


//-------------------------------------------------------
// MAVLink & MSP
//-------------------------------------------------------

#include "mavlink_interface_tx.h"

tTxMavlink mavlink;


uint8_t mavlink_vehicle_state(void)
{
    return mavlink.VehicleState();
}


#include "msp_interface_tx.h"

tTxMsp msp;


#include "sx_serial_interface_tx.h"

tTxSxSerial sx_serial;


//-------------------------------------------------------
// Display
//-------------------------------------------------------
// doing one, draw or update, every cycle in 50 Hz mode works, but
// doing both every cycle does not work! why ???

#include "../CommonTx/disp.h"

tTxDisp disp;


//-------------------------------------------------------
// Wifi Bridge
//-------------------------------------------------------

#include "esp.h"

tTxEspWifiBridge esp;

#include "hc04.h"

tTxHc04Bridge hc04;


//-------------------------------------------------------
// While transmit/receive tasks
//-------------------------------------------------------

#include "../Common/while.h"


class tWhileTransmit : public tWhileBase
{
  public:
    uint32_t dtmax_us(void) override { return sx.TimeOverAir_us() - 1000; }
    void handle_once(void) override;
#ifdef USE_DISPLAY
    void handle(void) override { disp.SpinI2C(); }
#endif
};

tWhileTransmit whileTransmit;


void tWhileTransmit::handle_once(void)
{
    cli.Do();

#ifdef USE_DISPLAY
    uint32_t tnow_ms = millis32();

    static uint32_t main_tlast_ms = 0;
    if (tnow_ms - main_tlast_ms >= 250) { // Update Main page at 4 Hz
        main_tlast_ms = tnow_ms;
        disp.UpdateMain();
    }

    if (bind.IsInBind()) disp.SetBind();

    // postpone to next cycle if just updated and it's a short time slot
    bool allow_draw = (dtmax_us() > 2000) || (main_tlast_ms != tnow_ms);

    static uint32_t draw_tlast_ms = 0;
    if (allow_draw && (tnow_ms - draw_tlast_ms >= 50)) { // effectively slows down (drawing takes time, ca 30 ms on G4, slower on other mcu)
        draw_tlast_ms = tnow_ms;
        disp.Draw();
    }
#endif
}


//-------------------------------------------------------
// Some helper
//-------------------------------------------------------

void start_bind(void)
{
    if (!bind.IsInBind()) bind.StartBind();
}


void stop_bind(void)
{
    if (bind.IsInBind()) bind.StopBind();
}


void enter_system_bootloader(void)
{
    disp.DrawBoot();
    BootLoaderInit();
}


//-------------------------------------------------------
// Init
//-------------------------------------------------------

void init_once(void)
{
    serial.InitOnce();
    comport.InitOnce();
    serial2.InitOnce();
}


void init_hw(void)
{
    // disable all interrupts, they may be enabled with restart
    __disable_irq();

    delay_init();
    systembootloader_init(); // after delay_init() since it may need delay
    timer_init();

    leds_init();
    button_init();
    esp_init();
    fiveway_init();

    serial.Init();
    serial2.Init();
    comport.Init();

    buzzer.Init();
    fan.Init();
    dbg.Init();

    setup_init();

    esp_enable(Setup.Tx[Config.ConfigId].SerialDestination);

    sx.Init(); // these take time
    sx2.Init();

    mbridge.Init(Config.UseMbridge, Config.UseCrsf); // these affect peripherals, hence do here
    crsf.Init(Config.UseCrsf);
    in.Init(Config.UseIn);

    __enable_irq();
}


//-------------------------------------------------------
// SX12xx
//-------------------------------------------------------

volatile uint16_t irq_status;
volatile uint16_t irq2_status;

IRQHANDLER(
void SX_DIO_EXTI_IRQHandler(void)
{
    sx_dio_exti_isr_clearflag();
    irq_status = sx.GetAndClearIrqStatus(SX_IRQ_ALL);
    if (irq_status & SX_IRQ_RX_DONE) {
        if (bind.IsInBind()) {
            uint64_t bind_signature;
            sx.ReadBuffer(0, (uint8_t*)&bind_signature, 8);
            if (bind_signature != bind.RxSignature) irq_status = 0; // not binding frame, so ignore it
        } else {
            uint16_t sync_word;
            sx.ReadBuffer(0, (uint8_t*)&sync_word, 2); // rxStartBufferPointer is always 0, so no need for sx.GetRxBufferStatus()
            if (sync_word != Config.FrameSyncWord) irq_status = 0; // not for us, so ignore it
        }
    }
})
#ifdef USE_SX2
IRQHANDLER(
void SX2_DIO_EXTI_IRQHandler(void)
{
    sx2_dio_exti_isr_clearflag();
    irq2_status = sx2.GetAndClearIrqStatus(SX2_IRQ_ALL);
    if (irq2_status & SX2_IRQ_RX_DONE) {
        if (bind.IsInBind()) {
            uint64_t bind_signature;
            sx2.ReadBuffer(0, (uint8_t*)&bind_signature, 8);
            if (bind_signature != bind.RxSignature) irq2_status = 0;
        } else {
            uint16_t sync_word;
            sx2.ReadBuffer(0, (uint8_t*)&sync_word, 2);
            if (sync_word != Config.FrameSyncWord) irq2_status = 0;
        }
    }
})
#endif


uint8_t link_rx1_status;
uint8_t link_rx2_status;


//-- Tx/Rx cmd frame handling

uint8_t link_task;
uint8_t transmit_frame_type;
uint16_t link_task_delay_ms;
bool doParamsStore;


void link_task_init(void)
{
    link_task = LINK_TASK_NONE;
    link_task_delay_ms = 0;
    transmit_frame_type = TRANSMIT_FRAME_TYPE_NORMAL;

    doParamsStore = false;
}


bool link_task_free(void)
{
    if (link_task != LINK_TASK_NONE) return false; // a task is running
    return true;
}


bool link_task_set(uint8_t task)
{
    if (link_task != LINK_TASK_NONE) return false; // a task is running

    link_task = task;
    transmit_frame_type = TRANSMIT_FRAME_TYPE_CMD;

    link_task_delay_ms = 0;

    // set a timeout if relevant, or do other things as needed
    switch (link_task) {
    case LINK_TASK_TX_GET_RX_SETUPDATA:
    case LINK_TASK_TX_GET_RX_SETUPDATA_WRELOAD:
        SetupMetaData.rx_available = false;
        break;
    case LINK_TASK_TX_STORE_RX_PARAMS: // store rx parameters
        link_task_delay_ms = 500; // we set a delay, the actual store is triggered when it expires
        break;
    }

    return true;
}


void link_task_reset(void)
{
    link_task = LINK_TASK_NONE;
    link_task_delay_ms = 0;
    transmit_frame_type = TRANSMIT_FRAME_TYPE_NORMAL;
}


void link_task_tick_ms(void)
{
    // if a delay has been set, count it down
    if (link_task_delay_ms) {
        link_task_delay_ms--;
        if (!link_task_delay_ms) {
            switch (link_task) {
            case LINK_TASK_TX_STORE_RX_PARAMS: doParamsStore = true; break;
            }
            link_task_reset();
            mbridge.Unlock();
        }
    }
}


void process_received_rxcmdframe(tRxFrame* const frame)
{
tCmdFrameHeader* head = (tCmdFrameHeader*)(frame->payload);

    switch (head->cmd) {
    case FRAME_CMD_RX_SETUPDATA:
        // received rx setup data
        unpack_rxcmdframe_rxsetupdata(frame);
        link_task_reset();
#ifdef USE_JRPIN5
        switch (mbridge.cmd_in_process) {
        case MBRIDGE_CMD_REQUEST_INFO: mbridge.HandleCmd(MBRIDGE_CMD_REQUEST_INFO); break;
        }
        mbridge.Unlock();
#endif
        break;
    }
}


void pack_txcmdframe(tTxFrame* const frame, tFrameStats* const frame_stats, tRcData* const rc)
{
    switch (link_task) {
    case LINK_TASK_TX_GET_RX_SETUPDATA:
        pack_txcmdframe_cmd(frame, frame_stats, rc, FRAME_CMD_GET_RX_SETUPDATA);
        break;
    case LINK_TASK_TX_GET_RX_SETUPDATA_WRELOAD:
        pack_txcmdframe_cmd(frame, frame_stats, rc, FRAME_CMD_GET_RX_SETUPDATA_WRELOAD);
        break;
    case LINK_TASK_TX_SET_RX_PARAMS:
        pack_txcmdframe_setrxparams(frame, frame_stats, rc);
        break;
    case LINK_TASK_TX_STORE_RX_PARAMS:
        pack_txcmdframe_cmd(frame, frame_stats, rc, FRAME_CMD_STORE_RX_PARAMS);
        transmit_frame_type = TRANSMIT_FRAME_TYPE_NORMAL;
        break;
    }
}


//-- normal Tx, Rx frames handling
// transmit
//   -> do_transmit()
//       -> prepare_transmit_frame()
// receive
//   isr:        -> irq2_status
//   isr loop:   -> do_receive()
//               -> link_rx1_status
//   post loop:  -> handle_receive() or handle_receive_none()
//                   if valid -> process_received_frame()

void prepare_transmit_frame(uint8_t antenna)
{
uint8_t payload[FRAME_TX_PAYLOAD_LEN];
uint8_t payload_len = 0;

    if (transmit_frame_type == TRANSMIT_FRAME_TYPE_NORMAL) {
        // read data from serial port
        if (connected()) {
            for (uint8_t i = 0; i < FRAME_TX_PAYLOAD_LEN; i++) {
                if (!sx_serial.available()) break;
                uint8_t c = sx_serial.getc();
                payload[payload_len++] = c;
            }

            stats.bytes_transmitted.Add(payload_len);
            stats.serial_data_transmitted.Inc();
        } else {
            sx_serial.flush();
        }
    }

    stats.last_transmit_antenna = antenna;

    tFrameStats frame_stats;
    frame_stats.seq_no = stats.transmit_seq_no;
    frame_stats.ack = rarq.AckSeqNo();
    frame_stats.antenna = stats.last_antenna;
    frame_stats.transmit_antenna = antenna;
    frame_stats.rssi = stats.GetLastRssi();
    frame_stats.LQ_rc = UINT8_MAX; // Tx has no valid value
    frame_stats.LQ_serial = stats.GetLQ_serial();

    if (transmit_frame_type == TRANSMIT_FRAME_TYPE_NORMAL) {
        pack_txframe(&txFrame, &frame_stats, &rcData, payload, payload_len);
    } else {
        pack_txcmdframe(&txFrame, &frame_stats, &rcData);
    }
}


void process_received_frame(bool do_payload, tRxFrame* const frame)
{
    bool accept_payload = rarq.AcceptPayload();

    stats.received_antenna = frame->status.antenna;
    stats.received_transmit_antenna = frame->status.transmit_antenna;
    stats.received_rssi = rssi_i8_from_u7(frame->status.rssi_u7);
    stats.received_LQ_rc = frame->status.LQ_rc;
    stats.received_LQ_serial = frame->status.LQ_serial;

    if (!do_payload) {
        return;
    }

    if (!accept_payload) return; // frame has no fresh payload

    // handle cmd frame
    if (frame->status.frame_type == FRAME_TYPE_TX_RX_CMD) {
        process_received_rxcmdframe(frame);
        return;
    }

    // output data on serial
    sx_serial.putbuf(frame->payload, frame->status.payload_len);

    stats.bytes_received.Add(frame->status.payload_len);
    stats.serial_data_received.Inc();
}


//-- receive/transmit handling api

void handle_receive(uint8_t antenna) // RX_STATUS_INVALID, RX_STATUS_VALID
{
uint8_t rx_status;
tRxFrame* frame;

    if (antenna == ANTENNA_1) {
        rx_status = link_rx1_status;
        frame = &rxFrame;
    } else {
        rx_status = link_rx2_status;
        frame = &rxFrame2;
    }

    if (bind.IsInBind()) {
        bind.handle_receive(antenna, rx_status);
        return;
    }

    if (rx_status < RX_STATUS_INVALID) { // must not happen
        FAIL_WSTATE(BLINK_4, "rx_status failure", 0,0, link_rx1_status, link_rx2_status);
    }

    // handle receive ARQ, must come before process_received_frame()
    if (rx_status == RX_STATUS_VALID) {
        rarq.Received(frame->status.seq_no);
    } else {
        rarq.FrameMissed();
    }
    // check this before received data may be passed to parsers
    if (rarq.FrameLost()) {
        mavlink.FrameLost();
        msp.FrameLost();
    }

    if (rx_status > RX_STATUS_INVALID) { // RX_STATUS_VALID

        bool do_payload = true; // has no rc data, so do_payload is always

        process_received_frame(do_payload, frame);

        stats.doValidFrameReceived(); // should we count valid payload only if rx frame ?

    } else { // RX_STATUS_INVALID
    }

    // we set it for all received frames
    stats.last_antenna = antenna;

    // we count all received frames
    stats.doFrameReceived();
}


void handle_receive_none(void) // RX_STATUS_NONE
{
    rarq.FrameMissed();
}


void do_transmit_prepare(uint8_t antenna) // we prepare a TX frame to be send to receiver
{
    if (bind.IsInBind()) {
        bind.do_transmit(antenna);
        return;
    }

    stats.transmit_seq_no++;

    prepare_transmit_frame(antenna);
}


void do_transmit_send(uint8_t antenna) // we send a TX frame to receiver
{
    if (bind.IsInBind()) {
       sxSendFrame(antenna, &txBindFrame, FRAME_TX_RX_LEN, SEND_FRAME_TMO_MS);
       return;
    }

    sxSendFrame(antenna, &txFrame, FRAME_TX_RX_LEN, SEND_FRAME_TMO_MS); // 10 ms tmo
}


uint8_t do_receive(uint8_t antenna) // we receive a RX frame from receiver
{
uint8_t res;
uint8_t rx_status = RX_STATUS_INVALID; // this also signals that a frame was received

    if (bind.IsInBind()) {
        return bind.do_receive(antenna, false);
    }

    // we don't need to read sx.GetRxBufferStatus(), but hey
    // we could save 2 byte's time by not reading sync_word again, but hey
    sxReadFrame(antenna, &rxFrame, &rxFrame2, FRAME_TX_RX_LEN);
    res = (antenna == ANTENNA_1) ? check_rxframe(&rxFrame) : check_rxframe(&rxFrame2);

    if (res) {
        DBG_MAIN(dbg.puts("fail ");dbg.putc('\n');)
//dbg.puts("fail a");dbg.putc(antenna+'0');dbg.puts(" ");dbg.puts(u8toHEX_s(res));dbg.putc('\n');
    }

    if (res == CHECK_ERROR_SYNCWORD) return RX_STATUS_INVALID; // must not happen !

    if (res == CHECK_OK) {
        rx_status = RX_STATUS_VALID;
    }

    // we want to have the rssi,snr stats even if it's a bad packet
    sxGetPacketStatus(antenna, &stats);

    return rx_status;
}


//##############################################################################################################
//*******************************************************
// MAIN routine
//*******************************************************

// the tx transmit needs to be jitter free, much less than the receiver's 1 ms gap
// the strategy is
// - block hard work one a first tx_tick, to ensure the next tx_tick comes in time
// - start a timer of 750 us, and do the preparation work for the next transmit frame
// - transmit
uint16_t tx_tick;
bool isInTimeGuard;
bool doPreTransmit;
uint16_t pretransmit_tstamp_us;

uint16_t tick_1hz;
uint16_t tick_1hz_commensurate;

uint16_t link_state;
uint8_t connect_state;
uint16_t connect_tmo_cnt;
uint8_t connect_sync_cnt;
bool connect_occured_once;


bool connected(void)
{
    return (connect_state == CONNECT_STATE_CONNECTED);
}

bool connected_and_rx_setup_available(void)
{
    return (connected() && SetupMetaData.rx_available);
}


void main_loop(void)
{
#ifdef BOARD_TEST_H
    main_test();
#endif
INITCONTROLLER_ONCE
    stack_check_init();
    init_once();
RESTARTCONTROLLER
    init_hw();
    DBG_MAIN(dbg.puts("\n\n\nHello\n\n");)

    serial.SetBaudRate(Config.SerialBaudrate);
    serial2.SetBaudRate(Config.SerialBaudrate);

    // startup sign of life
    leds.Init();

    // start up sx
    if (!sx.isOk()) { FAILALWAYS(BLINK_RD_GR_OFF, "Sx not ok"); } // fail!
    if (!sx2.isOk()) { FAILALWAYS(BLINK_GR_RD_OFF, "Sx2 not ok"); } // fail!
    irq_status = irq2_status = 0;
    IF_SX(sx.StartUp(&Config.Sx));
    IF_SX2(sx2.StartUp(&Config.Sx2));
    bind.Init();
    fhss.Init(&Config.Fhss, &Config.Fhss2);
    fhss.Start();
    rfpower.Init();

    sx.SetRfFrequency(fhss.GetCurrFreq());
    sx2.SetRfFrequency(fhss.GetCurrFreq2());

    tx_tick = 0;
    isInTimeGuard = false;
    doPreTransmit = false;
    pretransmit_tstamp_us = 0;
    link_state = LINK_STATE_IDLE;
    connect_state = CONNECT_STATE_LISTEN;
    connect_tmo_cnt = 0;
    connect_sync_cnt = 0;
    connect_occured_once = false;
    link_rx1_status = link_rx2_status = RX_STATUS_NONE;
    link_task_init();
    link_task_set(LINK_TASK_TX_GET_RX_SETUPDATA); // we start with wanting to get rx setup data

    stats.Init(Config.LQAveragingPeriod, Config.frame_rate_hz, Config.frame_rate_ms);
    rdiversity.Init();
    tdiversity.Init(Config.frame_rate_ms);
    rarq.Init();

    in.Configure(Setup.Tx[Config.ConfigId].InMode);
    mavlink.Init(&serial, &mbridge, &serial2); // ports selected by SerialDestination, ChannelsSource
    msp.Init(&serial, &serial2); // ports selected by SerialDestination
    sx_serial.Init(&serial, &mbridge, &serial2); // ports selected by SerialDestination, ChannelsSource
    cli.Init(&comport);
#ifdef USE_ESP_WIFI_BRIDGE
  #if defined JR_PIN5_FULL_DUPLEX
    esp.Init(&jrpin5serial, &serial, &serial2, Config.SerialBaudrate, &Setup.Tx[Config.ConfigId]);
  #else
    esp.Init(&comport, &serial, &serial2, Config.SerialBaudrate, &Setup.Tx[Config.ConfigId]);
  #endif
#endif
#ifdef DEVICE_HAS_HC04_MODULE_ON_SERIAL2
    hc04.Init(&comport, &serial2, Config.SerialBaudrate);
#else
    hc04.Init(&comport, &serial, Config.SerialBaudrate);
#endif
    fan.SetPower(sx.RfPower_dbm());
    whileTransmit.Init();
    disp.Init();

    config_id.Init();

    tick_1hz = 0;
    tick_1hz_commensurate = 0;
    resetSysTask(); // helps in avoiding too short first loop
INITCONTROLLER_END

    //-- SysTask handling

    if (doSysTask()) {
        // when we do long tasks, like display transfer, we miss ticks, so we need to catch up
        // the commands below must not be sensitive to strict ms timing

        if (connect_tmo_cnt) {
            connect_tmo_cnt--;
        }

        DECc(tx_tick, SYSTICK_DELAY_MS(Config.frame_rate_ms));

        if (tx_tick == 1) {
            isInTimeGuard = true; // prevent extra work
        }
        if (tx_tick == 0) {
            doPreTransmit = true; // trigger next cycle
            pretransmit_tstamp_us = micros16();
        }

        link_task_tick_ms();

        if (!doPreTransmit) {
            leds.Tick_ms(connected()); // can take long

            DECc(tick_1hz, SYSTICK_DELAY_MS(1000));

            if (!tick_1hz) {
                if (Setup.Tx[Config.ConfigId].Buzzer == BUZZER_RX_LQ && connect_occured_once) {
                    buzzer.BeepLQ(stats.received_LQ_rc);
                }
            }

            bind.Tick_ms();
            disp.Tick_ms(); // can take long
            fan.SetPower(sx.RfPower_dbm());
            fan.Tick_ms();

            if (!tick_1hz) {
                dbg.puts(".");
/*                dbg.puts("\nTX: ");
                dbg.puts(u8toBCD_s(stats.GetLQ_serial()));
                dbg.puts("(");
                dbg.puts(u8toBCD_s(stats.frames_received.GetLQ())); dbg.putc(',');
                dbg.puts(u8toBCD_s(stats.valid_frames_received.GetLQ()));
                dbg.puts("),");
                dbg.puts(u8toBCD_s(stats.received_LQ_rc)); dbg.puts(", ");

                dbg.puts(s8toBCD_s(stats.last_rssi1)); dbg.putc(',');
                dbg.puts(s8toBCD_s(stats.received_rssi)); dbg.puts(", ");
                dbg.puts(s8toBCD_s(stats.last_snr1)); dbg.puts("; ");

                dbg.puts(u16toBCD_s(stats.bytes_transmitted.GetBytesPerSec())); dbg.puts(", ");
                dbg.puts(u16toBCD_s(stats.bytes_received.GetBytesPerSec())); dbg.puts("; "); */
            }
        } // end of if (!doPreTransmit)
    }

    //-- SX handling

    switch (link_state) {
    case LINK_STATE_IDLE:
        break;

    case LINK_STATE_TRANSMIT:
        do_transmit_prepare(tdiversity.Antenna());
        link_state = LINK_STATE_TRANSMIT_SEND;
        DBG_MAIN_SLIM(dbg.puts("\nt");)
        break;

    case LINK_STATE_TRANSMIT_SEND: {
        uint16_t dt = micros16() - pretransmit_tstamp_us;
        if (dt < 750) break;
        isInTimeGuard = false;
        rfpower.Update();
        fhss.HopToNext();
        sx.SetRfFrequency(fhss.GetCurrFreq());
        sx2.SetRfFrequency(fhss.GetCurrFreq2());
        do_transmit_send(tdiversity.Antenna());
        link_state = LINK_STATE_TRANSMIT_WAIT;
        irq_status = irq2_status = 0;
        DBG_MAIN_SLIM(dbg.puts(">");)
        // auxiliaries
        crsf.TelemetryStart();
        whileTransmit.Trigger();
        break; }

    case LINK_STATE_RECEIVE:
        IF_ANTENNA1(sx.SetToRx(0));
        IF_ANTENNA2(sx2.SetToRx(0));
        link_state = LINK_STATE_RECEIVE_WAIT;
        link_rx1_status = link_rx2_status = RX_STATUS_NONE;
        irq_status = irq2_status = 0;
        DBG_MAIN_SLIM(dbg.puts("r");)
        break;
    }//end of switch(link_state)

IF_SX(
    if (irq_status) {
        if (link_state == LINK_STATE_TRANSMIT_WAIT) {
            if (irq_status & SX_IRQ_TX_DONE) {
                irq_status = 0;
                link_state = LINK_STATE_RECEIVE;
                DBG_MAIN_SLIM(dbg.puts("1!");)
            }
        } else
        if (link_state == LINK_STATE_RECEIVE_WAIT) {
            if (irq_status & SX_IRQ_RX_DONE) {
                irq_status = 0;
                link_rx1_status = do_receive(ANTENNA_1);
                DBG_MAIN_SLIM(dbg.puts("1<");)
            }
        }

        if (irq_status) { // these should not happen
            if (irq_status & SX_IRQ_TIMEOUT) {
            }
            if (irq_status & SX_IRQ_RX_DONE) {
                FAIL_WSTATE(BLINK_RD_GR_OFF, "IRQ RX DONE FAIL", irq_status, link_state, link_rx1_status, link_rx2_status);
            }
            if (irq_status & SX_IRQ_TX_DONE) {
                FAIL_WSTATE(BLINK_GR_RD_OFF, "IRQ TX DONE FAIL", irq_status, link_state, link_rx1_status, link_rx2_status);
            }
            irq_status = 0;
            link_state = LINK_STATE_IDLE;
            link_rx1_status = link_rx2_status = RX_STATUS_NONE;
            DBG_MAIN_SLIM(dbg.puts("1?");)
        }
    }//end of if(irq_status)
);
IF_SX2(
    if (irq2_status) {
        if (link_state == LINK_STATE_TRANSMIT_WAIT) {
            if (irq2_status & SX2_IRQ_TX_DONE) {
                irq2_status = 0;
                link_state = LINK_STATE_RECEIVE;
                DBG_MAIN_SLIM(dbg.puts("2!");)
            }
        } else
        if (link_state == LINK_STATE_RECEIVE_WAIT) {
            if (irq2_status & SX2_IRQ_RX_DONE) {
                irq2_status = 0;
                link_rx2_status = do_receive(ANTENNA_2);
                DBG_MAIN_SLIM(dbg.puts("2<");)
            }
        }

        if (irq2_status) { // this should not happen
            if (irq2_status & SX2_IRQ_TIMEOUT) {
            }
            if (irq2_status & SX2_IRQ_RX_DONE) {
                FAIL_WSTATE(BLINK_RD_GR_ON, "IRQ2 RX DONE FAIL", irq2_status, link_state, link_rx1_status, link_rx2_status);
            }
            if (irq2_status & SX2_IRQ_TX_DONE) {
                FAIL_WSTATE(BLINK_GR_RD_ON, "IRQ2 TX DONE FAIL", irq2_status, link_state, link_rx1_status, link_rx2_status);
            }
            irq2_status = 0;
            link_state = LINK_STATE_IDLE;
            link_rx1_status = link_rx2_status = RX_STATUS_NONE;
            DBG_MAIN_SLIM(dbg.puts("2?");)
        }
    }//end of if(irq2_status)
);

    // this happens before switching to transmit, i.e. after a frame was or should have been received
    if (doPreTransmit) {
        doPreTransmit = false;

        sx.SetToIdle();
        sx2.SetToIdle();

        bool frame_received, valid_frame_received;
        if (USE_ANTENNA1 && USE_ANTENNA2) {
            frame_received = (link_rx1_status > RX_STATUS_NONE) || (link_rx2_status > RX_STATUS_NONE);
            valid_frame_received = (link_rx1_status > RX_STATUS_INVALID) || (link_rx2_status > RX_STATUS_INVALID);
        } else if (USE_ANTENNA2) {
            frame_received = (link_rx2_status > RX_STATUS_NONE);
            valid_frame_received = (link_rx2_status > RX_STATUS_INVALID);
        } else { // use antenna1
            frame_received = (link_rx1_status > RX_STATUS_NONE);
            valid_frame_received = (link_rx1_status > RX_STATUS_INVALID);
        }

        if (frame_received) { // frame received
            uint8_t antenna = ANTENNA_1;
            if (USE_ANTENNA1 && USE_ANTENNA2) {
                antenna = rdiversity.Antenna(link_rx1_status, link_rx2_status, stats.last_rssi1, stats.last_rssi2);
            } else if (USE_ANTENNA2) {
                antenna = ANTENNA_2;
            }
            handle_receive(antenna);
        } else {
            handle_receive_none();
        }

        if (TRANSMIT_USE_ANTENNA1 && TRANSMIT_USE_ANTENNA2) {
            tdiversity.DoEstimate(link_rx1_status, link_rx2_status, stats.last_rssi1, stats.last_rssi2);
        } else if (TRANSMIT_USE_ANTENNA2) {
            tdiversity.SetAntenna(ANTENNA_2);
        } else {
            tdiversity.SetAntenna(ANTENNA_1);
        }

        // serial data is received if !IsInBind() && RX_STATUS_VALID && !FRAME_TYPE_TX_RX_CMD && sx_serial.IsEnabled()
        // valid_frame/frame lost logic is modified by ARQ
#ifndef USE_ARQ
        if (!valid_frame_received) {
            mavlink.FrameLost();
            msp.FrameLost();
        }
#endif

        stats.fhss_curr_i = fhss.CurrI();
        stats.rx1_valid = (link_rx1_status > RX_STATUS_INVALID);
        stats.rx2_valid = (link_rx2_status > RX_STATUS_INVALID);

        if (valid_frame_received) { // valid frame received
            switch (connect_state) {
            case CONNECT_STATE_LISTEN:
                connect_state = CONNECT_STATE_SYNC;
                connect_sync_cnt = 0;
                break;
            case CONNECT_STATE_SYNC:
                connect_sync_cnt++;
                uint8_t connect_sync_cnt_max = CONNECT_SYNC_CNT;
                if (!connect_occured_once) {
                    connect_sync_cnt_max = Config.connect_sync_cnt_max;
                }
                if (connect_sync_cnt >= connect_sync_cnt_max) {
                    if (!SetupMetaData.rx_available && !bind.IsInBind()) {
                        // should not have happen, but does very occasionally happen, so let's cope with
                        // we must have gotten it at least once, on first connect, since we need it
                        // later on we can accept to be gentle and be ok with not getting it again
                        // bottom line: the receiver must not change after first connection
                        if (connect_occured_once) {
                            link_task_reset();
                            SetupMetaData.rx_available = true;
                        } else {
                            // we could be more gentle and postpone connection by one cnt
                            FAILALWAYS(BLINK_3, "rx_available not true");
                        }
                    }
                    connect_state = CONNECT_STATE_CONNECTED;
                    connect_occured_once = true;
                }
                break;
            }
            connect_tmo_cnt = CONNECT_TMO_SYSTICKS;
        }

        // we are connected but tmo ran out
        if (connected() && !connect_tmo_cnt) {
            // so disconnect
            connect_state = CONNECT_STATE_LISTEN;
            // link_state will be set to LINK_STATE_TRANSMIT below
        }

        // we are connected but didn't receive a valid frame
        if (connected() && !valid_frame_received) {
            // reset sync counter, relevant if in sync
            //connect_sync_cnt = 0; //isn't needed, right? since when connected we can't be in sync
        }

        link_state = LINK_STATE_TRANSMIT;
        link_rx1_status = RX_STATUS_NONE;
        link_rx2_status = RX_STATUS_NONE;

        if (!connected()) rarq.Disconnected();

        if (connect_state == CONNECT_STATE_LISTEN) {
            link_task_reset(); // to ensure that the following set is enforced
            link_task_set(LINK_TASK_TX_GET_RX_SETUPDATA);
        }

        DECc(tick_1hz_commensurate, Config.frame_rate_hz);
        if (!tick_1hz_commensurate) {
            stats.Update1Hz();
        }
        stats.Next();
        if (!connected()) stats.Clear();

        if (Setup.Tx[Config.ConfigId].Buzzer == BUZZER_LOST_PACKETS && connect_occured_once && !bind.IsInBind()) {
            if (!valid_frame_received) buzzer.BeepLP();
        }

        // store parameters
        if (doParamsStore) {
            leds.SetToParamStore();
            setup_store_to_EEPROM();
            GOTO_RESTARTCONTROLLER;
        }

        bind.Do();
        switch (bind.Task()) {
        case BIND_TASK_CHANGED_TO_BIND:
            bind.ConfigForBind();
            fhss.SetToBind();
            leds.SetToBind();
            connect_state = CONNECT_STATE_LISTEN;
            // link_state was set to LINK_STATE_TRANSMIT already
            break;
        case BIND_TASK_TX_RESTART_CONTROLLER: GOTO_RESTARTCONTROLLER; break;
        }

//dbg.puts((valid_frame_received) ? "\nvalid" : "\ninval");

        return; // link state might have changed, process immediately
    }//end of if(doPreTransmit)

    //-- Update channels, MBridge handling, Crsf handling, In handling, etc

IF_MBRIDGE(
    // mBridge sends channels in regular 20 ms intervals, this we can use as sync
    if (mbridge.ChannelsUpdated(&rcData)) {
        // update channels, do only if we use mBridge also as channels source
        // note: mBridge is used when either CHANNEL_SOURCE_MBRIDGE or SERIAL_DESTINATION_MBRDIGE, so need to check here
        if (Setup.Tx[Config.ConfigId].ChannelsSource == CHANNEL_SOURCE_MBRIDGE) {
            channelOrder.Set(Setup.Tx[Config.ConfigId].ChannelOrder); //TODO: better than before, but still better place!?
            channelOrder.Apply(&rcData);
            rfpower.Set(&rcData, Setup.Tx[Config.ConfigId].PowerSwitchChannel, Setup.Tx[Config.ConfigId].Power);
        }
        // when we receive channels packet from transmitter, we send link stats to transmitter
        mbridge.TelemetryStart();
    }
    // mBridge sends mBridge cmd twice per 20 ms cycle, so we have 10 ms time to process
    // we can't send too fast, in OTX the receive buffer can hold 64 cmds
    uint8_t mbtask; uint8_t mbcmd;
    if (mbridge.TelemetryUpdate(&mbtask)) {
        switch (mbtask) {
        case TXBRIDGE_SEND_LINK_STATS: mbridge_send_LinkStats(); break;
        case TXBRIDGE_SEND_CMD:
            if (mbridge.CommandInFifo(&mbcmd)) mbridge_send_cmd(mbcmd);
            break;
        }
    }
);
IF_MBRIDGE_OR_CRSF( // to allow CRSF mBridge emulation
    // handle an incoming command
    uint8_t mbcmd;
    if (mbridge.CommandReceived(&mbcmd)) {
        switch (mbcmd) {
        case MBRIDGE_CMD_REQUEST_INFO:
            setup_reload();
            if (connected()) {
                link_task_set(LINK_TASK_TX_GET_RX_SETUPDATA_WRELOAD);
                mbridge.Lock(MBRIDGE_CMD_REQUEST_INFO); // lock mBridge
            } else {
                mbridge.HandleCmd(MBRIDGE_CMD_REQUEST_INFO);
            }
            break;
        case MBRIDGE_CMD_PARAM_REQUEST_LIST: mbridge.HandleCmd(MBRIDGE_CMD_PARAM_REQUEST_LIST); break;
        case MBRIDGE_CMD_REQUEST_CMD: mbridge.HandleRequestCmd(mbridge.GetPayloadPtr()); break;
        case MBRIDGE_CMD_PARAM_SET: {
            bool rx_param_changed;
            bool param_changed = mbridge_do_ParamSet(mbridge.GetPayloadPtr(), &rx_param_changed);
            if (param_changed && rx_param_changed && connected()) {
                link_task_set(LINK_TASK_TX_SET_RX_PARAMS); // set parameter on Rx side
                mbridge.Lock(MBRIDGE_CMD_PARAM_SET); // lock mBridge
            }
            }break;
        case MBRIDGE_CMD_PARAM_STORE:
            if (connected()) {
                link_task_set(LINK_TASK_TX_STORE_RX_PARAMS);
                mbridge.Lock(MBRIDGE_CMD_PARAM_STORE); // lock mBridge
            } else {
                doParamsStore = true;
            }
            break;
        case MBRIDGE_CMD_BIND_START: start_bind(); break;
        case MBRIDGE_CMD_BIND_STOP: stop_bind(); break;
        case MBRIDGE_CMD_SYSTEM_BOOTLOADER: enter_system_bootloader(); break;
        case MBRIDGE_CMD_FLASH_ESP: esp.EnterFlash(); break;
        case MBRIDGE_CMD_MODELID_SET:
//dbg.puts("\nmbridge model id "); dbg.puts(u8toBCD_s(mbridge.GetModelId()));
            config_id.Change(mbridge.GetModelId());
            break;
        }
    }
);
IF_CRSF(
    if (crsf.ChannelsUpdated(&rcData)) {
        // update channels
        channelOrder.Set(Setup.Tx[Config.ConfigId].ChannelOrder); //TODO: better than before, but still better place!?
        channelOrder.Apply(&rcData);
        rfpower.Set(&rcData, Setup.Tx[Config.ConfigId].PowerSwitchChannel, Setup.Tx[Config.ConfigId].Power);
    }
    uint8_t crsftask; uint8_t crsfcmd;
    uint8_t mbcmd; static uint8_t do_cnt = 0; // if it's too fast Lua script gets out of sync
    uint8_t* buf; uint8_t len;
    if (crsf.TelemetryUpdate(&crsftask, Config.frame_rate_ms)) {
        switch (crsftask) {
        case TXCRSF_SEND_LINK_STATISTICS: crsf.SendLinkStatistics(); do_cnt = 0; break;
        case TXCRSF_SEND_LINK_STATISTICS_TX: crsf.SendLinkStatisticsTx(); break;
        case TXCRSF_SEND_LINK_STATISTICS_RX: crsf.SendLinkStatisticsRx(); break;
        case TXCRSF_SEND_TELEMETRY_FRAME:
            if (!do_cnt && mbridge.CommandInFifo(&mbcmd)) {
                mbridge_send_cmd(mbcmd);
            }
            if (mbridge.CrsfFrameAvailable(&buf, &len)) {
                crsf.SendMBridgeFrame(buf, len);
            } else
            if (connected_and_rx_setup_available() &&
                (SERIAL_LINK_MODE_IS_MAVLINK(Setup.Rx.SerialLinkMode) || SERIAL_LINK_MODE_IS_MSP(Setup.Rx.SerialLinkMode))) {
                crsf.SendTelemetryFrame();
            }
            INCc(do_cnt, 3);
            break;
        }
    }
    if (crsf.CommandReceived(&crsfcmd)) {
        switch (crsfcmd) {
        case TXCRSF_CMD_MODELID_SET:
//dbg.puts("\ncrsf model select id "); dbg.puts(u8toBCD_s(crsf.GetCmdModelId()));
            config_id.Change(crsf.GetCmdModelId());
            break;
        case TXCRSF_CMD_MBRIDGE_IN:
//dbg.puts("\ncrsf mbridge ");
            mbridge.ParseCrsfFrame(crsf.GetPayloadPtr(), crsf.GetPayloadLen());
            break;
        }
    }
);
IF_IN(
    if (in.Update(&rcData)) {
        // update channels
        channelOrder.Set(Setup.Tx[Config.ConfigId].ChannelOrder); //TODO: better than before, but still better place!?
        channelOrder.Apply(&rcData);
        rfpower.Set(&rcData, Setup.Tx[Config.ConfigId].PowerSwitchChannel, Setup.Tx[Config.ConfigId].Power);
    }
);

    if (isInTimeGuard || link_state == LINK_STATE_TRANSMIT_SEND) return; // don't do anything else in this time slot, is important!

    //-- Do MAVLink & MSP

    mavlink.Do();
    msp.Do();

    //-- Do WhileTransmit stuff

    whileTransmit.Do();

    //-- Handle display or CLI or MAVLink task

    uint8_t tx_task = disp.Task();
    if (tx_task == TX_TASK_NONE) tx_task = mavlink.Task();
    if (tx_task == TX_TASK_NONE) tx_task = cli.Task();

    switch (tx_task) {
    case TX_TASK_RX_PARAM_SET:
        if (connected()) {
            link_task_set(LINK_TASK_TX_SET_RX_PARAMS);
            mbridge.Lock(); // lock mBridge
        }
        break;
    case TX_TASK_PARAM_STORE:
        if (connected()) {
            link_task_set(LINK_TASK_TX_STORE_RX_PARAMS);
            mbridge.Lock(); // lock mBridge
        } else {
            doParamsStore = true;
        }
        break;
    case TX_TASK_PARAM_RELOAD:
        setup_reload();
        if (connected()) {
            link_task_set(LINK_TASK_TX_GET_RX_SETUPDATA_WRELOAD);
            mbridge.Lock(); // lock mBridge
        }
        break;
    case TX_TASK_BIND: start_bind(); break;
    case TX_TASK_SYSTEM_BOOT: enter_system_bootloader(); break;
    case TX_TASK_FLASH_ESP: esp.EnterFlash(); break;
    case TX_TASK_ESP_PASSTHROUGH: esp.EnterPassthrough(); break;
    case TX_TASK_CLI_CHANGE_CONFIG_ID: config_id.Change(cli.GetTaskValue()); break;
    case TX_TASK_HC04_PASSTHROUGH: hc04.EnterPassthrough(); break;
    case TX_TASK_CLI_HC04_SETPIN: hc04.SetPin(cli.GetTaskValue()); break;
    }

    //-- Handle ESP wifi bridge

    esp.Do();
    uint8_t esp_task = esp.Task();
    if (esp_task == TX_TASK_RESTART_CONTROLLER) { GOTO_RESTARTCONTROLLER; }

    //-- more

    if (config_id.Do()) {
        doParamsStore = true;
    }

}//end of main_loop

