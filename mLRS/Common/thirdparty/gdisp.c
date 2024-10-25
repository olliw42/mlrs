//*******************************************************
// OlliW, OlliW42, www.olliw.eu
// Copyright & License: see comment
//*******************************************************
// Copyright & License:
// Some functions, like gdisp_drawbitmap() or the character drawing are
// derived from code of the Adafruit-GFX-Library project, and are licensed accordingly:
//   github repo: https://github.com/adafruit/Adafruit-GFX-Library
//   license: https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt
// The parts which are not covered by this copyright and license are licensed
//   GPL3
//   https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// Graphic Display Interface
// simplified 7.5.2024
// modified 17.8.2024, to do page cmds for ESP, makes it work for SSD1306 and NFP1115


#include <string.h>
#include "../../modules/stm32ll-lib/src/stdstm32.h"
#include "gdisp.h"


#ifdef STDSTM32_GDISP_USE_O3
#pragma GCC push_options
#pragma GCC optimize ("O3")
#endif


//-------------------------------------------------------
// Fonts
//-------------------------------------------------------

static const uint8_t font6x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sp
    0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, // !
    0x00, 0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, // #
    0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, // $
    0x00, 0x62, 0x64, 0x08, 0x13, 0x23, // %
    0x00, 0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, // (
    0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, // )
    0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x00, 0x00, 0xA0, 0x60, 0x00, // ,
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x00, 0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x00, 0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x00, 0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x00, 0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x00, 0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x00, 0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x00, 0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x00, 0x32, 0x49, 0x59, 0x51, 0x3E, // @
    0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C, // A
    0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x00, 0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x00, 0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x00, 0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x00, 0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x00, 0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55, // backslash
    0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x00, 0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x00, 0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x00, 0x01, 0x02, 0x04, 0x00, // '
    0x00, 0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x00, 0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x00, 0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x00, 0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C, // g
    0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x00, 0x40, 0x80, 0x84, 0x7D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x00, 0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x00, 0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x00, 0xFC, 0x24, 0x24, 0x24, 0x18, // p
    0x00, 0x18, 0x24, 0x24, 0x18, 0xFC, // q
    0x00, 0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x00, 0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x00, 0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x00, 0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C, // y
    0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x00, 0x08, 0x77, 0x41, 0x00, // {
    0x00, 0x00, 0x00, 0x63, 0x00, 0x00, // ¦
    0x00, 0x00, 0x41, 0x77, 0x08, 0x00, // }
    0x00, 0x08, 0x04, 0x08, 0x08, 0x04, // ~

    0x00, 0x08, 0x14, 0x08, 0x14, 0x08, // 0x7F = inf
};


//-------------------------------------------------------
// SSD1306 graphical display driver routines
//-------------------------------------------------------

static const uint8_t ssd1306_initstream[] = {
    0xAE,         // Display OFF
    0xD5, 0x80,   // Clock Divide Ratio and Oscillator Frequency
    0xA8, 0x3F,   // MUX Ratio
    0xD3, 0x00,   // Display Offset
    0x40,         // Display Start Line
    0x8D, 0x14,   // enable charge pump regulator
#if !(defined ESP8266 || defined ESP32)
    0x20, 0x00,   // Memory Addressing Mode
#else
    0x20, 0x10,   // Memory Addressing Mode
#endif
    0xA1,         // Segment re-map
    0xC8,         // COM Output Scan Direction
#if !(defined ESP8266 || defined ESP32)
    0xDA, 0x12,   // COM Pins hardware configuration
#endif
    0x81, 0xCF,   // Contrast Control
    0xD9, 0xF1,   // Pre-charge Period
    0xDB, 0x40,   // VCOMH Deselect Level
    0x2E,         // Deactivate scroll // added 31.5.2024
    0xA4,         // Entire Display ON
    0xA6,         // Normal/Inverse Display
    0xAF,         // Display ON
#if !(defined ESP8266 || defined ESP32)
    0x21, 0, 127, // Column Address
    0x22, 0, 7    // Page Address
#endif
};


void ssd1306_init()
{
    i2c_setdeviceadr(SSD1306_ADR);
    i2c_put_blocked(SSD1306_CMD, (uint8_t*)&ssd1306_initstream, sizeof(ssd1306_initstream));
}


