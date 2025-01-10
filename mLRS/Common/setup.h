//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// Setup
//*******************************************************
#ifndef SETUP_H
#define SETUP_H
#pragma once


#include "setup_types.h"
#include "hal/hal.h"


tSetupMetaData SetupMetaData;
tSetup Setup;
tGlobalConfig Config;


//-------------------------------------------------------
// Configure Setup MetaData
//-------------------------------------------------------

void setup_configure_metadata(void)
{
    SetupMetaData = {};

    //-- FrequencyBand: "2.4,915 FCC,868,433,70,866 IN"
#if defined DEVICE_HAS_DUAL_SX126x_SX128x
  // DUALBAND 2.4 GHz & 868/915 MHz !
  #if defined FREQUENCY_BAND_2P4_GHZ && defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b000110; // 915 FCC, 868
  #else
    #error Unknown Frequencyband !
  #endif
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
  // DUALBAND 868/915 MHz & 433 MHz !
  #if defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && defined FREQUENCY_BAND_433_MHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b000110; // 915 FCC, 868
  #else
    #error Unknown Frequencyband !
  #endif
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && \
      defined FREQUENCY_BAND_433_MHZ && defined FREQUENCY_BAND_70_CM_HAM
    SetupMetaData.FrequencyBand_allowed_mask = 0b011110; // 915 FCC, 868, 433, 70
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && defined FREQUENCY_BAND_866_MHZ_IN
    SetupMetaData.FrequencyBand_allowed_mask = 0b100110; // 915 FCC, 868, 866 IN
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b000110; // 915 FCC, 868
#elif defined FREQUENCY_BAND_433_MHZ && defined FREQUENCY_BAND_70_CM_HAM
    SetupMetaData.FrequencyBand_allowed_mask = 0b011000; // 433, 70
#elif defined FREQUENCY_BAND_2P4_GHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b000001; // 2.4 GHz, not editable
#elif defined FREQUENCY_BAND_915_MHZ_FCC
    SetupMetaData.FrequencyBand_allowed_mask = 0b000010; // 915 MHz FCC, not editable
#elif defined FREQUENCY_BAND_868_MHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b000100; // 868 MHz, not editable
#elif defined FREQUENCY_BAND_433_MHZ
    SetupMetaData.FrequencyBand_allowed_mask = 0b001000; // 433 MHz, not editable
#elif defined FREQUENCY_BAND_70_CM_HAM
    SetupMetaData.FrequencyBand_allowed_mask = 0b010000; // 70 cm HAM, not editable
#elif defined FREQUENCY_BAND_866_MHZ_IN
    SetupMetaData.FrequencyBand_allowed_mask = 0b100000; // 866 MHz IN, not editable
#else
    #error Unknown Frequencyband !
#endif

    //-- Mode: "50 Hz,31 Hz,19 Hz,FLRC,FSK"
#if defined DEVICE_HAS_DUAL_SX126x_SX128x
    // DUALBAND 2.4 GHz & 868/915 MHz !
    SetupMetaData.Mode_allowed_mask = 0b10110; // 31 Hz, 19 Hz, FSK  Note: FSK implies 50 Hz for SX128x
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 868/915 MHz & 433 MHz !
    SetupMetaData.Mode_allowed_mask = 0b00110; // 31 Hz, 19 Hz
#elif defined DEVICE_HAS_SX128x
  #ifdef USE_FEATURE_FLRC
    SetupMetaData.Mode_allowed_mask = 0b01111; // 50 Hz, 31 Hz, 19 Hz, FLRC
  #else
    SetupMetaData.Mode_allowed_mask = 0b00111; // 50 Hz, 31 Hz, 19 Hz
  #endif
#elif defined DEVICE_HAS_SX126x
    SetupMetaData.Mode_allowed_mask = 0b10110; // 31 Hz, 19 Hz, FSK
#elif defined DEVICE_HAS_SX127x
    SetupMetaData.Mode_allowed_mask = 0b00100; // 19 Hz, not editable
#elif defined DEVICE_HAS_LR11xx
    SetupMetaData.Mode_allowed_mask = 0b00110; // 19 Hz, 31 Hz
#endif

    //-- Ortho: "off,1/3,2/3,3/3"
    // we cannot work out all cases here, since it also depends on the actual selection, so we just do what we can do
#if defined FREQUENCY_BAND_2P4_GHZ || defined FREQUENCY_BAND_915_MHZ_FCC || defined FREQUENCY_BAND_70_CM_HAM
    SetupMetaData.Ortho_allowed_mask = 0b1111; // all
#else
    SetupMetaData.Ortho_allowed_mask = 0; // not available, do not display
#endif

    //-- Tx:

    power_optstr_from_rfpower_list(SetupMetaData.Tx_Power_optstr, rfpower_list, RFPOWER_LIST_NUM, 44);

    // Diversity: "enabled,antenna1,antenna2,r:e t:a1,r:e t:a2"
#if defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 2.4 GHz & 868/915 MHz or 868/915 MHz & 433 MHz !
    SetupMetaData.Tx_Diversity_allowed_mask = 0b00001; // only enabled, not editable
#elif defined DEVICE_HAS_DIVERSITY
    SetupMetaData.Tx_Diversity_allowed_mask = 0b11111; // all
#elif defined DEVICE_HAS_DIVERSITY_SINGLE_SPI
    SetupMetaData.Tx_Diversity_allowed_mask = 0b11011; // TODO: no antenna2 for the moment
#else
    SetupMetaData.Tx_Diversity_allowed_mask = 0b00010; // antenna1, not editable
#endif

    // Tx ChannelSource: "none,crsf,in,mbridge"
#if defined DEVICE_HAS_JRPIN5 && defined USE_IN
    SetupMetaData.Tx_ChannelsSource_allowed_mask = 0b1111; // none, crsf, in, mBridge
#elif defined DEVICE_HAS_JRPIN5
    SetupMetaData.Tx_ChannelsSource_allowed_mask = 0b1011; // none, crsf, mBridge
#elif defined USE_IN
    SetupMetaData.Tx_ChannelsSource_allowed_mask = 0b0101; // none, in
#else
    SetupMetaData.Tx_ChannelsSource_allowed_mask = 0b0001; // none, not editable
#endif

    // Tx InMode: "sbus,sbus inv"
#if defined DEVICE_HAS_IN || defined DEVICE_HAS_IN_ON_JRPIN5_RX || defined DEVICE_HAS_IN_ON_JRPIN5_TX
    SetupMetaData.Tx_InMode_allowed_mask = 0b11; // all
#elif defined DEVICE_HAS_IN_NORMAL
    SetupMetaData.Tx_InMode_allowed_mask = 0b10; // sbus inv, not editable
#elif defined DEVICE_HAS_IN_INVERTED
    SetupMetaData.Tx_InMode_allowed_mask = 0b01; // sbus, not editable
#else
    SetupMetaData.Tx_InMode_allowed_mask = 0; // not available, do not display
#endif

    // Tx SerialDestination: "serial,serial2,mbridge"
#if defined DEVICE_HAS_JRPIN5 && defined USE_SERIAL2
    SetupMetaData.Tx_SerialDestination_allowed_mask = 0b111; // all
#elif defined DEVICE_HAS_JRPIN5
    SetupMetaData.Tx_SerialDestination_allowed_mask = 0b101; // serial, mbridge
#elif defined USE_SERIAL2
    SetupMetaData.Tx_SerialDestination_allowed_mask = 0b011; // serial, serial2
#else
    SetupMetaData.Tx_SerialDestination_allowed_mask = 0b001; // serial, not editable
#endif

    // Tx Buzzer: ""off,LP,rxLQ"
#ifdef DEVICE_HAS_BUZZER
    SetupMetaData.Tx_Buzzer_allowed_mask = UINT16_MAX; // all
#else
    SetupMetaData.Tx_Buzzer_allowed_mask = 0; // not available, do not display
#endif

    //-- Rx:

    power_optstr_from_rfpower_list(SetupMetaData.Rx_Power_optstr, rfpower_list, RFPOWER_LIST_NUM, 44);

    // Rx Diversity: "enabled,antenna1,antenna2,r:e t:a1,r:e t:a2"
#if defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 2.4 GHz & 868/915 MHz or 868/915 MHz & 433 MHz !
    SetupMetaData.Rx_Diversity_allowed_mask = 0b00001; // only enabled, not editable
#elif defined DEVICE_HAS_DIVERSITY
    SetupMetaData.Rx_Diversity_allowed_mask = 0b11111; // all
#elif defined DEVICE_HAS_DIVERSITY_SINGLE_SPI
    SetupMetaData.Rx_Diversity_allowed_mask = 0b11011; // TODO: no antenna2 for the moment
#else
    SetupMetaData.Rx_Diversity_allowed_mask = 0b00010; // antenna1, not editable
#endif

    // Rx OutMode: "sbus,crsf,sbus inv"
#ifdef DEVICE_HAS_OUT
    SetupMetaData.Rx_OutMode_allowed_mask = 0b111; // all
#elif defined DEVICE_HAS_OUT_NORMAL
    SetupMetaData.Rx_OutMode_allowed_mask = 0b110; // crsf,sbus inv
#elif defined DEVICE_HAS_OUT_INVERTED
    SetupMetaData.Rx_OutMode_allowed_mask = 0b001; // sbus, not editable
#else
    SetupMetaData.Rx_OutMode_allowed_mask = 0; // not available, do not display
#endif

    // Rx SerialPort: "serial,can"
    SetupMetaData.Rx_SerialPort_allowed_mask = 0;  // not available, do not display
#ifdef USE_SERIAL
    SetupMetaData.Rx_SerialPort_allowed_mask |= 0b01; // add serial
#endif
#ifdef DEVICE_HAS_DRONECAN
    SetupMetaData.Rx_SerialPort_allowed_mask |= 0b10; // add can
#endif

    //-- Tx: Receiver setup meta data

    SetupMetaData.rx_available = false;

    SetupMetaData.rx_firmware_version = 0;
    SetupMetaData.rx_setup_layout = 0;
    strcpy(SetupMetaData.rx_device_name, "");
    SetupMetaData.rx_actual_power_dbm = INT8_MAX;
    SetupMetaData.rx_actual_diversity = DIVERSITY_NUM;
}


