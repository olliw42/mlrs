//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// LR11XX Driver
//*******************************************************

#ifndef LR11XX_DRIVER_H
#define LR11XX_DRIVER_H
#pragma once


//-------------------------------------------------------
// SX Driver
//-------------------------------------------------------

const tSxLoraConfiguration Lr11xxLoraConfiguration[] = {
    { .SpreadingFactor = LR11XX_LORA_SF5,
      .Bandwidth = LR11XX_LORA_BW_500,
      .CodingRate = LR11XX_LORA_CR_4_5,
      .PreambleLength = 12,
      .HeaderType = LR11XX_LORA_HEADER_DISABLE,
      .PayloadLength = FRAME_TX_RX_LEN,
      .CrcEnabled = LR11XX_LORA_CRC_DISABLE,
      .InvertIQ = LR11XX_LORA_IQ_NORMAL,
      .TimeOverAir = 13200,
      .ReceiverSensitivity = -111,
    },
    { .SpreadingFactor = LR11XX_LORA_SF6,
      .Bandwidth = LR11XX_LORA_BW_500,
      .CodingRate = LR11XX_LORA_CR_4_5,
      .PreambleLength = 12,
      .HeaderType = LR11XX_LORA_HEADER_DISABLE,
      .PayloadLength = FRAME_TX_RX_LEN,
      .CrcEnabled = LR11XX_LORA_CRC_DISABLE,
      .InvertIQ = LR11XX_LORA_IQ_NORMAL,
      .TimeOverAir = 22560,
      .ReceiverSensitivity = -112,
    }
};


#ifdef POWER_USE_DEFAULT_RFPOWER_CALC
void lr11xx_rfpower_calc(const int8_t power_dbm, uint8_t* sx_power, int8_t* actual_power_dbm, const uint8_t GAIN_DBM, const uint8_t LR11XX_MAX_DBM)
{
    int16_t power_sx = (int16_t)power_dbm - GAIN_DBM;

    if (power_sx < LR11XX_POWER_MIN) power_sx = LR11XX_POWER_MIN;
    if (power_sx > LR11XX_POWER_MAX) power_sx = LR11XX_POWER_MAX;
    if (power_sx > LR11XX_MAX_DBM) power_sx = LR11XX_MAX_DBM;

    *sx_power = power_sx;
    *actual_power_dbm = power_sx + GAIN_DBM;
}
#endif


class Lr11xxDriverCommon : public Lr11xxDriverBase
{
  public:

    void Init(void)
    {
        lora_configuration = nullptr;
    }

    bool isOk(void)
    {
        uint8_t hwVersion;
        uint8_t useCase;
        uint8_t fwMajor;
        uint8_t fwMinor;
        
        GetVersion(&hwVersion, &useCase, &fwMajor, &fwMinor);

        uint16_t firmwareRev = (static_cast<uint16_t>(fwMajor) << 8) | fwMinor;

        return ((firmwareRev != 0) && (firmwareRev != 65535));
    }

    void SetLoraConfiguration(const tSxLoraConfiguration* const config)
    {
        SetModulationParams(config->SpreadingFactor,
                            config->Bandwidth,
                            config->CodingRate,
                            LR11XX_LORA_LDR_OFF);

        SetPacketParams(config->PreambleLength,
                        config->HeaderType,
                        config->PayloadLength,
                        config->CrcEnabled,
                        config->InvertIQ);
    }

    void SetLoraConfigurationByIndex(uint8_t index)
    {
        if (index >= sizeof(Lr11xxLoraConfiguration)/sizeof(Lr11xxLoraConfiguration[0])) while(1){} // must not happen

        lora_configuration = &(Lr11xxLoraConfiguration[index]);
        SetLoraConfiguration(lora_configuration);
    }

    void ResetToLoraConfiguration(void)
    {
        SetStandby(LR11XX_STDBY_CONFIG_STDBY_RC);
        delay_us(1000);
        SetPacketType(LR11XX_PACKET_TYPE_LORA);
        SetLoraConfigurationByIndex(gconfig->LoraConfigIndex);
    }

    void SetRfPower_dbm(int8_t power_dbm)
    {
        RfPowerCalc(power_dbm, &sx_power, &actual_power_dbm);
        SetTxParams(sx_power, LR11XX_RAMPTIME_48_US); // Closest to 40 uS used by SX126x / SX127x
    }

    void UpdateRfPower(tSxGlobalConfig* const global_config)
    {
        gconfig->Power_dbm = global_config->Power_dbm;
        SetRfPower_dbm(gconfig->Power_dbm);
    }