void ssd1306_cmd2(uint8_t _cmd, uint8_t _data)
{
    uint8_t cmd[2] = {_cmd, _data};
    i2c_put_blocked(SSD1306_CMD, cmd, 2);
}


void ssd1306_cmdhome(void)
{
    uint8_t cmd[6] = {0x21, 0, 127, 0x22, 0, 7};
//    i2c_put_blocked(SSD1306_CMD, cmd, 6);
    i2c_put_blocked(SSD1306_CMD, cmd + 0, 3);
    i2c_put_blocked(SSD1306_CMD, cmd + 3, 3);
}


void ssd1306_contraststart(void)
{
    ssd1306_cmd2(0xD9, 0x2F);
    ssd1306_cmd2(0xDB, 0x00);
}


void ssd1306_contrastend(void)
{
    ssd1306_cmd2(0xD9, 0xF1);
    ssd1306_cmd2(0xDB, 0x40);
    ssd1306_cmd2(0x81, 0xCF);
}


void ssd1306_contrast(uint8_t c)
{
    ssd1306_cmd2(0x81, c);
}


/* not used
HAL_StatusTypeDef ssd1306_put(uint8_t* buf, uint16_t len)
{
    ssd1306_cmdhome();
    return i2c_put_blocked(SSD1306_DATA, buf, len);
} */


#if !(defined ESP8266 || defined ESP32)
HAL_StatusTypeDef ssd1306_put_noblock(uint8_t* const buf, uint16_t len)
{
    ssd1306_cmdhome();
    return i2c_put(SSD1306_DATA, buf, len);
}
#else
HAL_StatusTypeDef ssd1306_put_noblock(uint8_t* buf, uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t page = 0;
    while ((len > 0) && (ret == HAL_OK)) {
        uint8_t cmd[3] = {0x00, 0x10, 0xB0 + page};
        i2c_put(SSD1306_CMD, cmd, 3); // ATTENTION: this does it NOT blocking !
        ret = i2c_put(SSD1306_DATA, buf, MIN(128, len));
        page++;
        buf += 128;
        len -= 128;
    }
    return ret;
}
#endif


//-------------------------------------------------------
// Graphical display API
//-------------------------------------------------------

tGDisplay gdisp;


//-------------------------------------------------------
// HAL
//-------------------------------------------------------

void gdisp_hal_init(uint16_t type)
{
    gdisp.type = type;

    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: ssd1306_init(); break;
        case GDISPLAY_TYPE_SH1106: break;
    }
}


void gdisp_hal_cmdhome(void)
{
    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: ssd1306_cmdhome(); return;
        case GDISPLAY_TYPE_SH1106: return;
    }
}


HAL_StatusTypeDef gdisp_hal_put(uint8_t* const buf, uint16_t len)
{
    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: return ssd1306_put_noblock(buf, len);
        case GDISPLAY_TYPE_SH1106: return HAL_OK;
    }
    return HAL_OK;
}


void gdisp_hal_contraststart(void)
{
    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: ssd1306_contraststart(); return;
        case GDISPLAY_TYPE_SH1106: return;
    }
}


void gdisp_hal_contrastend(void)
{
    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: ssd1306_contrastend(); return;
        case GDISPLAY_TYPE_SH1106: return;
    }
}


void gdisp_hal_contrast(uint8_t c)
{
    switch (gdisp.type) {
        case GDISPLAY_TYPE_SSD1306: ssd1306_contrast(c); return;
        case GDISPLAY_TYPE_SH1106: return;
    }
}


//-------------------------------------------------------
// Low-level API
// general
// can be called by the user
//-------------------------------------------------------

void gdisp_update(void)
{
    // this must not be called too frequently, any I2C transfers must have been finished

    // for the moment we simply copy the complete buf to the display
    // we should be smarter and use the rectangle

    if (!gdisp.needsupdate) return;

    HAL_StatusTypeDef res = gdisp_hal_put(gdisp.buf, GDISPLAY_BUFSIZE);
//    HAL_StatusTypeDef res = ssd1306_put(gdisp.buf, GDISPLAY_BUFSIZE);
//    HAL_StatusTypeDef res = ssd1306_put_noblock(gdisp.buf, GDISPLAY_BUFSIZE);
//    while (i2c_device_ready() == HAL_BUSY) {};

    if (res != HAL_OK) return; // retry, needs update is not reset, so it will be tried next time again

    gdisp.needsupdate = 0;
}


