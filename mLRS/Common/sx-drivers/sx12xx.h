//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// SX12XX shim
//*******************************************************
#ifndef SX12XX_H
#define SX12XX_H
#pragma once


#include "math.h"
#include "../common_conf.h"
#include "../hal/device_conf.h"


#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX126x
  #include "../../modules/sx12xx-lib/src/sx126x.h"
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x
  #include "../../modules/sx12xx-lib/src/sx126x.h"
  #include "../../modules/sx12xx-lib/src/sx128x.h"
#elif defined DEVICE_HAS_SX127x
  #include "../../modules/sx12xx-lib/src/sx127x.h"
#elif defined DEVICE_HAS_LR11xx
  #include "../../modules/sx12xx-lib/src/lr11xx.h"
#else
  #include "../../modules/sx12xx-lib/src/sx128x.h"
#endif


//-------------------------------------------------------
// Helper, may eventually go into lib files
//-------------------------------------------------------

// let's just define them all here for all supported sx chips

#define SX126X_FREQ_XTAL_HZ            32000000
#define SX127X_FREQ_XTAL_HZ            32000000
#define SX1280_FREQ_XTAL_HZ            52000000
#define LR11XX_FREQ_XTAL_HZ            32000000

#define SX126X_REG_TO_FREQ_KHZ(f_reg)  roundf( (float)f_reg * ((double)SX126X_FREQ_XTAL_HZ * 1.0E-3 / (double)(1 << 25)) )
#define SX127X_REG_TO_FREQ_KHZ(f_reg)  roundf( (float)f_reg * ((double)SX127X_FREQ_XTAL_HZ * 1.0E-3 / (double)(1 << 19)) )
#define SX1280_REG_TO_FREQ_MHZ(f_reg)  roundf( (float)f_reg * ((double)SX1280_FREQ_XTAL_HZ * 1.0E-6 / (double)(1 << 18)) )
#define LR11XX_REG_TO_FREQ_KHZ(f_reg)  roundf( (float)f_reg * 1.0E3)


#endif // SX12XX_H
