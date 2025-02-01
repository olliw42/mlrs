//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// CRSF Interface TX Side
//*******************************************************
#ifndef CRSF_INTERFACE_TX_H
#define CRSF_INTERFACE_TX_H
#pragma once


#include "../Common/protocols/msp_protocol.h"


#ifdef USE_JRPIN5

#include "math.h"
#include "../Common/thirdparty/thirdparty.h"
#include "../Common/protocols/crsf_protocol.h"
#include "../Common/protocols/passthrough_protocol.h"
#include "../Common/protocols/ardupilot_protocol.h"
#include "jr_pin5_interface.h"


extern uint16_t micros16(void);
extern volatile uint32_t millis32(void);
extern tStats stats;


//-------------------------------------------------------
// Interface Implementation

typedef enum {
    TXCRSF_SEND_LINK_STATISTICS = 0,
    TXCRSF_SEND_LINK_STATISTICS_TX,
    TXCRSF_SEND_LINK_STATISTICS_RX,
    TXCRSF_SEND_TELEMETRY_FRAME, // native or passthrough telemetry frame
} TXCRSF_SEND_ENUM;


typedef enum {
    TXCRSF_CMD_MODELID_SET = 0,
    TXCRSF_CMD_MBRIDGE_IN,
} TXCRSF_CMD_ENUM;


class tTxCrsf : public tPin5BridgeBase
{
  public:
    void Init(bool enable_flag);
    bool ChannelsUpdated(tRcData* const rc);
    bool TelemetryUpdate(uint8_t* const task, uint16_t frame_rate_ms);

    bool CommandReceived(uint8_t* const cmd);
    uint8_t* GetPayloadPtr(void);
    uint8_t GetPayloadLen(void);
    uint8_t GetCmdModelId(void);

    void TelemetryHandleMavlinkMsg(fmav_message_t* const msg);
    void TelemetryHandleMspMsg(msp_message_t* const msg);
    void SendTelemetryFrame(void);

    void SendLinkStatistics(void); // in OpenTx this triggers telemetryStreaming
    void SendLinkStatisticsTx(void);
    void SendLinkStatisticsRx(void);

    void SendMBridgeFrame(void* const payload, uint8_t payload_len);

    // helper
    void send_frame(const uint8_t frame_id, void* const payload, uint8_t payload_len);

    uint8_t crc8(const uint8_t* const buf);
    void fill_rcdata(tRcData* const rc);

    // for in-isr processing, used in half-duplex mode
    void parse_nextchar(uint8_t c) override;
    bool transmit_start(void) override; // returns true if transmission should be started

    bool enabled;

    uint8_t frame[CRSF_FRAME_LEN_MAX + 16];
    volatile bool channels_received;
    volatile bool cmd_received;
    volatile bool cmd_modelid_received; // we handle it extra just to really catch it, could do also cmd fifo
    volatile uint8_t cmd_modelid_value;

    volatile bool tx_free; // to signal that the tx buffer can be filled
    uint8_t tx_frame[CRSF_FRAME_LEN_MAX + 16];
    volatile uint8_t tx_available; // this signals if something needs to be send to radio

    // CRSF telemetry

    tCrsfFlightMode flightmode; // collected from HEARTBEAT
    bool flightmode_updated;
    uint32_t flightmode_send_tlast_ms;

    tCrsfBattery battery; // collected from BATTERY_STATUS
    bool battery_updated;
    uint32_t battery_send_tlast_ms;

    tCrsfAttitude attitude; // collected from ATTITUDE
    bool attitude_updated;
    uint32_t attitude_send_tlast_ms;

    uint8_t gps_raw_int_sat;
    uint8_t gps2_raw_sat;
    float vfr_hud_groundspd_mps;
    tCrsfGps gps; // collected from several MAVLink messages: GPS_RAW_INT, GPS2_RAW, VFR_HUD, GLOBAL_POSITION_INT
    bool gps_updated;
    uint32_t gps_send_tlast_ms;

    tCrsfVario vario; // collected from VFR_HUD
    bool vario_updated;
    uint32_t vario_send_tlast_ms;

    tCrsfBaroAltitude baro_altitude; // not yet populated from a MAVLink message, AP does not appear to provide baro alt at all
    bool baro_altitude_updated;
    uint32_t baro_send_tlast_ms;

    // MAVLink handlers

    void handle_mavlink_msg_heartbeat(fmav_heartbeat_t* const payload);
    void handle_mavlink_msg_battery_status(fmav_battery_status_t* const payload);
    void handle_mavlink_msg_attitude(fmav_attitude_t* const payload);
    void handle_mavlink_msg_gps_raw_int(fmav_gps_raw_int_t* const payload);
    void handle_mavlink_msg_gps2_raw(fmav_gps2_raw_t* const payload);
    void handle_mavlink_msg_global_position_int(fmav_global_position_int_t* const payload);
    void handle_mavlink_msg_vfr_hud(fmav_vfr_hud_t* const payload);

