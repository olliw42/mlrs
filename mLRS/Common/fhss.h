//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// FHSS
//*******************************************************
#ifndef FHSS_H
#define FHSS_H
#pragma once


#include <stdint.h>
#include <string.h>
#include "common_conf.h"
#include "hal/device_conf.h"
#include "sx-drivers/sx12xx.h"
#include "setup_types.h"


#define FHSS_MAX_NUM            32
#define FHSS_FREQ_LIST_MAX_LEN  86 // 2.4 GHz is 80

//-------------------------------------------------------
// Frequency list
//-------------------------------------------------------

#ifdef FREQUENCY_BAND_433_MHZ
  #define FHSS_HAS_CONFIG_433_MHZ
#endif
#ifdef FREQUENCY_BAND_70_CM_HAM
  #define FHSS_HAS_CONFIG_70_CM_HAM
#endif
#ifdef FREQUENCY_BAND_868_MHZ
  #define FHSS_HAS_CONFIG_868_MHZ
#endif
#ifdef FREQUENCY_BAND_915_MHZ_FCC
  #define FHSS_HAS_CONFIG_915_MHZ_FCC
#endif
#ifdef FREQUENCY_BAND_866_MHZ_IN
  #define FHSS_HAS_CONFIG_866_MHZ_IN
#endif
#ifdef FREQUENCY_BAND_2P4_GHZ
  #define FHSS_HAS_CONFIG_2P4_GHZ
#endif


#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
  #define SX12XX_FREQ_MHZ_TO_REG(f_mhz)  SX126X_FREQ_MHZ_TO_REG(f_mhz)
#elif defined DEVICE_HAS_SX127x
  #define SX12XX_FREQ_MHZ_TO_REG(f_mhz)  SX127X_FREQ_MHZ_TO_REG(f_mhz)
#elif defined DEVICE_HAS_LR11xx
  #define SX12XX_FREQ_MHZ_TO_REG(f_mhz)  LR11XX_FREQ_MHZ_TO_REG(f_mhz)
#else // DEVICE_HAS_SX128x
  // for 2.4 GHz we directly use SX1280_FREQ_GHZ_TO_REG(), not SX12XX_FREQ_MHZ_TO_REG()
#endif


#ifdef FHSS_HAS_CONFIG_433_MHZ
// 433.050 ... 434.790 in 0.506 MHz steps

const uint32_t fhss_freq_list_433[] = {
    SX12XX_FREQ_MHZ_TO_REG(433.360),
    SX12XX_FREQ_MHZ_TO_REG(433.920),
    SX12XX_FREQ_MHZ_TO_REG(433.480),
};

const uint8_t fhss_bind_channel_list_433[] = {
    0 // just pick some
};

#endif
#ifdef FHSS_HAS_CONFIG_70_CM_HAM
// USA Ham Bands w/ Tech License
// https://www.arrl.org/files/file/Tech%20Band%20Chart/US%20Amateur%20Radio%20Technician%20Privileges.pdf
// 33 Channels w/ 0.6 MHz spacing