//-------------------------------------------------------
// Setup
//-------------------------------------------------------

// override default setup setting from common_conf.h
// TODO: when AP4.6 is out, it should become SEND_RC_CHANNELS_RADIORCCHANNELS
#if !defined USE_OUT || defined ESP32 || defined ESP8266
  #undef SETUP_RX_SEND_RC_CHANNELS
  #define SETUP_RX_SEND_RC_CHANNELS  SEND_RC_CHANNELS_RCCHANNELSOVERRIDE
#endif


void inc_bindphrase_char(char* const s, uint8_t pos)
{
    char* cptr = strchr(bindphrase_chars, s[pos]);
    uint8_t n = (cptr) ? cptr - bindphrase_chars + 1 : 0; // must not happen that c is not found, but play it safe
    if (n >= strlen(bindphrase_chars)) n = 0;
    s[pos] = bindphrase_chars[n];
}


// make the default bind phrases all different for all config id's
// this provides a sort of model match per default
void setup_default_bindphrase(char* const s, uint8_t config_id, const char* bindphrase_default)
{
    strcpy(s, bindphrase_default);
    if (config_id == 0) return;

#ifdef FREQUENCY_BAND_2P4_GHZ // a bit more to do because of the "except" option
    switch (config_id) {
        case 1: s[5] = '5'; break;
        case 2: s[5] = 'a'; break;
        case 3: s[5] = 'f'; break;
        case 4: s[5] = 'k'; break;
        case 5: s[5] = 'p'; break;
        case 6: s[5] = 'u'; break;
        case 7: s[5] = 'z'; break;
        case 8: s[5] = '1'; break;
        case 9: s[5] = '2'; break;
    }
    if (s[5] == bindphrase_default[5]) inc_bindphrase_char(s, 5);
#else
    for (uint8_t cnt = 0; cnt < config_id; cnt++) { // inc s[5] config_id many times
        inc_bindphrase_char(s, 5);
    }
#endif
}