    uint8_t vehicle_sysid;

    // CRSF passthrough telemetry

    tPassThrough passthrough;

    // MSP handlers

    int32_t inav_baro_altitude; // needed to make INAV happy
    uint16_t msp_inav_status_sensor_status;
    uint32_t msp_inav_status_arming_flags;
};

tTxCrsf crsf;


//-------------------------------------------------------
// CRSF half-duplex interface, used for radio <-> mLRS tx module

// to avoid error: ISO C++ forbids taking the address of a bound member function to form a pointer to member function
void crsf_pin5_rx_callback(uint8_t c) { crsf.pin5_rx_callback(c); }
void crsf_pin5_tc_callback(void) { crsf.pin5_tc_callback(); }


// is called in isr context
bool tTxCrsf::transmit_start(void)
{
    tx_free = true; // tell external code that tx_frame can be filled with new data

    if (!tx_available) { // nothing to send
        return false;
    }

    pin5_putbuf(tx_frame, tx_available);

    tx_available = 0;

    return true;
}


// a frame is sent every 4 ms, frame length is max 64 bytes
// a byte is 25 us
// gaps between frames are 3 ms or so
#define CRSF_PARSE_NEXTCHAR_TMO_US  500


// CRSF frame format:
// address len type payload crc
// len is the length including type, payload, crc