uint8_t gdisp_update_completed(void)
{
    return (i2c_device_ready() != HAL_BUSY) ? 1 : 0;
}


void gdisp_setpixelfuncptr_(uint16_t rotation);

void gdisp_setrotation(uint16_t rotation)
{
    // in this routine we may take advantage of hardware acceleration possibilities
    gdisp.rotation = rotation;

    gdisp_setpixelfuncptr_(gdisp.rotation);

    if ((gdisp.rotation == GDISPLAY_ROTATION_90) || (gdisp.rotation == GDISPLAY_ROTATION_270)) {
        gdisp.width = GDISPLAY_ROWS;
        gdisp.height = GDISPLAY_COLUMNS;
    } else {
        gdisp.width = GDISPLAY_COLUMNS;
        gdisp.height = GDISPLAY_ROWS;
    }

    // clear
    gdisp.needsupdate = 0;
    memset(gdisp.buf, 0, GDISPLAY_BUFSIZE);
}


//-------------------------------------------------------
// the following routines work only on the display buffer


//-------------------------------------------------------
// Low-level API
// helper
// should never be directly called by the user
//-------------------------------------------------------

void gdisp_setpixel_(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x < 0) || (x >= GDISPLAY_COLUMNS)) return;
    if ((y < 0) || (y >= GDISPLAY_ROWS)) return;

    uint16_t i = x + (y >> 3) * GDISPLAY_COLUMNS;
    if (i >= GDISPLAY_BUFSIZE) return;

    gdisp.needsupdate = 1;

    if (color & 0x01) {
        gdisp.buf[i] |= (1 << (y % 8));
    } else {
        gdisp.buf[i] &= ~(1 << (y % 8));
    }
}


void gdisp_setpixel_rot90_(uint16_t x, uint16_t y, uint16_t color)
{
    gdisp_setpixel_((GDISPLAY_COLUMNS-1)-y, x, color);
}

void gdisp_setpixel_rot180_(uint16_t x, uint16_t y, uint16_t color)
{
    gdisp_setpixel_((GDISPLAY_COLUMNS-1)-x, (GDISPLAY_ROWS-1)-y, color);
}

void gdisp_setpixel_rot270_(uint16_t x, uint16_t y, uint16_t color)
{
    gdisp_setpixel_(y, (GDISPLAY_ROWS-1)-x, color);
}


void gdisp_setpixelfuncptr_(uint16_t rotation)
{
    switch (rotation) {
        case GDISPLAY_ROTATION_NORMAL: gdisp.setpixel_ptr = &gdisp_setpixel_; break;
        case GDISPLAY_ROTATION_90: gdisp.setpixel_ptr = &gdisp_setpixel_rot90_; break;
        case GDISPLAY_ROTATION_180: gdisp.setpixel_ptr = &gdisp_setpixel_rot180_; break;
        case GDISPLAY_ROTATION_270: gdisp.setpixel_ptr = &gdisp_setpixel_rot270_; break;
        default:
            gdisp.setpixel_ptr = &gdisp_setpixel_;
    }
}


//-------------------------------------------------------
// High-level API
// Draw primitives
//-------------------------------------------------------

void gdisp_clear(void)
{
    gdisp.needsupdate = 1;
    memset(gdisp.buf, 0, GDISPLAY_BUFSIZE);
}


void gdisp_drawpixel(int16_t x, int16_t y, uint16_t color)
{
    // we allow x to run beyond the COLUMNS width
    // x += y / 8 * gdisp.width;

    // in this routine we may take advantage of hardware acceleration possibilities
    gdisp.setpixel_ptr(x, y, color);
}


void gdisp_drawbitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h,  uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // bitmap scanline pad = whole byte
    uint8_t bits = 0;

    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                bits <<= 1;
            else
                bits = bitmap[j * byteWidth + i / 8];

            if (bits & 0x80) gdisp_drawpixel(x + i, y, color);
        }
        y++;
    }
}


//-------------------------------------------------------
// High-level API
// Draw objects (lines, rectangles, ...)
//-------------------------------------------------------

void gdisp_writeline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    while (1) {} // TODO
}