const uint32_t fhss_freq_list_70_cm_ham[] = {
    SX12XX_FREQ_MHZ_TO_REG(430.4),
    SX12XX_FREQ_MHZ_TO_REG(431.0),
    SX12XX_FREQ_MHZ_TO_REG(431.6),
    SX12XX_FREQ_MHZ_TO_REG(432.2),
    SX12XX_FREQ_MHZ_TO_REG(432.8),
    SX12XX_FREQ_MHZ_TO_REG(433.4),
    SX12XX_FREQ_MHZ_TO_REG(434.0),
    SX12XX_FREQ_MHZ_TO_REG(434.6),
    SX12XX_FREQ_MHZ_TO_REG(435.2),
    SX12XX_FREQ_MHZ_TO_REG(435.8),

    SX12XX_FREQ_MHZ_TO_REG(436.4),
    SX12XX_FREQ_MHZ_TO_REG(437.0),
    SX12XX_FREQ_MHZ_TO_REG(437.6),
    SX12XX_FREQ_MHZ_TO_REG(438.2),
    SX12XX_FREQ_MHZ_TO_REG(438.8),
    SX12XX_FREQ_MHZ_TO_REG(439.4),
    SX12XX_FREQ_MHZ_TO_REG(440.0),
    SX12XX_FREQ_MHZ_TO_REG(440.6),
    SX12XX_FREQ_MHZ_TO_REG(441.2),
    SX12XX_FREQ_MHZ_TO_REG(441.8),

    SX12XX_FREQ_MHZ_TO_REG(442.4),
    SX12XX_FREQ_MHZ_TO_REG(443.0),
    SX12XX_FREQ_MHZ_TO_REG(443.6),
    SX12XX_FREQ_MHZ_TO_REG(444.2),
    SX12XX_FREQ_MHZ_TO_REG(444.8),
    SX12XX_FREQ_MHZ_TO_REG(445.4),
    SX12XX_FREQ_MHZ_TO_REG(446.0),
    SX12XX_FREQ_MHZ_TO_REG(446.6),
    SX12XX_FREQ_MHZ_TO_REG(447.2),
    SX12XX_FREQ_MHZ_TO_REG(447.8),

    SX12XX_FREQ_MHZ_TO_REG(448.4),
    SX12XX_FREQ_MHZ_TO_REG(449.0),
    SX12XX_FREQ_MHZ_TO_REG(449.6),
};

const uint8_t fhss_bind_channel_list_70_cm_ham[] = {
    10, 20 // picked 2
};

#endif
#ifdef FHSS_HAS_CONFIG_868_MHZ
// 863.275 ... 869.575  in 0.525 MHz steps

const uint32_t fhss_freq_list_868[] = {
    SX12XX_FREQ_MHZ_TO_REG(863.275),
    SX12XX_FREQ_MHZ_TO_REG(863.800),
    SX12XX_FREQ_MHZ_TO_REG(864.325),
    SX12XX_FREQ_MHZ_TO_REG(864.850),
    SX12XX_FREQ_MHZ_TO_REG(865.375),
    SX12XX_FREQ_MHZ_TO_REG(865.900),
    SX12XX_FREQ_MHZ_TO_REG(866.425),
    SX12XX_FREQ_MHZ_TO_REG(866.950),
    SX12XX_FREQ_MHZ_TO_REG(867.475),
    SX12XX_FREQ_MHZ_TO_REG(868.000),

    // SX12XX_FREQ_MHZ_TO_REG(868.525), // overlap with Alarmanlagen
    // SX12XX_FREQ_MHZ_TO_REG(869.050), // overlap with Alarmanlagen
    // SX12XX_FREQ_MHZ_TO_REG(869.575), // overlap with Alarmanlagen
};

const uint8_t fhss_bind_channel_list_868[] = {
    0, // just pick some
};

#endif
#ifdef FHSS_HAS_CONFIG_915_MHZ_FCC
// 902-928Mhz w/ 0.6MHz spacing means 26 / 0.6 = 43(.3) channels
// Start at 902.4MHz to use the middle of the frequency range

