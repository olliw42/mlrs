//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// rx hal splicer
//*******************************************************
#ifndef HAL_H
#define HAL_H
#pragma once

// enter define for the board (RX_xxx, TX_xxx) into "MCU G++ Compiler"->"Preprocessor" !!!

/* Documentation

The availability of the many features are handled via #define declarations. In order to keep somewhat track,
this naming convention is used (with few exceptions):

- MLRS_FEATURE_XXXX: For setting different features for a target; should ONLY be set globally/externally
  by the build system! Must NOT be defined in code, only globally/externally.
  Note: There can be MLRS_DEV_FEATURE_XXXX flags also, but these are for dev only.

- DEVICE_HAS_XXXX: Are set in a device's hal file to indicate the availability/non-availability of a feature.
  May be defined by user in the hal file as needed. May also be made dependent on MLRS_FEATURE_XXXX flags
  in hal file.

- USE_XXXX: These are determined through some processing, which can involve the DEVICE_HAS_XXXX flags. Must
  NEVER be modified or set by user.

For devs: In follow-up code therefore the USE_XXXX flags should be used (if available) to enable/disable code
for a feature. If a USE_XXXX flag is not available (example: DEVICE_HAS_DIVERSITY) then of course the respective
DEVICE_HAS_XXXX flag needs to be used. Also, DEVICE_HAS_XXXX flags may have to be used to distinguish the "flavor"
of the feature (example: IN feature with normal or inverted UART levels).

Many DEVICE_XXXX feature flags are available, which can be set in the device hal files. They are listed in the
following for the tx-hal and rx-hal files.

In tx-hal files:

#define DEVICE_HAS_DIVERSITY        // board supports diversity
#define DEVICE_HAS_JRPIN5           // board has a pin for JR bay Pin5/SPort
#define DEVICE_HAS_IN               // board has an IN port, which supports both normal and inverted UART signals
#define DEVICE_HAS_IN_NORMAL        // board has an IN port, which supports only normal UART signals
#define DEVICE_HAS_IN_INVERTED      // board has an IN port, which supports only inverted UART signals
#define DEVICE_HAS_IN_ON_JRPIN5_RX  // board shares IN with JRPin5 on RX pin, implies support of normal and inverted UART signals
#define DEVICE_HAS_IN_ON_JRPIN5_TX  // board shares IN with JRPin5 on TX pin, implies support of normal and inverted UART signals
#define DEVICE_HAS_SERIAL_OR_COM    // board has UART which is shared between Serial or Com, selected by e.g. a switch
#define DEVICE_HAS_NO_SERIAL        // board has no Serial port
#define DEVICE_HAS_SERIAL_ON_USB    // board has the Serial port on native USB
#define DEVICE_HAS_NO_COM           // board has no Com port
#define DEVICE_HAS_COM_ON_USB       // board has the Com port on native USB
#define DEVICE_HAS_NO_DEBUG         // board has no Debug port
#define DEVICE_HAS_DEBUG_SWUART     // implement Debug as software UART
#define DEVICE_HAS_I2C_DISPLAY          // board has a DISPLAY on I2C, and 5-way switch
#define DEVICE_HAS_I2C_DISPLAY_ROT180   // board has a DISPLAY on I2C, rotated 180°, and 5-way switch
#define DEVICE_HAS_FIVEWAY          // board has 5-way switch (without display)
#define DEVICE_HAS_BUZZER           // board has a Buzzer
#define DEVICE_HAS_FAN_ONOFF        // board has a Fan, which can be set on or off
#define DEVICE_HAS_I2C_DAC          // board has a DAC for power control on I2C
#define DEVICE_HAS_SERIAL2          // board has a Serial2 port
#define DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL  // board has ESP32 or ESp82xx with RESET,GPIO support, on Serial port
#define DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL2 // board has ESP32 or ESp82xx with RESET,GPIO support, on Serial2 port
#define DEVICE_HAS_ESP_WIFI_BRIDGE_CONFIGURE  // board has ESP32 which allows configuration
#define DEVICE_HAS_HC04_MODULE_ON_SERIAL      // board has HC04 module on Serial port
#define DEVICE_HAS_HC04_MODULE_ON_SERIAL2     // board has HC04 module on Serial2 port
#define DEVICE_HAS_SYSTEMBOOT       // board has a means to invoke the system bootloader on startup
#define DEVICE_HAS_SINGLE_LED       // board has only one LED

In rx-hal files:

#define DEVICE_HAS_DIVERSITY        // board supports diversity
#define DEVICE_HAS_DIVERSITY_SINGLE_SPI // board supports diversity on a single SPI bus
#define DEVICE_HAS_OUT              // board has an OUT port, which supports both normal and inverted UART signals
#define DEVICE_HAS_OUT_NORMAL       // board has an OUT port, which supports only normal UART signals
#define DEVICE_HAS_OUT_INVERTED     // board has an OUT port, which supports only inverted UART signals
#define DEVICE_HAS_SERIAL_OR_DEBUG  // is selected by DEBUG_ENABLED define
#define DEVICE_HAS_NO_DEBUG         // board has no Debug port
#define DEVICE_HAS_DEBUG_SWUART     // implement Debug as software UART
#define DEVICE_HAS_I2C_DAC          // board has a DAC for power control on I2C
#define DEVICE_HAS_SYSTEMBOOT       // board has a means to invoke the system bootloader on startup
#define DEVICE_HAS_SINGLE_LED       // board has only one LED
#define DEVICE_HAS_SINGLE_LED_RGB   // board has only one LED which is RGB WS2812
#define DEVICE_HAS_FAN_ONOFF        // board has a Fan, which can be set on or off
#define DEVICE_HAS_DRONECAN         // board has a DroneCAN port

Note: Some "high-level" features are set for each device in the device_conf.h file, and not in the device's hal file.
*/


