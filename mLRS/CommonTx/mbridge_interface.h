//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// MBridge Interface Header
//********************************************************
#ifndef MBRIDGE_INTERFACE_H
#define MBRIDGE_INTERFACE_H
#pragma once


#ifdef USE_JRPIN5

#include "../Common/libs/fifo.h"
#include "setup_tx.h"
#include "jr_pin5_interface.h"
#include "../Common/protocols/mbridge_protocol.h"


extern uint16_t micros16(void);
extern bool connected(void);
extern uint8_t mavlink_vehicle_state(void);
extern tStats stats;


//-------------------------------------------------------
// Interface Implementation

typedef enum {
    TXBRIDGE_SEND_LINK_STATS = 0,
    TXBRIDGE_SEND_CMD,
} TXMBRIDGE_SEND_ENUM;


class tMBridge : public tPin5BridgeBase, public tSerialBase
{
  public:
    void Init(bool enable_flag, bool crsf_emulation_flag);
    bool ChannelsUpdated(tRcData* const rc);
    bool TelemetryUpdate(uint8_t* const task);

    bool CommandReceived(uint8_t* const cmd);
    uint8_t* GetPayloadPtr(void);
    uint8_t GetModelId(void);
    void SendCommand(uint8_t cmd, uint8_t* const payload);
    bool CommandInFifo(uint8_t* const cmd);
    void Lock(uint8_t cmd);
    void Unlock(void);
    uint8_t HandleRequestCmd(uint8_t* const payload);
    uint8_t HandleCmd(uint8_t cmd);

    void ParseCrsfFrame(uint8_t* const crsf, uint8_t len);
    bool CrsfFrameAvailable(uint8_t** const buf, uint8_t* const len);

    // helper
    void fill_rcdata(tRcData* const rc);

    // for in-isr processing
    void parse_nextchar(uint8_t c) override;
    bool transmit_start(void) override; // returns true if transmission should be started
    uint8_t send_serial(void);
    void send_command(void);

    bool enabled;
    bool crsf_emulation;

    volatile bool channels_received;
    tMBridgeChannelBuffer channels;

    uint8_t cmd_r2m_frame[MBRIDGE_R2M_COMMAND_FRAME_LEN_MAX];
    volatile bool cmd_received;

    volatile bool tx_free; // to signal that the tx buffer can be filled
    uint8_t cmd_m2r_frame[MBRIDGE_M2R_COMMAND_FRAME_LEN_MAX];
    volatile uint8_t cmd_m2r_available;

    // front end to communicate with mBridge
    // mimics a serial interface to the main code
    void putc(char c) { tx_fifo.Put(c); }
    void putbuf(uint8_t* const buf, uint16_t len) { tx_fifo.PutBuf(buf, len); }
    bool available(void) { return rx_fifo.Available(); }
    char getc(void) { return rx_fifo.Get(); }
    void flush(void) { rx_fifo.Flush(); }

    // backend
    // fills/reads the fifos with the mBridge uart
    void serial_putc(char c) { rx_fifo.Put(c); }
    bool serial_rx_available(void) { return tx_fifo.Available(); }
    char serial_getc(void) { return tx_fifo.Get(); }

    tFifo<char,TX_MBRIDGE_TXBUFSIZE> tx_fifo; // TODO: how large do they really need to be?
    tFifo<char,TX_MBRIDGE_RXBUFSIZE> rx_fifo;

    // for communication
    tFifo<uint8_t,128> cmd_fifo; // TODO: how large does it really need to be?
    uint8_t cmd_in_process;
    uint8_t ack_cmd;
    bool ack_ok;
};

tMBridge mbridge;


//-------------------------------------------------------
// MBridge half-duplex interface, used for radio <-> mLRS tx module

// to avoid error: ISO C++ forbids taking the address of a bound member function to form a pointer to member function
void mbridge_pin5_rx_callback(uint8_t c) { mbridge.pin5_rx_callback(c); }
void mbridge_pin5_tc_callback(void) { mbridge.pin5_tc_callback(); }