    void Configure(tSxGlobalConfig* const global_config)
    {
      gconfig = global_config;

        if (gconfig->modeIsLora()) {
            SetPacketType(LR11XX_PACKET_TYPE_LORA);
        } else {
            // SetPacketType(LR11XX_PACKET_TYPE_GFSK);
        }

      switch (gconfig->FrequencyBand) {
        case SX_FHSS_CONFIG_FREQUENCY_BAND_915_MHZ_FCC: CalibImage(LR11XX_CAL_IMG_902_MHZ_1, LR11XX_CAL_IMG_902_MHZ_2); break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_868_MHZ: CalibImage(LR11XX_CAL_IMG_863_MHZ_1, LR11XX_CAL_IMG_863_MHZ_2); break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_866_MHZ_IN: CalibImage(LR11XX_CAL_IMG_863_MHZ_1, LR11XX_CAL_IMG_863_MHZ_2); break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_433_MHZ: CalibImage(LR11XX_CAL_IMG_430_MHZ_1, LR11XX_CAL_IMG_430_MHZ_2); break;
        case SX_FHSS_CONFIG_FREQUENCY_BAND_70_CM_HAM: CalibImage(LR11XX_CAL_IMG_430_MHZ_1, LR11XX_CAL_IMG_430_MHZ_2); break;
        default:
            while(1){} // protection
      }

      SetRxTxFallbackMode(LR11XX_RX_TX_FALLBACK_MODE_FS);
      SetRxBoosted(LR11XX_RX_GAIN_BOOSTED_GAIN);

      SetPaConfig(LR11XX_PA_SELECT_HP_PA, LR11XX_REG_PA_SUPPLY_VBAT, LR11XX_PA_CONFIG_22_DBM_PA_DUTY_CYCLE, LR11XX_PA_CONFIG_22_DBM_HP_MAX);

      SetDioAsRfSwitch(0b0000111, 0, 0b0000010, 0b0000100,  0b00001000, 0b00000010);  // Clean up?

      SetDioIrqParams(LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TIMEOUT, 0);  // DIO1 only
      ClearIrq(LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TIMEOUT);  // DIO1 only

      if (gconfig->modeIsLora()) {
            SetLoraConfigurationByIndex(gconfig->LoraConfigIndex);
      } else {
          // FSK reserve
      }

      SetFs();
    }

    //-- these are the API functions used in the loop

    void ReadFrame(uint8_t* const data, uint8_t len)
    {
        uint8_t rxStartBufferPointer;
        uint8_t rxPayloadLength;

        GetRxBufferStatus(&rxPayloadLength, &rxStartBufferPointer);
        ReadBuffer(rxStartBufferPointer, data, len);
    }

    void SendFrame(uint8_t* const data, uint8_t len, uint16_t tmo_ms)
    {
        WriteBuffer(0, data, len);
        ClearIrq(LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TIMEOUT);
        SetTx(tmo_ms * 33); // 0 = no timeout. TimeOut period in ms. LR11xx have static 30.517 uS (1 / 32768) period base, so for 1 ms needs 33 tmo value
    }

    void SetToRx(uint16_t tmo_ms)
    {
        ClearIrq(LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TIMEOUT);
        SetRx(tmo_ms * 33); // 0 = no timeout
    }

    void SetToIdle(void)
    {
        SetFs();
        ClearIrq(LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TX_DONE | LR11XX_IRQ_TIMEOUT);
    }

    void GetPacketStatus(int8_t* const RssiSync, int8_t* const Snr)
    {
        int16_t rssi;

        if (gconfig->modeIsLora()) {
            Lr11xxDriverBase::GetPacketStatus(&rssi, Snr);
        } else {
            // FSK reserve
        }

        if (rssi > -1) rssi = -1; // we do not support values larger than this
        if (rssi < -127) rssi = -127; // we do not support values lower than this

        *RssiSync = rssi;
    }

    void HandleAFC(void) {} // ???

    //-- RF power interface

    virtual void RfPowerCalc(int8_t power_dbm, uint8_t* sx_power, int8_t* actual_power_dbm) = 0;

    //-- helper

    void config_calc(void)
    {
        int8_t power_dbm = gconfig->Power_dbm;
        RfPowerCalc(power_dbm, &sx_power, &actual_power_dbm);

        if (gconfig->modeIsLora()) {
            uint8_t index = gconfig->LoraConfigIndex;
            if (index >= sizeof(Lr11xxLoraConfiguration)/sizeof(Lr11xxLoraConfiguration[0])) while(1){} // must not happen
            lora_configuration = &(Lr11xxLoraConfiguration[index]);
        } else {
            // FSK reserve
        }
    }

