/*
 * pal-ssd1306.cpp - part of Pico PAL.
 *
 * Copyright (c) 2021 Pete Favelle <pico@fsquared.co.uk>
 * This file is released under the MIT License; see LICENSE for details.
 *
 * This class provides a driver for the SSD1306 OLED display; this is a fairly
 * common I2C monochrome display available in various dimensions - most commonly
 * 128x64 and 128x32.
 */

/* Header files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>

#include "pal-ssd1306.h"


/* Functions. */

/*
 * Constructor; allocates the screen buffer, and initialises the device.
 */

pal::SSD1306::SSD1306( uint8_t p_width, uint8_t p_height, i2c_inst_t *p_i2c, 
                       uint8_t p_address, bool p_ext_vcc )
{
  /* Save our basic parameters. */
  width = p_width;
  height = p_height;
  address = p_address;
  external_vcc = p_ext_vcc;
  i2c_instance = p_i2c;

  /* Work out the size of the send buffer; these needs to be the screen size */
  /* which is a bitfield, so width * height/8).                              */
  pagesize = ( height + 7 ) / 8;
  screen_buffer_sz = ( width * pagesize ) + 1;

  /* Allocate that buffer. */
  screen_buffer = new uint8_t[screen_buffer_sz];
  screen_ptr = screen_buffer+1;

  /* Last thing to do is to send our initialisation commands to the device. */
  /* This command sequence is mostly derived from Adafruits SSD1306 driver. */
  write_cmd( DISPLAYOFF );
  write_cmd( SETDISPLAYCLOCKDIV, 0x80 );
  write_cmd( SETMULTIPLEX, height - 1 );
  write_cmd( SETDISPLAYOFFSET, 0x00 );
  write_cmd( SETSTARTLINE );
  write_cmd( CHARGEPUMP, external_vcc ? 0x10 : 0x14 );
  write_cmd( MEMORYMODE, 0x00 );
  write_cmd( SEGREMAP );
  write_cmd( COMSCANDEC );
  write_cmd( SETCOMPINS, height == 64 ? 0x12 : 0x02 );
  write_cmd( SETCONTRAST, 0xFF );
  write_cmd( SETPRECHARGE, external_vcc ? 0x22 : 0xF1 );
  write_cmd( SETVCOMDETECT, 0x40 );
  write_cmd( DISPLAYALLON );
  write_cmd( NORMALDISPLAY );
  write_cmd( DISPLAYON );

  /* All sorted then. */
  return;
}


/*
 * Destructor; frees up the screen buffer memory, and we're done. 
 */

pal::SSD1306::~SSD1306()
{
  /* Free up our allocated memory. */
  delete screen_buffer;
  screen_buffer = screen_ptr = nullptr;

  /* That's all! */
  return;
}


/*
 * write_cmd; internal function that assembles command sequences and sends
 *            them to the display via write_buffer
 */

bool pal::SSD1306::write_cmd( ssd1306_cmd_t p_cmd, int16_t p_arg1, int16_t p_arg2 )
{
  static uint8_t l_buffer[2];

  /* The command and (possible) argument need to be written seperately, from */
  /* the look of things.                                                     */
  l_buffer[0] = 0x00;
  l_buffer[1] = p_cmd;

  if ( p_arg1 >= 0 )
  {
    /* Flush the command, and assuming that's fine build the arg buffer. */
    if ( !write_buffer( l_buffer, 2 ) )
    {
      return false;
    }
    l_buffer[0] = 0x00;
    l_buffer[1] = p_arg1 & 0xFF;
  }

  if ( p_arg2 >= 0 )
  {
    /* Flush the command, and assuming that's fine build the arg buffer. */
    if ( !write_buffer( l_buffer, 2 ) )
    {
      return false;
    }
    l_buffer[0] = 0x00;
    l_buffer[1] = p_arg2 & 0xFF;
  }

  return write_buffer( l_buffer, 2 );
}


/*
 * write_buffer; internal function to send an arbitrary buffer to the display
 *               via i2c.
 */

bool pal::SSD1306::write_buffer( uint8_t *p_buffer, size_t p_length )
{
  return ( i2c_write_blocking( i2c_instance, address, p_buffer, p_length, false ) > 0 );
  return true;
}


/*
 * clear; turns off all pixels in the screen buffer, giving us a blank slate
 *        on which to draw.
 */