// is called in isr context
bool tMBridge::transmit_start(void)
{
    if (crsf_emulation) while(1){}; // must not happen

    tx_free = true; // tell external code that next slot can be filled

    if (cmd_m2r_available) {
        send_command(); // uses cmd_m2r_available
        cmd_m2r_available = 0;
        return true;
    }

    if (!serial_rx_available()) { // nothing to do
        return false;
    }

    send_serial();

    return true;
}


// is called in isr context
// we can assume that there is at least one byte available
// send in one chunk to help ensure it is transmitted with no gaps
uint8_t tMBridge::send_serial(void)
{
    uint8_t buf[MBRIDGE_M2R_SERIAL_PAYLOAD_LEN_MAX + 1];
    uint8_t cnt = 0;
    buf[cnt++] = 0x00; // we can send anything we want which is not a command, send 0x00 so it is easy to recognize
    while (serial_rx_available() && cnt < (MBRIDGE_M2R_SERIAL_PAYLOAD_LEN_MAX + 1)) {
        buf[cnt++] = serial_getc();
    }
    pin5_putbuf(buf, cnt);
    return 1;
}


// is called in isr context
void tMBridge::send_command(void)
{
    pin5_putbuf(cmd_m2r_frame, cmd_m2r_available);
}


#define MBRIDGE_TMO_US  250


// is called in isr context, or in ParseCrsfFrame() in case of CRSF emulation
void tMBridge::parse_nextchar(uint8_t c)
{
    uint16_t tnow_us = micros16();

    if (state != STATE_IDLE) {
        uint16_t dt = tnow_us - tlast_us;
        if (dt > MBRIDGE_TMO_US) state = STATE_IDLE; // timeout error
    }

    tlast_us = tnow_us;

    switch (state) {
    case STATE_IDLE:
        if (c == MBRIDGE_STX1) {
            state = STATE_RECEIVE_MBRIDGE_STX2;
#ifdef USE_DEBUG
            if (discarded) {
                if (discarded > 1) {
                    dbg.puts(u16toBCD_s(discarded));
                    dbg.puts(" bytes lost!\n");
                }
                discarded = 0;
            }
        }
        else {
            // Detect discarded bytes
            discarded++;
#endif
        }
        break;

    case STATE_RECEIVE_MBRIDGE_STX2:
        if (c == MBRIDGE_STX2) state = STATE_RECEIVE_MBRIDGE_LEN; else state = STATE_IDLE; // error
        break;
    case STATE_RECEIVE_MBRIDGE_LEN:
        cnt = 0;
        if (c == MBRIDGE_CHANNELPACKET_STX) {
            len = MBRIDGE_CHANNELPACKET_SIZE;
            state = STATE_RECEIVE_MBRIDGE_CHANNELPACKET;
            if (crsf_emulation) state = STATE_IDLE; // we don't allow it // TODO, we could?
        } else
        if (c >= MBRIDGE_COMMANDPACKET_STX) {
            uint8_t cmd = c & (~MBRIDGE_COMMANDPACKET_MASK);
            cmd_r2m_frame[cnt++] = cmd;
            len = mbridge_cmd_payload_len(cmd);
            if (len == 0) {
                cmd_received = true;
                state = STATE_TRANSMIT_START;
            } else {
                state = STATE_RECEIVE_MBRIDGE_COMMANDPACKET;
            }
        } else
        if (c > MBRIDGE_R2M_SERIAL_PAYLOAD_LEN_MAX) {
            state = STATE_IDLE; // error
        } else
        if (c > 0) {
            len = c;
            state = STATE_RECEIVE_MBRIDGE_SERIALPACKET;
            if (crsf_emulation) state = STATE_IDLE; // we don't allow it // TODO, we could?
        } else {
            state = STATE_TRANSMIT_START; // tx_len = 0, no payload
        }
        break;
    case STATE_RECEIVE_MBRIDGE_SERIALPACKET:
        serial_putc(c);
        cnt++;
        if (cnt >= len) state = STATE_TRANSMIT_START;
        break;
    case STATE_RECEIVE_MBRIDGE_CHANNELPACKET:
        channels.c[cnt++] = c;
        if (cnt >= len) {
            channels_received = true;
            state = STATE_TRANSMIT_START;
        }
        break;
    case STATE_RECEIVE_MBRIDGE_COMMANDPACKET:
        cmd_r2m_frame[cnt++] = c;
        if (cnt >= len + 1) {
            cmd_received = true;
            state = STATE_TRANSMIT_START;
        }
        break;
    }
}


