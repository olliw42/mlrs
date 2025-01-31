//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// FHSS
//*******************************************************

#include <stdint.h>
#include "fhss.h"


// https://en.wikipedia.org/wiki/Linear_congruential_generator: Microsoft Visual/Quick C/C++
// also used by ELRS
// generates values in range [0, 0x7FFF]
// spektrum:
// appears to use val = val * 0x0019660D + 0x3C6EF35F; (val >> 8) % 73
// is numerical recipes method in the wikipedia source
// picks 24 channels out of 74, ensures it's distributed into three slots
// redpine:
// same prng as spektrum, picks 50 channels, ensures not close and not 0,1

//QUESTION:
// how many different fhss sequences can be generated with the 32bit seed value ??

uint16_t tFhssBase::prng(void)
{
    const uint32_t a = 214013;
    const uint32_t c = 2531011;
    const uint32_t m = 2147483648;

    _seed = (a * _seed + c) % m;

    return _seed >> 16;
}


void tFhssBase::generate(uint32_t seed)
{
    _seed = seed;
    _ortho = ORTHO_NONE;
    _except = EXCEPT_NONE;

    bool used_flag[FHSS_FREQ_LIST_MAX_LEN];
    for (uint8_t ch = 0; ch < FHSS_FREQ_LIST_MAX_LEN; ch++) used_flag[ch] = false;

    uint8_t k = 0;
    while (k < cnt) {

        uint8_t rn = prng() % (FREQ_LIST_LEN - k); // get a random number in the remaining range

        uint8_t i = 0;
        uint8_t ch;
        for (ch = 0; ch < FREQ_LIST_LEN; ch++) {
            if (used_flag[ch]) continue;
            if (i == rn) break; // ch is our next index
            i++;
        }

        if (ch >= FREQ_LIST_LEN) { // argh, must not happen !
            ch = 0;
        }

        // do not pick a bind channel
        bool is_bind_channel = false;
        for (uint8_t bi = 0; bi < BIND_CHANNEL_LIST_LEN; bi++) {
            if (ch == fhss_bind_channel_list[bi]) is_bind_channel = true;
        }
        if (is_bind_channel) continue;

        // ensure it is not too close to the previous
        // do only if we have plenty of channels at our disposal
        bool is_too_close = false;
        if ((config_i != SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ && config_i != SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN) &&
            (k > 0)) { // TODO: use smarter method, e.g., cnt < 2/3
            int8_t last_ch = ch_list[k - 1];
            if (last_ch == 0) { // special treatment for this case
                if (ch < 2) is_too_close = true;
            } else {
                if ((ch >= last_ch - 1) && (ch <= last_ch + 1)) is_too_close = true;
            }
        }
        if (is_too_close) continue;

        // we got a new ch, so register it
        ch_list[k] = ch;
        fhss_list[k] = fhss_freq_list[ch];
        used_flag[ch] = true;

        k++;
    }

    // the following is not related to the generation, but does initialization
    // is done here to allow calling generate separately, at least in principle

    // start with first entry
    curr_i = 0;

    // mark all channels as equally bad
    for (uint8_t k = 0; k < cnt; k++) {
        fhss_last_rssi[k] = INT8_MIN;
    }
}


void tFhssBase::generate_ortho_except(uint32_t seed, uint8_t ortho, uint8_t except)
{
    _seed = seed;
    _ortho = ortho; // assumes that FHSS_ORTHO & ORTHO enums are aligned!
    _except = except; // assumes that FHSS_EXCEPT & EXCEPT enums are aligned!

    bool used_flag[FHSS_FREQ_LIST_MAX_LEN];
    for (uint8_t ch = 0; ch < FHSS_FREQ_LIST_MAX_LEN; ch++) used_flag[ch] = false;

    uint8_t freq_len = FREQ_LIST_LEN;
    uint8_t ch_ofs = 0;
    uint8_t ch_inc = 1;

    if (_ortho >= ORTHO_1_3 && _ortho <= ORTHO_3_3) {
        ch_ofs = _ortho - ORTHO_1_3; // 0, 1, 2
        ch_inc = 3;
        freq_len = FREQ_LIST_LEN / 3; // we use only 1/3 of the available channels
    }

    uint8_t k = 0;
    uint8_t last_ch_eff = 0;

    while (k < cnt) {

        uint8_t rn = prng() % (freq_len - k); // get a random number in the remaining range

        uint8_t i = 0;
        uint8_t ch_eff;
        for (ch_eff = 0; ch_eff < freq_len; ch_eff++) {
            if (used_flag[ch_eff]) continue;
            if (i == rn) break; // ch_eff is our next index
            i++;
        }

        if (ch_eff >= freq_len) { // argh, must not happen !
            ch_eff = freq_len;
        }

        uint8_t ch = ch_eff * ch_inc + ch_ofs; // that's the true channel

        // do not pick a bind channel
        bool is_bind_channel = false;
        for (uint8_t bi = 0; bi < BIND_CHANNEL_LIST_LEN; bi++) {
            if (ch == fhss_bind_channel_list[bi]) is_bind_channel = true;
        }
        if (is_bind_channel) continue;

        // do not pick a channel in an excepted wifi band
        // https://en.wikipedia.org/wiki/List_of_WLAN_channels
#ifdef FHSS_HAS_CONFIG_2P4_GHZ
        uint32_t freq = fhss_freq_list[ch];
        switch (_except) {
        case EXCEPT_2P4_GHZ_WIFIBAND_1:
            // #1, 2.412 GHz +- 11 MHz = ]0 , 17[
            if (SX12XX_FREQ_GHZ_TO_REG(2.401) <= freq && freq <= SX12XX_FREQ_GHZ_TO_REG(2.423)) continue;
            break;
        case EXCEPT_2P4_GHZ_WIFIBAND_6:
            // #6, 2.437 GHz +- 11 MHz = ]20 , 42[
            if (SX12XX_FREQ_GHZ_TO_REG(2.426) <= freq && freq <= SX12XX_FREQ_GHZ_TO_REG(2.448)) continue;
            break;
        case EXCEPT_2P4_GHZ_WIFIBAND_11:
            // #11, 2.462 GHz +- 11 MHz = ]45 , 67[
            if (SX12XX_FREQ_GHZ_TO_REG(2.451) <= freq && freq <= SX12XX_FREQ_GHZ_TO_REG(2.473)) continue;
            break;
        case EXCEPT_2P4_GHZ_WIFIBAND_13:
            // #13, 2.472 GHz +- 11 MHz = ]55, 67[
            if (SX12XX_FREQ_GHZ_TO_REG(2.461) <= freq && freq <= SX12XX_FREQ_GHZ_TO_REG(2.483)) continue;
            break;
        }
#endif

        // ensure it is not too close to the previous
        bool is_too_close = false;
        if (k > 0) {
            if (last_ch_eff == 0) { // special treatment for this case
                if (ch_eff <= 1) is_too_close = true;
            } else {
                if ((ch_eff >= last_ch_eff - 1) && (ch_eff <= last_ch_eff + 1)) is_too_close = true;
            }
        }
        if (is_too_close) continue;

        last_ch_eff = ch_eff;

        // we got a new ch, so register it
        ch_list[k] = ch;
        fhss_list[k] = fhss_freq_list[ch];
        used_flag[ch_eff] = true;

        k++;
    }

    // the following is not related to the generation, but does initialization
    // is done here to allow calling generate separately, at least in principle

    // start with first entry
    curr_i = 0;

    // mark all channels as equally bad
    for (uint8_t k = 0; k < cnt; k++) {
        fhss_last_rssi[k] = INT8_MIN;
    }
}