const uint32_t fhss_freq_list_915_fcc[] = {
    SX12XX_FREQ_MHZ_TO_REG(902.4),
    SX12XX_FREQ_MHZ_TO_REG(903.0),
    SX12XX_FREQ_MHZ_TO_REG(903.6),
    SX12XX_FREQ_MHZ_TO_REG(904.2),
    SX12XX_FREQ_MHZ_TO_REG(904.8),
    SX12XX_FREQ_MHZ_TO_REG(905.4),
    SX12XX_FREQ_MHZ_TO_REG(906.0),
    SX12XX_FREQ_MHZ_TO_REG(906.6),
    SX12XX_FREQ_MHZ_TO_REG(907.2),
    SX12XX_FREQ_MHZ_TO_REG(907.8),

    SX12XX_FREQ_MHZ_TO_REG(908.4),
    SX12XX_FREQ_MHZ_TO_REG(909.0),
    SX12XX_FREQ_MHZ_TO_REG(909.6),
    SX12XX_FREQ_MHZ_TO_REG(910.2),
    SX12XX_FREQ_MHZ_TO_REG(910.8),
    SX12XX_FREQ_MHZ_TO_REG(911.4),
    SX12XX_FREQ_MHZ_TO_REG(912.0),
    SX12XX_FREQ_MHZ_TO_REG(912.6),
    SX12XX_FREQ_MHZ_TO_REG(913.2),
    SX12XX_FREQ_MHZ_TO_REG(913.8),

    SX12XX_FREQ_MHZ_TO_REG(914.4),
    SX12XX_FREQ_MHZ_TO_REG(915.0),
    SX12XX_FREQ_MHZ_TO_REG(915.6),
    SX12XX_FREQ_MHZ_TO_REG(916.2),
    SX12XX_FREQ_MHZ_TO_REG(916.8),
    SX12XX_FREQ_MHZ_TO_REG(917.4),
    SX12XX_FREQ_MHZ_TO_REG(918.0),
    SX12XX_FREQ_MHZ_TO_REG(918.6),
    SX12XX_FREQ_MHZ_TO_REG(919.2),
    SX12XX_FREQ_MHZ_TO_REG(919.8),

    SX12XX_FREQ_MHZ_TO_REG(920.4),
    SX12XX_FREQ_MHZ_TO_REG(921.0),
    SX12XX_FREQ_MHZ_TO_REG(921.6),
    SX12XX_FREQ_MHZ_TO_REG(922.2),
    SX12XX_FREQ_MHZ_TO_REG(922.8),
    SX12XX_FREQ_MHZ_TO_REG(923.4),
    SX12XX_FREQ_MHZ_TO_REG(924.0),
    SX12XX_FREQ_MHZ_TO_REG(924.6),
    SX12XX_FREQ_MHZ_TO_REG(925.2),
    SX12XX_FREQ_MHZ_TO_REG(925.8),

    SX12XX_FREQ_MHZ_TO_REG(926.4),
    SX12XX_FREQ_MHZ_TO_REG(927.0),
    SX12XX_FREQ_MHZ_TO_REG(927.6),
};

const uint8_t fhss_bind_channel_list_915_fcc[] = {
    19 // just pick some
};

#endif
#ifdef FHSS_HAS_CONFIG_866_MHZ_IN
// 4 channels in range 865.375 - 866.950 MHz ??

const uint32_t fhss_freq_list_866_in[] = { // !! NEEDS TO BE ADJUSTED TO PROPER FREQUENCIES !!
    SX12XX_FREQ_MHZ_TO_REG(865.375),
    SX12XX_FREQ_MHZ_TO_REG(865.900),
    SX12XX_FREQ_MHZ_TO_REG(866.425),
    SX12XX_FREQ_MHZ_TO_REG(866.950),
};

const uint8_t fhss_bind_channel_list_866_in[] = {
    0 // just pick some
};

#endif
#ifdef FHSS_HAS_CONFIG_2P4_GHZ
// 2401.0 ... 2480.0  in 1 MHz steps
// = 80 channels