#include "device_conf.h"


//-- MATEKSYS mLRS devices

#ifdef RX_MATEK_MR24_30_G431KB
#include "matek/rx-hal-matek-mr24-30-g431kb.h"
#endif

#ifdef TX_MATEK_MR24_30_G431KB
#include "matek/tx-hal-matek-mr24-30-g431kb.h"
#endif

#ifdef RX_MATEK_MR900_30_G431KB
#include "matek/rx-hal-matek-mr900-30-g431kb.h"
#endif

#ifdef TX_MATEK_MR900_30_G431KB
#include "matek/tx-hal-matek-mr900-30-g431kb.h"
#endif

#ifdef RX_MATEK_MR900_22_WLE5CC
#include "matek/rx-hal-matek-mr900-22-wle5cc.h"
#endif

#ifdef RX_MATEK_MR900_TD30_G474CE
#include "matek/rx-hal-matek-mr900-td30-g474ce.h"
#endif

#ifdef RX_MATEK_MR900_30C_G431KB
#include "matek/rx-hal-matek-mr900-30c-g431kb.h"
#endif


//-- FrsKy R9 system

#ifdef RX_R9MX_868_L433CB
#include "stm32/rx-hal-R9MX-868-l433cb.h"
#endif
#ifdef RX_R9M_868_F103C8
#include "stm32/rx-hal-R9M-868-f103c8.h"
#endif
#ifdef RX_R9MM_868_F103RB
#include "stm32/rx-hal-R9MM-868-f103rb.h"
#endif
#ifdef RX_R9MLITEPRO_F303CC
#include "stm32/rx-hal-R9MLitePro-v15-f303cc.h"
#endif

#ifdef TX_R9M_868_F103C8
#include "stm32/tx-hal-R9M-868-f103c8.h"
#endif
#ifdef TX_R9MX_868_L433CB
#include "stm32/tx-hal-R9MX-868-l433cb.h"
#endif


//-- SeeedStudio WioE5 boards

#ifdef RX_WIO_E5_GROVE_WLE5JC
#include "stm32/rx-hal-WioE5-Grove-wle5jc.h"
#endif
#ifdef RX_WIO_E5_MINI_WLE5JC
#include "stm32/rx-hal-WioE5-Mini-wle5jc.h"
#endif

#ifdef TX_WIO_E5_MINI_WLE5JC
#include "stm32/tx-hal-WioE5-Mini-wle5jc.h"
#endif


//-- EByte MBL Evaluation Kits

#ifdef RX_E77_MBLKIT_WLE5CC
#include "stm32/rx-hal-E77-MBLKit-wle5cc.h"
#endif