void pal::SSD1306::clear( void )
{
  /* Don't try and do this if we don't have a screen buffer. */
  if ( screen_buffer == nullptr )
  {
    return;
  }

  /* Fairly simple, just blank the buffer. */
  memset( screen_buffer, 0, screen_buffer_sz );
  return;
}


/*
 * render; sends the current screen buffer to the display.
 */

void pal::SSD1306::render( void )
{
  static uint8_t l_cmd_buffer[7];

  /* First, send the draw commands, setting the page and column ranges. */
  l_cmd_buffer[0] = PAGEADDR;
  l_cmd_buffer[1] = 0;
  l_cmd_buffer[2] = ( height / 8 ) - 1;

  l_cmd_buffer[3] = COLUMNADDR;
  l_cmd_buffer[4] = 0;
  l_cmd_buffer[5] = width - 1;

  write_cmd( PAGEADDR, 0, pagesize - 1 );
  write_cmd( COLUMNADDR, 0, width - 1 );

  /* Then, write the screen buffer with a suitable command byte. */
  screen_buffer[0] = 0x40;
  write_buffer( screen_buffer, screen_buffer_sz );
  return;
}


/*
 * set_contrast; the contrast of the display varies from 0 to 255. Note that 
 *               this is a display-wide setting.
 */

void pal::SSD1306::set_contrast( uint8_t p_contrast )
{
  write_cmd( SETCONTRAST, p_contrast );
  return;
}


/*
 * set_invert; sets the display to be normal (false) or inverted (true)
 */

void pal::SSD1306::set_invert( bool p_invert )
{
  /* Simple boolean choice. */
  if ( p_invert )
  {
    write_cmd( INVERTDISPLAY );
  }
  else
  {
    write_cmd( NORMALDISPLAY );
  }

  /* And we're done. */
  return;
}


/*
 * set_pixel; basic drawing primitive; turns on the pixel at the specified
 *            location in the screen buffer.
 */

void pal::SSD1306::set_pixel( uint8_t p_x, uint8_t p_y )
{
  /* Sanity check the co-ordinates. */
  if ( p_x >= width | p_y >= height )
  {
    return;
  }

  /* Good, so just set the bit in the buffer. */
  screen_ptr[(width*(p_y>>3))+p_x] |= 0x01<<(p_y&0x07);
  return;
}


/*
 * clear_pixel; basic drawing primitive; turns off the pixel at the specified
 *              location in the screen buffer.
 */

void pal::SSD1306::clear_pixel( uint8_t p_x, uint8_t p_y )
{
  /* Sanity check the co-ordinates. */
  if ( p_x >= width | p_y >= height )
  {
    return;
  }

  /* Good, so just set the bit in the buffer. */
  screen_ptr[(width*(p_y>>3))+p_x] &= ~(0x01<<(p_y&0x07));
  return;
}


/*
 * draw_line; draws a straight line between two provided points; the line
 *            includes both these points.
 */

void pal::SSD1306::draw_line( uint8_t p_x1, uint8_t p_y1, uint8_t p_x2, uint8_t p_y2, bool p_set )
{
  int16_t l_dx, l_dy, l_sx, l_sy;
  int16_t l_err, l_e2;
  uint8_t l_x, l_y;

  /* Work out the deltas and slopes. */
  l_dx = abs( p_x2 - p_x1 );
  l_dy = abs( p_y2 - p_y1 ) * -1;

  l_sx = ( p_x1 < p_x2 ) ? 1 : -1;
  l_sy = ( p_y1 < p_y2 ) ? 1 : -1;

  /* Set the initial error, and the initial pixel. */
  l_err = l_dx + l_dy;
  l_x = p_x1;
  l_y = p_y1;

  /* Now loop until we hit the final pixel. */
  while( true )
  {
    /* Draw the current pixel. */
    if ( p_set )
    {
      set_pixel( l_x, l_y );
    }
    else
    {
      clear_pixel( l_x, l_y );
    }

    /* End if that was the end of the line. */
    if ( l_x == p_x2 && l_y == p_y2 )
    {
      break;
    }

    /* Work out the next step. */
    l_e2 = l_err * 2;

    if ( l_e2 >= l_dy )
    {
      l_err += l_dy;
      l_x += l_sx;
    }

    if ( l_e2 <= l_dx )
    {
      l_err += l_dx;
      l_y += l_sy;
    }
  }

  /* All done. */
  return;
}


