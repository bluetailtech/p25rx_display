

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
#include "main.h"
#include <stdlib.h>

#define LCD_PAGE2 2

//#include "p25rx_img.h"

static button_info *bptr;
static int vol = 25; 

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_page2_init()
{

  lcd_button_init();

  bptr = new_button_hslider( "V", 75, 200, SCALE_2X, LCD_PAGE2, BUT_GLOBAL_OFF, &handle_button_evt );

};

/////////////////////////////////////////////
/////////////////////////////////////////////
void draw_lcd_page2()
{


  char str[64];

  float log_val = 20.0f * log10( ((float) bptr->x_pos * 0.33f) / 100.0f );

  sprintf( str, "Volume %2.0f dB  ", log_val );

  int newvol = (int) ( (float) bptr->x_pos * 0.33f );
  if(newvol!=vol || do_refresh) {
    vol = newvol;
    set_volume(newvol); 
    lcd_draw_str_scaled( str, SCALE_2X, 25, 160, ILI9341_ORANGE, ILI9341_BLACK, 1 );       //8x16, 2x
  }

  do_refresh=0;

  return;
}
