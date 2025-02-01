//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// ESP RGB
//*******************************************************
#ifndef ESPLIB_RGB_H
#define ESPLIB_RGB_H

// RGB led driver that uses I2S peripheral on ESP32, modified and cutdown from ELRS

//-------------------------------------------------------
// Defines
//-------------------------------------------------------

#include "driver/i2s.h"

#define I2S_NUM i2s_port_t(0)

#if defined(CONFIG_IDF_TARGET_ESP32S2)
#define SAMPLE_RATE (360000)
#define MCLK 48000000
static const int bitorder[] = {0x40, 0x80, 0x10, 0x20, 0x04, 0x08, 0x01, 0x02};
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define SAMPLE_RATE (800000)
#define MCLK 160000000
static const int bitorder[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define SAMPLE_RATE (800000)
#define MCLK 160000000
static const int bitorder[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
#elif defined(CONFIG_IDF_TARGET_ESP32)
#define SAMPLE_RATE (360000)
#define MCLK 48000000
static const int bitorder[] = {0x40, 0x80, 0x10, 0x20, 0x04, 0x08, 0x01, 0x02};
#endif


//-------------------------------------------------------
//  RgbColor Class
//-------------------------------------------------------

class RgbColor
{
public:
    RgbColor(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b) {}

    uint8_t R;
    uint8_t G;
    uint8_t B;
};


//-------------------------------------------------------
//  ESP32LedDriver Class
//-------------------------------------------------------

class ESP32LedDriver
{
public:
    ESP32LedDriver(int count, int pin);

    void Show();
    void ClearTo(RgbColor color, uint16_t first, uint16_t last);
    void SetPixelColor(uint16_t indexPixel, RgbColor color);
    
    void redOff(uint16_t first, uint16_t last);
    void redOn(uint16_t first, uint16_t last);
    void redToggle(uint16_t first, uint16_t last);
    void greenOff(uint16_t first, uint16_t last);
    void greenOn(uint16_t first, uint16_t last);
    void greenToggle(uint16_t first, uint16_t last);
    void blueOff(uint16_t first, uint16_t last);
    void blueOn(uint16_t first, uint16_t last);
    void blueToggle(uint16_t first, uint16_t last);

private:
    RgbColor *ledsbuff = nullptr;
    uint16_t *out_buffer = nullptr;
    size_t out_buffer_size;
    int num_leds;
    int gpio_pin;
    
    bool ledRedState;
    bool ledGreenState;
    bool ledBlueState;

};


ESP32LedDriver::ESP32LedDriver(int count, int pin) : num_leds(count), gpio_pin(pin)
{
    out_buffer_size = num_leds * 24 * sizeof(uint16_t);
    out_buffer = (uint16_t *)heap_caps_malloc(out_buffer_size, MALLOC_CAP_8BIT);

    i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .use_apll = true,
    .tx_desc_auto_clear = true,
    .fixed_mclk = MCLK,
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = -1,
        .ws_io_num = -1,
        .data_out_num = gpio_pin,
        .data_in_num = -1,
    };

    i2s_config.dma_buf_len = out_buffer_size;
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    delay(1); // without this it fails to boot and gets stuck!
    i2s_set_pin(I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM);
    i2s_stop(I2S_NUM);
}

IRAM_ATTR void ESP32LedDriver::Show()
{
    size_t bytes_written = 0;
    i2s_stop(I2S_NUM);
    i2s_write(I2S_NUM, out_buffer, out_buffer_size, &bytes_written, portMAX_DELAY);
    i2s_start(I2S_NUM);
}

IRAM_ATTR void ESP32LedDriver::ClearTo(RgbColor color, uint16_t first, uint16_t last)
{
    for (uint16_t i=first ; i<=last; i++)
    {
        SetPixelColor(i, color);
    }
}

IRAM_ATTR void ESP32LedDriver::SetPixelColor(uint16_t indexPixel, RgbColor color)
{
    int loc = indexPixel * 24;
    for(int bitpos = 0 ; bitpos < 8 ; bitpos++)
    {
        int bit = bitorder[bitpos];
        out_buffer[loc + bitpos + 0] = (color.G & bit) ? 0xFFE0 : 0xF000;
        out_buffer[loc + bitpos + 8] = (color.R & bit) ? 0xFFE0 : 0xF000;
        out_buffer[loc + bitpos + 16] = (color.B & bit) ? 0xFFE0 : 0xF000;
    }
}

IRAM_ATTR void ESP32LedDriver::redOff(uint16_t first, uint16_t last)
{
    if (!ledRedState) return;
    ClearTo(RgbColor(0, 0, 0), first, last);
    Show();
    ledRedState = 0;
}

IRAM_ATTR void ESP32LedDriver::redOn(uint16_t first, uint16_t last)
{
    if (ledRedState) return;
    ClearTo(RgbColor(255, 0, 0), first, last);
    Show();
    ledRedState = 1;
}

IRAM_ATTR void ESP32LedDriver::redToggle(uint16_t first, uint16_t last)
{
    if (ledRedState) { redOff(first, last); } else { redOn(first, last); }
}

IRAM_ATTR void ESP32LedDriver::greenOff(uint16_t first, uint16_t last)
{
    if (!ledGreenState) return;
    ClearTo(RgbColor(0, 0, 0), first, last);
    Show();
    ledGreenState = 0;
}

IRAM_ATTR void ESP32LedDriver::greenOn(uint16_t first, uint16_t last)
{
    if (ledGreenState) return;
    ClearTo(RgbColor(0, 255, 0), first, last);
    Show();
    ledGreenState = 1;
}

IRAM_ATTR void ESP32LedDriver::greenToggle(uint16_t first, uint16_t last)
{
    if (ledGreenState) { greenOff(first, last); } else { greenOn(first, last); }
}

IRAM_ATTR void ESP32LedDriver::blueOff(uint16_t first, uint16_t last)
{
    if (!ledBlueState) return;
    ClearTo(RgbColor(0, 0, 0), first, last);
    Show();
    ledBlueState = 0;
}

IRAM_ATTR void ESP32LedDriver::blueOn(uint16_t first, uint16_t last)
{
    if (ledBlueState) return;
    ClearTo(RgbColor(0, 0, 255), first, last);
    Show();
    ledBlueState = 1;
}

IRAM_ATTR void ESP32LedDriver::blueToggle(uint16_t first, uint16_t last)
{
    if (ledBlueState) { blueOff(first, last); } else { blueOn(first, last); }
}


#endif // ESPLIB_RGB_H