// is called in isr context
void tTxCrsf::parse_nextchar(uint8_t c)
{
    uint16_t tnow_us = micros16();

    if (state != STATE_IDLE) {
        uint16_t dt = tnow_us - tlast_us;
        if (dt > CRSF_PARSE_NEXTCHAR_TMO_US) state = STATE_IDLE;

        if (cnt >= sizeof(frame)) state = STATE_IDLE; // prevent buffer overflow
    }

    tlast_us = tnow_us;

    switch (state) {
    case STATE_IDLE:
        if ((c == CRSF_ADDRESS_TRANSMITTER_MODULE) || (c == CRSF_OPENTX_SYNC)) {
            cnt = 0;
            frame[cnt++] = c;
            state = STATE_RECEIVE_CRSF_LEN;
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

    case STATE_RECEIVE_CRSF_LEN:
        frame[cnt++] = c;
        len = c;
        state = STATE_RECEIVE_CRSF_PAYLOAD;
        break;
    case STATE_RECEIVE_CRSF_PAYLOAD:
        frame[cnt++] = c;
        if (cnt >= len + 1) {
            state = STATE_RECEIVE_CRSF_CRC;
        }
        break;
    case STATE_RECEIVE_CRSF_CRC:
        frame[cnt++] = c;
        // let's ignore the crc here
        // this is called in isr, so we want to do crc check later, if we want to do it all
        if (frame[2] == CRSF_FRAME_ID_RC_CHANNELS) { // frame_id
            channels_received = true;
        } else
        if (frame[0] == CRSF_OPENTX_SYNC && frame[2] == CRSF_FRAME_ID_COMMAND &&
            frame[5] == CRSF_COMMAND_ID && frame[6] == CRSF_COMMAND_SET_MODEL_SELECTION) {
            cmd_modelid_received = true;
            cmd_modelid_value = frame[7];
        } else {
            cmd_received = true;
        }
        state = STATE_TRANSMIT_START;
        break;
    }
}


//-------------------------------------------------------
// miscellaneous

// CRSF:
// 11 bit, 173 ... 992 .. 1811 for +-100%
// so: 9 ... 173 ... 992 .. 1811 ... 1965  for -120%  -100%    0%    +100%    +120%
// 100% = 819 span
// 120% = 983 span
// rcData: 11 bits,  1 .. 1024 .. 2047 for +-120%
// see design_decissions.h

void tTxCrsf::fill_rcdata(tRcData* const rc)
{
tCrsfRcChannelBuffer buf;

    memcpy(buf.c, &(frame[3]), CRSF_RCCHANNELPACKET_SIZE);
    rc->ch[0] = rc_from_crsf(buf.ch0);
    rc->ch[1] = rc_from_crsf(buf.ch1);
    rc->ch[2] = rc_from_crsf(buf.ch2);
    rc->ch[3] = rc_from_crsf(buf.ch3);
    rc->ch[4] = rc_from_crsf(buf.ch4);
    rc->ch[5] = rc_from_crsf(buf.ch5);
    rc->ch[6] = rc_from_crsf(buf.ch6);
    rc->ch[7] = rc_from_crsf(buf.ch7);
    rc->ch[8] = rc_from_crsf(buf.ch8);
    rc->ch[9] = rc_from_crsf(buf.ch9);
    rc->ch[10] = rc_from_crsf(buf.ch10);
    rc->ch[11] = rc_from_crsf(buf.ch11);
    rc->ch[12] = rc_from_crsf(buf.ch12);
    rc->ch[13] = rc_from_crsf(buf.ch13);
    rc->ch[14] = rc_from_crsf(buf.ch14);
    rc->ch[15] = rc_from_crsf(buf.ch15);
}


uint8_t tTxCrsf::crc8(const uint8_t* const buf)
{
    return crsf_crc8_update(CRSF_CRC8_INIT, &(buf[2]), buf[1] - 1);
}


//-------------------------------------------------------
// CRSF user interface

void tTxCrsf::Init(bool enable_flag)
{
    enabled = enable_flag;

    if (!enabled) return;

    tx_available = 0;
    tx_free = false;
    channels_received = false;
    cmd_received = false;
    cmd_modelid_received = false;

    flightmode_updated = false;
    flightmode_send_tlast_ms = 0;
    battery_updated = false;
    battery_send_tlast_ms = 0;
    attitude_updated = false;
    attitude_send_tlast_ms = 0;
    gps_raw_int_sat = UINT8_MAX; // unknown
    gps2_raw_sat = UINT8_MAX; // unknown
    vfr_hud_groundspd_mps = NAN; // unknown
    gps_updated = false;
    gps_send_tlast_ms = 0;
    vario_updated = false;
    vario_send_tlast_ms = 0;
    baro_altitude_updated = false;
    baro_send_tlast_ms = 0;

    vehicle_sysid = 0;
    passthrough.Init();

    inav_baro_altitude = 0;
    msp_inav_status_sensor_status = 0;
    msp_inav_status_arming_flags = 0;

    uart_rx_callback_ptr = &crsf_pin5_rx_callback;
    uart_tc_callback_ptr = &crsf_pin5_tc_callback;

    tPin5BridgeBase::Init();
}


// polled in main loop
bool tTxCrsf::ChannelsUpdated(tRcData* const rc)
{
    if (!enabled) return false;

    CheckAndRescue();

    if (!channels_received) return false;
    channels_received = false;

    // check crc before we accept it
    uint8_t crc = crc8(frame);
    if (crc != frame[frame[1] + 1]) return false;

    fill_rcdata(rc);
    return true;
}


// polled in main loop
bool tTxCrsf::TelemetryUpdate(uint8_t* const task, uint16_t frame_rate_ms)
{
    if (!enabled) return false;

    // check if we can transmit
    if (!tx_free) return false;
    tx_free = false;

    // check if we should restart telemetry sequence
    if (telemetry_start_next_tick) {
        telemetry_start_next_tick = false;

        // slow it down if frame time is too short
        if (frame_rate_ms <= 19) {
            static uint8_t cnt = 0;
            if (!cnt) telemetry_state = 0;
            cnt++;
            if (cnt > 2) cnt = 0;
        } else {
            telemetry_state = 0;
        }
    }

    // next slot
    uint8_t curr_telemetry_state = telemetry_state;
    telemetry_state++;

    // now determine what to transmit
    // frame rate is
    //   20 ms -> ca 5  = 3  + 2
    //   32 ms -> ca 8  = 3 + 5
    //   53 ms -> ca 13 = 3 + 10
    //   7 ms  -> 3x = 21 ms -> ca 5 = 3 + 2

    switch (curr_telemetry_state) {
        case 0: *task = TXCRSF_SEND_LINK_STATISTICS; return true;
        case 1: *task = TXCRSF_SEND_LINK_STATISTICS_TX; return true;
        case 2: *task = TXCRSF_SEND_LINK_STATISTICS_RX; return true;
    }

    *task = TXCRSF_SEND_TELEMETRY_FRAME;
    return true;
}


// polled in main loop
bool tTxCrsf::CommandReceived(uint8_t* const cmd)
{
    if (!enabled) return false;

    if (cmd_modelid_received) {
        cmd_modelid_received = false;
        *cmd = TXCRSF_CMD_MODELID_SET;
        return true;
    }

    if (!cmd_received) return false;
    cmd_received = false;

    // TODO: we could check crc if we wanted to

    tCrsfFrameHeader* header = (tCrsfFrameHeader*)frame;

    // mBridge emulation
    if (header->address == CRSF_ADDRESS_TRANSMITTER_MODULE &&
        header->frame_id == CRSF_FRAME_ID_MBRIDGE_TO_MODULE) {
        *cmd = TXCRSF_CMD_MBRIDGE_IN;
        return true;
    }

    return false;
}


uint8_t* tTxCrsf::GetPayloadPtr(void)
{
    return ((tCrsfFrameHeader*)frame)->payload;
}


uint8_t tTxCrsf::GetPayloadLen(void)
{
    return ((tCrsfFrameHeader*)frame)->len - 2;
}


uint8_t tTxCrsf::GetCmdModelId(void)
{
    return cmd_modelid_value;
}


void tTxCrsf::SendMBridgeFrame(void* const payload, uint8_t payload_len)
{
    send_frame(CRSF_FRAME_ID_MBRIDGE_TO_RADIO, payload, payload_len);
}


//-------------------------------------------------------
// helper

void tTxCrsf::send_frame(const uint8_t frame_id, void* const payload, uint8_t payload_len)
{
    tx_frame[0] = CRSF_ADDRESS_RADIO; // correct? OpenTx accepts CRSF_ADDRESS_RADIO or CRSF_OPENTX_SYNC, so correct
    tx_frame[1] = (4-2) + payload_len;
    tx_frame[2] = frame_id;
    memcpy(&(tx_frame[3]), payload, payload_len);
    tx_frame[3 + payload_len] = crc8(tx_frame);

    tx_available = 4 + payload_len;
}


//-------------------------------------------------------
// CRSF Telemetry Handler

// called in main loop, when crsf.TelemetryUpdate() true
void tTxCrsf::SendTelemetryFrame(void)
{
    // native CRSF

    uint32_t tnow_ms = millis32();

    #define CRSF_REFRESH_TIME_MS  2500 // what is actually a proper value ??

    // auto update to prevent OTX telemetry/sensor lost message, do only if at least once seen
    if (flightmode_send_tlast_ms && (tnow_ms - flightmode_send_tlast_ms) > CRSF_REFRESH_TIME_MS) flightmode_updated = true;
    if (battery_send_tlast_ms && (tnow_ms - battery_send_tlast_ms) > CRSF_REFRESH_TIME_MS) battery_updated = true;
    if (gps_send_tlast_ms && (tnow_ms - gps_send_tlast_ms) > CRSF_REFRESH_TIME_MS) gps_updated = true;
    if (vario_send_tlast_ms && (tnow_ms - vario_send_tlast_ms) > CRSF_REFRESH_TIME_MS) vario_updated = true;
    if (attitude_send_tlast_ms && (tnow_ms - attitude_send_tlast_ms) > CRSF_REFRESH_TIME_MS) attitude_updated = true;
    if (baro_send_tlast_ms && (tnow_ms - baro_send_tlast_ms) > CRSF_REFRESH_TIME_MS) baro_altitude_updated = true;

    if (flightmode_updated) {
        flightmode_updated = false;
        flightmode_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_FLIGHT_MODE, &flightmode, CRSF_FLIGHTMODE_LEN);
        return; // only send one per slot
    }
    if (battery_updated) {
        battery_updated = false;
        battery_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_BATTERY, &battery, CRSF_BATTERY_LEN);
        return; // only send one per slot
    }
    if (gps_updated) {
        gps_updated = false;
        gps_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_GPS, &gps, CRSF_GPS_LEN);
        return; // only send one per slot
    }
    if (vario_updated) {
        vario_updated = false;
        vario_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_VARIO, &vario, CRSF_VARIO_LEN);
        return; // only send one per slot
    }
    if (attitude_updated) {
        attitude_updated = false;
        attitude_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_ATTITUDE, &attitude, CRSF_ATTITUDE_LEN);
        return; // only send one per slot
    }
    if (baro_altitude_updated) {
        baro_altitude_updated = false;
        baro_send_tlast_ms = tnow_ms;
        send_frame(CRSF_FRAME_ID_BARO_ALTITUDE, &baro_altitude, CRSF_BARO_ALTITUDE_LEN);
        return; // only send one per slot
    }

    // passthrough

    uint8_t data[64+10];
    uint8_t len;

    if (passthrough.GetTelemetryFrameMulti(data, &len)) {
        send_frame(CRSF_FRAME_ID_AP_CUSTOM_TELEM, data, len);
        return;
    }
}