#ifdef TX_E77_MBLKIT_WLE5CC
#include "stm32/tx-hal-E77-MBLKit-wle5cc.h"
#endif


//-- FlySky FRM303 2.4 GHz Device

#ifdef RX_FRM303_F072CB
#include "stm32/rx-hal-FRM303-f072cb.h"
#endif

#ifdef TX_FRM303_F072CB
#include "stm32/tx-hal-FRM303-f072cb.h"
#endif


//-- DIY Boards, 2.4 GHz Devices

#ifdef RX_DIY_BOARD01_F103CB
#include "stm32/rx-hal-diy-board01-f103cb.h"
#endif
#ifdef RX_DIY_E28DUAL_BOARD02_F103CB
#include "stm32/rx-hal-diy-e28dual-board02-f103cb.h"
#endif
#ifdef RX_DIY_E28_G441KB
#include "stm32/rx-hal-diy-e28-g441kb.h"
#endif

#ifdef TX_DIY_E28DUAL_BOARD02_F103CB
#include "stm32/tx-hal-diy-e28dual-board02-f103cb.h"
#endif
#ifdef TX_DIY_E28_G431KB
#include "stm32/tx-hal-diy-e28-g431kb.h"
#endif
#ifdef TX_DIY_BOARD01_G491RE
#include "stm32/tx-hal-diy-board01-g491re.h"
#endif
#ifdef TX_DIY_SXDUAL_MODULE02_G491RE
#include "stm32/tx-hal-diy-sxdual-module02-g491re.h"
#endif
#ifdef TX_DIY_E28DUAL_MODULE02_G491RE
#include "stm32/tx-hal-diy-e28dual-module02-g491re.h"
#endif


//-- DIY Boards, 868/915 MHz Devices

#ifdef RX_DIY_E22_G441KB
#include "stm32/rx-hal-diy-e22-g441kb.h"
#endif

#ifdef TX_DIY_E22_G431KB
#include "stm32/tx-hal-diy-e22-g431kb.h"
#endif

#ifdef TX_DIY_E22DUAL_MODULE02_G491RE
#include "stm32/tx-hal-diy-e22dual-module02-g491re.h"
#endif

#ifdef RX_DIY_WIOE5_E22_WLE5JC
#include "stm32/rx-hal-diy-wioe5-e22-wle5jc.h"
#endif

#ifdef TX_DIY_WIOE5_E22_WLE5JC
#include "stm32/tx-hal-diy-wioe5-e22-wle5jc.h"
#endif


//-- DIY "easy-to-solder" Boards

#if defined RX_DIY_E77_E22_WLE5CC || defined RX_DIY_E77_E22_DUALBAND_WLE5CC
#include "stm32/rx-hal-easysolder-e77-e22-wle5cc.h"
#endif

#if defined TX_DIY_E77_E22_WLE5CC || defined TX_DIY_E77_E22_DUALBAND_WLE5CC
#include "stm32/tx-hal-easysolder-e77-e22-wle5cc.h"
#endif

#ifdef RX_DIY_E77_E28_DUALBAND_WLE5CC
#include "stm32/rx-hal-easysolder-e77-e28-dualband-wle5cc.h"
#endif

#ifdef TX_DIY_E77_E28_DUALBAND_WLE5CC
#include "stm32/tx-hal-easysolder-e77-e28-dualband-wle5cc.h"
#endif


//-------------------------------------------------------
// ESP Boards
//-------------------------------------------------------

#if defined ESP8266 || defined ESP32
#include "esp/esp-hal.h"
#endif


//-------------------------------------------------------
// Derived Defines
//-------------------------------------------------------
// should go somewhere else !?

#define RFPOWER_LIST_NUM  sizeof(rfpower_list)/sizeof(rfpower_t)


#ifdef DEVICE_IS_RECEIVER
#if defined DEVICE_HAS_SERIAL_OR_DEBUG
  #if !defined DEBUG_ENABLED
    #define USE_SERIAL
  #else
    #define USE_DEBUG
  #endif
#else
  #if !defined DEVICE_HAS_NO_SERIAL
    #define USE_SERIAL
  #endif
  #if defined DEBUG_ENABLED && !defined DEVICE_HAS_NO_DEBUG
    #define USE_DEBUG
  #endif