//-------------------------------------------------------
// miscellaneous

// mBridge: ch0-15    11 bits, 1 .. 1024 .. 2047 for +-120%
//          ch16-17:  1 bit, 0 .. 1
// rcData:            11 bit, 1 .. 1024 .. 2047 for +-120%

void tMBridge::fill_rcdata(tRcData* const rc)
{
    rc->ch[0] = channels.ch0;
    rc->ch[1] = channels.ch1;
    rc->ch[2] = channels.ch2;
    rc->ch[3] = channels.ch3;
    rc->ch[4] = channels.ch4;
    rc->ch[5] = channels.ch5;
    rc->ch[6] = channels.ch6;
    rc->ch[7] = channels.ch7;
    rc->ch[8] = channels.ch8;
    rc->ch[9] = channels.ch9;
    rc->ch[10] = channels.ch10;
    rc->ch[11] = channels.ch11;
    rc->ch[12] = channels.ch12;
    rc->ch[13] = channels.ch13;
    rc->ch[14] = channels.ch14;
    rc->ch[15] = channels.ch15;
    rc->ch[16] = (channels.ch16) ? 1876 : 172; // +-100%
    rc->ch[17] = (channels.ch17) ? 1876 : 172; // +-100%
}


//-------------------------------------------------------
// CRSF MBridge emulation

void tMBridge::ParseCrsfFrame(uint8_t* const crsf, uint8_t len)
{
    if (!crsf_emulation) return;

// UUUPPPS: state is used also in isr! parse_nextchar() is not reentrant, is synchronized through tx_free variable

    state = STATE_IDLE; // to start the parser, also resets time gap check

    for (uint8_t i = 0; i < len; i++) parse_nextchar(crsf[i]);

    state = STATE_IDLE; // this is to suppress that mBridge sends
    channels_received = false; // this should not have happened, but let's play it safe

    // we should have now a good cmd in cmd_r2m_frame[]
    // mbridge.ChannelsUpdated() should not trigger
    // mbridge.TelemetryUpdate() should not trigger, since mbridge.TelemetryStart() not called
    // mbridge.CommandReceived() should however trigger and should be called"
}


bool tMBridge::CrsfFrameAvailable(uint8_t** const buf, uint8_t* const len)
{
    if (!crsf_emulation) return false;

    if (cmd_m2r_available) {
        *buf = cmd_m2r_frame;
        *len = cmd_m2r_available;
        cmd_m2r_available = 0;
        return true;
    }

    return false;
}


//-------------------------------------------------------
// MBridge user interface

void tMBridge::Init(bool enable_flag, bool crsf_emulation_flag)
{
    enabled = enable_flag;
    crsf_emulation = crsf_emulation_flag;
    if (crsf_emulation) enabled = true;

    if (!enabled) return;

    tx_free = false;
    channels_received = false;
    cmd_received = false;
    cmd_m2r_available = 0;

    tx_fifo.Init();
    rx_fifo.Init();

    cmd_fifo.Init();
    cmd_in_process = 0;

    if (!crsf_emulation) {
        uart_rx_callback_ptr = &mbridge_pin5_rx_callback;
        uart_tc_callback_ptr = &mbridge_pin5_tc_callback;

        tPin5BridgeBase::Init();
        tSerialBase::Init();
    }
}


