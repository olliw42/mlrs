//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// ESP I2C
//*******************************************************
#ifndef ESPLIB_I2C_H
#define ESPLIB_I2C_H
#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------
// Defines
//-------------------------------------------------------

#include "esp-peripherals.h"
#include <Wire.h>
// https://github.com/espressif/arduino-esp32/blob/master/libraries/Wire/src/Wire.h


//-------------------------------------------------------
//  I2C user routines
//-------------------------------------------------------

uint8_t i2c_dev_adr;


// call this before transaction
IRAM_ATTR void i2c_setdeviceadr(uint8_t dev_adr)
{
    i2c_dev_adr = dev_adr;
}


IRAM_ATTR uint8_t i2c_getdeviceadr(void)
{
    return i2c_dev_adr;
}


IRAM_ATTR HAL_StatusTypeDef i2c_put_blocked(uint8_t reg_adr, uint8_t* buf, uint16_t len)
{
    Wire.beginTransmission(i2c_dev_adr);
    Wire.write(reg_adr);
    Wire.write(buf, len);
    uint8_t error = Wire.endTransmission(true);
    return (error == 0) ? HAL_OK : HAL_ERROR;
}


/*
IRAM_ATTR HAL_StatusTypeDef i2c_put(uint8_t reg_adr, uint8_t* buf, uint16_t len)
{
    Wire.beginTransmission(i2c_dev_adr);
    Wire.write(reg_adr);
    Wire.write(buf, len);
    uint8_t error = Wire.endTransmission(true);
    return (error == 0) ? HAL_OK : HAL_ERROR;
}


IRAM_ATTR HAL_StatusTypeDef i2c_device_ready(void)
{
    Wire.beginTransmission(i2c_dev_adr);
    uint8_t error = Wire.endTransmission(true);
    return (error == 0) ? HAL_OK : HAL_ERROR;
}
*/

#define I2C_WORK_CHUNK_SIZE 128
#define I2C_WORK_QUEUE_SIZE 16

typedef struct
{
    uint8_t buf[I2C_WORK_CHUNK_SIZE];
    uint8_t reg_adr;
    uint16_t pos;
    uint16_t len;
} i2c_chunk_t;

struct
{
    i2c_chunk_t queue[I2C_WORK_QUEUE_SIZE];
    uint8_t next;
    uint8_t end;
} i2c_work;


IRAM_ATTR HAL_StatusTypeDef i2c_put(uint8_t reg_adr, uint8_t* buf, uint16_t len)
{
    if (i2c_work.end < I2C_WORK_QUEUE_SIZE) {  // not work overflow
        i2c_chunk_t* i2c_buf = i2c_work.queue + i2c_work.end;
        if (len <= I2C_WORK_CHUNK_SIZE) { // not buf overflow
            i2c_buf->reg_adr = reg_adr;
            memcpy(i2c_buf->buf, buf, len);
            i2c_buf->len = len;
            i2c_buf->pos = 0;
            i2c_work.end++;
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}


IRAM_ATTR void i2c_spin(uint16_t chunksize)
{
    if (i2c_work.end == i2c_work.next) return; // nothing to do

    while (true) {
        i2c_chunk_t* i2c_buf = i2c_work.queue + i2c_work.next;
        uint16_t writesize = chunksize;

        if (writesize > i2c_buf->len - i2c_buf->pos) writesize = i2c_buf->len - i2c_buf->pos;

        if (writesize) {
            Wire.beginTransmission(i2c_dev_adr);
            Wire.write(i2c_buf->reg_adr);
            Wire.write(&(i2c_buf->buf[i2c_buf->pos]), writesize);
            Wire.endTransmission(true);
        }

        i2c_buf->pos += writesize;
        chunksize -= writesize;

        if (i2c_buf->pos >= i2c_buf->len) { // done with this work item
            i2c_buf->len = 0;
            i2c_buf->pos = 0;
            i2c_work.next++;
            if (i2c_work.next >= i2c_work.end) { // done with all work items
                i2c_work.next = 0;
                i2c_work.end = 0;
                return;
            }
        }
        if (chunksize <= 0) return;
    }
}


IRAM_ATTR HAL_StatusTypeDef i2c_device_ready(void)
{
    if (i2c_work.end != 0) return HAL_BUSY;

    Wire.beginTransmission(i2c_dev_adr);
    uint8_t error = Wire.endTransmission(true);
    return (error == 0) ? HAL_OK : HAL_ERROR;
}


//-------------------------------------------------------
// INIT routines
//-------------------------------------------------------

void i2c_init(void)
{
    Wire.begin(I2C_SDA_IO, I2C_SCL_IO, I2C_CLOCKSPEED);
    Wire.setBufferSize(I2C_BUFFER_SIZE);
    Wire.setTimeout(2);

    i2c_work.next = 0;
    i2c_work.end = 0;
}


#ifdef __cplusplus
}
#endif
#endif // ESPLIB_I2C_H
