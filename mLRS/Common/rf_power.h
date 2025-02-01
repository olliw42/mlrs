//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// RF Power dynamic handling
//********************************************************
#ifndef RF_POWER_H
#define RF_POWER_H
#pragma once


#include "hal/hal.h"


class tRfPower
{
  public:
    void Init(bool _antenna1, bool _antenna2);
    void Update(void);
    void Set(tRcData* const rc, uint8_t power_switch_channel, uint8_t power);
    void Set(uint8_t power);

  private:
    uint8_t rfpower_current_idx;
    uint8_t rfpower_new_idx;
    bool do_antenna1;
    bool do_antenna2;
};


void tRfPower::Init(bool _antenna1, bool _antenna2)
{
    rfpower_current_idx = 0;
    rfpower_new_idx = rfpower_current_idx; // to prevent update before first Set

    do_antenna1 = _antenna1;
    do_antenna2 = _antenna2;
}


void tRfPower::Update(void)
{
    if (rfpower_new_idx == rfpower_current_idx) return; // no change required

    rfpower_current_idx = rfpower_new_idx;

    if (rfpower_current_idx >= RFPOWER_LIST_NUM) { // should not happen, play it safe
        rfpower_current_idx = RFPOWER_LIST_NUM - 1;
        rfpower_new_idx = rfpower_current_idx;
    }

    Config.Sx.Power_dbm = rfpower_list[rfpower_current_idx].dbm;
    Config.Sx2.Power_dbm = Config.Sx.Power_dbm;

    if (do_antenna1) sx.UpdateRfPower(&Config.Sx);
    if (do_antenna2) sx2.UpdateRfPower(&Config.Sx2);
}


void tRfPower::Set(uint8_t power)
{
    rfpower_new_idx = power;
}


void tRfPower::Set(tRcData* const rc, uint8_t power_switch_channel, uint8_t power)
{
    if (power_switch_channel == POWER_SWITCH_CHANNEL_OFF) { // disabled
        rfpower_new_idx = power;
        return;
    }

    if (power_switch_channel < POWER_SWITCH_CHANNEL_CH5 || power_switch_channel > POWER_SWITCH_CHANNEL_CH16) {
        return; // argh
    }

    if (power_switch_channel + 3 >= RC_DATA_LEN) while(1){} // should not happen

    uint16_t rc_val = rc->ch[power_switch_channel + 3]; // ch5 .. ch16 -> 3 .. 15

    int8_t new_idx = 0;
    if (RFPOWER_LIST_NUM <= 5) {
        // rcData: 11 bits,  1 .. 1024 .. 2047 for +-120%
        // 5 slots in Rc range
        // slot 5, up:        > 75% = 639
        // slot 4, mid-up:     25% ... 75% = 213
        // slot 3, mid:       -25% ... 25%
        // slot 2, mid-down:  -75% ... -25%
        // slot 1, down:      < -75%
        if (rc_val >= (1024 + 639)) {   // slot 5 -> max
            new_idx = (int8_t)power;
        } else
        if (rc_val <= (1024 - 639)) {   // slot 1 -> max - 2
            new_idx = (int8_t)power - 2;
        } else
        if (rc_val >= (1024 + 213)) {   // slot 4 -> min + 1
            new_idx = 1;
            if (new_idx > (int8_t)power - 3) new_idx = (int8_t)power - 3;
        } else
        if (rc_val <= (1024 - 213)) {   // slot 2 -> min
            new_idx = 0;
        } else {                        // slot 3 -> max - 1
            new_idx = (int8_t)power - 1;
        }
    } else {
        // 7 slots in Rc range
        // slot 7: > 75% = 639
        // slot 6: 50% ... 75% = 426
        // slot 5: 25% ... 50% = 213
        // slot 4: -25% ... 25%
        // slot 3: -50% ... -25%
        // slot 2: -50% ... -75%
        // slot 1: < -75%
        if (rc_val >= (1024 + 639)) {   // slot 7 -> max
            new_idx = (int8_t)power;
        } else
        if (rc_val <= (1024 - 639)) {   // slot 1 -> max - 2
            new_idx = (int8_t)power - 2;
        } else
        if (rc_val >= (1024 + 426)) {   // slot 6 -> min + 3
            new_idx = 3;
            if (new_idx > (int8_t)power - 3) new_idx = (int8_t)power - 3;
        } else
        if (rc_val <= (1024 - 426)) {   // slot 2 -> min
            new_idx = 0;
        } else
        if (rc_val >= (1024 + 213)) {   // slot 5 -> min + 2
            new_idx = 2;
            if (new_idx > (int8_t)power - 3) new_idx = (int8_t)power - 3;
        } else
        if (rc_val <= (1024 - 213)) {   // slot 3 -> min + 1
            new_idx = 1;
            if (new_idx > (int8_t)power - 3) new_idx = (int8_t)power - 3;
        } else {                        // slot 4 -> max - 1
            new_idx = (int8_t)power - 1;
        }
    }

    if (new_idx < 0) new_idx = 0; // constrain to min
    if (new_idx > power) new_idx = power; // constrain by Setup Power setting

    rfpower_new_idx = new_idx;
}


#endif // RF_POWER_H