/*
 * draw_box; draws a box to the display - the filled flag indicates if this is
 *           just an outline, or filled in.
 */

void pal::SSD1306::draw_box( uint8_t p_x, uint8_t p_y, uint8_t p_width, uint8_t p_height, bool p_filled, bool p_set )
{
  /* We do things differently depending on whether or not we're filled. */
  if ( p_filled )
  {
    /* Draw a line for each row, to produce a filled box. */
    for( uint8_t l_index = p_y+p_height+1; l_index > p_y; l_index-- )
    {
      draw_line( p_x, l_index-1, p_x + p_width, l_index-1 );
    }
  }
  else
  {
    /* Simply draw four lines then! */
    draw_line( p_x, p_y, p_x + p_width, p_y, p_set );
    draw_line( p_x, p_y + p_height, p_x + p_width, p_y + p_height, p_set );
    draw_line( p_x, p_y, p_x, p_y + p_height, p_set );
    draw_line( p_x + p_width, p_y, p_x + p_width, p_y + p_height, p_set );
  }

  /* All done. */
  return;
}


/*
 * draw_char; draws a single character at the specified location. 
 */

void pal::SSD1306::draw_char( uint8_t p_x, uint8_t p_y, char p_char, bool p_set )
{
  static uint8_t l_font[][5] = {
    { 0b0000000, 0b0000000, 0b0000000, 0b0000000, 0b0000000 }, // space
    { 0b0000000, 0b0000000, 0b1111101, 0b0000000, 0b0000000 }, // !
    { 0b0000000, 0b1110000, 0b0000000, 0b1110000, 0b0000000 }, // "
    { 0b0010100, 0b1111111, 0b0010100, 0b1111111, 0b0010100 }, // #
    { 0b0010010, 0b0101010, 0b1111111, 0b0101010, 0b0100100 }, // $
    { 0b1100010, 0b1100100, 0b0001000, 0b0010011, 0b0100011 }, // %
    { 0b0110110, 0b1001001, 0b1010101, 0b0100010, 0b0000101 }, // &
    { 0b0000000, 0b0000000, 0b1100000, 0b0000000, 0b0000000 }, // ’
    { 0b0000000, 0b0011100, 0b0100010, 0b1000001, 0b0000000 }, // (
    { 0b0000000, 0b1000001, 0b0100010, 0b0011100, 0b0000000 }, // )
    { 0b0010100, 0b0001000, 0b0111110, 0b0001000, 0b0010100 }, // *
    { 0b0001000, 0b0001000, 0b0111110, 0b0001000, 0b0001000 }, // +
    { 0b0000000, 0b0000101, 0b0000110, 0b0000000, 0b0000000 }, // ,
    { 0b0001000, 0b0001000, 0b0001000, 0b0001000, 0b0001000 }, // -
    { 0b0000000, 0b0000011, 0b0000011, 0b0000000, 0b0000000 }, // .
    { 0b0000010, 0b0000100, 0b0001000, 0b0010000, 0b0100000 }, // /
    { 0b0111110, 0b1000101, 0b1001001, 0b1010001, 0b0111110 }, // 0
    { 0b0000000, 0b0100001, 0b1111111, 0b0000001, 0b0000000 }, // 1
    { 0b0100011, 0b1000101, 0b1001001, 0b1001001, 0b0110001 }, // 2
    { 0b0100010, 0b1000001, 0b1001001, 0b1001001, 0b0110110 }, // 3
    { 0b0001100, 0b0010100, 0b0100100, 0b1111111, 0b0000100 }, // 4
    { 0b1110010, 0b1010001, 0b1010001, 0b1010001, 0b1001110 }, // 5
    { 0b0011110, 0b0101001, 0b1001001, 0b1001001, 0b0000110 }, // 6
    { 0b1000000, 0b1000111, 0b1001000, 0b1010000, 0b1100000 }, // 7
    { 0b0110110, 0b1001001, 0b1001001, 0b1001001, 0b0110110 }, // 8
    { 0b0110000, 0b1001001, 0b1001001, 0b1001010, 0b0111100 }, // 9
    { 0b0000000, 0b0110110, 0b0110110, 0b0000000, 0b0000000 }, // :
    { 0b0000000, 0b0110101, 0b0110110, 0b0000000, 0b0000000 }, // ;
    { 0b0001000, 0b0010100, 0b0100010, 0b1000001, 0b0000000 }, // <
    { 0b0010100, 0b0010100, 0b0010100, 0b0010100, 0b0010100 }, // =
    { 0b0000000, 0b1000001, 0b0100010, 0b0010100, 0b0001000 }, // >
    { 0b0100000, 0b1000000, 0b1000101, 0b1001000, 0b0110000 }, // ?
    { 0b0100110, 0b1001001, 0b1001111, 0b1000001, 0b0111110 }, // @
    { 0b0011111, 0b0100100, 0b1000100, 0b0100100, 0b0011111 }, // A
    { 0b1000001, 0b1111111, 0b1001001, 0b1001001, 0b0110110 }, // B
    { 0b0111110, 0b1000001, 0b1000001, 0b1000001, 0b0100010 }, // C
    { 0b1000001, 0b1111111, 0b1000001, 0b1000001, 0b0111110 }, // D
    { 0b1111111, 0b1001001, 0b1001001, 0b1001001, 0b1000001 }, // E
    { 0b1111111, 0b1001000, 0b1001000, 0b1001000, 0b1000000 }, // F
    { 0b0111110, 0b1000001, 0b1000001, 0b1001001, 0b0101111 }, // G
    { 0b1111111, 0b0001000, 0b0001000, 0b0001000, 0b1111111 }, // H
    { 0b0000000, 0b1000001, 0b1111111, 0b1000001, 0b0000000 }, // I
    { 0b0000010, 0b0000001, 0b1000001, 0b1111110, 0b1000000 }, // J
    { 0b1111111, 0b0001000, 0b0010100, 0b0100010, 0b1000001 }, // K
    { 0b1111111, 0b0000001, 0b0000001, 0b0000001, 0b0000001 }, // L
    { 0b1111111, 0b0100000, 0b0011000, 0b0100000, 0b1111111 }, // M
    { 0b1111111, 0b0010000, 0b0001000, 0b0000100, 0b1111111 }, // N
    { 0b0111110, 0b1000001, 0b1000001, 0b1000001, 0b0111110 }, // O
    { 0b1111111, 0b1001000, 0b1001000, 0b1001000, 0b0110000 }, // P
    { 0b0111110, 0b1000001, 0b1000101, 0b1000010, 0b0111101 }, // Q
    { 0b1111111, 0b1001000, 0b1001100, 0b1001010, 0b0110001 }, // R
    { 0b0110010, 0b1001001, 0b1001001, 0b1001001, 0b0100110 }, // S
    { 0b1000000, 0b1000000, 0b1111111, 0b1000000, 0b1000000 }, // T
    { 0b1111110, 0b0000001, 0b0000001, 0b0000001, 0b1111110 }, // U
    { 0b1111100, 0b0000010, 0b0000001, 0b0000010, 0b1111100 }, // V
    { 0b1111110, 0b0000001, 0b0001110, 0b0000001, 0b1111110 }, // W
    { 0b1100011, 0b0010100, 0b0001000, 0b0010100, 0b1100011 }, // X
    { 0b1110000, 0b0001000, 0b0000111, 0b0001000, 0b1110000 }, // Y
    { 0b1000011, 0b1000101, 0b1001001, 0b1010001, 0b1100001 }, // Z
    { 0b0000000, 0b1111111, 0b1000001, 0b1000001, 0b0000000 }, // [
    { 0b0100000, 0b0010000, 0b0001000, 0b0000100, 0b0000010 }, // backslash
    { 0b0000000, 0b1000001, 0b1000001, 0b1111111, 0b0000000 }, // ]
    { 0b0010000, 0b0100000, 0b1000000, 0b0100000, 0b0010000 }, // ^
    { 0b0000001, 0b0000001, 0b0000001, 0b0000001, 0b0000001 }, // _
    { 0b0000000, 0b1000000, 0b0100000, 0b0010000, 0b0000000 }, // `
    { 0b0000010, 0b0010101, 0b0010101, 0b0010101, 0b0001111 }, // a
    { 0b1111111, 0b0001001, 0b0010001, 0b0010001, 0b0001110 }, // b
    { 0b0001110, 0b0010001, 0b0010001, 0b0010001, 0b0000010 }, // c
    { 0b0001110, 0b0010001, 0b0010001, 0b0001001, 0b1111111 }, // d
    { 0b0001110, 0b0010101, 0b0010101, 0b0010101, 0b0001100 }, // e
    { 0b0001000, 0b0111111, 0b1001000, 0b1000000, 0b0100000 }, // f
    { 0b0001000, 0b0010101, 0b0010101, 0b0010101, 0b0011110 }, // g
    { 0b1111111, 0b0001000, 0b0010000, 0b0010000, 0b0001111 }, // h
    { 0b0000000, 0b0001001, 0b1011111, 0b0000001, 0b0000000 }, // i
    { 0b0000010, 0b0000001, 0b0010001, 0b1011110, 0b0000000 }, // j
    { 0b1111111, 0b0000100, 0b0001010, 0b0010001, 0b0000000 }, // k
    { 0b0000000, 0b1000001, 0b1111111, 0b0000001, 0b0000000 }, // l
    { 0b0011111, 0b0010000, 0b0001111, 0b0010000, 0b0001111 }, // m
    { 0b0011111, 0b0001000, 0b0010000, 0b0010000, 0b0001111 }, // n
    { 0b0001110, 0b0010001, 0b0010001, 0b0010001, 0b0001110 }, // o
    { 0b0011111, 0b0010100, 0b0010100, 0b0010100, 0b0001000 }, // p
    { 0b0001000, 0b0010100, 0b0010100, 0b0001100, 0b0011111 }, // q
    { 0b0011111, 0b0001000, 0b0010000, 0b0010000, 0b0001000 }, // r
    { 0b0001001, 0b0010101, 0b0010101, 0b0010101, 0b0000010 }, // s
    { 0b0010000, 0b1111110, 0b0010001, 0b0000001, 0b0000010 }, // t
    { 0b0011110, 0b0000001, 0b0000001, 0b0000010, 0b0011111 }, // u
    { 0b0011100, 0b0000010, 0b0000001, 0b0000010, 0b0011100 }, // v
    { 0b0011110, 0b0000001, 0b0000110, 0b0000001, 0b0011110 }, // w
    { 0b0010001, 0b0001010, 0b0000100, 0b0001010, 0b0010001 }, // x
    { 0b0011000, 0b0000101, 0b0000101, 0b0000101, 0b0011110 }, // y
    { 0b0010001, 0b0010011, 0b0010101, 0b0011001, 0b0010001 }, // z
    { 0b0000000, 0b0001000, 0b0110110, 0b1000001, 0b0000000 }, // {
    { 0b0000000, 0b0000000, 0b1111111, 0b0000000, 0b0000000 }, // |
    { 0b0000000, 0b1000001, 0b0110110, 0b0001000, 0b0000000 }, // }
    { 0b0000100, 0b0001000, 0b0001000, 0b0000100, 0b0001000 }, // ~
    { 0b0000110, 0b0001001, 0b1010001, 0b0000001, 0b0000010 }  // undef
  };

  uint8_t l_char = ( p_char < 0x20 || p_char > 0x7E ) ? 0x7F : p_char - 0x20;

  /* So, each character is a simple 5x7 font grid. */
  for( uint8_t l_x = 0; l_x < 5; l_x++ )
  {
    for( uint8_t l_y = 0; l_y < 7; l_y++ )
    {
      /* If the bit is set, draw the pixel. */
      if ( ( 0x01 << ( 6 - l_y ) ) & l_font[l_char][l_x] )
      {
        if ( p_set )
        {
          set_pixel( p_x + l_x, p_y + l_y );
        }
        else
        {
          clear_pixel( p_x + l_x, p_y + l_y );          
        }
      }
    }
  }

  /* All done. */
  return;
}


/*
 * draw_text; draws a text string at the specified location. Note that text
 *            does not wrap!
 */

void pal::SSD1306::draw_text( uint8_t p_x, uint8_t p_y, const char *p_text, bool p_set )
{
  size_t l_strlen;

  /* Work through the string, one character at a time. */
  l_strlen = strlen( p_text );
  for ( size_t l_index = 0; l_index < l_strlen; l_index++ )
  {
    draw_char( p_x + (l_index * 6), p_y, p_text[l_index], p_set );
  }

  /* All done. */
  return;
}


 /* End of file pal-ssd1306.cpp */
