//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// MSP Interface RX Side
//*******************************************************
#ifndef MSP_INTERFACE_RX_H
#define MSP_INTERFACE_RX_H
#pragma once


#ifdef USE_FEATURE_MAVLINKX
#include "../Common/libs/fifo.h"
#include "../Common/thirdparty/mspx.h"


extern volatile uint32_t millis32(void);
static inline bool connected(void);


#define MSP_BUF_SIZE  (MSP_FRAME_LEN_MAX + 16) // needs to be larger than max supported msp frame size


class tRxMsp
{
  public:
    void Init(void);
    void Do(void);
    void SendRcData(tRcData* rc_out, bool frame_missed, bool failsafe);
    void FrameLost(void);

    void putc(char c);
    bool available(void);
    uint8_t getc(void);
    void flush(void);

  private:

    // fields for link in -> parser -> serial out
    msp_status_t status_link_in;
    msp_message_t msp_msg_link_in;

    // fields for serial in -> parser -> link out
    msp_status_t status_ser_in;
    msp_message_t msp_msg_ser_in;
    FifoBase<char,2*512> fifo_link_out; // needs to be at least ??

    // to inject MSP_SET_RAW_RC
    bool inject_rc_channels;
    uint16_t rc_chan[16]; // holds the rc data in MSP format

    uint8_t _buf[MSP_BUF_SIZE]; // temporary working buffer, to not burden stack
};


void tRxMsp::Init(void)
{
    msp_init();

    status_link_in = {};
    status_ser_in = {};
    fifo_link_out.Init();

    inject_rc_channels = false;
}


void tRxMsp::SendRcData(tRcData* rc_out, bool frame_missed, bool failsafe)
{
    if (Setup.Rx.SendRcChannels == SEND_RC_CHANNELS_OFF) return;

    uint8_t failsafe_mode = Setup.Rx.FailsafeMode;

    if (failsafe) {
        switch (failsafe_mode) {
        case FAILSAFE_MODE_NO_SIGNAL:
            // do not output anything, so jump out
            return;
        case FAILSAFE_MODE_CH1CH4_CENTER:
            // in this mode do not report bad signal
            break;
        }
    }

    for (uint8_t i = 0; i < 16; i++) {
        rc_chan[i] = rc_to_mavlink(rc_out->ch[i]);
    }

    inject_rc_channels = true;
}


void tRxMsp::Do(void)
{
    if (!connected()) {
        fifo_link_out.Flush();
    }

    // parse serial in -> link out
    if (fifo_link_out.HasSpace(MSP_FRAME_LEN_MAX + 16)) { // we have space for a full MSP message, so can safely parse
        while (serial.available()) {
            char c = serial.getc();
            if (msp_parse_to_msg(&msp_msg_ser_in, &status_ser_in, c)) {
#ifdef USE_MSPX
                uint16_t len = msp_msg_to_frame_bufX(_buf, &msp_msg_ser_in); // converting to mspX !!
#else
                uint16_t len = msp_msg_to_frame_buf(_buf, &msp_msg_ser_in);
#endif

                fifo_link_out.PutBuf(_buf, len);

/*
dbg.puts("\n");
dbg.putc(msp_msg_ser_in.type);
char s[32]; msp_function_str_from_msg(s, &msp_msg_ser_in); dbg.puts(s);
dbg.puts(" ");
dbg.puts(u16toBCD_s(msp_msg_ser_in.len));
*/
            }

        }
    }

    // inject radio rc channels

    if (inject_rc_channels) { // give it priority // && serial.tx_is_empty()) // check available size!?
        inject_rc_channels = false;
        switch (Setup.Rx.SendRcChannels) {
        case SEND_RC_CHANNELS_RCCHANNELSOVERRIDE:
        case SEND_RC_CHANNELS_RADIORCCHANNELS: {
            uint16_t len = msp_generate_frame_buf(_buf, MSP_TYPE_REQUEST, MSP_SET_RAW_RC, (uint8_t*)rc_chan, 32);
            serial.putbuf(_buf, len);
            return; }
        }
    }
}


void tRxMsp::FrameLost(void)
{
    msp_parse_reset(&status_link_in);
}


void tRxMsp::putc(char c)
{
    // parse link in -> serial out
#ifdef USE_MSPX
    if (msp_parseX_to_msg(&msp_msg_link_in, &status_link_in, c)) { // converting from mspX
#else
    if (msp_parse_to_msg(&msp_msg_link_in, &status_link_in, c)) {
#endif
        uint16_t len = msp_msg_to_frame_buf(_buf, &msp_msg_link_in);
        serial.putbuf(_buf, len);

/*
dbg.puts("\n");
dbg.putc(msp_msg_link_in.type);
char s[32]; msp_function_str_from_msg(s, &msp_msg_link_in); dbg.puts(s);
dbg.puts(" ");
dbg.puts(u16toBCD_s(msp_msg_link_in.len));
dbg.puts(" ");
dbg.puts(u8toHEX_s(msp_msg_link_in.checksum));
uint8_t crc8 = crsf_crc8_update(0, &(_buf[3]), len - 4);
dbg.puts(" ");
dbg.puts(u8toHEX_s(crc8));
*/
    }
}


bool tRxMsp::available(void)
{
    return fifo_link_out.Available();
    //return serial.available();
}


uint8_t tRxMsp::getc(void)
{
    return fifo_link_out.Get();
    //return serial.getc();
}


void tRxMsp::flush(void)
{
    fifo_link_out.Flush();
    // serial is flushed by caller
}


#else //!USE_FEATURE_MAVLINKX

class tRxMsp
{
  public:
    void Init(void) {}
    void Do(void) {}
    void SendRcData(tRcData* rc_out, bool frame_missed, bool failsafe) {}
    void FrameLost(void) {}

    void putc(char c) {}
    bool available(void) { return false; }
    uint8_t getc(void) { return 0; }
    void flush(void) {}
};

#endif //USE_FEATURE_MAVLINKX


#endif // MSP_INTERFACE_RX_H