void gdisp_drawline_H(int16_t x0, int16_t y0, int16_t w, uint16_t color)
{
    if (w < 0) {
        for (int16_t i = 0; i < -w; i++) gdisp_drawpixel(x0 - i, y0, color);
    } else {
        for (int16_t i = 0; i < w; i++) gdisp_drawpixel(x0 + i, y0, color);
    }
}


void gdisp_drawline_V(int16_t x0, int16_t y0, int16_t h, uint16_t color)
{
    if (h < 0) {
        for (int16_t i = 0; i < -h; i++) gdisp_drawpixel(x0, y0 - i, color);
    } else {
        for (int16_t i = 0; i < h; i++) gdisp_drawpixel(x0, y0 + i, color);
    }
}


void gdisp_drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if (x0 == x1) {
        if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; } // swap
        gdisp_drawline_V(x0, y0, y1 - y0 + 1, color);
    } else
    if (y0 == y1) {
        if (x0 > x1) { int16_t t = x0; x0 = x1; x1 = t; } // swap
        gdisp_drawline_H(x0, y0, x1 - x0 + 1, color);
    } else {
        gdisp_writeline(x0, y0, x1, y1, color);
    }
}


void gdisp_drawrect_WH(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color)
{
    int16_t dy = (h < 0) ? h + 1 : h -1;
    int16_t dx = (w < 0) ? w + 1 : w - 1;
    gdisp_drawline_H(x0, y0, w, color);
    gdisp_drawline_H(x0, y0 + dy, w, color);
    gdisp_drawline_V(x0, y0, h, color);
    gdisp_drawline_V(x0 + dx, y0, h, color);
}


void gdisp_drawrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    gdisp_drawrect_WH(x0, y0, x1 - x0 + 1, y1 - y0 + 1, color);
}


void gdisp_fillrect_WH(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color)
{
    if (h < 0) {
        for (int16_t y = y0; y > y0 + h; y--) gdisp_drawline_H(x0, y, w, color);
    } else {
        for (int16_t y = y0; y < y0 + h; y++) gdisp_drawline_H(x0, y, w, color);
    }
}


void gdisp_fillrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    gdisp_fillrect_WH(x0, y0, x1 - x0 + 1, y1 - y0 + 1, color);
}


//-------------------------------------------------------
// High-level API
// Text functions
//-------------------------------------------------------

void gdisp_setfontbackground(void) { gdisp.font_background = GDISPLAY_FONT_BG_FULL; }
void gdisp_unsetfontbackground(void) { gdisp.font_background = GDISPLAY_FONT_BG_NONE; }
void gdisp_setfont(const GFXfont* const f) { gdisp.font = (GFXfont*)f; }
void gdisp_unsetfont(void) { gdisp.font = NULL; }

void gdisp_setkerning(int16_t k) { gdisp.kerning = k; }
void gdisp_unsetkerning(void) { gdisp.kerning = 0; }

void gdisp_setinverted(void) { gdisp.inverted = 1; }
void gdisp_unsetinverted(void) { gdisp.inverted = 0; }

void gdisp_setcurX(int16_t x) { gdisp.curX = x; }
void gdisp_setcurY(int16_t y) { gdisp.curY = y; }
void gdisp_setcurXY(int16_t x, int16_t y) {  gdisp.curX = x; gdisp.curY = y; }
void gdisp_movecurX(int16_t dx) { gdisp.curX += dx; }
void gdisp_movecurY(int16_t dy) { gdisp.curY += dy; }


void gdisp_w(char c)
{
    if (c =='\n') { gdisp.curX = 0; gdisp.curY += (8 + gdisp.spacing); return; }

    if (c < ' ') return;
    c -= ' ';

    if (gdisp.curX + 6 >= gdisp.width) { gdisp.curX = 0; gdisp.curY += (8 + gdisp.spacing); } //glyph doesn't fit on screen, so next line

    int16_t x = gdisp.curX;
    int16_t y = gdisp.curY - 6; // * (8 + gdisp.spacing);

    gdisp.curX += 6;

    if (gdisp.font_background == GDISPLAY_FONT_BG_NONE) {
        for (uint16_t i = 0; i < 6; i++) {
            uint8_t b = font6x8[c * 6 + i];
            if (gdisp.inverted) b = ~b;
            for (uint16_t j = 0; j < 8; j++) {
                if (b & 0x01) gdisp_drawpixel(x + i, y + j, 1);
                b >>= 1;
            }
        }
        return;
    }

    for (uint16_t i = 0; i < 6; i++) {
        uint8_t b = font6x8[c * 6 + i];
        if (gdisp.inverted) b = ~b;

        for (uint16_t j = 0; j < 8; j++) {
            gdisp_drawpixel(x + i, y + j, b);
            b >>= 1;
        }
    }
}