    uint32_t TimeOverAir_us(void)
    {
        if (lora_configuration == nullptr) config_calc(); // ensure it is set

        return lora_configuration->TimeOverAir;
    }

    int16_t ReceiverSensitivity_dbm(void)
    {
        if (lora_configuration == nullptr) config_calc(); // ensure it is set

        return lora_configuration->ReceiverSensitivity;
    }

    int8_t RfPower_dbm(void)
    {
        if (lora_configuration == nullptr) config_calc(); // ensure it is set

        return actual_power_dbm;
    }

  protected:
    tSxGlobalConfig* gconfig;

  private:
    const tSxLoraConfiguration* lora_configuration;
    uint8_t sx_power;
    int8_t actual_power_dbm;
};


//-------------------------------------------------------
// Driver for SX1
//-------------------------------------------------------

#ifndef SX_BUSY
  #error SX must have a BUSY pin!
#endif
#ifndef SX_RESET
  #error SX must have a RESET pin!
#endif

// map the irq bits
typedef enum {
    SX_IRQ_TX_DONE = LR11XX_IRQ_TX_DONE,
    SX_IRQ_RX_DONE = LR11XX_IRQ_RX_DONE,
    SX_IRQ_TIMEOUT = LR11XX_IRQ_TIMEOUT,
    SX_IRQ_ALL     = 0,
} SX_IRQ_ENUM;


class Lr11xxDriver : public Lr11xxDriverCommon
{
  public:

    //-- interface to SPI peripheral

    void WaitOnBusy(void) override
    {
        while (sx_busy_read()) { __NOP(); };
    }

    void SpiSelect(void) override
    {
        spi_select();
        delay_ns(50); // datasheet says t1 = 25 ns, semtech driver doesn't do it, helps so do it
    }

    void SpiDeselect(void) override
    {
        delay_ns(50); // datasheet says t8 = 25 ns, semtech driver doesn't do it, helps so do it
        spi_deselect();
    }

    void SpiTransfer(uint8_t* dataout, uint8_t* datain, uint8_t len) override
    {
        spi_transfer(dataout, datain, len);
    }

    void SpiRead(uint8_t* datain, uint8_t len) override
    {
        spi_read(datain, len);
    }

    void SpiWrite(uint8_t* dataout, uint8_t len) override
    {
        spi_write(dataout, len);
    }

    //-- RF power interface

    void RfPowerCalc(int8_t power_dbm, uint8_t* sx_power, int8_t* actual_power_dbm) override
    {
#ifdef POWER_USE_DEFAULT_RFPOWER_CALC
        lr11xx_rfpower_calc(power_dbm, sx_power, actual_power_dbm, POWER_GAIN_DBM, POWER_LR11XX_MAX_DBM);
#else
        // reserved
#endif
    }

    //-- init API functions

    void _reset(void)
    {
        gpio_low(SX_RESET);
        delay_ms(5); // 10 us seems to be sufficient, play it safe, semtech driver uses 50 ms
        gpio_high(SX_RESET);
        delay_ms(50); // semtech driver says "typically 2ms observed"
        WaitOnBusy();
    }

    void Init(void)
    {
        Lr11xxDriverCommon::Init();

        spi_init();
        spi_setnop(0x00); // 0x00 = NOP
        sx_init_gpio();
        sx_dio_exti_isr_clearflag();
        sx_dio_init_exti_isroff();

        // we could probably speed up by using WaitOnBusy()
        delay_ms(300);
        
        _reset(); // this is super crucial !
        SetStandby(LR11XX_STDBY_CONFIG_STDBY_XOSC); // should be in STDBY_RC after reset

        delay_us(1000); // this is important, 500 us ok
    }

    //-- high level API functions

    void StartUp(tSxGlobalConfig* const global_config)
    {
#ifdef SX_USE_REGULATOR_MODE_DCDC // here ??? ELRS does it as last !!!
        SetRegulatorMode(LR11XX_REGULATOR_MODE_DCDC);
#endif

        Configure(global_config);
        delay_us(125); // may not be needed if busy available

        sx_dio_enable_exti_isr();
    }

    //-- this are the API functions used in the loop

    void SendFrame(uint8_t* const data, uint8_t len, uint16_t tmo_ms)
    {
        sx_amp_transmit();
        Lr11xxDriverCommon::SendFrame(data, len, tmo_ms);
        //delay_us(125); // may not be needed if busy available
    }

    void SetToRx(uint16_t tmo_ms)
    {
        sx_amp_receive();
        Lr11xxDriverCommon::SetToRx(tmo_ms);
       //delay_us(125); // may not be needed if busy available
    }
};


