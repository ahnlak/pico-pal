/*
 * pal-ssd1306.h - part of Pico PAL.
 *
 * Copyright (c) 2021 Pete Favelle <pico@fsquared.co.uk>
 * This file is released under the MIT License; see LICENSE for details.
 *
 * This class provides a driver for the SSD1306 OLED display; this is a fairly
 * common I2C monochrome display available in various dimensions - most commonly
 * 128x64 and 128x32.
 */

#ifndef   PAL_SSD1306_H
#define   PAL_SSD1306_H

#include <hardware/i2c.h>

namespace pal
{
  typedef enum 
  {
    MEMORYMODE = 0x20,
    COLUMNADDR = 0x21,
    PAGEADDR = 0x22,
    SETSTARTLINE = 0x40,
    SETCONTRAST = 0x81,
    CHARGEPUMP = 0x8D,
    SEGREMAP = 0xA1,
    DISPLAYALLON = 0xA4,
    NORMALDISPLAY = 0xA6,
    INVERTDISPLAY = 0xA7,
    SETMULTIPLEX = 0xA8,
    DISPLAYOFF = 0xAE,
    DISPLAYON = 0xAF,
    COMSCANDEC = 0xC8,
    SETDISPLAYOFFSET = 0xD3,
    SETDISPLAYCLOCKDIV = 0xD5,
    SETPRECHARGE = 0xD9,
    SETCOMPINS = 0xDA,
    SETVCOMDETECT = 0xDB
  } ssd1306_cmd_t;

  class SSD1306
  {
  private:
    uint8_t     width;
    uint8_t     height;
    uint8_t     pagesize;
    uint8_t     address;
    bool        external_vcc;
    i2c_inst_t *i2c_instance;
    uint8_t    *screen_buffer;
    uint8_t    *screen_ptr;
    size_t      screen_buffer_sz;

    bool write_cmd( ssd1306_cmd_t p_cmd, int16_t p_arg1 = -1, int16_t p_arg2 = -1 );
    bool write_buffer( uint8_t *p_buffer, size_t p_length );

  public:
    SSD1306( uint8_t p_width, uint8_t p_height, i2c_inst_t *p_i2c, uint8_t p_address = 0x3C, bool p_ext_vcc = false );
    ~SSD1306();

    void clear( void );
    void render( void );
    void set_contrast( uint8_t p_contrast );
    void set_invert( bool p_invert );

    void set_pixel( uint8_t p_x, uint8_t p_y );
    void clear_pixel( uint8_t p_x, uint8_t p_y );

    void draw_line( uint8_t p_x1, uint8_t p_y1, uint8_t p_x2, uint8_t p_y2, bool p_set = true );
    void draw_box( uint8_t p_x, uint8_t p_y, uint8_t p_width, uint8_t p_height, bool p_filled, bool p_set = true );
    void draw_char( uint8_t p_x, uint8_t p_y, char p_char, bool p_set = true );
    void draw_text( uint8_t p_x, uint8_t p_y, const char *p_text, bool p_set = true );

  };
}

#endif /* PAL_SSD1306_H */

/* End of file pal-ssd1306.h */
 