// polled in main loop
bool tMBridge::ChannelsUpdated(tRcData* const rc)
{
    if (crsf_emulation) return false; // CRSF: just don't ever do it, should not happen

    if (!enabled) return false;

    CheckAndRescue();

    if (!channels_received) return false;
    channels_received = false;

    fill_rcdata(rc);
    return true;
}


// polled in main loop
bool tMBridge::TelemetryUpdate(uint8_t* const task)
{
    if (crsf_emulation) return false; // CRSF: just don't ever do it, should not happen

    if (!enabled) return false;

    // check if we can handle the next slot
    if (!tx_free) return false;
    tx_free = false;

    // check if we should restart telemetry sequence
    if (telemetry_start_next_tick) {
        telemetry_start_next_tick = false;
        telemetry_state = 0;
    }

    // next slot
    uint8_t curr_telemetry_state = telemetry_state;
    telemetry_state++;

    switch (curr_telemetry_state) {
    case 1:
        *task = TXBRIDGE_SEND_LINK_STATS;
        return true;
    case 5: case 9:
        *task = TXBRIDGE_SEND_CMD;
        return true;
    }

    return false;
}


// polled in main loop
bool tMBridge::CommandReceived(uint8_t* const cmd)
{
    if (!enabled) return false;
    if (!cmd_received) return false;

    cmd_received = false;

    *cmd = cmd_r2m_frame[0] & (~MBRIDGE_COMMANDPACKET_MASK);

    return true;
}


uint8_t* tMBridge::GetPayloadPtr(void)
{
    return &(cmd_r2m_frame[1]);
}


uint8_t tMBridge::GetModelId(void)
{
    return cmd_r2m_frame[1];
}


/* void tMBridge::GetCommand(uint8_t* cmd, uint8_t* payload)
{
    *cmd = cmd_r2m_frame[0] & (~MBRIDGE_COMMANDPACKET_MASK);

    uint8_t payload_len = mbridge_cmd_payload_len(*cmd);
    memcpy(payload, &(cmd_r2m_frame[1]), payload_len);
} */


void tMBridge::SendCommand(uint8_t cmd, uint8_t* const payload)
{
    memset(cmd_m2r_frame, 0, MBRIDGE_M2R_COMMAND_FRAME_LEN_MAX);

    uint8_t payload_len = mbridge_cmd_payload_len(cmd);

    cmd_m2r_frame[0] = MBRIDGE_COMMANDPACKET_STX + (cmd & (~MBRIDGE_COMMANDPACKET_MASK));
    memcpy(&(cmd_m2r_frame[1]), payload, payload_len);

    cmd_m2r_available = payload_len + 1;
}


bool tMBridge::CommandInFifo(uint8_t* const cmd)
{
    if (cmd_in_process) return false;

    if (!cmd_fifo.Available()) return false;

    cmd_in_process = 0;

    *cmd = cmd_fifo.Get();

    return true;
}


void tMBridge::Lock(uint8_t cmd = 0xFF)
{
    cmd_in_process = cmd;
}


void tMBridge::Unlock(void)
{
    cmd_in_process = 0;
}


//-------------------------------------------------------
// handler

void mbridge_start_ParamRequestList(void);
void mbridge_start_ParamRequestByIndex(uint8_t idx);