void setup_default(uint8_t config_id)
{
    char bind_phrase[6+1];
    setup_default_bindphrase(bind_phrase, config_id, BIND_PHRASE);
    strcpy(Setup.Common[config_id].BindPhrase, bind_phrase);

    Setup.Common[config_id].FrequencyBand = SETUP_RF_BAND;
    Setup.Common[config_id].Mode = SETUP_MODE;
    Setup.Common[config_id].Ortho = SETUP_RF_ORTHO;

    Setup.Tx[config_id].Power = SETUP_TX_POWER;
    Setup.Tx[config_id].Diversity = SETUP_TX_DIVERSITY;
    Setup.Tx[config_id].SerialDestination = SETUP_TX_SERIAL_DESTINATION;
    Setup.Tx[config_id].ChannelsSource = SETUP_TX_CHANNELS_SOURCE;
    Setup.Tx[config_id].ChannelOrder = SETUP_TX_CHANNEL_ORDER;
    Setup.Tx[config_id].InMode = SETUP_TX_IN_MODE;
    Setup.Tx[config_id].SerialBaudrate = SETUP_TX_SERIAL_BAUDRATE;
    Setup.Tx[config_id].SendRadioStatus = SETUP_TX_SEND_RADIO_STATUS;
    Setup.Tx[config_id].Buzzer = SETUP_TX_BUZZER;
    Setup.Tx[config_id].MavlinkComponent = SETUP_TX_MAV_COMPONENT;
    Setup.Tx[config_id].PowerSwitchChannel = POWER_SWITCH_CHANNEL_OFF; //SETUP_TX_POWER_SW_CH;

    Setup.Tx[config_id].WifiProtocol = WIFI_PROTOCOL_UDP;
    Setup.Tx[config_id].WifiChannel = WIFI_CHANNEL_6;
    Setup.Tx[config_id].WifiPower = WIFI_POWER_MED;

    Setup.Rx.Power = SETUP_RX_POWER;
    Setup.Rx.Diversity = SETUP_RX_DIVERSITY;
    Setup.Rx.ChannelOrder = SETUP_RX_CHANNEL_ORDER;
    Setup.Rx.OutMode = SETUP_RX_OUT_MODE;
    Setup.Rx.OutRssiChannelMode = SETUP_RX_OUT_RSSI_CHANNEL;
    Setup.Rx.FailsafeMode = SETUP_RX_FAILSAFE_MODE;
    Setup.Rx.SerialPort = SETUP_RX_SERIAL_PORT;
    Setup.Rx.SerialBaudrate = SETUP_RX_SERIAL_BAUDRATE;
    Setup.Rx.SerialLinkMode = SETUP_RX_SERIAL_LINK_MODE;
    Setup.Rx.SendRadioStatus = SETUP_RX_SEND_RADIO_STATUS;
    Setup.Rx.SendRcChannels = SETUP_RX_SEND_RC_CHANNELS;
    Setup.Rx.PowerSwitchChannel = POWER_SWITCH_CHANNEL_OFF; //SETUP_RX_POWER_SW_CH;

    for (uint8_t ch = 0; ch < 12; ch++) { Setup.Rx.FailsafeOutChannelValues_Ch1_Ch12[ch] = 0; }
    for (uint8_t ch = 0; ch < 4; ch++) { Setup.Rx.FailsafeOutChannelValues_Ch13_Ch16[ch] = 1; }
}