#endif
#endif // DEVICE_IS_RECEIVER

#ifdef DEVICE_IS_TRANSMITTER
#if defined DEVICE_HAS_SERIAL_OR_COM // some devices have device dependent ways to select serial or com
  #define USE_SERIAL
  #define USE_COM_ON_SERIAL
  #ifdef DEVICE_HAS_SERIAL_ON_USB
    #define USE_USB
  #endif
  #if defined DEBUG_ENABLED && !defined DEVICE_HAS_NO_DEBUG
    #define USE_DEBUG
  #endif
#else
  #if !defined DEVICE_HAS_NO_SERIAL
    #define USE_SERIAL
    #ifdef DEVICE_HAS_SERIAL_ON_USB
      #define USE_USB
    #endif
  #endif
  #if !defined DEVICE_HAS_NO_COM
    #define USE_COM
    #ifdef DEVICE_HAS_COM_ON_USB
      #define USE_USB
    #endif
  #endif
  #if defined DEBUG_ENABLED && !defined DEVICE_HAS_NO_DEBUG
    #define USE_DEBUG
  #endif
#endif

#if defined DEVICE_HAS_SERIAL2 || defined DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL2 || defined DEVICE_HAS_HC04_MODULE_ON_SERIAL2
  #define USE_SERIAL2
#endif
#endif // DEVICE_IS_TRANSMITTER


#if defined DEVICE_HAS_IN || defined DEVICE_HAS_IN_NORMAL || defined DEVICE_HAS_IN_INVERTED || \
    defined DEVICE_HAS_IN_ON_JRPIN5_RX || defined DEVICE_HAS_IN_ON_JRPIN5_TX
  #define USE_IN
#endif


#if defined DEVICE_HAS_OUT || defined DEVICE_HAS_OUT_NORMAL || defined DEVICE_HAS_OUT_INVERTED
  #define USE_OUT
#endif


#if defined DEVICE_HAS_I2C_DISPLAY || defined DEVICE_HAS_I2C_DISPLAY_ROT180
  #define USE_DISPLAY
#endif


#if defined DEVICE_HAS_I2C_DAC || defined DEVICE_HAS_I2C_DISPLAY || defined DEVICE_HAS_I2C_DISPLAY_ROT180
  #define USE_I2C
  #ifndef HAL_I2C_MODULE_ENABLED
    #error HAL_I2C_MODULE_ENABLED is not defined, but I2C is used!
  #endif
#endif


#if defined DEVICE_HAS_FAN_ONOFF || defined DEVICE_HAS_FAN_TEMPCONTROLLED_ONOFF
  #define USE_FAN
#endif


#if defined DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL || defined DEVICE_HAS_ESP_WIFI_BRIDGE_ON_SERIAL2
  #define USE_ESP_WIFI_BRIDGE
  #if defined ESP_RESET && defined ESP_GPIO0
    #define USE_ESP_WIFI_BRIDGE_RST_GPIO0
  #endif
  #if defined ESP_DTR && defined ESP_RTS
    #define USE_ESP_WIFI_BRIDGE_DTR_RTS
  #endif
#endif

#if defined DEVICE_HAS_HC04_MODULE_ON_SERIAL || defined DEVICE_HAS_HC04_MODULE_ON_SERIAL2
  #define USE_HC04_MODULE
#endif


#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
  #define SX_DRIVER Sx126xDriver
#elif defined DEVICE_HAS_SX127x
  #define SX_DRIVER Sx127xDriver
#elif defined DEVICE_HAS_LR11xx
  #define SX_DRIVER Lr11xxDriver
#else
  #define SX_DRIVER Sx128xDriver
#endif

#if defined DEVICE_HAS_DIVERSITY || defined DEVICE_HAS_DIVERSITY_SINGLE_SPI
  #ifdef DEVICE_HAS_SX126x
    #define SX2_DRIVER Sx126xDriver2
  #elif defined DEVICE_HAS_SX127x
    #define SX2_DRIVER Sx127xDriver2
  #elif defined DEVICE_HAS_LR11xx
    #define SX2_DRIVER Lr11xxDriver2
  #else
    #define SX2_DRIVER Sx128xDriver2
  #endif
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x
  #define SX2_DRIVER Sx128xDriver2