uint8_t tMBridge::HandleRequestCmd(uint8_t* const payload)
{
tMBridgeRequestCmd* request = (tMBridgeRequestCmd*)payload;

    switch (request->cmd_requested) {
    case MBRIDGE_CMD_DEVICE_ITEM_TX:
        cmd_fifo.Put(MBRIDGE_CMD_DEVICE_ITEM_TX);
        break;

    case MBRIDGE_CMD_DEVICE_ITEM_RX:
        cmd_fifo.Put(MBRIDGE_CMD_DEVICE_ITEM_RX);
        break;

    case MBRIDGE_CMD_INFO:
        cmd_fifo.Put(MBRIDGE_CMD_INFO);
        break;

    case MBRIDGE_CMD_REQUEST_INFO:
        cmd_fifo.Put(MBRIDGE_CMD_DEVICE_ITEM_TX);
        cmd_fifo.Put(MBRIDGE_CMD_DEVICE_ITEM_RX);
        cmd_fifo.Put(MBRIDGE_CMD_INFO);
        break;

    case MBRIDGE_CMD_PARAM_REQUEST_LIST:
        mbridge_start_ParamRequestList();
        break;

    case MBRIDGE_CMD_PARAM_ITEM: {
        uint8_t idx = request->param_item.index;
        //if (request->name[0] != 0) { // name is specified, so search for index of parameter
        //}
        mbridge_start_ParamRequestByIndex(idx);
        break; }
    }

    return request->cmd_requested;
}


uint8_t tMBridge::HandleCmd(uint8_t cmd)
{
    // this is somewhat dirty, but does the job :)
    return HandleRequestCmd(&cmd);
}


//-------------------------------------------------------
// convenience helper

void mbridge_send_LinkStats(void)
{
tMBridgeLinkStats lstats = {};

    lstats.LQ_serial = stats.GetLQ_serial(); // = LQ_valid_received; // number of valid packets received on transmitter side
    lstats.rssi1_instantaneous = stats.last_rssi1;
    lstats.rssi2_instantaneous = stats.last_rssi2;
    lstats.snr_instantaneous = stats.GetLastSnr();
    lstats.receive_antenna = stats.last_antenna;
    lstats.transmit_antenna = stats.last_transmit_antenna;
    lstats.rx1_valid = stats.rx1_valid;
    lstats.rx2_valid = stats.rx2_valid;

    lstats.rssi_instantaneous_percent = crsf_cvt_rssi_percent(stats.GetLastRssi(), sx.ReceiverSensitivity_dbm());

    // receiver side of things

    lstats.receiver_LQ_rc = stats.received_LQ_rc; // valid_crc1_received, number of rc data packets received on receiver side
    lstats.receiver_LQ_serial = stats.received_LQ_serial; // valid_frames_received, number of completely valid packets received on receiver side
    lstats.receiver_rssi_instantaneous = stats.received_rssi;
    lstats.receiver_receive_antenna = stats.received_antenna;
    lstats.receiver_transmit_antenna = stats.received_transmit_antenna;

    lstats.receiver_rssi_instantaneous_percent = crsf_cvt_rssi_percent(stats.received_rssi, sx.ReceiverSensitivity_dbm());

    // further stats acquired on transmitter side

    lstats.LQ_fresh_serial_packets_transmitted = stats.serial_data_transmitted.GetLQ();
    lstats.bytes_per_sec_transmitted = stats.GetTransmitBandwidthUsage();

    lstats.LQ_valid_received = stats.valid_frames_received.GetLQ(); // number of completely valid packets received per sec
    lstats.LQ_fresh_serial_packets_received = stats.serial_data_received.GetLQ();
    lstats.bytes_per_sec_received = stats.GetReceiveBandwidthUsage();

    //lstats.__LQ_received = stats.frames_received.GetLQ(); // number of packets received per sec, pretty useless, so deprecated
    lstats.mavlink_packet_LQ_received = stats.GetMavlinkLQ();

    lstats.fhss_curr_i = stats.fhss_curr_i;
    lstats.fhss_cnt = fhss.Cnt();

    lstats.vehicle_state = mavlink_vehicle_state(); // 3 = invalid

    lstats.link_state_connected = connected();
    lstats.link_state_binding = bind.IsInBind();

    mbridge.SendCommand(MBRIDGE_CMD_TX_LINK_STATS, (uint8_t*)&lstats);
}