//-------------------------------------------------------
// CRSF Telemetry Mavlink Handling
// we have to kinds to consider:
// - native CRSF telemetry frames:
//   these are filled from MAVLink messages by the tTxCrsf class
// - passthrough packets which are packed into CRSF passthrough telemetry frames:
//   these are filled from MAVLink messages through the tPassThrough class

#define CRSF_REV_U16(x)  __REV16(x)
#define CRSF_REV_I16(x)  __REVSH(x)
#define CRSF_REV_U32(x)  __REV(x)


void tTxCrsf::handle_mavlink_msg_heartbeat(fmav_heartbeat_t* const payload)
{
    memset(flightmode.flight_mode, 0, sizeof(flightmode.flight_mode));

    if (payload->autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA) {
        ap_flight_mode_name4(flightmode.flight_mode, ap_vehicle_from_mavtype(payload->type), payload->custom_mode);

        if ((payload->base_mode & MAV_MODE_FLAG_SAFETY_ARMED) == 0) {
            // if (flightmode.flight_mode[3] == ' ') flightmode.flight_mode[3] = '\0';
            strcat(flightmode.flight_mode, "*");
        }
    }

    flightmode_updated = true;
}


int32_t mav_battery_voltage(fmav_battery_status_t* payload)
{
    int32_t voltage = 0;
    for (uint8_t i = 0; i < 10; i++) {
        if (payload->voltages[i] != UINT16_MAX) {
            voltage += payload->voltages[i]; // uint16_t mV, UINT16_MAX if not known
        }
    }
    for (uint8_t i = 0; i < 4; i++) { // we assume this never is relevant if validcellcount = false
        if (payload->voltages_ext[i] != 0) {
            voltage += payload->voltages_ext[i]; // uint16_t mV, 0 if not known
        }
    }
    return voltage;
}