void setup_sanitize_config(uint8_t config_id)
{
// note: if allowed_mask = 0, this also triggers
// we may have to distinguish between not editable and not displayed, but currently
// not displayed applies only to settings for which the default is ok

#define TST_NOTALLOWED_TYPED(amask,pfield,val,ptype) \
    if ((SetupMetaData.amask & (1 << Setup.pfield)) == 0) { Setup.pfield = (ptype)val; } \
    if (SetupMetaData.amask != 0 && (SetupMetaData.amask & (1 << Setup.pfield)) == 0) { \
      for (uint8_t i = 0; i < 16; i++) { if (SetupMetaData.amask & (1 << i)) { Setup.pfield = (ptype)i; break; } } \
    }

#define TST_NOTALLOWED(amask,pfield,val) \
    if ((SetupMetaData.amask & (1 << Setup.pfield)) == 0) { Setup.pfield = val; } \
    if (SetupMetaData.amask != 0 && (SetupMetaData.amask & (1 << Setup.pfield)) == 0) { \
      for (uint8_t i = 0; i < 16; i++) { if (SetupMetaData.amask & (1 << i)) { Setup.pfield = i; break; } } \
    }

#define SANITIZE(pfield,max,setupval,val) \
    if (Setup.pfield >= max) { Setup.pfield = setupval; } \
    if (Setup.pfield >= max) { Setup.pfield = val; }

    //-- BindPhrase, FrequencyBand, Mode, Ortho

    char bind_phrase[6+1];
    setup_default_bindphrase(bind_phrase, config_id, BIND_PHRASE);
    sanitize_bindphrase(Setup.Common[config_id].BindPhrase, bind_phrase);

#if defined DEVICE_HAS_DUAL_SX126x_SX128x
  // DUALBAND 2.4 GHz & 868/915 MHz !
  #if defined FREQUENCY_BAND_2P4_GHZ && defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
  #endif
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
  // DUALBAND 868/915 MHz & 433 MHz !
  #if defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && defined FREQUENCY_BAND_433_MHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
  #endif
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && \
      defined FREQUENCY_BAND_433_MHZ && defined FREQUENCY_BAND_70_CM_HAM
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ && defined FREQUENCY_BAND_866_MHZ_IN
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
#elif defined FREQUENCY_BAND_915_MHZ_FCC && defined FREQUENCY_BAND_868_MHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
#elif defined FREQUENCY_BAND_433_MHZ && defined FREQUENCY_BAND_70_CM_HAM
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_433_MHZ;
#elif defined FREQUENCY_BAND_2P4_GHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_2P4_GHZ;
#elif defined FREQUENCY_BAND_915_MHZ_FCC
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_915_MHZ_FCC;
#elif defined FREQUENCY_BAND_868_MHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_868_MHZ;
#elif defined FREQUENCY_BAND_433_MHZ
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_433_MHZ;
#elif defined FREQUENCY_BAND_70_CM_HAM
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_70_CM_HAM;
#elif defined FREQUENCY_BAND_866_MHZ_IN
    uint8_t frequency_band_default = SETUP_FREQUENCY_BAND_866_MHZ_IN;
#endif
    if (Setup.Common[config_id].FrequencyBand >= SETUP_FREQUENCY_BAND_NUM) {
        Setup.Common[config_id].FrequencyBand = (SETUP_FREQUENCY_BAND_ENUM)frequency_band_default;
    }
    TST_NOTALLOWED_TYPED(FrequencyBand_allowed_mask, Common[config_id].FrequencyBand, frequency_band_default, SETUP_FREQUENCY_BAND_ENUM);

#ifdef DEVICE_HAS_SX128x
    constexpr uint8_t fallback_mode = MODE_50HZ;
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x || defined DEVICE_HAS_SX126x
    constexpr uint8_t fallback_mode = MODE_31HZ;
#elif defined DEVICE_HAS_SX127x
    constexpr uint8_t fallback_mode = MODE_19HZ;
#elif defined DEVICE_HAS_LR11xx
    constexpr uint8_t fallback_mode = MODE_31HZ;
#endif
    SANITIZE(Common[config_id].Mode, MODE_NUM, SETUP_MODE, fallback_mode); // MODE_19HZ
    TST_NOTALLOWED(Mode_allowed_mask, Common[config_id].Mode, fallback_mode); // MODE_19HZ

    SANITIZE(Common[config_id].Ortho, ORTHO_NUM, SETUP_RF_ORTHO, ORTHO_NONE);
    TST_NOTALLOWED(Ortho_allowed_mask, Common[config_id].Ortho, ORTHO_NONE);
    switch (Setup.Common[config_id].FrequencyBand) { // restrict ortho to 2.4GHz, 915FCC, 70CM
    case SETUP_FREQUENCY_BAND_2P4_GHZ:
    case SETUP_FREQUENCY_BAND_915_MHZ_FCC:
    case SETUP_FREQUENCY_BAND_70_CM_HAM:
        break;
    default:
        Setup.Common[config_id].Ortho = ORTHO_NONE;
    }

    //-- Tx:

    SANITIZE(Tx[config_id].Power, RFPOWER_LIST_NUM, SETUP_TX_POWER, RFPOWER_LIST_NUM - 1);

    SANITIZE(Tx[config_id].Diversity, DIVERSITY_NUM, SETUP_TX_DIVERSITY, DIVERSITY_DEFAULT);
    TST_NOTALLOWED(Tx_Diversity_allowed_mask, Tx[config_id].Diversity, DIVERSITY_ANTENNA1);

    SANITIZE(Tx[config_id].ChannelsSource, CHANNEL_SOURCE_NUM, SETUP_TX_CHANNELS_SOURCE, CHANNEL_SOURCE_NONE);
    TST_NOTALLOWED(Tx_ChannelsSource_allowed_mask, Tx[config_id].ChannelsSource, CHANNEL_SOURCE_NONE);

    SANITIZE(Tx[config_id].InMode, IN_CONFIG_NUM, SETUP_TX_IN_MODE, IN_CONFIG_SBUS);
    TST_NOTALLOWED(Tx_InMode_allowed_mask, Tx[config_id].InMode, IN_CONFIG_SBUS);

    SANITIZE(Tx[config_id].SerialDestination, SERIAL_DESTINATION_NUM, SETUP_TX_SERIAL_DESTINATION, SERIAL_DESTINATION_SERIAL);
    TST_NOTALLOWED(Tx_SerialDestination_allowed_mask, Tx[config_id].SerialDestination, SERIAL_DESTINATION_SERIAL);

    SANITIZE(Tx[config_id].ChannelOrder, CHANNEL_ORDER_NUM, SETUP_TX_CHANNEL_ORDER, CHANNEL_ORDER_AETR);

    SANITIZE(Tx[config_id].SerialBaudrate, SERIAL_BAUDRATE_NUM, SETUP_TX_SERIAL_BAUDRATE, SERIAL_BAUDRATE_115200);

    SANITIZE(Tx[config_id].SendRadioStatus, TX_SEND_RADIO_STATUS_NUM, SETUP_TX_SEND_RADIO_STATUS, TX_SEND_RADIO_STATUS_OFF);
    SANITIZE(Tx[config_id].MavlinkComponent, TX_MAVLINK_COMPONENT_NUM, SETUP_TX_MAV_COMPONENT, TX_MAVLINK_COMPONENT_OFF);

    SANITIZE(Tx[config_id].Buzzer, BUZZER_NUM, SETUP_TX_BUZZER, BUZZER_OFF);
    TST_NOTALLOWED(Tx_Buzzer_allowed_mask, Tx[config_id].Buzzer, BUZZER_OFF);

    SANITIZE(Tx[config_id].PowerSwitchChannel, POWER_SWITCH_CHANNEL_NUM, POWER_SWITCH_CHANNEL_OFF, POWER_SWITCH_CHANNEL_OFF);

    // device cannot use mBridge (pin5) and CRSF (pin5) at the same time !
    // dest\src | NONE    | CRSF    | INPORT  | MBRIDGE
    // -------------------------------------------------
    //  SERIAL  |  -      | CRSF    | -       | mBridge
    //  SERIAL2 |  -      | CRSF    | -       | mBridge
    //  MBRDIGE | mBridge | CRSF !! | mBridge | mBridge
    if ((Setup.Tx[config_id].ChannelsSource == CHANNEL_SOURCE_CRSF) &&
        (Setup.Tx[config_id].SerialDestination == SERIAL_DESTINATION_MBRDIGE)) {
        Setup.Tx[config_id].SerialDestination = SERIAL_DESTINATION_SERIAL;
    }

#ifdef USE_ESP_WIFI_BRIDGE_RST_GPIO0
    SANITIZE(Tx[config_id].WifiProtocol, WIFI_PROTOCOL_NUM, WIFI_PROTOCOL_UDP, WIFI_PROTOCOL_UDP);
    SANITIZE(Tx[config_id].WifiChannel, WIFI_CHANNEL_NUM, WIFI_CHANNEL_6, WIFI_CHANNEL_6);
    SANITIZE(Tx[config_id].WifiPower, WIFI_POWER_NUM, WIFI_POWER_MED, WIFI_POWER_MED);
#else
    Setup.Tx[config_id].WifiProtocol = WIFI_PROTOCOL_UDP; // force them to default
    Setup.Tx[config_id].WifiChannel = WIFI_CHANNEL_6;
    Setup.Tx[config_id].WifiPower = WIFI_POWER_MED;
#endif

    //-- Rx:

    SANITIZE(Rx.Power, RFPOWER_LIST_NUM, SETUP_RX_POWER, RFPOWER_LIST_NUM - 1);

    SANITIZE(Rx.Diversity, DIVERSITY_NUM, SETUP_RX_DIVERSITY, DIVERSITY_DEFAULT);
    TST_NOTALLOWED(Rx_Diversity_allowed_mask, Rx.Diversity,  DIVERSITY_ANTENNA1);

    SANITIZE(Rx.ChannelOrder, CHANNEL_ORDER_NUM, SETUP_RX_CHANNEL_ORDER, CHANNEL_ORDER_AETR);

    SANITIZE(Rx.OutMode, OUT_CONFIG_NUM, SETUP_RX_OUT_MODE, OUT_CONFIG_SBUS);
    TST_NOTALLOWED(Rx_OutMode_allowed_mask, Rx.OutMode, OUT_CONFIG_SBUS);

    SANITIZE(Rx.OutRssiChannelMode, OUT_RSSI_LQ_CHANNEL_NUM, SETUP_RX_OUT_RSSI_CHANNEL, 0);

    SANITIZE(Rx.OutLqChannelMode, OUT_RSSI_LQ_CHANNEL_NUM, SETUP_RX_OUT_LQ_CHANNEL, 0);

    SANITIZE(Rx.FailsafeMode, FAILSAFE_MODE_NUM, SETUP_RX_FAILSAFE_MODE, FAILSAFE_MODE_NO_SIGNAL);

    for (uint8_t ch = 0; ch < 12; ch++) {
        if (Setup.Rx.FailsafeOutChannelValues_Ch1_Ch12[ch] < -120) Setup.Rx.FailsafeOutChannelValues_Ch1_Ch12[ch] = 0;
        if (Setup.Rx.FailsafeOutChannelValues_Ch1_Ch12[ch] > 120) Setup.Rx.FailsafeOutChannelValues_Ch1_Ch12[ch] = 0;
    }
    for (uint8_t ch = 0; ch < 4; ch++) {
        if (Setup.Rx.FailsafeOutChannelValues_Ch13_Ch16[ch] > 2) Setup.Rx.FailsafeOutChannelValues_Ch13_Ch16[ch] = 1;
    }

    SANITIZE(Rx.SerialPort, RX_SERIAL_PORT_NUM, SETUP_RX_SERIAL_PORT, RX_SERIAL_PORT_SERIAL);
    TST_NOTALLOWED(Rx_SerialPort_allowed_mask, Rx.SerialPort, RX_SERIAL_PORT_SERIAL);

    SANITIZE(Rx.SerialBaudrate, SERIAL_BAUDRATE_NUM, SETUP_RX_SERIAL_BAUDRATE, SERIAL_BAUDRATE_57600);

    SANITIZE(Rx.SerialLinkMode, SERIAL_LINK_MODE_NUM, SETUP_RX_SERIAL_LINK_MODE, SERIAL_LINK_MODE_TRANSPARENT);
    if (SERIAL_LINK_MODE_IS_MSP(Setup.Rx.SerialLinkMode)) Setup.Rx.SerialPort = RX_SERIAL_PORT_SERIAL; // don't do CAN when MSPX

    SANITIZE(Rx.SendRadioStatus, RX_SEND_RADIO_STATUS_NUM, SETUP_RX_SEND_RADIO_STATUS, RX_SEND_RADIO_STATUS_OFF);

    SANITIZE(Rx.SendRcChannels, SEND_RC_CHANNELS_NUM, SETUP_RX_SEND_RC_CHANNELS, SEND_RC_CHANNELS_OFF);

    SANITIZE(Rx.PowerSwitchChannel, POWER_SWITCH_CHANNEL_NUM, POWER_SWITCH_CHANNEL_OFF, POWER_SWITCH_CHANNEL_OFF);

    //-- Spares and deprecated options:
    // should be 0xFF'ed

    Setup.Tx[config_id].__SerialLinkMode = 0xFF;
    Setup.Tx[config_id].__CliLineEnd = 0xFF;
    Setup.Rx.__Buzzer = 0xFF;
    Setup.Rx.__RadioStatusMethod = 0xFF;

    for (uint8_t n = 0; n < sizeof(Setup.spare)/sizeof(Setup.spare[0]); n++) Setup.spare[n] = 0xFF;
    for (uint8_t n = 0; n < sizeof(Setup.Common[config_id].spare)/sizeof(Setup.Common[config_id].spare[0]); n++) Setup.Common[config_id].spare[n] = 0xFF;
    for (uint8_t n = 0; n < sizeof(Setup.Tx[config_id].spare)/sizeof(Setup.Tx[config_id].spare[0]); n++) Setup.Tx[config_id].spare[n] = 0xFF;
    for (uint8_t n = 0; n < sizeof(Setup.Rx.spare)/sizeof(Setup.Rx.spare[0]); n++) Setup.Rx.spare[n] = 0xFF;
}