void mbridge_send_Info(void)
{
tMBridgeInfo info = {};

    info.tx_config_id = Config.ConfigId;

    info.receiver_sensitivity = sx.ReceiverSensitivity_dbm(); // is equal for Tx and Rx
    info.tx_actual_power_dbm = sx.RfPower_dbm();
    info.tx_actual_diversity = Config.Diversity;

    if (SetupMetaData.rx_available) {
        info.rx_available = 1;
        info.rx_actual_power_dbm = SetupMetaData.rx_actual_power_dbm;
        info.rx_actual_diversity = SetupMetaData.rx_actual_diversity;
    } else {
        info.rx_available = 0;
        info.rx_actual_power_dbm = INT8_MAX; // INT8_MAX = invalid
        info.rx_actual_diversity = DIVERSITY_NUM; // 5 = invalid
    }

    info.has_status = 1; // to indicate it has these flags
    info.binding = (bind.IsInBind()) ? 1 : 0;
    info.connected = (connected()) ? 1 : 0;
    info.rx_LQ_low = (stats.received_LQ_rc < 65) ? 1 : 0;
    info.tx_LQ_low = (stats.GetLQ_serial() < 65) ? 1 : 0;

    info.param_num = SETUP_PARAMETER_NUM; // known if non-zero

    mbridge.SendCommand(MBRIDGE_CMD_INFO, (uint8_t*)&info);
}


void mbridge_send_DeviceItemTx(void)
{
tMBridgeDeviceItem item = {};

    item.firmware_version_u16 = version_to_u16(VERSION);
    item.setup_layout_u16 = version_to_u16(SETUPLAYOUT);
    strbufstrcpy(item.device_name_20, DEVICE_NAME, 20);
    mbridge.SendCommand(MBRIDGE_CMD_DEVICE_ITEM_TX, (uint8_t*)&item);
}


void mbridge_send_DeviceItemRx(void)
{
tMBridgeDeviceItem item = {};

    if (SetupMetaData.rx_available) {
        item.firmware_version_u16 = version_to_u16(SetupMetaData.rx_firmware_version);
        item.setup_layout_u16 = version_to_u16(SetupMetaData.rx_setup_layout);
        strbufstrcpy(item.device_name_20, SetupMetaData.rx_device_name, 20);
    } else {
        item.firmware_version_u16 = 0;
        item.setup_layout_u16 = 0;
        strbufstrcpy(item.device_name_20, "", 20);
    }
    mbridge.SendCommand(MBRIDGE_CMD_DEVICE_ITEM_RX, (uint8_t*)&item);
}


uint8_t param_idx; // next param index to send
uint8_t param_itemtype_to_send; // count through sending PARAM_ITEM, PARAM_ITEM2, PARAM_ITEM3
bool param_by_index; // to indicate sending requested by list or by index
char param_optstr[96]; // is currently limited to 67 max


// we have to send (much) more than SETUP_PARAMETER_NUM PARAM_ITEM messages
// since all parameters need 2 and some even 3 or 4 of them
// currently it are about 80 for the 36 parameters => 80 x 20ms = 1600 ms

// shorten parameter's option string, as follows:
// - each not allowed option is replaced by a '-'
// - keep however option for the current setting (this handles allowed mask = 0)
void param_get_opt_shortened_str(char* const out, uint8_t param_idx)
{
    const char* optstr = SetupParameter[param_idx].optstr;
    uint16_t allowed_mask = param_get_allowed_mask(param_idx);

    if (SetupParameter[param_idx].type != SETUP_PARAM_TYPE_LIST || allowed_mask == UINT16_MAX) {
        strcpy(out, optstr);
        return;
    }

    uint8_t val = *(int8_t*)SetupParameterPtr(param_idx);

    // we have something like "50 Hz,31 Hz,19 Hz,FLRC,FSK"
    uint8_t out_pos = 0;
    char s[24];
    uint8_t pos = 0;
    uint8_t opt_i = 0;
    for (uint8_t n = 0; n < strlen(optstr) + 1; n++) {
        s[pos++] = optstr[n];
        if (optstr[n] == ',' || optstr[n] == '\0') {
            if (opt_i == val || allowed_mask & (1 << opt_i)) { // is current selection or is allowed option, keep it
                for (uint8_t i = 0; i < pos; i++) out[out_pos++] = s[i];
            } else {
                out[out_pos++] = '-';
                out[out_pos++] = optstr[n]; // finish with ',' or '\0'
            }
            opt_i++;
            pos = 0;
            if (out_pos > 80) while(1){} // must not happen
        }
    }
/*
dbg.puts("\nparam   ");dbg.puts(SetupParameter[param_idx].name);
dbg.puts("\n  idx   ");dbg.puts(u8toBCD_s(param_idx));
dbg.puts("\n  opt   ");dbg.puts(optstr);
dbg.puts("\n  mask x");dbg.puts(u16toHEX_s(allowed_mask));
dbg.puts("\n  val   ");dbg.puts(u8toBCD_s(val));
dbg.puts("\n->      ");dbg.puts(out);*/
}


