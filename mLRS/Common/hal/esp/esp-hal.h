//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// rx hal splicer for ESP targets
//*******************************************************

//-------------------------------------------------------
// ESP Boards
//-------------------------------------------------------

//-- ELRS 868/915 MHz Generic Devices

#ifdef RX_ELRS_GENERIC_900_ESP8285
#include "rx-hal-generic-900-esp8285.h"
#endif

#ifdef RX_ELRS_GENERIC_900_PA_ESP8285
#include "rx-hal-generic-900-pa-esp8285.h"
#endif

#ifdef RX_ELRS_GENERIC_900_TD_PA_ESP32
#include "rx-hal-generic-900-td-pa-esp32.h"
#endif

//-- Generic 868/915 MHz ELRS devices with overlays or other variations

#ifdef RX_ELRS_RADIOMASTER_BR3_900_ESP8285
#include "rx-hal-radiomaster-br3-900-esp8285.h"
#endif

#ifdef RX_ELRS_IFLIGHT_ELRS_TD_900_ESP32
#include "rx-hal-generic-900-td-pa-esp32.h"
#endif


//-- ELRS 2.4 GHz Generic Devices

#ifdef RX_ELRS_GENERIC_2400_ESP8285
#include "rx-hal-generic-2400-esp8285.h"
#endif

#ifdef RX_ELRS_GENERIC_2400_PA_ESP8285
#include "rx-hal-generic-2400-pa-esp8285.h"
#endif

#ifdef RX_ELRS_GENERIC_2400_D_PA_ESP8285
#include "rx-hal-generic-2400-d-pa-esp8285.h"
#endif

#ifdef RX_ELRS_GENERIC_2400_TD_PA_ESP32
#include "rx-hal-generic-2400-td-pa-esp32.h"
#endif

//-- Generic 2.4 GHz ELRS boards with overlays or other variations

#ifdef RX_ELRS_IFLIGHT_ELRS_TD_2400_ESP32
#include "rx-hal-iflight-elrs-td-2400-esp32.h"
#endif

#ifdef RX_ELRS_IFLIGHT_ELRS_2400_ESP8285
#include "rx-hal-iflight-elrs-2400-esp8285.h"
#endif


//-- ELRS Selected Devices

#ifdef RX_ELRS_BAYCK_NANO_PRO_900_ESP8285
#include "rx-hal-generic-900-pa-esp8285.h"
#endif

#ifdef RX_ELRS_SPEEDYBEE_NANO_2400_ESP8285
#include "rx-hal-generic-2400-pa-esp8285.h"
#endif

#ifdef RX_ELRS_RADIOMASTER_RP4TD_2400_ESP32
#include "rx-hal-radiomaster-rp4td-2400-esp32.h"
#endif

#ifdef RX_ELRS_BETAFPV_SUPERD_2400_ESP32
#include "rx-hal-generic-2400-td-pa-esp32.h"
#endif


#ifdef TX_ELRS_RADIOMASTER_RP4TD_2400_ESP32
#include "tx-hal-radiomaster-rp4td-2400-esp32.h"
#endif

#ifdef TX_ELRS_BETAFPV_MICRO_1W_2400_ESP32
#include "tx-hal-betafpv-micro-1w-2400-esp32.h"
#endif

#ifdef TX_ELRS_RADIOMASTER_BANDIT_MICRO_900_ESP32
#include "tx-hal-radiomaster-bandit-micro-900-esp32.h"
#endif


// -- DIY

#ifdef RX_DIY_DEV_900_ESP8266
#include "rx-hal-diy-dev-900-esp8266.h"
#endif

#ifdef RX_DIY_DEV_900_ESP32
#include "rx-hal-diy-dev-900-esp32.h"
#endif