//-------------------------------------------------------
// Configure
//-------------------------------------------------------

// also called by bind
void configure_mode(uint8_t mode)
{
    Config.Mode = mode;

    switch (Config.Mode) {
    case MODE_50HZ:
        Config.frame_rate_ms = 20; // 20 ms = 50 Hz
        Config.frame_rate_hz = 50;
        Config.Sx.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF5_CRLI4_5;
        Config.send_frame_tmo_ms = MODE_50HZ_SEND_FRAME_TMO_MS; // 10;
        break;

    case MODE_31HZ:
        Config.frame_rate_ms = 32; // 32 ms = 31.25 Hz
        Config.frame_rate_hz = 31;
#ifdef DEVICE_HAS_SX128x
        Config.Sx.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF6_CRLI4_5;
#elif defined DEVICE_HAS_LR11xx
        Config.Sx.LoraConfigIndex = LR11xx_LORA_CONFIG_BW500_SF5_CR4_5;
#else
        Config.Sx.LoraConfigIndex = SX126x_LORA_CONFIG_BW500_SF5_CR4_5;

#endif
        Config.send_frame_tmo_ms = MODE_31HZ_SEND_FRAME_TMO_MS; // 15
        break;

  case MODE_19HZ:
        Config.frame_rate_ms = 53; // 53 ms = 18.9 Hz
        Config.frame_rate_hz = 19;
#ifdef DEVICE_HAS_SX128x
        Config.Sx.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF7_CRLI4_5;
#elif defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
        Config.Sx.LoraConfigIndex = SX126x_LORA_CONFIG_BW500_SF6_CR4_5;
#elif defined DEVICE_HAS_LR11xx
        Config.Sx.LoraConfigIndex = LR11xx_LORA_CONFIG_BW500_SF6_CR4_5;
#else
        Config.Sx.LoraConfigIndex = SX127x_LORA_CONFIG_BW500_SF6_CR4_5;
#endif
        Config.send_frame_tmo_ms = MODE_19HZ_SEND_FRAME_TMO_MS; // 25;
        break;

    case MODE_FLRC_111HZ:
        Config.frame_rate_ms = 9; // 9 ms = 111 Hz
        Config.frame_rate_hz = 111,
        Config.Sx.LoraConfigIndex = 0;
        Config.send_frame_tmo_ms = MODE_FLRC_111HZ_SEND_FRAME_TMO_MS; // 7;
        break;

    case MODE_FSK_50HZ:
        Config.frame_rate_ms = 20; // 20 ms = 50 Hz
        Config.frame_rate_hz = 50;
        Config.Sx.LoraConfigIndex = 0;
        Config.send_frame_tmo_ms = MODE_FSK_50HZ_SEND_FRAME_TMO_MS; // 10;
        break;

    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()

    }

    // Sx2
    Config.Sx2.LoraConfigIndex = Config.Sx.LoraConfigIndex;

#ifdef DEVICE_HAS_DUAL_SX126x_SX128x
    // DUALBAND 2.4 GHz & 868/915 MHz !
    switch (Config.Mode) {
    case MODE_31HZ:
        Config.Sx2.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF6_CRLI4_5;
        break;
    case MODE_19HZ:
        Config.Sx2.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF7_CRLI4_5;
        break;
    case MODE_FSK_50HZ: // FSK for SX126x implies 50 Hz mode for SX128x
        Config.Sx2.LoraConfigIndex = SX128x_LORA_CONFIG_BW800_SF5_CRLI4_5;
        break;
    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()
    }
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 868/915 MHz & 433 MHz !
    // nothing to, is the same as for 868/915
#endif

    // helper for sx drivers
    Config.Sx.is_lora = (Config.Mode != MODE_FLRC_111HZ && Config.Mode != MODE_FSK_50HZ);

    Config.Sx2.is_lora = Config.Sx.is_lora;

#ifdef DEVICE_HAS_DUAL_SX126x_SX128x
    // DUALBAND 2.4 GHz & 868/915 MHz !
    // FSK for SX126x implies 50 Hz mode for SX128x, i.e. LoRa mode
    Config.Sx2.is_lora = true;
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 868/915 MHz & 433 MHz !
    // nothing to, is the same as for 868/915
#endif
}