void tTxCrsf::handle_mavlink_msg_battery_status(fmav_battery_status_t* const payload)
{
    if (payload->id != 0) return;

    battery.voltage = CRSF_REV_U16(mav_battery_voltage(payload) / 100);
    battery.current = CRSF_REV_U16((payload->current_battery == -1) ? 0 : payload->current_battery / 10); // CRSF is in 0.1 A, MAVLink is in 0.01 A
    uint32_t capacity = (payload->current_consumed == -1) ? 0 : payload->current_consumed;
    if (capacity > 8388607) capacity = 8388607; // int 24 bit
    battery.capacity[0] = (capacity >> 16);
    battery.capacity[1] = (capacity >> 8);
    battery.capacity[2] = capacity;
    battery.remaining = (payload->battery_remaining == -1) ? 0 : payload->battery_remaining;
    battery_updated = true;
}


void tTxCrsf::handle_mavlink_msg_attitude(fmav_attitude_t* const payload)
{
    attitude.pitch = CRSF_REV_I16(10000.0f * payload->pitch);
    attitude.roll = CRSF_REV_I16(10000.0f * payload->roll);
    attitude.yaw = CRSF_REV_I16(10000.0f * payload->yaw);
    attitude_updated = true;
}


void tTxCrsf::handle_mavlink_msg_gps_raw_int(fmav_gps_raw_int_t* const payload)
{
    gps_raw_int_sat = payload->satellites_visible;
}


void tTxCrsf::handle_mavlink_msg_gps2_raw(fmav_gps2_raw_t* const payload)
{
    gps2_raw_sat = payload->satellites_visible;
}


void tTxCrsf::handle_mavlink_msg_global_position_int(fmav_global_position_int_t* const payload)
{
    gps.latitude = CRSF_REV_U32(payload->lat);
    gps.longitude = CRSF_REV_U32(payload->lon);
    int32_t alt = payload->alt / 1000 + 1000;
    if (alt < 0) alt = 0;
    if (alt > UINT16_MAX) alt = UINT16_MAX;
    gps.altitude = CRSF_REV_U16(alt);
    gps.gps_heading = CRSF_REV_U16(payload->hdg);

    // take the ground speed from VFR_HUD
    if (vfr_hud_groundspd_mps != NAN) {
        gps.groundspeed = CRSF_REV_U16(100.0f * vfr_hud_groundspd_mps / 3.6f);
    } else {
        gps.groundspeed = 0;
    }

    // take the satellites of the previous reports
    if (gps_raw_int_sat != UINT8_MAX && gps2_raw_sat != UINT8_MAX) { // we have two gps
        gps.satellites = (gps_raw_int_sat > gps2_raw_sat) ? gps_raw_int_sat : gps2_raw_sat; // take the larger
    } else
    if (gps_raw_int_sat != UINT8_MAX) {
        gps.satellites = gps_raw_int_sat;
    } else {
        gps.satellites = 0;
    }

    // mark as updated
    gps_updated = true;
}


void tTxCrsf::handle_mavlink_msg_vfr_hud(fmav_vfr_hud_t* const payload)
{
    vfr_hud_groundspd_mps = payload->groundspeed;

    vario.climb_rate = CRSF_REV_I16(100.0f * payload->climb);
    vario_updated = true;
}