const uint32_t fhss_freq_list_2p4[] = {

    SX1280_FREQ_GHZ_TO_REG(2.401), // channel 0
    SX1280_FREQ_GHZ_TO_REG(2.402),
    SX1280_FREQ_GHZ_TO_REG(2.403),
    SX1280_FREQ_GHZ_TO_REG(2.404),
    SX1280_FREQ_GHZ_TO_REG(2.405),
    SX1280_FREQ_GHZ_TO_REG(2.406),
    SX1280_FREQ_GHZ_TO_REG(2.407),
    SX1280_FREQ_GHZ_TO_REG(2.408),
    SX1280_FREQ_GHZ_TO_REG(2.409),
    SX1280_FREQ_GHZ_TO_REG(2.410),

    SX1280_FREQ_GHZ_TO_REG(2.411), // channel 10
    SX1280_FREQ_GHZ_TO_REG(2.412),
    SX1280_FREQ_GHZ_TO_REG(2.413),
    SX1280_FREQ_GHZ_TO_REG(2.414),
    SX1280_FREQ_GHZ_TO_REG(2.415),
    SX1280_FREQ_GHZ_TO_REG(2.416),
    SX1280_FREQ_GHZ_TO_REG(2.417),
    SX1280_FREQ_GHZ_TO_REG(2.418),
    SX1280_FREQ_GHZ_TO_REG(2.419),
    SX1280_FREQ_GHZ_TO_REG(2.420),

    SX1280_FREQ_GHZ_TO_REG(2.421), // channel 20
    SX1280_FREQ_GHZ_TO_REG(2.422),
    SX1280_FREQ_GHZ_TO_REG(2.423),
    SX1280_FREQ_GHZ_TO_REG(2.424),
    SX1280_FREQ_GHZ_TO_REG(2.425),
    SX1280_FREQ_GHZ_TO_REG(2.426),
    SX1280_FREQ_GHZ_TO_REG(2.427),
    SX1280_FREQ_GHZ_TO_REG(2.428),
    SX1280_FREQ_GHZ_TO_REG(2.429),
    SX1280_FREQ_GHZ_TO_REG(2.430),

    SX1280_FREQ_GHZ_TO_REG(2.431), // channel 30
    SX1280_FREQ_GHZ_TO_REG(2.432),
    SX1280_FREQ_GHZ_TO_REG(2.433),
    SX1280_FREQ_GHZ_TO_REG(2.434),
    SX1280_FREQ_GHZ_TO_REG(2.435),
    SX1280_FREQ_GHZ_TO_REG(2.436),
    SX1280_FREQ_GHZ_TO_REG(2.437),
    SX1280_FREQ_GHZ_TO_REG(2.438),
    SX1280_FREQ_GHZ_TO_REG(2.439),
    SX1280_FREQ_GHZ_TO_REG(2.440),

    SX1280_FREQ_GHZ_TO_REG(2.441), // channel 40
    SX1280_FREQ_GHZ_TO_REG(2.442),
    SX1280_FREQ_GHZ_TO_REG(2.443),
    SX1280_FREQ_GHZ_TO_REG(2.444),
    SX1280_FREQ_GHZ_TO_REG(2.445),
    SX1280_FREQ_GHZ_TO_REG(2.446),
    SX1280_FREQ_GHZ_TO_REG(2.447),
    SX1280_FREQ_GHZ_TO_REG(2.448),
    SX1280_FREQ_GHZ_TO_REG(2.449),
    SX1280_FREQ_GHZ_TO_REG(2.450),

    SX1280_FREQ_GHZ_TO_REG(2.451), // channel 50
    SX1280_FREQ_GHZ_TO_REG(2.452),
    SX1280_FREQ_GHZ_TO_REG(2.453),
    SX1280_FREQ_GHZ_TO_REG(2.454),
    SX1280_FREQ_GHZ_TO_REG(2.455),
    SX1280_FREQ_GHZ_TO_REG(2.456),
    SX1280_FREQ_GHZ_TO_REG(2.457),
    SX1280_FREQ_GHZ_TO_REG(2.458),
    SX1280_FREQ_GHZ_TO_REG(2.459),
    SX1280_FREQ_GHZ_TO_REG(2.460),

    SX1280_FREQ_GHZ_TO_REG(2.461), // channel 60
    SX1280_FREQ_GHZ_TO_REG(2.462),
    SX1280_FREQ_GHZ_TO_REG(2.463),
    SX1280_FREQ_GHZ_TO_REG(2.464),
    SX1280_FREQ_GHZ_TO_REG(2.465),
    SX1280_FREQ_GHZ_TO_REG(2.466),
    SX1280_FREQ_GHZ_TO_REG(2.467),
    SX1280_FREQ_GHZ_TO_REG(2.468),
    SX1280_FREQ_GHZ_TO_REG(2.469),
    SX1280_FREQ_GHZ_TO_REG(2.470),

    SX1280_FREQ_GHZ_TO_REG(2.471), // channel 70
    SX1280_FREQ_GHZ_TO_REG(2.472),
    SX1280_FREQ_GHZ_TO_REG(2.473),
    SX1280_FREQ_GHZ_TO_REG(2.474),
    SX1280_FREQ_GHZ_TO_REG(2.475),
    SX1280_FREQ_GHZ_TO_REG(2.476),
    SX1280_FREQ_GHZ_TO_REG(2.477),
    SX1280_FREQ_GHZ_TO_REG(2.478),
    SX1280_FREQ_GHZ_TO_REG(2.479),
    SX1280_FREQ_GHZ_TO_REG(2.480), // channel 79
};