void setup_configure_config(uint8_t config_id)
{
    //-- SyncWord
    // note: FrameSyncWord will be modified below by Ortho

    uint32_t bind_dblword = u32_from_bindphrase(Setup.Common[config_id].BindPhrase);

    Config.FrameSyncWord = fmav_crc_calculate((uint8_t*)&bind_dblword, 4); // condense it into a u16

    //-- SX12xx Config: SyncWord
    // note: SyncWord will be modified below by Ortho

    Config.Sx.FlrcSyncWord = bind_dblword;
    Config.Sx2.FlrcSyncWord = Config.Sx.FlrcSyncWord;

    //-- Diversity

#if defined DEVICE_HAS_DIVERSITY || defined DEVICE_HAS_DIVERSITY_SINGLE_SPI
  #ifdef DEVICE_IS_TRANSMITTER
    switch (Setup.Tx[config_id].Diversity) {
  #endif
  #ifdef DEVICE_IS_RECEIVER
    switch (Setup.Rx.Diversity) {
  #endif
    case DIVERSITY_DEFAULT:
        Config.Diversity = DIVERSITY_DEFAULT;
        Config.ReceiveUseAntenna1 = true;
        Config.ReceiveUseAntenna2 = true;
        Config.TransmitUseAntenna1 = true;
        Config.TransmitUseAntenna2 = true;
        break;
    case DIVERSITY_ANTENNA1:
        Config.Diversity = DIVERSITY_ANTENNA1;
        Config.ReceiveUseAntenna1 = true;
        Config.ReceiveUseAntenna2 = false;
        Config.TransmitUseAntenna1 = true;
        Config.TransmitUseAntenna2 = false;
        break;
    case DIVERSITY_ANTENNA2:
        Config.Diversity = DIVERSITY_ANTENNA2;
        Config.ReceiveUseAntenna1 = false;
        Config.ReceiveUseAntenna2 = true;
        Config.TransmitUseAntenna1 = false;
        Config.TransmitUseAntenna2 = true;
        break;
    case DIVERSITY_R_ENABLED_T_ANTENNA1:
        Config.Diversity = DIVERSITY_R_ENABLED_T_ANTENNA1;
        Config.ReceiveUseAntenna1 = true;
        Config.ReceiveUseAntenna2 = true;
        Config.TransmitUseAntenna1 = true;
        Config.TransmitUseAntenna2 = false;
        break;
    case DIVERSITY_R_ENABLED_T_ANTENNA2:
        Config.Diversity = DIVERSITY_R_ENABLED_T_ANTENNA2;
        Config.ReceiveUseAntenna1 = true;
        Config.ReceiveUseAntenna2 = true;
        Config.TransmitUseAntenna1 = false;
        Config.TransmitUseAntenna2 = true;
        break;
    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()
    }
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 2.4 GHz & 868/915 MHz or 868/915 MHz & 433 MHz !
    Config.Diversity = DIVERSITY_DEFAULT; // treat it like diversity
    Config.ReceiveUseAntenna1 = true;
    Config.ReceiveUseAntenna2 = true;
    Config.TransmitUseAntenna1 = true;
    Config.TransmitUseAntenna2 = true;
#else
    Config.Diversity = DIVERSITY_ANTENNA1;
    Config.ReceiveUseAntenna1 = true;
    Config.ReceiveUseAntenna2 = false;
    Config.TransmitUseAntenna1 = true;
    Config.TransmitUseAntenna2 = false;
#endif

    //-- FrequencyBand

    Config.FrequencyBand = Setup.Common[config_id].FrequencyBand;

    //-- Mode, Mode dependent settings

    configure_mode(Setup.Common[config_id].Mode); // sets also Sx.LoraConfigIndex

    //-- Sx12xx

    // note: the actually used power will be determined later when the SX are set up
#ifdef DEVICE_IS_TRANSMITTER
    Config.Sx.Power_dbm = rfpower_list[Setup.Tx[config_id].Power].dbm;
#endif
#ifdef DEVICE_IS_RECEIVER
    Config.Sx.Power_dbm = rfpower_list[Setup.Rx.Power].dbm;
#endif
    Config.Sx2.Power_dbm = Config.Sx.Power_dbm;

    switch (Config.FrequencyBand) {
    case SETUP_FREQUENCY_BAND_2P4_GHZ: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ; break;
    case SETUP_FREQUENCY_BAND_915_MHZ_FCC: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC; break;
    case SETUP_FREQUENCY_BAND_868_MHZ: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ; break;
    case SETUP_FREQUENCY_BAND_433_MHZ: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ; break;
    case SETUP_FREQUENCY_BAND_70_CM_HAM: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM; break;
    case SETUP_FREQUENCY_BAND_866_MHZ_IN: Config.Sx.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN; break;
    }

    Config.Sx2.FrequencyBand = Config.Sx.FrequencyBand;

#ifdef DEVICE_HAS_DUAL_SX126x_SX128x
    // DUALBAND 2.4 GHz & 868/915 MHz !
    Config.Sx2.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ;
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 868/915 MHz & 433 MHz !
    Config.Sx2.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ;
#endif

    //-- Fhss

    Config.Fhss.FrequencyBand = Config.Sx.FrequencyBand;

    Config.Fhss.FrequencyBand_allowed_mask = 0;
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_2P4_GHZ)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ);
    }
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_915_MHZ_FCC)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC);
    }
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_868_MHZ)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ);
    }
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_433_MHZ)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ);
    }
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_70_CM_HAM)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM);
    }
    if (SetupMetaData.FrequencyBand_allowed_mask & (1 << SETUP_FREQUENCY_BAND_866_MHZ_IN)) {
        Config.Fhss.FrequencyBand_allowed_mask |= (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN);
    }

    Config.Fhss.Except = EXCEPT_NONE;
    if (Config.Fhss.FrequencyBand == SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ) {
        Config.Fhss.Except = except_from_bindphrase(Setup.Common[config_id].BindPhrase);
    }

    Config.Fhss.Ortho = Setup.Common[config_id].Ortho;

    // modify also FrameSyncWord, Sx.FlrcSyncWord
    if (Config.Fhss.Ortho > ORTHO_NONE) {
        Config.FrameSyncWord += 0x1111 * Config.Fhss.Ortho;
        Config.Sx.FlrcSyncWord += 0x11111111 * Config.Fhss.Ortho;
        Config.Sx2.FlrcSyncWord = Config.Sx.FlrcSyncWord;
    }

    //Config.FhssSeed = bind_dblword;
    // this is much better for narrow bands, like 868 MHz
    // we could make it dependable with a #ifdef, but what's the point
    Config.Fhss.Seed = Config.FrameSyncWord;

    switch (Config.Fhss.FrequencyBand) {
    case SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ:
        switch (Config.Mode) {
        case MODE_50HZ: Config.Fhss.Num = FHSS_NUM_BAND_2P4_GHZ; break;
        case MODE_31HZ: Config.Fhss.Num = FHSS_NUM_BAND_2P4_GHZ_31HZ_MODE; break;
        case MODE_19HZ: Config.Fhss.Num = FHSS_NUM_BAND_2P4_GHZ_19HZ_MODE; break;
        case MODE_FLRC_111HZ: Config.Fhss.Num = FHSS_NUM_BAND_2P4_GHZ; break;
        default:
            while(1){} // must not happen, should have been resolved in setup_sanitize()
        }
        break;
    case SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC:
        Config.Fhss.Num = FHSS_NUM_BAND_915_MHZ_FCC;
        break;
    case SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ:
        Config.Fhss.Num = FHSS_NUM_BAND_868_MHZ;
        break;
    case SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ:
        Config.Fhss.Num = FHSS_NUM_BAND_433_MHZ;
        break;
    case SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM:
        switch (Config.Mode) {
        case MODE_31HZ: Config.Fhss.Num = FHSS_NUM_BAND_70_CM_HAM; break;
        case MODE_19HZ: Config.Fhss.Num = FHSS_NUM_BAND_70_CM_HAM_19HZ_MODE; break;
        default:
            while(1){} // must not happen, should have been resolved in setup_sanitize()
        }
        break;
    case SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN:
        Config.Fhss.Num = FHSS_NUM_BAND_866_MHZ_IN;
        break;
    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()
    }

    Config.Fhss2 = Config.Fhss;

