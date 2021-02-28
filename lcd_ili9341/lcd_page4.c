

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

#define LCD_PAGE4 4

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_page4_init()
{

  lcd_button_init();

  new_button_c1( "TEST P4", 90, 200, SCALE_2X, LCD_PAGE4, BUT_GLOBAL_OFF, &handle_button_evt );

};

/////////////////////////////////////////////
/////////////////////////////////////////////
void draw_lcd_page4()
{


  if( !do_refresh ) goto show_elapsed;

  do_refresh = 0;

  draw_page_number();

show_elapsed:

  return;
}