const uint8_t fhss_bind_channel_list_2p4[] = {
    46, 14, 68 // just pick some
};
#endif


//-------------------------------------------------------
// FHSS Class
//-------------------------------------------------------
// SX_FHSS_CONFIG_FREQUENCY_BAND_ENUM is in setup_types.h, for convenience
// no FHSS_ORTHO_ENUM, we use the enum in setup_types.h
// no FHSS_EXCEPT_ENUM, we use the enum in setup_types.h

typedef struct
{
    const uint32_t* freq_list;
    uint8_t freq_list_len;
    const uint8_t* bind_channel_list;
    uint8_t bind_channel_list_len;
} tFhssConfig;


// ATTENTION: this must be in exactly the same order as SX_FHSS_CONFIG_FREQUENCY_BAND_ENUM
const tFhssConfig fhss_config[] = {
#ifdef FHSS_HAS_CONFIG_2P4_GHZ
    {
        .freq_list = fhss_freq_list_2p4,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_2p4) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_2p4,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_2p4) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
#ifdef FHSS_HAS_CONFIG_915_MHZ_FCC
    {
        .freq_list = fhss_freq_list_915_fcc,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_915_fcc) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_915_fcc,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_915_fcc) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
#ifdef FHSS_HAS_CONFIG_868_MHZ
    {
        .freq_list = fhss_freq_list_868,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_868) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_868,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_868) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
#ifdef FHSS_HAS_CONFIG_866_MHZ_IN
    {
        .freq_list = fhss_freq_list_866_in,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_866_in) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_866_in,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_866_in) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
#ifdef FHSS_HAS_CONFIG_433_MHZ
    {
        .freq_list = fhss_freq_list_433,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_433) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_433,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_433) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
#ifdef FHSS_HAS_CONFIG_70_CM_HAM
    {
        .freq_list = fhss_freq_list_70_cm_ham,
        .freq_list_len = (uint8_t)(sizeof(fhss_freq_list_70_cm_ham) / sizeof(uint32_t)),
        .bind_channel_list = fhss_bind_channel_list_70_cm_ham,
        .bind_channel_list_len = (uint8_t)(sizeof(fhss_bind_channel_list_70_cm_ham) / sizeof(uint8_t))
    },
#else
    { .freq_list = nullptr },
#endif
};