#ifdef DEVICE_HAS_DUAL_SX126x_SX128x
    // DUALBAND 2.4 GHz & 868/915 MHz !
    Config.Fhss2.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ;
    Config.Fhss2.FrequencyBand_allowed_mask = (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_2P4_GHZ);
    switch (Config.Mode) {
    case MODE_31HZ: Config.Fhss2.Num = FHSS_NUM_BAND_2P4_GHZ_31HZ_MODE; break;
    case MODE_19HZ: Config.Fhss2.Num = FHSS_NUM_BAND_2P4_GHZ_19HZ_MODE; break;
    case MODE_FSK_50HZ: Config.Fhss2.Num = FHSS_NUM_BAND_2P4_GHZ; break; // FSK for SX126x implies 50 Hz mode for SX128x
    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()
    }
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
    // DUALBAND 868/915 MHz & 433 MHz !
    Config.Fhss2.FrequencyBand = SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ;
    Config.Fhss2.FrequencyBand_allowed_mask = (1 << SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ);
    switch (Config.Mode) {
    case MODE_31HZ: Config.Fhss2.Num = FHSS_NUM_BAND_433_MHZ; break;
    case MODE_19HZ: Config.Fhss2.Num = FHSS_NUM_BAND_433_MHZ; break;
    default:
        while(1){} // must not happen, should have been resolved in setup_sanitize()
    }
#endif

    //-- More Config, may depend on above config settings

    Config.connect_tmo_systicks = SYSTICK_DELAY_MS(CONNECT_TMO_MS);
    Config.connect_listen_hop_cnt = (uint8_t)(1.5f * Config.Fhss.Num);

    if (Config.Fhss.Num >= Config.Fhss2.Num) {
        Config.connect_sync_cnt_max = Config.Fhss.Num / 2;
    } else {
        Config.connect_sync_cnt_max = Config.Fhss2.Num / 2;
    }
    if (Config.connect_sync_cnt_max < CONNECT_SYNC_CNT) Config.connect_sync_cnt_max = CONNECT_SYNC_CNT;

    Config.LQAveragingPeriod = (LQ_AVERAGING_MS/Config.frame_rate_ms);

    //-- Serial