#elif defined DEVICE_HAS_DUAL_SX126x_SX126x
  #define SX2_DRIVER Sx126xDriver2
#else
  #define SX2_DRIVER SxDriverDummy
#endif


#if defined DEVICE_HAS_DIVERSITY || defined DEVICE_HAS_DIVERSITY_SINGLE_SPI || \
    defined DEVICE_HAS_DUAL_SX126x_SX128x || defined DEVICE_HAS_DUAL_SX126x_SX126x
  #define USE_SX2
#endif

#ifdef USE_SX2
  #define IF_SX(x)                  if (Config.ReceiveUseAntenna1 || Config.TransmitUseAntenna1) { x; }
  #define IF_SX2(x)                 if (Config.ReceiveUseAntenna2 || Config.TransmitUseAntenna2) { x; }
  #define IF_ANTENNA1(x)            if (Config.ReceiveUseAntenna1) { x; }
  #define IF_ANTENNA2(x)            if (Config.ReceiveUseAntenna2) { x; }
  #define USE_ANTENNA1              (Config.ReceiveUseAntenna1)
  #define USE_ANTENNA2              (Config.ReceiveUseAntenna2)
  #define TRANSMIT_USE_ANTENNA1     (Config.TransmitUseAntenna1)
  #define TRANSMIT_USE_ANTENNA2     (Config.TransmitUseAntenna2)
#else
  #define IF_SX(x)                  x;
  #define IF_SX2(x)
  #define IF_ANTENNA1(x)            x;
  #define IF_ANTENNA2(x)
  #define USE_ANTENNA1              true
  #define USE_ANTENNA2              false
  #define TRANSMIT_USE_ANTENNA1     true
  #define TRANSMIT_USE_ANTENNA2     false
#endif

#ifdef DEVICE_HAS_JRPIN5
  #define IF_MBRIDGE(x)             if (Config.UseMbridge) { x; }
  #define IF_CRSF(x)                if (Config.UseCrsf) { x; }
  #define IF_MBRIDGE_OR_CRSF(x)     if (Config.UseMbridge | Config.UseCrsf) { x; }
#else
  #define IF_MBRIDGE(x)
  #define IF_CRSF(x)
  #define IF_MBRIDGE_OR_CRSF(x)
#endif
#ifdef USE_IN
  #define IF_IN(x)                  if (Config.UseIn) { x; }
#else
  #define IF_IN(x)
#endif


//-- checks

#if !defined DEVICE_IS_TRANSMITTER && !defined DEVICE_IS_RECEIVER
  #error Must be either transmitter or receiver !
#endif

#if !defined DEVICE_HAS_SX128x && !defined DEVICE_HAS_SX127x && !defined DEVICE_HAS_SX126x && \
    !defined DEVICE_HAS_DUAL_SX126x_SX128x && !defined DEVICE_HAS_DUAL_SX126x_SX126x  && !defined DEVICE_HAS_LR11xx
  #error Must be either SX128x or SX127x or SX126x or LR11xx !
#endif

#if !defined FREQUENCY_BAND_2P4_GHZ && \
    !defined FREQUENCY_BAND_915_MHZ_FCC && !defined FREQUENCY_BAND_868_MHZ && !defined FREQUENCY_BAND_866_MHZ_IN && \
    !defined FREQUENCY_BAND_433_MHZ && !defined FREQUENCY_BAND_70_CM_HAM
  #error At least one frequency band must be defined !
#endif


//-------------------------------------------------------
// Empty Prototypes
//-------------------------------------------------------
// should be in the device hal files, but is just so much more convenient to have them here

#ifndef SYSTICK_TIMESTEP
  #define SYSTICK_TIMESTEP          1000 // 1 ms
#endif


#ifndef DEVICE_HAS_SYSTEMBOOT
  void systembootloader_init(void) {}
#endif

#ifndef USE_ESP_WIFI_BRIDGE
  void esp_init(void) {}
#endif

#if !defined DEVICE_HAS_FIVEWAY && !defined USE_DISPLAY
  void fiveway_init(void) {}
#endif

#endif // HAL_H