class tFhssBase
{
  public:
    void Init(uint8_t fhss_num, uint32_t seed, SX_FHSS_CONFIG_FREQUENCY_BAND_ENUM frequency_band, uint16_t fb_allowed_mask, uint8_t ortho, uint8_t except)
    {
        if (fhss_num > FHSS_MAX_NUM) while(1){} // should not happen, but play it safe

        config_i = frequency_band;

        if (config_i >= SX_FHSS_CONFIG_FREQUENCY_BAND_NUM) while(1){} // should not happen, but play it safe
        if (fhss_config[config_i].freq_list == nullptr) while(1){} // should not happen, but play it safe

        fhss_freq_list = fhss_config[config_i].freq_list;
        FREQ_LIST_LEN = fhss_config[config_i].freq_list_len;
        fhss_bind_channel_list = fhss_config[config_i].bind_channel_list;
        BIND_CHANNEL_LIST_LEN = fhss_config[config_i].bind_channel_list_len;
        curr_bind_config_i = config_i; // we start with what setup suggests

        bind_scan_mask = 0;
        // looks a bit silly but is to mask out invalid bits
        // could do bind_scan_mask = fb_allowed_mask & ~(0xFFFF << SX_FHSS_CONFIG_FREQUENCY_BAND_NUM);
        for (uint8_t i = 0; i < SX_FHSS_CONFIG_FREQUENCY_BAND_NUM; i++) {
            if (fb_allowed_mask & (1 << i)) bind_scan_mask |= (1 << i);
        }

        if (bind_scan_mask == 0) while(1){} // should not happen, but play it safe

        uint8_t cnt_max = (FREQ_LIST_LEN - BIND_CHANNEL_LIST_LEN);
        if (fhss_num > cnt_max) fhss_num = cnt_max;

        cnt = fhss_num;

        switch (config_i) {
        case SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ:
            // we may need to adapt cnt in case of ortho != NONE,
            // since the cnt_max = 80 - 3 = 77 channels may not be enough channels
            // in case of also except we actually only have 77 - 22 = 55 channels or so...
            // it is not so nice to do it here locally, as we break configuration by defines, but let's be ok for now
            // probably should be done in setup_configure()?
            //
            // cnt should be 12, 18, 24 for the 19 Hz, 31 Hz, 50 Hz modes
            //
            // TODO: check what everything is related to fhss_num/cnt
            // disconnect/connect etc should NOT depend on it
            // TODO: check if modifying cnt from the defines doesn't have any side effects
            //
            // Config.connect_listen_hop_cnt does depend on Config.FhssNum !!!
            // is used by rx to cycle through frequencies when in LISTEN
            if (ortho >= ORTHO_1_3 && ortho <= ORTHO_3_3) {
                // we narrow down to 12 or 18
                if (except >= EXCEPT_2P4_GHZ_WIFIBAND_1 && except <= EXCEPT_2P4_GHZ_WIFIBAND_13) {
                    // we only have 55 channels or so, so narrow down to 12 (12 * 3 = 36 < 55)
                    if (cnt > 12) cnt = 12;
                } else {
                    // we have 77 channels, so can accommodate up to 18 frequencies (18 * 3 = 54 < 77)
                    if (cnt > 18) cnt = 18;
                    except = EXCEPT_NONE;
                }
                if (cnt > fhss_num) cnt = fhss_num;
            } else {
                ortho = ORTHO_NONE;
                if (except > EXCEPT_2P4_GHZ_WIFIBAND_13) except = EXCEPT_NONE;
            }
            generate_ortho_except(seed, ortho, except);
            break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC:
            if (ortho >= ORTHO_1_3 && ortho <= ORTHO_3_3) {
                if (cnt > 12) cnt = 12; // 42 channels, so can accommodate up to 12 frequencies (12 * 3 = 36 < 42)
                if (cnt > fhss_num) cnt = fhss_num;
            } else {
                ortho = ORTHO_NONE;
            }
            generate_ortho_except(seed, ortho, EXCEPT_NONE);
            break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM:
            if (ortho >= ORTHO_1_3 && ortho <= ORTHO_3_3) {
                if (cnt > 8) cnt = 8; // 31 channels, so can accommodate up to 8 frequencies (8 * 3 = 24 < 31)
                if (cnt > fhss_num) cnt = fhss_num;
            } else {
                ortho = ORTHO_NONE;
            }
            generate_ortho_except(seed, ortho, EXCEPT_NONE);
            break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ:
        case SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN:
        case SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ:
            generate(seed);
            break;
        default:
            while(1){} // should not happen, but play it safe
        }

        is_in_binding = false;

        curr_i = 0;
    }

    void Start(void)
    {
        curr_i = 0;
    }

    // only used for statistics
    uint8_t Cnt(void)
    {
        return cnt;
    }

    // only used for statistics
    uint8_t CurrI(void)
    {
        return curr_i;
    }

    uint32_t GetCurrFreq(void)
    {
        if (is_in_binding) {
            const uint32_t* curr_bind_freq_list = fhss_config[curr_bind_config_i].freq_list;
            const uint8_t* curr_bind_channel_list = fhss_config[curr_bind_config_i].bind_channel_list;
            return curr_bind_freq_list[curr_bind_channel_list[0]];
        }

        return fhss_list[curr_i];
    }

    void HopToNext(void)
    {
        curr_i++;
        if (curr_i >= cnt) curr_i = 0;
    }

    void SetToBind(uint16_t frame_rate_ms = 1) // preset so it is good for transmitter
    {
        is_in_binding = true;
        bind_listen_cnt = (5000 / frame_rate_ms); // should be 5 secs
        bind_listen_i = 0;
    }