#ifdef DEVICE_IS_TRANSMITTER
    switch (Setup.Tx[config_id].SerialBaudrate) {
#endif
#ifdef DEVICE_IS_RECEIVER
    switch (Setup.Rx.SerialBaudrate) {
#endif
    case SERIAL_BAUDRATE_9600: Config.SerialBaudrate = 9600; break;
    case SERIAL_BAUDRATE_19200: Config.SerialBaudrate = 19200; break;
    case SERIAL_BAUDRATE_38400: Config.SerialBaudrate = 38400; break;
    case SERIAL_BAUDRATE_57600: Config.SerialBaudrate = 57600; break;
    case SERIAL_BAUDRATE_115200: Config.SerialBaudrate = 115200; break;
    case SERIAL_BAUDRATE_230400: Config.SerialBaudrate = 230400; break;
    default:
#ifdef DEVICE_IS_TRANSMITTER
        Config.SerialBaudrate = 115200;
#else
        Config.SerialBaudrate = 57600;
#endif
    }

    //-- Mbridge, Crsf, In

    Config.UseMbridge = false;
    Config.UseCrsf = false;
    Config.UseIn = false;
#ifdef DEVICE_IS_TRANSMITTER
    // conflicts must have been sorted out before in setup_sanitize()
  #ifdef DEVICE_HAS_JRPIN5
    if ((Setup.Tx[config_id].ChannelsSource == CHANNEL_SOURCE_MBRIDGE) ||
        (Setup.Tx[config_id].SerialDestination == SERIAL_DESTINATION_MBRDIGE)) {
        Config.UseMbridge = true;
    }
    if (Setup.Tx[config_id].ChannelsSource == CHANNEL_SOURCE_CRSF) {
        Config.UseCrsf = true;
    }
    if (Config.UseMbridge && Config.UseCrsf) {
        while(1){} // mBridge and CRSF cannot be used simultaneously, must not happen
    }
  #endif
  #ifdef USE_IN
    if (Setup.Tx[Config.ConfigId].ChannelsSource == CHANNEL_SOURCE_INPORT) {
        Config.UseIn = true;
    }
    if ((Config.UseMbridge && Config.UseIn) || (Config.UseCrsf && Config.UseIn)) {
        while(1){} // In and mBridge or CRSF cannot be used simultaneously, must not happen
    }
  #endif
#endif
}


//-------------------------------------------------------
// helper
//-------------------------------------------------------

void setup_clear(void)
{
    memset((uint8_t*)&Setup, 0xff, sizeof(tSetup));
}


EE_STATUS_ENUM setup_store_to_EEPROM(void)
{
    return ee_writedata(&Setup, sizeof(tSetup));
}


EE_STATUS_ENUM setup_retrieve_from_EEPROM(void)
{
    return ee_readdata(&Setup, sizeof(tSetup));
}


void setup_reload(void)
{
    setup_retrieve_from_EEPROM();
    setup_sanitize_config(Config.ConfigId);
}


//-------------------------------------------------------
// Init
//-------------------------------------------------------

#define SETUPLAYOUT_L0_0_02    2 // layout version up to v0.3.28
#define SETUPLAYOUT_L0_3_29  329 // layout version from v0.3.29 - v0.3.34
#define SETUPLAYOUT_L0_3_35  335 // layout version from v0.3.35 onwards

#ifdef DEVICE_IS_TRANSMITTER
#define SETUP_MARKER_NEW_STR  SETUP_MARKER_TX_STR
#endif
#ifdef DEVICE_IS_RECEIVER
#define SETUP_MARKER_NEW_STR  SETUP_MARKER_RX_STR
#endif


void setup_init(void)
{
EE_STATUS_ENUM ee_status;
bool doEEPROMwrite;

    setup_clear();
    ee_status = ee_init();
    if (ee_status == EE_STATUS_OK) { ee_status = setup_retrieve_from_EEPROM(); }
    if (ee_status != EE_STATUS_OK) { // try it a 2nd time
        setup_clear();
        ee_status = ee_init();
        if (ee_status == EE_STATUS_OK) { ee_status = setup_retrieve_from_EEPROM(); }
    }

    if (ee_status != EE_STATUS_OK) setup_clear(); // force default

    if ((strncmp(Setup.MarkerStr, SETUP_MARKER_NEW_STR, 16) != 0) &&
        (strncmp(Setup.MarkerStr, SETUP_MARKER_OLD_STR, 16) != 0)) {
        setup_clear(); // force default
    }

#ifdef SETUP_FORCE_COMMON_CONF
    setup_clear(); // force default
#endif

    doEEPROMwrite = false;
    if (Setup.Layout != SETUPLAYOUT) {
        if (Setup.Layout < SETUPLAYOUT) {
            // was 335 or lower, there would be lots to do but didn't do layout version-ing properly so far
        } else {
            // ups, > 10304, should not happen
            for (uint8_t id = 0; id < SETUP_CONFIG_NUM; id++) setup_default(id);
            Setup._ConfigId = 0;
        }
        Setup.Layout = SETUPLAYOUT;
        doEEPROMwrite = true;
    }
    if (Setup.Version != VERSION) { // do after Layout, ensures that these flags are correct irrespective of Layout handling
        Setup.Version = VERSION;
        doEEPROMwrite = true;
    }
    if ((strncmp(Setup.MarkerStr, SETUP_MARKER_NEW_STR, 16) != 0)) {
        strbufstrcpy((char*)Setup.MarkerStr, SETUP_MARKER_NEW_STR, 16);
        strbufstrcpy((char*)Setup.MarkerEnd, SETUP_MARKEREND_STR, 8);
        doEEPROMwrite = true;
    }
    if (doEEPROMwrite) {
        setup_store_to_EEPROM();
    }

#ifdef DEVICE_IS_TRANSMITTER
    if (Setup._ConfigId >= SETUP_CONFIG_NUM) Setup._ConfigId = 0;

    // ensure that all config id's have the same channel_source
    // otherwise it can be very confusing concerning how to communicate with the module
    for (uint8_t id = 0; id < SETUP_CONFIG_NUM; id++) {
        Setup.Tx[id].ChannelsSource = Setup.Tx[Setup._ConfigId].ChannelsSource;
    }
#else
    Setup._ConfigId = 0;
#endif

    Config.ConfigId = Setup._ConfigId;

    setup_configure_metadata();

    setup_sanitize_config(Config.ConfigId);

    setup_configure_config(Config.ConfigId);
}


#endif // SETUP_H
