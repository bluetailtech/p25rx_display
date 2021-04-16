

//MIT License
//
//Copyright (c) 2020 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "stm32h7xx_hal.h"
#include "lcd.h"
#include "fonts.h"
#include <stdlib.h>
#include "main.h" 

const uint8_t display_init[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1, 1, 0x23,
  ILI9341_PWCTR2, 1, 0x10,
  ILI9341_VMCTR1, 2, 0x3e, 0x28,
  ILI9341_VMCTR2, 1, 0x86,
  ILI9341_MADCTL, 1, 0x48,
  ILI9341_VSCRSADD, 1, 0x00,
  ILI9341_PIXFMT, 1, 0x55,
  ILI9341_FRMCTR1, 2, 0x00, 0x18,
  ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27,
  0xF2, 1, 0x00,
  ILI9341_GAMMASET, 1, 0x01,
  ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
  0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
  0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT, 0x80,
  ILI9341_DISPON, 0x80,
  0x00
};

uint16_t lcd_width;
uint16_t lcd_height;
uint16_t lcd_rotation;
int delay_val = 5;

uint8_t pen_down;

const int pen_w = 4;
uint8_t draw_txt[128];
uint16_t line_data[240];

uint16_t prev_x;
uint16_t prev_y;
uint32_t npoints;

uint16_t scroll_val = 130 + 60;
static int did_init;