    // only used by receiver, bool determines if it needs to switch back to LINK_STATE_RECEIVE
    bool HopToNextBind(void)
    {
        if (!is_in_binding) return false;

        bind_listen_i++;
        if (bind_listen_i >= bind_listen_cnt) {
            bind_listen_i = 0;
            // find next bind frequency
            uint8_t iii = curr_bind_config_i;
            for (uint8_t i = 0; i < SX_FHSS_CONFIG_FREQUENCY_BAND_NUM; i++) { // we give it at most that much attempts
                iii++;
                if (iii >= SX_FHSS_CONFIG_FREQUENCY_BAND_NUM) iii = 0;
                if ((bind_scan_mask & (1 << iii)) != 0) {
                    if (fhss_config[iii].freq_list == nullptr) while(1){} // should not happen, but play it safe
                    curr_bind_config_i = iii;
                    return true;
                }
            }
        }

        return false;
    }

    // only used by receiver
    SETUP_FREQUENCY_BAND_ENUM GetCurrBindSetupFrequencyBand(void)
    {
        switch (curr_bind_config_i) {
        case SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ: return SETUP_FREQUENCY_BAND_2P4_GHZ;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC: return SETUP_FREQUENCY_BAND_915_MHZ_FCC;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ: return SETUP_FREQUENCY_BAND_868_MHZ;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN: return SETUP_FREQUENCY_BAND_866_MHZ_IN;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ: return SETUP_FREQUENCY_BAND_433_MHZ;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM: return SETUP_FREQUENCY_BAND_70_CM_HAM;
        }
        while(1){} // should not happen, but play it safe
        return (SETUP_FREQUENCY_BAND_ENUM)0;
    }

    // used by RADIO_LINK_STATS_MLRS
    float GetCurrFreq_Hz(void)
    {
#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
        return 1.0E3f * SX126X_REG_TO_FREQ_KHZ(GetCurrFreq());
#elif defined DEVICE_HAS_SX127x
        return 1.0E3f * SX127X_REG_TO_FREQ_KHZ(GetCurrFreq());
#elif defined DEVICE_HAS_LR11xx
        return 1.0E3f * LR11XX_REG_TO_FREQ_KHZ(GetCurrFreq());
#else // DEVICE_HAS_SX128x
        return 1.0E6f * SX1280_REG_TO_FREQ_MHZ(GetCurrFreq());
#endif
    }

    uint32_t bestX(void)
    {
        uint8_t i_best = 0;
        for (uint8_t i = 0; i < cnt; i++) {
          if (fhss_last_rssi[i] > fhss_last_rssi[i_best]) i_best = i;
        }

        curr_i = i_best;
        return fhss_list[curr_i];
    }

    // used by CLI
    uint8_t ChList(uint8_t i) { return ch_list[i]; }
    uint32_t FhssList(uint8_t i) { return fhss_list[i]; }

    uint32_t GetFreq_x1000(char* const unit_str, uint8_t i)
    {
#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
        strcpy(unit_str, " kHz");
        return (uint32_t)SX126X_REG_TO_FREQ_KHZ(fhss_list[i]);
#elif defined DEVICE_HAS_SX127x
        strcpy(unit_str, " kHz");
        return (uint32_t)SX127X_REG_TO_FREQ_KHZ(fhss_list[i]);
#elif defined DEVICE_HAS_LR11xx
        strcpy(unit_str, " kHz");
        return (uint32_t)LR11XX_REG_TO_FREQ_KHZ(fhss_list[i]);
#else // DEVICE_HAS_SX128x
        strcpy(unit_str, " MHz");
        return (uint32_t)SX1280_REG_TO_FREQ_MHZ(fhss_list[i]);
#endif
    }

  private:
    uint32_t _seed;
    uint8_t _ortho;
    uint8_t _except;

    uint8_t config_i;
    uint8_t FREQ_LIST_LEN;
    const uint32_t* fhss_freq_list;
    uint8_t BIND_CHANNEL_LIST_LEN;
    const uint8_t* fhss_bind_channel_list;
    uint16_t bind_scan_mask;

    uint8_t curr_i;
    uint8_t cnt;
    uint8_t ch_list[FHSS_MAX_NUM]; // that's our list of randomly selected channels
    uint32_t fhss_list[FHSS_MAX_NUM]; // that's our list of randomly selected frequencies

