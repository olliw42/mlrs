//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// SX12XX driver shim
//*******************************************************
#ifndef SX12XX_DRIVER_H
#define SX12XX_DRIVER_H
#pragma once


#include "../hal/hal.h"


#ifdef DEVICE_HAS_I2C_DAC
class tI2cDac : public tI2cBase
{
  public:
    void Init(void) override
    {
        i2c_init();
        i2c_setdeviceadr(SX_PA_DAC_I2C_DEVICE_ADR);
        HAL_StatusTypeDef res = i2c_device_ready();
        initialized = (res == HAL_OK);
    }

    bool put_buf_blocking(uint8_t device_adr, uint8_t* buf, uint16_t len) override
    {
        if (!initialized) return false;
        i2c_setdeviceadr(device_adr);
        HAL_StatusTypeDef res = i2c_put_buf_blocked(buf, len);
        return (res == HAL_OK);
    }
};

tI2cDac dac;
#endif


#ifdef DEVICE_HAS_INTERNAL_DAC_TWOCHANNELS
class tInternalDac : public tInternalDacBase
{
  public:
    void Init(void) override
    {
        dac_init(SX_PA_DAC);
        dac_config_channel(SX_PA_DAC, SX_PA_DAC_CHANNEL1, SX_PA_DAC_IO1);
        dac_config_channel(SX_PA_DAC, SX_PA_DAC_CHANNEL2, SX_PA_DAC_IO2);
    }

    void put_channel1(uint16_t value) override
    {
        dac_write_channel(SX_PA_DAC, SX_PA_DAC_CHANNEL1, value);
    }

    void put_channel2(uint16_t value) override
    {
        dac_write_channel(SX_PA_DAC, SX_PA_DAC_CHANNEL2, value);
    }
};

tInternalDac dac;
#endif


typedef struct
{
    uint8_t SpreadingFactor;
    uint8_t Bandwidth;
    uint8_t CodingRate;
    uint8_t PreambleLength;
    uint8_t HeaderType;
    uint8_t PayloadLength;
    uint8_t CrcEnabled;
    uint8_t InvertIQ;
    uint32_t TimeOverAir; // in us
    int16_t ReceiverSensitivity;
} tSxLoraConfiguration;


class SxDriverDummy
{
  public:
    void Init(void) {}
    bool isOk(void) { return true; }
    void StartUp(tSxGlobalConfig* const global_config) {}
    void SetPacketType(uint8_t PacketType) {}
    void SetRfFrequency(uint32_t RfFrequency) {}
    void GetPacketStatus(int8_t* const RssiSync, int8_t* const Snr) {}
    void SendFrame(uint8_t* const data, uint8_t len, uint16_t tmo_ms) {}
    void ReadFrame(uint8_t* const data, uint8_t len) {}
    void SetToRx(uint16_t tmo_ms) {}
    void SetToIdle(void) {}

    void ResetToLoraConfiguration() {}
    void SetRfPower_dbm(int8_t power_dbm) {}
    void UpdateRfPower(tSxGlobalConfig* const global_config) {}
    void ClearIrqStatus(uint16_t IrqMask) {}

    int16_t ReceiverSensitivity_dbm(void) { return 0; }
    int8_t RfPower_dbm(void) { return INT8_MIN; }

    void HandleAFC(void) {}
};


#if defined DEVICE_HAS_SX126x || defined DEVICE_HAS_DUAL_SX126x_SX126x
  #include "sx126x_driver.h"
#elif defined DEVICE_HAS_SX127x
  #include "sx127x_driver.h"
#elif defined DEVICE_HAS_DUAL_SX126x_SX128x
  #include "sx126x_driver.h"
  #include "sx128x_driver.h"
#elif defined DEVICE_HAS_LR11xx
  #include "lr11xx_driver.h"
#else
  #include "sx128x_driver.h"
#endif


#endif // SX12XX_DRIVER_H