// called by MAVLink interface, when a MAVLink frame has been received
void tTxCrsf::TelemetryHandleMavlinkMsg(fmav_message_t* const msg)
{
    if (msg->sysid == 0) return; // this can't be anything meaningful

    // autodetect vehicle sysid
/* we don't really need this for as long as we can assume that there is only one autopilot, so play it simple
    if (!vehicle_sysid) {
        if (msg->msgid == FASTMAVLINK_MSG_ID_HEARTBEAT) {
            fmav_heartbeat_t payload;
            fmav_msg_heartbeat_decode(&payload, msg);
            if ((msg->compid == MAV_COMP_ID_AUTOPILOT1) || (payload.autopilot != MAV_AUTOPILOT_INVALID)) {
                vehicle_sysid = msg->sysid;
            }
        }
        if (!vehicle_sysid) return;
    }
    if (msg->sysid != vehicle_sysid) return;
*/

    if (msg->compid != MAV_COMP_ID_AUTOPILOT1) return;

    // from here on we only see the MAVLink messages from our vehicle

    switch (msg->msgid) {

    // these are for CRSF telemetry, some are also for passthrough

    case FASTMAVLINK_MSG_ID_HEARTBEAT: {
        fmav_heartbeat_t payload;
        fmav_msg_heartbeat_decode(&payload, msg);
        handle_mavlink_msg_heartbeat(&payload);
        passthrough.handle_mavlink_msg_heartbeat(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_BATTERY_STATUS: {
        fmav_battery_status_t payload;
        fmav_msg_battery_status_decode(&payload, msg);
        handle_mavlink_msg_battery_status(&payload);
        passthrough.handle_mavlink_msg_battery_status(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_ATTITUDE: {
        fmav_attitude_t payload;
        fmav_msg_attitude_decode(&payload, msg);
        handle_mavlink_msg_attitude(&payload);
        passthrough.handle_mavlink_msg_attitude(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_GPS_RAW_INT: {
        fmav_gps_raw_int_t payload;
        fmav_msg_gps_raw_int_decode(&payload, msg);
        handle_mavlink_msg_gps_raw_int(&payload);
        passthrough.handle_mavlink_msg_gps_raw_int(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_GPS2_RAW: { // not used by passthrough
        fmav_gps2_raw_t payload;
        fmav_msg_gps2_raw_decode(&payload, msg);
        handle_mavlink_msg_gps2_raw(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_VFR_HUD: {
        fmav_vfr_hud_t payload;
        fmav_msg_vfr_hud_decode(&payload, msg);
        handle_mavlink_msg_vfr_hud(&payload);
        passthrough.handle_mavlink_msg_vfr_hud(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
        fmav_global_position_int_t payload;
        fmav_msg_global_position_int_decode(&payload, msg);
        handle_mavlink_msg_global_position_int(&payload);
        passthrough.handle_mavlink_msg_global_position_int(&payload);
        }break;

    // these are for passthrough only

    case FASTMAVLINK_MSG_ID_SYS_STATUS: {
        fmav_sys_status_t payload;
        fmav_msg_sys_status_decode(&payload, msg);
        passthrough.handle_mavlink_msg_sys_status(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_RAW_IMU: {
        fmav_raw_imu_t payload;
        fmav_msg_raw_imu_decode(&payload, msg);
        passthrough.handle_mavlink_msg_raw_imu(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_MISSION_CURRENT: {
        fmav_mission_current_t payload;
        fmav_msg_mission_current_decode(&payload, msg);
        passthrough.handle_mavlink_msg_mission_current(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT: {
        fmav_nav_controller_output_t payload;
        fmav_msg_nav_controller_output_decode(&payload, msg);
        passthrough.handle_mavlink_msg_nav_controller_output(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_TERRAIN_REPORT: {
        fmav_terrain_report_t payload;
        fmav_msg_terrain_report_decode(&payload, msg);
        passthrough.handle_mavlink_msg_terrain_report(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_FENCE_STATUS: {
        fmav_fence_status_t payload;
        fmav_msg_fence_status_decode(&payload, msg);
        passthrough.handle_mavlink_msg_fence_status(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_RANGEFINDER: {
        fmav_rangefinder_t payload;
        fmav_msg_rangefinder_decode(&payload, msg);
        passthrough.handle_mavlink_msg_rangefinder(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_RPM: {
        fmav_rpm_t payload;
        fmav_msg_rpm_decode(&payload, msg);
        passthrough.handle_mavlink_msg_rpm(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_HOME_POSITION: {
        fmav_home_position_t payload;
        fmav_msg_home_position_decode(&payload, msg);
        passthrough.handle_mavlink_msg_home_position(&payload);
        }break;

    case FASTMAVLINK_MSG_ID_STATUSTEXT: {
        fmav_statustext_t payload;
        fmav_msg_statustext_decode(&payload, msg);
        passthrough.handle_mavlink_msg_statustext(&payload);
        }break;

    }
}


//-------------------------------------------------------
// CRSF Telemetry MSP Handling

#define DEG2RADF  1.745329252E-02f

int16_t wrap180_cdeg(int16_t angle_cdeg)
{
    while (angle_cdeg > 1800) { angle_cdeg -= 3600; }
    while (angle_cdeg < -1800) { angle_cdeg += 3600; }
    return angle_cdeg;
}


void tTxCrsf::TelemetryHandleMspMsg(msp_message_t* const msg)
{
    // conversions deduced from comparing
    //  src/main/fc/fc_msp.c for MSP units
    //  src/main/telemetry/crsf.c for CRSF telemetry units

    // to suppress sensor auto updating
    // assumes that this function is being called within 1500 ms
    if (!(msp_inav_status_sensor_status & (1 << INAV_SENSOR_STATUS_GPS))) gps_send_tlast_ms = 0;
    if (!(msp_inav_status_sensor_status & (1 << INAV_SENSOR_STATUS_BARO))) baro_send_tlast_ms = 0;

    switch (msg->function) {
#if 0 // was a test to see if one can provide more sensors for Yaapu
    case MSP_SONAR_ALTITUDE: { //
        tMspSonarAltitude* payload = (tMspSonarAltitude*)(msg->payload);
        if (msp_inav_status_sensor_status & (1 << INAV_SENSOR_STATUS_RANGEFINDER)) {
            // for the moment we do it by creating a fake mavlink message
            fmav_rangefinder_t p;
            p.distance = (float)((payload->rangefinder_altitude + 5) / 10); // float m <- uint32_t cm
            p.voltage = 0.0f;
            passthrough.handle_mavlink_msg_rangefinder(&p);
        }
        } break;
#endif

    case MSP_ATTITUDE: { // tCrsfAttitude, CRSF_FRAME_ID_ATTITUDE = 0x1E
        tMspAttitude* payload = (tMspAttitude*)(msg->payload);
        attitude.pitch = CRSF_REV_I16((DEG2RADF * 1000.0f) * wrap180_cdeg(payload->pitch)); // int16_t rad * 1e4  // cdeg -> rad * 1e4
        attitude.roll = CRSF_REV_I16((DEG2RADF * 1000.0f) * wrap180_cdeg(payload->roll));   // int16_t rad * 1e4  // cdeg -> rad * 1e4
        attitude.yaw = CRSF_REV_I16((DEG2RADF * 10000.0f) * wrap180_cdeg(payload->yaw));    // int16_t rad * 1e4  // deg -> rad * 1e4
        attitude_updated = true;
        }break;

    case MSP2_INAV_ANALOG: { // tCrsfBattery, CRSF_FRAME_ID_BATTERY = 0x08
        tMspInavAnalog* payload = (tMspInavAnalog*)(msg->payload);
        battery.voltage = CRSF_REV_U16(payload->battery_voltage / 10);  // uint16_t mV * 100      // uint16_t  seems to be 0.01 V
        battery.current = CRSF_REV_U16(payload->amperage / 10);         // uint16_t mA * 100      // uint16_t  send amperage in 0.01 A steps
        uint32_t capacity = payload->mAh_drawn;                         // uint8_t[3] mAh         // uint32_t  milliamp hours drawn from battery
        battery.capacity[0] = (capacity >> 16);
        battery.capacity[1] = (capacity >> 8);
        battery.capacity[2] = capacity;
        battery.remaining = payload->battery_percentage;                // uint8_t percent        // uint8_t
        battery_updated = true;
        }break;

    case MSP_RAW_GPS: { // tCrsfGps, CRSF_FRAME_ID_GPS = 0x02
        if (!(msp_inav_status_sensor_status & (1 << INAV_SENSOR_STATUS_GPS))) break;
        tMspRawGps* payload = (tMspRawGps*)(msg->payload);
        gps.latitude = CRSF_REV_U32(payload->lat);                    // int32_t degree / 1e7           // uint32_t  1 / 10 000 000 deg
        gps.longitude = CRSF_REV_U32(payload->lon);                   // int32_t degree / 1e7           // uint32_t 1 / 10 000 000 deg
        gps.groundspeed = CRSF_REV_U16((payload->ground_speed * 36 + 50) / 100);  // uint16_t km/h / 100            // uint16_t  cm/s
        gps.gps_heading = CRSF_REV_U16(payload->ground_course * 10);  // uint16_t degree / 100          // uint16_t  degree*10
        // INAV wants the baro alt in the gps alt field
        //gps.altitude = CRSF_REV_U16(payload->alt + 1000);             // uint16_t meter - 1000m offset  // uint16_t  meters
        gps.altitude = CRSF_REV_U16(inav_baro_altitude / 100 + 1000);
        gps.satellites = payload->numSat;                             // uint8_t                        // uint8_t
        gps_updated = true;
        }break;

    case MSP_ALTITUDE: {
        tMspAltitude* payload = (tMspAltitude*)(msg->payload);
        // tCrsfVario, CRSF_FRAME_ID_VARIO = 0x07
        vario.climb_rate = CRSF_REV_I16(payload->estimated_velocity_z);   // int16_t cm/s    // int16_t  cm/s
        vario_updated = true;
        // tCrsfBaroAltitude, CRSF_FRAME_ID_BARO_ALTITUDE = 0x09
        if (msp_inav_status_sensor_status & (1 << INAV_SENSOR_STATUS_BARO)) {
            int32_t alt = payload->baro_altitude / 10 + 10000; // uint32_t seems to be cm, convert to dm - 1000m
            if (alt < 0) alt = 0;
            if (alt > 0x7FFF) alt = 0x7FFF; // 0x7FFF = 32767
            baro_altitude.altitude = CRSF_REV_U16(alt); // uint16_t dm -1000m if 0x8000 not set
            // INAV wants the baro alt in the gps alt field
            // so we store the baro altitude, and tell that gps is updated
            //baro_altitude_updated = true;
            inav_baro_altitude = payload->baro_altitude;
            gps_updated = true;
        }
        }break;

    case MSP2_INAV_STATUS: {
        tMspInavStatus* payload = (tMspInavStatus*)(msg->payload);
        // report it
        msp_inav_status_sensor_status = payload->sensor_status;
        msp_inav_status_arming_flags = payload->arming_flags;
        }break;

    case MSPX_STATUS: { // this is send by the rx shortly after MSP2_INAV_STATUS
        uint32_t flight_mode = *(uint32_t*)(msg->payload);
        inav_flight_mode_str5(flightmode.flight_mode, flight_mode, msp_inav_status_arming_flags);
        flightmode_updated = true;
        }break;
    }
}


//-------------------------------------------------------
// CRSF Link Statistics

// on CRSF rssi
// rssi = 255 -> red in otx
//      = 130 -> -126 dB
//      = 129 -> -127 dB
//      = 128 -> -128 dB
//      = 127 ->  127dB
//      = 126 ->  126dB
// hmhm ...

// uplink:   Tx (tx -> rx)
// downlink: Rx (rx -> tx)
// somehow the OpenTx naming/usage doesn't make fully sense
// so we "correct" things here such that the names make sense, irrespective of uplink/downlink notation

void tTxCrsf::SendLinkStatistics(void)
{
tCrsfLinkStatistics clstats;

    clstats.uplink_rssi1 = crsf_cvt_rssi_tx(stats.received_rssi);           // OpenTX -> "1RSS"
    clstats.uplink_rssi2 = 0; // we don't know it                           // OpenTX -> "2RSS"
    clstats.uplink_LQ = stats.received_LQ_rc; // this sets main rssi in OpenTx, 0 = resets main rssi   // OpenTx -> "RQly"
    clstats.uplink_snr = 0; // we don't know it                             // OpenTx -> "RSNR"
    clstats.active_antenna = stats.received_antenna;                        // OpenTx -> "ANT"
    clstats.mode = crsf_cvt_mode(Config.Mode);                              // OpenTx -> "RFMD"
    clstats.uplink_transmit_power = crsf_cvt_power(sx.RfPower_dbm());       // OpenTx -> "TPw2"

    clstats.downlink_rssi = crsf_cvt_rssi_tx(stats.GetLastRssi());          // OpenTx -> "TRSS"
    clstats.downlink_LQ = stats.GetLQ_serial();                             // OpenTx -> "TQly"
    clstats.downlink_snr = stats.GetLastSnr();                              // OpenTx -> "TSNR"

    // misuse 2RSS for reporting the MAVLink packet link quality
    clstats.uplink_rssi2 = stats.GetMavlinkLQ();

    send_frame(CRSF_FRAME_ID_LINK_STATISTICS, &clstats, CRSF_LINK_STATISTICS_LEN);
}


void tTxCrsf::SendLinkStatisticsTx(void)
{
tCrsfLinkStatisticsTx clstats;

    clstats.uplink_rssi = crsf_cvt_rssi_tx(stats.GetLastRssi());                  // ignored by OpenTx
    clstats.uplink_rssi_percent = crsf_cvt_rssi_percent(stats.GetLastRssi(),      // OpenTx -> "TRSP" // ??? uplink but "T" ??
                                                sx.ReceiverSensitivity_dbm());
    clstats.uplink_LQ = stats.GetLQ_serial();                                     // ignored by OpenTx
    clstats.uplink_snr = stats.GetLastSnr();                                      // ignored by OpenTx
    clstats.downlink_transmit_power = UINT8_MAX; // we don't know it              // OpenTx -> "RPWR"
    clstats.uplink_fps = crsf_cvt_fps(Config.Mode); // *10 in OpenTx              // OpenTx -> "TFPS"

    send_frame(CRSF_FRAME_ID_LINK_STATISTICS_TX, &clstats, CRSF_LINK_STATISTICS_TX_LEN);
}


void tTxCrsf::SendLinkStatisticsRx(void)
{
tCrsfLinkStatisticsRx clstats;

    clstats.downlink_rssi = crsf_cvt_rssi_tx(stats.received_rssi);                // ignored by OpenTx
    clstats.downlink_rssi_percent = crsf_cvt_rssi_percent(stats.received_rssi,    // OpenTx -> "RRSP" // ??? downlink but "R" ??
                                                  sx.ReceiverSensitivity_dbm());
    clstats.downlink_LQ = stats.received_LQ_rc;                                   // ignored by OpenTx
    clstats.downlink_snr = 0; // we don't know it                                 // ignored by OpenTx
    clstats.uplink_transmit_power = sx.RfPower_dbm();                             // OpenTx -> "TPWR"

    send_frame(CRSF_FRAME_ID_LINK_STATISTICS_RX, &clstats, CRSF_LINK_STATISTICS_RX_LEN);
}


#else

class tTxCrsfDummy
{
  public:
    void Init(bool enable_flag) {}
    bool Update(tRcData* const rc) { return false;}
    void TelemetryStart(void) {}
    void TelemetryTick_ms(void) {}
    bool TelemetryUpdate(uint8_t* const task, uint16_t frame_rate_ms);
    void TelemetryHandleMavlinkMsg(fmav_message_t* const msg) {}
    void TelemetryHandleMspMsg(msp_message_t* const msg) {}

    void SendLinkStatistics(void) {}
    void SendLinkStatisticsTx(void) {}
    void SendLinkStatisticsRx(void) {}
};

tTxCrsfDummy crsf;

#endif // ifdef USE_JRPIN5

#endif // CRSF_INTERFACE_TX_H

