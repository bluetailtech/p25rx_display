
#ifndef _ILI9341H_BUTTONS
#define _ILI9341H_BUTTONS

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

#include <stdint.h>
#include "lcd.h"

enum {
  BUTTON_TYPE_PRESS,
  BUTTON_TYPE_TOGGLE,
  BUTTON_TYPE_HSLIDER,
};

#define BUTTON_STATE_IDLE 0
#define BUTTON_STATE_SELECTED 1<<0
#define BUTTON_STATE_DOREDRAW 1<<1
#define BUTTON_STATE_MOVING 1<<2
#define BUTTON_STATE_PRESSED 1<<3
#define BUTTON_STATE_NOTPRESSED 1<<4

typedef struct {
  char button_str[64];  //limit is really more like 54 chars
  int x_pos;
  int y_pos; //x,y is lower left corner of button area
  int scale;  //SCALE_1X, ...5X
  int width;
  int height;
  int button_type;
  uint8_t button_debounce;
  uint8_t page_number;
  uint8_t pad2;
  uint8_t pad3;
  int button_prev_state;
  int button_state;
  uint16_t fg_color;
  uint16_t bg_color;
  int do_fill;
  int global;
  void ( *button_evt_handler )( void *bp );
} button_info;

button_info *new_button( char *name, int x, int y, int font_scale, int type,
                         int page, uint16_t fg_color, uint16_t bg_color, int do_fill, int global, void ( *button_evt_handler ) );
button_info *new_button_c1( char *name, int x, int y, int font_scale, int page, int global, void ( *button_evt_handler ) );
button_info *new_button_hslider( char *name, int x, int y, int font_scale, int page, int global, void ( *button_evt_handler ) );
int check_button_touch_evt( int x, int y, uint8_t state );
void redraw_button( void );
void lcd_button_init( void );

void handle_button_evt( button_info *bp );

extern int current_page;
extern int tg_hold;
extern int mute_audio;

#endif