int lcd_is_usb_connected;
int volatile update_usb_connect;
volatile int usb_rx_count;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void lcd_init()
{

  uint8_t cmd, x, numArgs;
  const uint8_t *ptr = display_init;

  delay_ms_ni( 5 );
  HAL_GPIO_WritePin( LCD_RESET_PORT, LCD_RESET, GPIO_PIN_RESET ); //lcd reset
  delay_ms_ni( 50 );
  HAL_GPIO_WritePin( LCD_RESET_PORT, LCD_RESET, GPIO_PIN_SET ); //lcd reset
  delay_ms_ni( 50 );

  while( 1 ) {
    cmd = *ptr++;  //command
    if( cmd == 0 ) break;

    //if(cmd==0xff) cmd=0; //NOP

    x = *ptr++;    //n args

    numArgs = ( x & 0x7f );
    lcd_send_command( cmd, ptr, numArgs );

    ptr += numArgs;
    if( x & 0x80 ) {
      delay_ms_ni( 150 );
    }
  }

  lcd_width = ILI9341_TFTWIDTH;
  lcd_height = ILI9341_TFTHEIGHT;
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high


  lcd_set_draw_port( 0, 0, lcd_width, lcd_height );
  lcd_fill( ILI9341_BLACK );
  lcd_set_rotation( 2 );

  lcd_set_scroll_def( 130, 60, 130 ); //should add up to 320
  scroll_val = 130;


  //touch display related
  HAL_GPIO_WritePin( LCD_T_CLK_PORT, LCD_T_CLK, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LCD_T_CS_PORT, LCD_T_CS, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LCD_LED_PORT, LCD_LED, GPIO_PIN_SET );//lcd led

  lcd_page1_init();
  lcd_page2_init();
  lcd_page3_init();
  lcd_page4_init();
  lcd_page5_init();

}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void lcd_set_rotation( uint8_t rot )
{

  lcd_rotation = ( rot & 0x03 );

  uint8_t m;

  switch( ( int ) lcd_rotation ) {
  case 0:
    m = ( MADCTL_MX | MADCTL_BGR );
    lcd_width = ILI9341_TFTWIDTH;
    lcd_height = ILI9341_TFTHEIGHT;
    break;
  case 1:
    m = ( MADCTL_MV | MADCTL_BGR );
    lcd_width = ILI9341_TFTHEIGHT;
    lcd_height = ILI9341_TFTWIDTH;
    break;
  case 2:
    m = ( MADCTL_MY | MADCTL_BGR );
    lcd_width = ILI9341_TFTWIDTH;
    lcd_height = ILI9341_TFTHEIGHT;
    break;
  case 3:
    m = ( MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR );
    lcd_width = ILI9341_TFTHEIGHT;
    lcd_height = ILI9341_TFTWIDTH;
    break;
  }

  lcd_send_command( ILI9341_MADCTL, &m, 1 );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void lcd_scroll_to( uint16_t y )
{

  uint8_t data[2];
  data[0] = ( uint8_t )( y >> 8 );
  data[1] = ( uint8_t )( y & 0xff );
  lcd_send_command( ILI9341_VSCRSADD, ( uint8_t * )data, 2 );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void lcd_set_scroll_def( uint16_t top, uint16_t sa, uint16_t bottom )
{

  //if( top + bottom <= ILI9341_TFTHEIGHT ) {
  uint8_t data[6];
  data[0] = ( uint8_t )( top >> 8 );
  data[1] = ( uint8_t )( top & 0xff );
  data[2] = ( uint8_t )( sa >> 8 );
  data[3] = ( uint8_t )( sa & 0xff );
  data[4] = ( uint8_t )( bottom >> 8 );
  data[5] = ( uint8_t )( bottom & 0xff );
  lcd_send_command( ILI9341_VSCRDEF, ( uint8_t * )data, 6 );
  //}
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void lcd_set_draw_port( uint16_t x1, uint16_t y1, uint16_t w, uint16_t h )
{

  lcd_write_command( ILI9341_CASET ); // Column address set
  lcd_spi_write8( x1 >> 8 );
  lcd_spi_write8( x1 & 0xff );
  w = x1 + w - 1;
  lcd_spi_write8( w >> 8 );
  lcd_spi_write8( w & 0xff );

  lcd_cs_high();
  lcd_write_command( ILI9341_PASET ); // Row address set
  lcd_spi_write8( y1 >> 8 );
  lcd_spi_write8( y1 & 0xff );
  h = y1 + h - 1;
  lcd_spi_write8( h >> 8 );
  lcd_spi_write8( h & 0xff );

  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
  //lcd_write_command(ILI9341_RAMWR); // Write to RAM
  //lcd_cs_high();
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_fill_rand_rect()
{

  uint16_t yy = rand() % lcd_height;
  uint16_t xx = rand() % lcd_width;
  uint16_t ww = ( rand() % ( lcd_height - yy ) );
  uint16_t hh = ( rand() % ( lcd_width - xx ) );

  lcd_set_draw_port( xx, yy, ww, hh );

  lcd_write_command( ILI9341_RAMWR );
  int i;
  uint8_t col = rand() % 255;
  for( i = 0; i < ( ww * hh * 2 ); i++ ) {

    if( col % 16 == 0 ) {
      lcd_spi_write8( rand() % 255 );
    } else {
      lcd_spi_write8( col );
    }
  }
  lcd_cs_high();

}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void lcd_draw_rect( int x, int y, int w, int h, uint16_t color )
{
  lcd_drawline( color, x, y, x + w, y );
  lcd_drawline( color, x + w, y, x + w, y + h );
  lcd_drawline( color, x + w, y + h, x, y + h );
  lcd_drawline( color, x, y + h, x, y );
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void lcd_draw_rect_hl( int x, int y, int w, int h )
{
  lcd_drawline( ILI9341_WHITE, x, y, x + w, y );
  lcd_drawline( ILI9341_LIGHTGREY, x + w, y, x + w, y + h );
  lcd_drawline( ILI9341_LIGHTGREY, x + w, y + h, x, y + h );
  lcd_drawline( ILI9341_WHITE, x, y + h, x, y );
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void lcd_fill_rect( int x, int y, int w, int h, uint16_t color )
{
  y = 240 - y - h;
  _lcd_fill( x, y, w, h, color );
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_fill( uint16_t color )
{
  _lcd_fill( 0, 0, lcd_width, lcd_height, color );
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void _lcd_fill( int x, int y, int w, int h, uint16_t color )
{

  uint16_t xx = y;
  uint16_t yy = x;
  uint16_t ww = h;
  uint16_t hh = w;

  lcd_set_draw_port( xx, yy, ww, hh );

  lcd_write_command( ILI9341_RAMWR );
  int i;

  for( i = 0; i < ( ww * hh ); i++ ) {
    lcd_spi_write8( ( uint8_t )( color >> 8 ) );
    lcd_spi_write8( ( uint8_t )( color & 0xff ) );
  }
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high

}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_fillimg( uint8_t *imgptr )
{

  uint16_t xx = 0;
  uint16_t yy = 0;
  uint16_t ww = 240;
  uint16_t hh = 320;

  lcd_set_draw_port( xx, yy, ww, hh );

  lcd_write_command( ILI9341_RAMWR );
  int i;
  int off = 0;

  uint16_t *iptr = ( uint16_t * ) imgptr;

  for( i = 0; i < ( ww * hh ); i++ ) {
    lcd_spi_write8( ( uint16_t )( iptr[off] >> 8 ) );
    lcd_spi_write8( ( uint16_t )( iptr[off] & 0xff ) );
    off++;
  }
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high

}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_draw_pixel( int16_t x, int16_t y, uint16_t color )
{

  y = 240 - y;

  lcd_set_draw_port( y, x, 1, 1 );

  lcd_write_command( ILI9341_RAMWR );

  lcd_spi_write8( ( uint8_t )( color >> 8 ) & 0xff );
  lcd_spi_write8( ( uint8_t )( color ) & 0xff );

  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
  //DelayClk3(500);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void lcd_drawline( uint16_t color, int x0, int y0, int x1, int y1 )
{

  int dy = y1 - y0;
  int dx = x1 - x0;
  int stepx, stepy;
  int fraction;

  if( dy < 0 ) {
    dy = -dy;
    stepy = -1;
  } else {
    stepy = 1;
  }
  if( dx < 0 ) {
    dx = -dx;
    stepx = -1;
  } else {
    stepx = 1;
  }
  dy <<= 1;                           // dy is now 2*dy
  dx <<= 1;                           // dx is now 2*dx
  lcd_draw_pixel( x0, y0, color );
  if( dx > dy ) {
    fraction = dy - ( dx >> 1 ); // same as 2*dy - dx
    while( x0 != x1 ) {
      if( fraction >= 0 ) {
        y0 += stepy;
        fraction -= dx;         // same as fraction -= 2*dx
      }
      x0 += stepx;
      fraction += dy;             // same as fraction -= 2*dy
      lcd_draw_pixel( x0, y0, color );
    }
  } else {
    fraction = dx - ( dy >> 1 );
    while( y0 != y1 ) {
      if( fraction >= 0 ) {
        x0 += stepx;
        fraction -= dy;
      }
      y0 += stepy;
      fraction += dx;
      lcd_draw_pixel( x0, y0, color );
    }
  }
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_draw_str_v( char *str, int font_size, int y, int x, uint16_t fcolor, uint16_t bcolor, int fillbg )
{

  unsigned int i, j, offset;
  unsigned char mask = 0, z = 0, xme = 0, yme = 0, mult = 0;
  const uint8_t *font = 0;

  y = 240 - y;

  if( font_size == _FONT6x8 ) {
    font = FONT6x8;
    xme = 6;
    yme = 8;
    mult = 8;
  } else if( font_size == _FONT8x8 ) {
    font = FONT8x8;
    xme = 8;
    yme = 8;
    mult = 8;
  } else if( font_size == _FONT8x16 ) {
    font = FONT8x16;
    xme = 8;
    yme = 16;
    mult = 16;
  }


  do {

    offset = ( mult * ( int )( *str - 32 ) );

    for( i = 0; i < yme; i++ ) {

      mask |= 0x80;
      for( j = x; j < ( x + xme ); j++ ) {
        z = 240 - ( y + i );
        if( font[offset] & mask ) {
          lcd_draw_pixel( z, j, fcolor );
        } else {
          if( fillbg ) lcd_draw_pixel( z, j, bcolor );
        }
        mask >>= 1;
      }
      offset++;
    }

    x += xme;

    str++;

  } while( *str != '\0' );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_draw_str_h( char *str, int font_size, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg )
{

  unsigned int i, j, offset;
  unsigned char mask = 0, z = 0, xme = 0, yme = 0, mult = 0;
  const uint8_t *font = 0;

  y = 240 - y;

  if( font_size == _FONT6x8 ) {
    font = FONT6x8;
    xme = 6;
    yme = 8;
    mult = 8;
  } else if( font_size == _FONT8x8 ) {
    font = FONT8x8;
    xme = 8;
    yme = 8;
    mult = 8;
  } else if( font_size == _FONT8x16 ) {
    font = FONT8x16;
    xme = 8;
    yme = 16;
    mult = 16;
  }


  do {

    offset = ( mult * ( int )( *str - 32 ) );
    offset += yme - 1;

    for( i = 0; i < yme; i++ ) {

      mask |= 0x80;
      for( j = x; j < ( x + xme ); j++ ) {
        z = 240 - ( y + i );
        if( font[offset] & mask ) {
          lcd_draw_pixel( j, z, fcolor );
        } else {
          if( fillbg ) lcd_draw_pixel( j, z, bcolor );
        }
        mask >>= 1;
      }
      offset--;
    }

    x += xme;

    str++;

  } while( *str != '\0' );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_draw_slider( char *str, int scale_8x16, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg, int old_xpos )
{

  int x1 = x;
  int y1 = y;


  int slen = strlen( str );
  int scale = 1;
  if( scale_8x16 == SCALE_1X ) scale = 1;
  if( scale_8x16 == SCALE_2X ) scale = 2;
  if( scale_8x16 == SCALE_3X ) scale = 3;
  if( scale_8x16 == SCALE_4X ) scale = 4;
  if( scale_8x16 == SCALE_5X ) scale = 5;

  int w = ( slen * 8 * scale ) + 3 + 5;
  int h = ( 16 * scale );


  y -= ( 16 * scale );
  x -= 1;
  w += -2;


  lcd_drawline( ILI9341_GREEN, 2, y+h/2, 318, y+h/2 );
  lcd_fill_rect( old_xpos-10, y, w+25, h, ILI9341_BLACK );
  lcd_fill_rect( x, y, w, h, bcolor );
  //lcd_draw_str_scaled( str, scale_8x16, x1, y1, fcolor, bcolor, fillbg );
  //lcd_fill_rect( x + w - 6, y, 7, h, bcolor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_draw_button( char *str, int scale_8x16, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg )
{

  lcd_draw_str_scaled( str, scale_8x16, x, y, fcolor, bcolor, fillbg );

  int slen = strlen( str );
  int scale = 1;
  if( scale_8x16 == SCALE_1X ) scale = 1;
  if( scale_8x16 == SCALE_2X ) scale = 2;
  if( scale_8x16 == SCALE_3X ) scale = 3;
  if( scale_8x16 == SCALE_4X ) scale = 4;
  if( scale_8x16 == SCALE_5X ) scale = 5;

  int w = ( slen * 8 * scale ) + 3 + 5;
  int h = ( 16 * scale );

  y -= ( 16 * scale );
  x -= 1;
  w += -2;

  lcd_fill_rect( x + w - 6, y, 7, h, bcolor );
  //lcd_draw_rect_hl( x,y, w, h);  //font 16x32
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  draw scaled version of the 8x16 bitmap font
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_draw_str_scaled( char *str, int scale_8x16, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg )
{

  unsigned int i, j, offset;
  unsigned char mask = 0, z = 0, xme = 0, yme = 0, mult = 0;
  const uint8_t *font = 0;

  y = 240 - y;

  font = FONT8x16;
  xme = 8;
  yme = 16;
  mult = 16;

  int scale = 2;

  switch( scale_8x16 ) {
  case SCALE_1X  :
    scale = 1;
    break;
  case SCALE_2X  :
    scale = 2;
    break;
  case SCALE_3X  :
    scale = 3;
    break;
  case SCALE_4X  :
    scale = 4;
    break;
  case SCALE_5X  :
    scale = 5;
    break;

  default  :
    scale = 2;
    break;
  }


  do {

    offset = ( mult * ( int )( *str - 32 ) );
    offset += yme - 1;
    int jj;
    int vmod = 0;
    int yy = 0;
    int k = 0;

    for( i = 0; i < yme * scale; i++ ) {

      mask |= 0x80;
      jj = x;
      for( j = x; j < ( x + xme ); j++ ) {
        z = 240 - ( y + i + yy );
        if( font[offset] & mask ) {
          for( k = 0; k < scale; k++ ) {
            lcd_draw_pixel( jj++, z, fcolor );
          }
        } else {
          if( fillbg ) {
            for( k = 0; k < scale; k++ ) {
              lcd_draw_pixel( jj++, z, bcolor );
            }
          } else {
            jj += scale;
          }
        }
        mask >>= 1;
      }
      if( ++vmod % scale == 0 ) {
        offset--;
      }
    }

    x += xme * scale;

    str++;

  } while( *str != '\0' );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void lcd_drawcircle( uint16_t color, int x0, int y0, int radius, int fill )
{
  int i;

  if( fill ) {
    for( i = 0; i < radius; i++ ) {
      __lcd_drawcircle( color, x0, y0, i );
    }
  } else {
    __lcd_drawcircle( color, x0, y0, radius );
  }
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void __lcd_drawcircle( uint16_t color, int x0, int y0, int radius )
{

  int f = 1 - radius;
  int ddF_x = 0;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;
  lcd_draw_pixel( x0, y0 + radius, color );
  lcd_draw_pixel( x0, y0 - radius, color );
  lcd_draw_pixel( x0 + radius, y0, color );
  lcd_draw_pixel( x0 - radius, y0, color );
  while( x < y ) {
    if( f >= 0 ) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;
    lcd_draw_pixel( x0 + x, y0 + y, color );
    lcd_draw_pixel( x0 - x, y0 + y, color );
    lcd_draw_pixel( x0 + x, y0 - y, color );
    lcd_draw_pixel( x0 - x, y0 - y, color );
    lcd_draw_pixel( x0 + y, y0 + x, color );
    lcd_draw_pixel( x0 - y, y0 + x, color );
    lcd_draw_pixel( x0 + y, y0 - x, color );
    lcd_draw_pixel( x0 - y, y0 - x, color );
  }
}
///////////////////////////////////////////////////////////////////////////////
// lcd_data_len is length of data only.  (does not include command)
// high-speed bit-bang
///////////////////////////////////////////////////////////////////////////////
void lcd_send_command( uint8_t lcd_cmd, uint8_t *lcd_data, int lcd_data_len )
{

  int i;


  HAL_GPIO_WritePin( LCD_DC_PORT, LCD_DC, GPIO_PIN_RESET );//lcd DC for command
  //DelayClk3(500);
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_RESET );//lcd CS
  //DelayClk3(500);

  for( i = 0; i < 8; i++ ) {
    //bit 7 of cmd
    if( ( lcd_cmd & 0x80 ) ) {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
    } else {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
    }
    //delay_us( delay_val );

    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
    //delay_us( delay_val );
    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
    //delay_us( delay_val );

    lcd_cmd <<= 1;
  }


  HAL_GPIO_WritePin( LCD_DC_PORT, LCD_DC, GPIO_PIN_SET );//lcd DC for data
  //delay_us( delay_val );

  int j;
  uint8_t data;
  for( i = 0; i < lcd_data_len; i++ ) {

    data = lcd_data[i];
    for( j = 0; j < 8; j++ ) {
      //bit 7 of cmd
      if( ( data & 0x80 ) ) {
        HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
      } else {
        HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
      }
      //delay_us( delay_val );

      HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
      //delay_us( delay_val );
      HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
      //delay_us( delay_val );

      data <<= 1;
    }

  }

  //delay_us( delay_val );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
  //DelayClk3(500);

}


///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_write_command( uint8_t lcd_cmd )
{
  int i;

  HAL_GPIO_WritePin( LCD_DC_PORT, LCD_DC, GPIO_PIN_RESET );//lcd DC for command
  //DelayClk3(500);
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_RESET );//lcd CS
  //DelayClk3(500);

  for( i = 0; i < 8; i++ ) {
    //bit 7 of cmd
    if( ( lcd_cmd & 0x80 ) ) {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
    } else {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
    }
    //delay_us( delay_val );

    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
    //delay_us( delay_val );
    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
    //delay_us( delay_val );

    lcd_cmd <<= 1;
  }

}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_spi_write16( uint16_t val )
{

  uint8_t data1 = ( uint8_t )( val >> 8 ) & 0xff;
  uint8_t data2 = ( uint8_t )( val & 0xff );
  int i;

  HAL_GPIO_WritePin( LCD_DC_PORT, LCD_DC, GPIO_PIN_SET );//lcd DC for data
  //delay_us( delay_val );

  for( i = 0; i < 8; i++ ) {
    //bit 7 of cmd
    if( ( data1 & 0x80 ) ) {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
    } else {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
    }
    //delay_us( delay_val );

    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
    //delay_us( delay_val );
    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
    //delay_us( delay_val );

    data1 <<= 1;
  }


  for( i = 0; i < 8; i++ ) {
    //bit 7 of cmd
    if( ( data2 & 0x80 ) ) {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
    } else {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
    }
    //delay_us( delay_val );

    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
    //delay_us( delay_val );
    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
    //delay_us( delay_val );

    data2 <<= 1;
  }

}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_spi_write8( uint16_t val )
{

  int i;

  HAL_GPIO_WritePin( LCD_DC_PORT, LCD_DC, GPIO_PIN_SET );//lcd DC for data
  //delay_us( delay_val );


  for( i = 0; i < 8; i++ ) {
    //bit 7 of cmd
    if( ( val & 0x80 ) ) {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_SET );//lcd MOSI
    } else {
      HAL_GPIO_WritePin( LCD_MOSI_PORT, LCD_MOSI, GPIO_PIN_RESET );//lcd MOSI
    }
    //delay_us( delay_val );

    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_SET );//lcd SCK
    //delay_us( delay_val );
    HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
    //delay_us( delay_val );

    val <<= 1;
  }

}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
void lcd_cs_high()
{
  //delay_us( delay_val );
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd CS high
  //DelayClk3(500);
}



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
uint16_t get_touch_pos( uint8_t cmd )
{
  int i;
  uint16_t reg;
  int ts_delay = 1;

  HAL_GPIO_WritePin( LCD_T_CS_PORT, LCD_T_CS, GPIO_PIN_RESET );


  reg = 0;
  for( i = 0; i < 8; i++ ) {
    if( ( cmd & 0x80 ) == 0x80 ) {
      HAL_GPIO_WritePin( LCD_T_DIN_PORT, LCD_T_DIN, GPIO_PIN_SET );
    } else {
      HAL_GPIO_WritePin( LCD_T_DIN_PORT, LCD_T_DIN, GPIO_PIN_RESET );
    }
    cmd <<= 1;

    delay_us( ts_delay );
    HAL_GPIO_WritePin( LCD_T_CLK_PORT, LCD_T_CLK, GPIO_PIN_SET );
    delay_us( ts_delay );
    HAL_GPIO_WritePin( LCD_T_CLK_PORT, LCD_T_CLK, GPIO_PIN_RESET );
    delay_us( ts_delay );

    reg <<= 1;
    if( HAL_GPIO_ReadPin( LCD_T_DOUT_PORT, LCD_T_DOUT ) ) reg |= 0x01;
    delay_us( ts_delay );
  }

  delay_us( ts_delay );

  reg = 0;
  for( i = 0; i < 16; i++ ) {
    delay_us( ts_delay );
    HAL_GPIO_WritePin( LCD_T_CLK_PORT, LCD_T_CLK, GPIO_PIN_SET );
    delay_us( ts_delay );
    HAL_GPIO_WritePin( LCD_T_CLK_PORT, LCD_T_CLK, GPIO_PIN_RESET );
    delay_us( ts_delay );

    reg <<= 1;
    if( HAL_GPIO_ReadPin( LCD_T_DOUT_PORT, LCD_T_DOUT ) ) reg |= 0x01;
  }

  HAL_GPIO_WritePin( LCD_T_CS_PORT, LCD_T_CS, GPIO_PIN_SET );//lcd T_CS high

  reg >>= 4;
  return reg;
}
////////////////////////////////////////////////////////////////////
// XPT2046 Touch Scree Controller
// send 8-bit cmd   (0xd0==x, 0x90==y)
// ready 16-bit return val
// x,y vals are 16-bit return >> 4,  12-bit (need to transfer
// all 16 bits to start next conversion)
////////////////////////////////////////////////////////////////////
int check_touch_screen_irqn()
{
#if 1
  //scroll test
//  if(x>300 && y>200) {
  //lcd_scroll_to( scroll_val+=1 );
  //if(scroll_val>=130+60) scroll_val=130;
// }

  if( HAL_GPIO_ReadPin( LCD_T_IRQ_PORT, LCD_T_IRQ ) == 0 ) {
    pen_down <<= 1;
    pen_down |= 0x01;
  } else {
    pen_down <<= 1;
  }

  uint8_t pen_mask = 0x01;


  if( ( pen_down & pen_mask ) == 0x00 ) {
    if( do_clear ) {
      do_clear = 0;
      lcd_fill( ILI9341_BLACK );
      lcd_set_rotation( 2 );

      npoints = 0;
      pen_down = 0;

      scroll_val = 130;
      lcd_scroll_to( scroll_val );
      redraw_buttons();
    } else {
      check_button_touch_evt( prev_x, prev_y, ( pen_down & pen_mask ) );
    }
    return 0; //active low
  }


  //do_clear = 1;
  //do_refresh=1;

  uint16_t x = 0;
  uint16_t y = 0;
  int i;
  uint16_t tsval;

  for( i = 0; i < 16; i++ ) {
    tsval = get_touch_pos( T_CMD_READX );
    if( tsval & 0x8000 || tsval > 5000 ) goto out_range;
    y += tsval;
  }

  for( i = 0; i < 16; i++ ) {
    tsval = get_touch_pos( T_CMD_READY );
    if( tsval & 0x8000 || tsval > 5000 ) goto out_range;
    x += tsval;
  }

  y >>= 4; //16x avg
  x >>= 4; //16x avg

  if( x > 65000 ) x = 0;  //returns all ones on edges of screen
  if( y > 65000 ) y = 0;  //returns all ones on edges of screen

#if 0
  printf( "\r\nx, %d, y, %d", ( int )x, ( int )y );
#endif

  float xx = ( ( float )x * 0.090771f ) + -38.380996f;
  float yy = ( ( float )y * -6.6203e-02f ) + 2.5681e+02f;

  x = ( int ) xx;
  y = ( int ) yy;




  if( x > ( lcd_width - 1 ) - pen_w ) {
    x = ( lcd_width - 1 ) - pen_w;
    goto out_range;
  }
  if( x < 0 + pen_w ) {
    x = 0 + pen_w;
    goto out_range;
  }

  if( y > ( lcd_height - 1 ) - pen_w ) {
    y = ( lcd_height - 1 ) - pen_w;
    goto out_range;
  }
  if( y < 0 + pen_w ) {
    y = 0 + pen_w;
    goto out_range;
  }

  int button_evt = check_button_touch_evt( x, y, ( pen_down & pen_mask ) );
  if( button_evt ) {
    do_clear = 0;
    do_refresh = 0;
    goto lcd_done;
  }

  /*
  #if 0
    lcd_drawcircle( ILI9341_GREEN, x, y, pen_w, 1 );
  #else
  int ww=0;
  #define LINE_W 3
  if(npoints>1) { //line width of 3
    for(i=0;i<LINE_W;i++) {
      lcd_drawline( ILI9341_GREEN, x, y-ww, prev_x, prev_y-ww );
      ww++;
    }
  }
  #endif

  #if 1
  sprintf( draw_txt, "x %d, y %d    \0", x,y);
  lcd_draw_str_h( draw_txt, _FONT8x16, 216, 25, ILI9341_ORANGE, ILI9341_BLACK, 1 );
  printf( "\r\nxh, %d, yh, %d", ( int )x, ( int )y );
  #endif
  */

lcd_done:
#endif
  prev_x = x;
  prev_y = y;
  npoints++;


out_range:
  return 1;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void lcd_refresh( void )
{
  do_refresh = 1;
  lcd_set_rotation( 2 );
  //npoints=0;
  //pen_down=0;

  scroll_val = 130;
  lcd_scroll_to( scroll_val );
  lcd_fill( ILI9341_BLACK );
  lcd_test_tick();
  redraw_buttons();

  uint32_t prim;
  prim = __get_PRIMASK();
  __disable_irq();
  update_usb_connect = 1;
  if( !prim ) {
    __enable_irq();
  }
}

/////////////////////////////////////////////
/////////////////////////////////////////////
void lcd_test_tick()
{
  int i;

  if( !did_init ) {
    did_init = 1;
    lcd_button_init();
  }

  if( check_touch_screen_irqn() ) {
    return;
  }

  switch( current_page ) {
  case  1 :
    draw_lcd_page1();
    break;
  case  2 :
    draw_lcd_page2();
    break;
  case  3 :
    draw_lcd_page3();
    break;
  case  4 :
    draw_lcd_page4();
    break;
  case  5 :
    draw_lcd_page5();
    break;
  }
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void draw_page_number()
{
  char str[64];
  sprintf( str, "Page# %d  Blank.", current_page );
  lcd_draw_str_scaled( str, SCALE_2X, 25, 120, ILI9341_ORANGE, ILI9341_BLACK, 0 );       //8x16, 2x

}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void lcd_usb_connected( void )
{
  lcd_is_usb_connected = 1;
  uint32_t prim;
  prim = __get_PRIMASK();
  __disable_irq();
  update_usb_connect = 1;
  did_usb_start=1;
  start_rx_cnt=0;


  if( !prim ) {
    __enable_irq();
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void lcd_usb_disconnected( void )
{
  lcd_is_usb_connected = 0;
  uint32_t prim;
  prim = __get_PRIMASK();
  __disable_irq();
  update_usb_connect = 1;
  if( !prim ) {
    __enable_irq();
  }
}
