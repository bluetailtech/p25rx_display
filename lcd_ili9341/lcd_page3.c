

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
#include "std_io.h"

#define LCD_PAGE3 3

static int draw_mod;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_page3_init()
{

  lcd_button_init();

  new_button_c1( "TEST P3", 90, 200, SCALE_2X, LCD_PAGE3, BUT_GLOBAL_OFF, &handle_button_evt );

};

/////////////////////////////////////////////
/////////////////////////////////////////////
void draw_lcd_page3()
{
  char str[64];

  //draw_page_number();
  if( !do_refresh ) goto show_elapsed;

  do_refresh = 0;



show_elapsed:
  if( update_usb_connect ) {

    //uint32_t prim;
    //prim = __get_PRIMASK();
    //__disable_irq();
    update_usb_connect = 0;
    //if( !prim ) {
    //__enable_irq();
    //}

    if( ++draw_mod % 8 == 0 ) {
      lcd_fill_rect( 185, 150 - 32, 9 * 20, 32, ILI9341_BLACK );
      sprintf( str, "USB Connect %d,%d", lcd_is_usb_connected, usb_rx_count );
      lcd_draw_str_scaled( str, SCALE_2X, 0, 150, ILI9341_ORANGE, ILI9341_BLACK, 0 );       //8x16, 2x
    }

  }

  return;

}