void mbridge_start_ParamRequestByIndex(uint8_t idx)
{
    param_idx = idx;
    param_itemtype_to_send = 0;
    param_by_index = true;

    mbridge.cmd_fifo.Put(MBRIDGE_CMD_PARAM_ITEM); // trigger sending out
}


void mbridge_start_ParamRequestList(void)
{
    param_idx = 0;
    param_itemtype_to_send = 0;
    param_by_index = false;

    mbridge.cmd_fifo.Put(MBRIDGE_CMD_PARAM_ITEM); // trigger sending out first
}


void mbridge_send_ParamItem(void)
{
    if (param_idx >= SETUP_PARAMETER_NUM) {
        // we send a mBridge message, but don't put a MBRIDGE_CMD_PARAM_ITEM into the fifo, this stops it
        tMBridgeParamItem item = {};
        item.index = UINT8_MAX; // indicates end of list
        mbridge.SendCommand(MBRIDGE_CMD_PARAM_ITEM, (uint8_t*)&item);
        return;
    }

    bool item3_needed = false; // if a LIST parameter has a long option string, we send a 3rd or 4th ParamItem

    if (param_itemtype_to_send == 0) {
        tMBridgeParamItem item = {};
        item.index = param_idx;
        switch (SetupParameter[param_idx].type) {
        case SETUP_PARAM_TYPE_INT8:
            item.type = MBRIDGE_PARAM_TYPE_INT8;
            item.value.i8 = *(int8_t*)SetupParameterPtr(param_idx);
            break;
        case SETUP_PARAM_TYPE_LIST:
            item.type = MBRIDGE_PARAM_TYPE_LIST;
            item.value.u8 = *(uint8_t*)SetupParameterPtr(param_idx);
            break;
        case SETUP_PARAM_TYPE_STR6:
            item.type = MBRIDGE_PARAM_TYPE_STR6;
            strbufstrcpy(item.str6_6, (char*)SetupParameterPtr(param_idx), 6);
            break;
        }
        strbufstrcpy(item.name_16, SetupParameter[param_idx].name, 16);

        mbridge.SendCommand(MBRIDGE_CMD_PARAM_ITEM, (uint8_t*)&item);

        param_itemtype_to_send = 1; // send the 2nd ParamItem in the next call

        param_get_opt_shortened_str(param_optstr, param_idx); // set it for the next items

    } else
    if (param_itemtype_to_send == 1) {
        tMBridgeParamItem2 item2 = {};
        item2.index = param_idx;
        switch (SetupParameter[param_idx].type) {
        case SETUP_PARAM_TYPE_INT8:
            item2.dflt.i8 = SetupParameter[param_idx].dflt.INT8_value;
            item2.min.i8 = SetupParameter[param_idx].min.INT8_value;
            item2.max.i8 = SetupParameter[param_idx].max.INT8_value;
            strbufstrcpy(item2.unit_6, SetupParameter[param_idx].unit, 6);
            break;
        case SETUP_PARAM_TYPE_LIST:
            if (SetupParameter[param_idx].allowed_mask_ptr != nullptr) {
                item2.allowed_mask = *SetupParameter[param_idx].allowed_mask_ptr;
            } else {
                item2.allowed_mask = UINT16_MAX;
            }
            strbufstrcpy(item2.options_21, param_optstr, 21);
            if (strlen(param_optstr) >= 21) item3_needed = true;
            break;
        }

        mbridge.SendCommand(MBRIDGE_CMD_PARAM_ITEM2, (uint8_t*)&item2);

        if (item3_needed) {
            param_itemtype_to_send = 2; // we need to send a 3rd ParamItem
        } else {
            // next param item
            param_itemtype_to_send = 0; // done with this parameter
            param_idx++;

            if (param_by_index) return; // if requested by index we stop
        }
    } else
    if (param_itemtype_to_send == 2) {
        tMBridgeParamItem3 item3 = {};
        item3.index = param_idx;
        strbufstrcpy(item3.options2_23, param_optstr + 21, 23);
        if (strlen(param_optstr) >= 21+23) item3_needed = true; // we need yet another one

        mbridge.SendCommand(MBRIDGE_CMD_PARAM_ITEM3, (uint8_t*)&item3);

        if (item3_needed) {
            param_itemtype_to_send = 3; // we need to send a 4th ParamItem
        } else {
            // next param item
            param_itemtype_to_send = 0; // done with this parameter
            param_idx++;

            if (param_by_index) return; // if requested by index we stop
        }

    } else
    if (param_itemtype_to_send >= 3) {
        tMBridgeParamItem3 item3 = {};
        item3.index = param_idx;
        strbufstrcpy(item3.options2_23, param_optstr + 21 + 23, 23);

        // we would have to match MAVLink4OpenTx code
        // to avoid this let's play foul: set highest bit of index
        item3.index += 128;
        mbridge.SendCommand(MBRIDGE_CMD_PARAM_ITEM3, (uint8_t*)&item3);

        // next param item
        param_itemtype_to_send = 0; // done with this parameter
        param_idx++;

        if (param_by_index) return; // if requested by index we stop
    }

    mbridge.cmd_fifo.Put(MBRIDGE_CMD_PARAM_ITEM); // trigger sending out next
}


