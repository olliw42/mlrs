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
//*******************************************************
#ifndef STDSTM32_GDISP_H
#define STDSTM32_GDISP_H
#ifdef __cplusplus
extern "C" {
#endif


#ifdef ESP32
#include "../../Common/hal/esp-glue.h"
#else
#include "main.h" // this is to include the correct stm32XXxx_hal.h
#endif
#include "../thirdparty/gfxfont.h"


#define STDSTM32_GDISP_USE_O3


//-------------------------------------------------------
// I2C interface
//-------------------------------------------------------
// forward declarations

void i2c_setdeviceadr(uint8_t dev_adr);
HAL_StatusTypeDef i2c_device_ready(void);
HAL_StatusTypeDef i2c_put_blocked(uint8_t reg_adr, uint8_t* const buf, uint16_t len);
HAL_StatusTypeDef i2c_put(uint8_t reg_adr, uint8_t* const buf, uint16_t len);


//-------------------------------------------------------
// SSD1306 graphical display driver routines
// these (and only these) use the I2C functions
//-------------------------------------------------------

#define SSD1306_ADR               0x3C // address 0111100 or 0111101, that's the real adr, needs be shifted up by one
#define SSD1306_CMD               0x00 // adr = 0x00 for command, 0x40 for data
#define SSD1306_DATA              0x40 // adr = 0x00 for command, 0x40 for data

void ssd1306_init();
void ssd1306_cmd2(uint8_t _cmd, uint8_t _data);
void ssd1306_cmdhome(void);
void ssd1306_contraststart(void);
void ssd1306_contrastend(void);
void ssd1306_contrast(uint8_t c);
HAL_StatusTypeDef ssd1306_put_noblock(uint8_t* const buf, uint16_t len);


//-------------------------------------------------------
// Graphical display API
//-------------------------------------------------------

// these are currently assumed to be equal for all supported displays:

#define GDISPLAY_COLUMNS          128
#define GDISPLAY_ROWS             64
#define GDISPLAY_PAGES            8
#define GDISPLAY_ROWS_PER_PAGE    (GDISPLAY_ROWS / GDISPLAY_PAGES)
#define GDISPLAY_BUFSIZE          (GDISPLAY_COLUMNS * GDISPLAY_PAGES)


typedef enum {
    GDISPLAY_TYPE_SSD1306 = 0,
    GDISPLAY_TYPE_SH1106,
    GDISPLAY_TYPE_UNDEFINED,
} GDISPLAY_TYPE_ENUM;


typedef enum {
    GDISPLAY_ROTATION_NORMAL = 0,
    GDISPLAY_ROTATION_90,
    GDISPLAY_ROTATION_180,
    GDISPLAY_ROTATION_270,
    GDISPLAY_ROTATION_UNDEFINED,
} GDISPLAY_ROTATION_ENUM;


typedef enum {
    GDISPLAY_FONT_BG_NONE = 0, // no background, default since fastest
    GDISPLAY_FONT_BG_FULL,
} GDISPLAY_FONT_BG_ENUM;


typedef struct
{
    uint16_t type;

    uint16_t width;
    uint16_t height;
    uint16_t rotation;
    void (*setpixel_ptr)(uint16_t x, uint16_t y, uint16_t color);

    int16_t curX;
    int16_t curY;
    int16_t spacing;
    int16_t kerning;
    int16_t inverted; // only affects text

    GFXfont* font;
    uint16_t font_background;

    // to catch that it needs to be updated
    uint16_t needsupdate;

    uint8_t buf[GDISPLAY_BUFSIZE] ALIGNED8_ATTR;
} tGDisplay;


//-------------------------------------------------------
// HAL
//-------------------------------------------------------

void gdisp_hal_init(uint16_t type);
void gdisp_hal_cmdhome(void);
HAL_StatusTypeDef gdisp_hal_put(uint8_t* const buf, uint16_t len);
void gdisp_hal_contraststart(void);
void gdisp_hal_contrastend(void);
void gdisp_hal_contrast(uint8_t c);


//-------------------------------------------------------
// Low-level API
// general
// can be called by the user
//-------------------------------------------------------

void gdisp_update(void);
uint8_t gdisp_update_completed(void);
void gdisp_setrotation(uint16_t rotation);


//-------------------------------------------------------
// the following routines work only on the display buffer

//-------------------------------------------------------
// Low-level API
// helper
// should never be directly called by the user
//-------------------------------------------------------

//static inline void gdisp_u_(int16_t minx, int16_t maxx, int16_t miny, int16_t maxy);
//void gdisp_setpixel_(uint16_t x, uint16_t y, uint16_t color);


//-------------------------------------------------------
// High-level API
// Draw primitives
//-------------------------------------------------------

void gdisp_clear(void);
void gdisp_drawpixel(int16_t x, int16_t y, uint16_t color);
void gdisp_drawbitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h,  uint16_t color);


//-------------------------------------------------------
// High-level API
// Draw objects (lines, rectangles, ...)
//-------------------------------------------------------

void gdisp_writeline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void gdisp_drawline_H(int16_t x0, int16_t y0, int16_t w, uint16_t color);
void gdisp_drawline_V(int16_t x0, int16_t y0, int16_t h, uint16_t color);
void gdisp_drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void gdisp_drawrect_WH(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color);
void gdisp_drawrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void gdisp_fillrect_WH(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color);
void gdisp_fillrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);


//-------------------------------------------------------
// High-level API
// Text functions
//-------------------------------------------------------

void gdisp_setfontbackground(void);
void gdisp_unsetfontbackground(void);
void gdisp_setfont(const GFXfont* const f);
void gdisp_unsetfont(void);
void gdisp_setkerning(int16_t k);
void gdisp_unsetkerning(void);
void gdisp_setinverted(void);
void gdisp_unsetinverted(void);

void gdisp_setcurX(int16_t x);
void gdisp_setcurY(int16_t y);
void gdisp_setcurXY(int16_t x, int16_t y);
void gdisp_movecurX(int16_t dx);
void gdisp_movecurY(int16_t dy);

void gdisp_w(char c);
void gdisp_wf(char c);
uint16_t gdisp_strwidth(const char* s);
void gdisp_putc(char c);
void gdisp_puts(const char* s);
void gdisp_puts_XCentered(const char* s);


//-------------------------------------------------------
// INIT routines
//-------------------------------------------------------

void gdisp_init(uint16_t type);


//-------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif // STDSTM32_GDISP_H