void gdisp_wf(char c)
{
    uint16_t ya = gdisp.font->yAdvance;

    if (c =='\n') { gdisp.curX = 0; gdisp.curY += (ya + gdisp.spacing); return; }

    uint8_t first = gdisp.font->first;
    if ((c < first) || (c > gdisp.font->last)) return;

    uint8_t* bitmap = gdisp.font->bitmap;

    GFXglyph* glyph = &(gdisp.font->glyph[c - first]);
    uint16_t w = glyph->width;
    uint16_t h = glyph->height;
    int16_t  xo = glyph->xOffset;
    int16_t  yo = glyph->yOffset;
    uint16_t xa = glyph->xAdvance + gdisp.kerning;

    if (gdisp.curX + xo + w > gdisp.width) { gdisp.curX = 0; gdisp.curY += (ya + gdisp.spacing); } // glyph doesn't fit on screen, new line
    int16_t x = gdisp.curX;
    int16_t y = gdisp.curY;
    gdisp.curX += xa;

    if (gdisp.inverted) {
        gdisp_fillrect_WH(x + xo - 1, y + yo - 1, xa + 2, h + 2, 1);
    } else
    if (gdisp.font_background == GDISPLAY_FONT_BG_FULL) {
        gdisp_fillrect_WH(x + xo - 1, y + yo - 1, xa + 2, h + 2, 0);
    }

    uint16_t bit = 0; // counts how many pixels were set
    uint16_t bits = 0;
    uint16_t bo = glyph->bitmapOffset;
    uint16_t col = (gdisp.inverted) ? 0 : 1;
    for (uint16_t yy = 0; yy < h; yy++) {
        for (uint16_t xx = 0; xx < w; xx++) {
            if (!(bit++ & 7)) bits = bitmap[bo++];
            if (bits & 0x80) gdisp_drawpixel(x + xo + xx, y + yo + yy, col);
            bits <<= 1;
        }
    }
}


uint16_t gdisp_strwidth(const char* s)
{
    if (gdisp.font) {
        uint16_t w = 0;
        uint8_t first = gdisp.font->first;
        uint8_t last = gdisp.font->last;
        while (*s) {
            char c = *s;
            s++;
            if ((c < first) || (c > last)) continue;
            GFXglyph* glyph = &(gdisp.font->glyph[c - first]);
            uint16_t xa = glyph->xAdvance + gdisp.kerning;
            w += xa;
        }
        return w;
    }
    return strlen(s)*6;
}


void gdisp_putc(char c)
{
    if (gdisp.font) gdisp_wf(c); else gdisp_w(c);
}


void gdisp_puts(const char* s)
{
    while (*s) { gdisp_putc(*s); s++; }
}


void gdisp_puts_XCentered(const char* s)
{
    uint16_t w = gdisp_strwidth(s);
    gdisp_setcurX((gdisp.width - w) / 2);
    gdisp_puts(s);
}


//-------------------------------------------------------
// INIT routines
//-------------------------------------------------------

void gdisp_init(uint16_t type)
{
    gdisp_hal_init(type);

    gdisp.rotation = GDISPLAY_ROTATION_NORMAL;
    gdisp.width = GDISPLAY_COLUMNS;
    gdisp.height = GDISPLAY_ROWS;

    gdisp.setpixel_ptr = &gdisp_setpixel_;

    gdisp.curX = 0;
    gdisp.curY = 6; // that's the base line of the default font
    gdisp.font = NULL;
    gdisp.font_background = GDISPLAY_FONT_BG_NONE;
    gdisp.spacing = 0;
    gdisp.kerning = 0;
    gdisp.inverted = 0;

    gdisp_clear(); // sets also gdisp.needsupdate = 1;
    gdisp_update();
}


#ifdef STDSTM32_GDISP_USE_O3
#pragma GCC pop_options
#endif