bool mbridge_do_ParamSet(uint8_t* payload, bool* rx_param_changed)
{
tMBridgeParamSet* param = (tMBridgeParamSet*)payload;

    *rx_param_changed = false;

    if (param->index >= SETUP_PARAMETER_NUM) return false;

    if (SetupParameter[param->index].type <= SETUP_PARAM_TYPE_LIST) {
        *rx_param_changed = setup_set_param(param->index, param->value);
        return true;
    }

    if (SetupParameter[param->index].type == SETUP_PARAM_TYPE_STR6) {
        *rx_param_changed = setup_set_param_str6(param->index, param->str6_6);
        return true;
    }

    return false;
}


void mbridge_send_cmd(uint8_t cmd)
{
    switch (cmd) {
    case MBRIDGE_CMD_DEVICE_ITEM_TX:
        mbridge_send_DeviceItemTx();
        break;
    case MBRIDGE_CMD_DEVICE_ITEM_RX:
        mbridge_send_DeviceItemRx();
        break;
    case MBRIDGE_CMD_PARAM_ITEM:
        mbridge_send_ParamItem();
        break;
    case MBRIDGE_CMD_INFO:
        mbridge_send_Info();
        break;
    }
}


#else

class tMBridge : public tSerialBase
{
  public:
    void Init(bool enable_flag, bool crsf_emulation_flag) {}
    void TelemetryStart(void) {}
    void TelemetryTick_ms(void) {}
    void Lock(void) {}
    void Unlock(void) {}
};

tMBridge mbridge;

#endif // ifdef USE_JRPIN5

#endif // MBRIDGE_INTERFACE_H