//-------------------------------------------------------
// Driver for SX2
//-------------------------------------------------------
#if defined DEVICE_HAS_DIVERSITY || defined DEVICE_HAS_DIVERSITY_SINGLE_SPI

#ifndef SX2_BUSY
  #error SX2 must have a BUSY pin!
#endif
#ifndef SX2_RESET
  #error SX2 must have a RESET pin!
#endif

// map the irq bits
typedef enum {
    SX2_IRQ_TX_DONE = LR11XX_IRQ_TX_DONE,
    SX2_IRQ_RX_DONE = LR11XX_IRQ_RX_DONE,
    SX2_IRQ_TIMEOUT = LR11XX_IRQ_TIMEOUT,
    SX2_IRQ_ALL     = 0,
} SX2_IRQ_ENUM;


class Lr11xxDriver2 : public Lr11xxDriverCommon
{
  public:

    void WaitOnBusy(void) override
    {
        while (sx2_busy_read()) { __NOP(); };
    }

    void SpiSelect(void) override
    {
        spib_select();
        delay_ns(50); // datasheet says t1 = 25 ns, semtech driver doesn't do it, helps so do it
    }

    void SpiDeselect(void) override
    {
        delay_ns(50); // datasheet says t8 = 25 ns, semtech driver doesn't do it, helps so do it
        spib_deselect();
    }

#ifndef DEVICE_HAS_DIVERSITY_SINGLE_SPI
    void SpiTransfer(uint8_t* dataout, uint8_t* datain, uint8_t len) override
    {
        spib_transfer(dataout, datain, len);
    }

    void SpiRead(uint8_t* datain, uint8_t len) override
    {
        spib_read(datain, len);
    }

    void SpiWrite(uint8_t* dataout, uint8_t len) override
    {
        spib_write(dataout, len);
    }
#else
    void SpiTransfer(uint8_t* dataout, uint8_t* datain, uint8_t len) override
    {
        spi_transfer(dataout, datain, len);
    }

    void SpiRead(uint8_t* datain, uint8_t len) override
    {
        spi_read(datain, len);
    }

    void SpiWrite(uint8_t* dataout, uint8_t len) override
    {
        spi_write(dataout, len);
    }
#endif

    //-- RF power interface

    void RfPowerCalc(int8_t power_dbm, uint8_t* sx_power, int8_t* actual_power_dbm) override
    {
#ifdef POWER_USE_DEFAULT_RFPOWER_CALC
        lr11xx_rfpower_calc(power_dbm, sx_power, actual_power_dbm, POWER_GAIN_DBM, POWER_LR11XX_MAX_DBM);
#else
        // reserved
#endif
    }

    //-- init API functions

    void _reset(void)
    {
        gpio_low(SX2_RESET);
        delay_ms(5); // 10 us seems to be sufficient, play it safe, semtech driver uses 50 ms
        gpio_high(SX2_RESET);
        delay_ms(50); // semtech driver says "typically 2ms observed"
        WaitOnBusy();
    }

    void Init(void)
    {
        Lr11xxDriverCommon::Init();

#ifndef DEVICE_HAS_DIVERSITY_SINGLE_SPI
        spib_init();
        spib_setnop(0x00); // 0x00 = NOP
#else
        // spi init done already by driver1
#endif
        sx2_init_gpio();
        sx2_dio_exti_isr_clearflag();
        sx2_dio_init_exti_isroff();

        // we could probably speed up by using WaitOnBusy()
        delay_ms(300);
        _reset(); // this is super crucial !

        SetStandby(LR11XX_STDBY_CONFIG_STDBY_XOSC); // should be in STDBY_RC after reset
        delay_us(1000); // this is important, 500 us ok
    }

    //-- high level API functions

    void StartUp(tSxGlobalConfig* const global_config)
    {


#ifdef SX2_USE_REGULATOR_MODE_DCDC // here ??? ELRS does it as last !!!
        SetRegulatorMode(LR11XX_REGULATOR_MODE_DCDC);
#endif

        Configure(global_config);
        delay_us(125); // may not be needed if busy available

        sx2_dio_enable_exti_isr();
    }

    //-- this are the API functions used in the loop

    void SendFrame(uint8_t* const data, uint8_t len, uint16_t tmo_ms)
    {
        sx2_amp_transmit();
        Lr11xxDriverCommon::SendFrame(data, len, tmo_ms);
        //delay_us(125); // may not be needed if busy available
    }

    void SetToRx(uint16_t tmo_ms)
    {
        sx2_amp_receive();
        Lr11xxDriverCommon::SetToRx(tmo_ms);
        //delay_us(125); // may not be needed if busy available
    }
};

#endif

#endif // LR11XX_DRIVER_H