    int8_t fhss_last_rssi[FHSS_MAX_NUM];

    bool is_in_binding;
    uint8_t curr_bind_config_i;
    uint16_t bind_listen_cnt;
    uint16_t bind_listen_i;

    uint16_t prng(void);
    void generate(uint32_t seed);
    void generate_ortho_except(uint32_t seed, uint8_t ortho, uint8_t except);
};


#if !defined DEVICE_HAS_DUAL_SX126x_SX128x && !defined DEVICE_HAS_DUAL_SX126x_SX126x

class tFhss : public tFhssBase
{
  public:
    void Init(tFhssGlobalConfig* const fhss, tFhssGlobalConfig* const fhss2)
    {
        tFhssBase::Init(fhss->Num, fhss->Seed,
                          fhss->FrequencyBand, fhss->FrequencyBand_allowed_mask,
                          fhss->Ortho, fhss->Except);
    }

    uint32_t GetCurrFreq2(void) { return GetCurrFreq(); }

    float GetCurrFreq2_Hz(void) { return GetCurrFreq_Hz(); }
};

#else
// DUALBAND !

class tFhss
{
  public:
    void Init(tFhssGlobalConfig* const fhss, tFhssGlobalConfig* const fhss2)
    {
        fhss1stBand.Init(fhss->Num, fhss->Seed,
                          fhss->FrequencyBand, fhss->FrequencyBand_allowed_mask,
                          fhss->Ortho, fhss->Except);
        fhss2ndBand.Init(fhss2->Num, fhss2->Seed,
                          fhss2->FrequencyBand, fhss2->FrequencyBand_allowed_mask,
                          fhss2->Ortho, fhss2->Except);
    }

    void Start(void)
    {
        fhss1stBand.Start();
        fhss2ndBand.Start();
    }

    uint8_t Cnt(void)
    {
        return fhss1stBand.Cnt();
    }

    uint8_t CurrI(void)
    {
        return fhss1stBand.CurrI();
    }

    uint32_t GetCurrFreq(void) { return fhss1stBand.GetCurrFreq(); }
    uint32_t GetCurrFreq2(void) { return fhss2ndBand.GetCurrFreq(); }

    void HopToNext(void)
    {
        fhss1stBand.HopToNext();
        fhss2ndBand.HopToNext();
    }

    void SetToBind(uint16_t frame_rate_ms = 1) // preset so it is good for transmitter
    {
        fhss1stBand.SetToBind(frame_rate_ms);
        fhss2ndBand.SetToBind(frame_rate_ms);
    }

    // only used by receiver, bool determines if it needs to switch back to LINK_STATE_RECEIVE
    bool HopToNextBind(void)
    {
        bool hop1 = fhss1stBand.HopToNextBind();
        bool hop2 = fhss2ndBand.HopToNextBind();
        return hop1 || hop2;
    }

    // only used by receiver
    SETUP_FREQUENCY_BAND_ENUM GetCurrBindSetupFrequencyBand(void) { return fhss1stBand.GetCurrBindSetupFrequencyBand(); }
    float GetCurrFreq_Hz(void) { return fhss1stBand.GetCurrFreq_Hz(); }

    float GetCurrFreq2_Hz(void)
    {
#if defined DEVICE_HAS_DUAL_SX126x_SX126x
        return 1.0E3f * SX126X_REG_TO_FREQ_KHZ(GetCurrFreq2());
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x
        return 1.0E6f * SX1280_REG_TO_FREQ_MHZ(GetCurrFreq2());
#else
        #error Something wrong with dual band config !
#endif
    }

    // only used by tx cli
    uint8_t ChList(uint8_t i) { return fhss1stBand.ChList(i); }
    uint32_t FhssList(uint8_t i) { return fhss1stBand.FhssList(i); }
    uint32_t GetFreq_x1000(char* const unit_str, uint8_t i) { return fhss1stBand.GetFreq_x1000(unit_str, i); }

  private:
    tFhssBase fhss1stBand;
    tFhssBase fhss2ndBand;
};

#endif

#endif // FHSS_H
