

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

#include <stdlib.h>
#include "stm32h7xx_hal.h"
#include "lcd.h"


#define MAX_BUTTONS 128
static button_info _buttons[MAX_BUTTONS];
static int _button_idx = 0;

static int did_init = 0;
int current_page = 1;
static int number_of_pages = 2;
int tg_hold;
int mute_audio;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void draw_button( button_info *bp )
{

  char str[128];

  if( bp->button_type == BUTTON_TYPE_HSLIDER ) {
    draw_button_hslider(bp);
    return;
  } 

  int strl = strlen( bp->button_str );
  if( strl == 6 && strcmp( bp->button_str, "TGHOLD" ) == 0 ) {
    if( tg_hold ) {
      lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, ILI9341_GREEN, bp->do_fill );
      sprintf(str, "f %d\r\n\0", current_talkgroup);  //update P25RX with follow command
      usb_send_command(str);
    } else {
      lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill );
      usb_send_command("f 0\r\n\0");  //update P25RX with un-follow command
    }
  }
  else if( strl == 4 && strcmp( bp->button_str, "MUTE" ) == 0 ) {
    if( mute_audio ) {
      lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, ILI9341_GREEN, bp->do_fill );
    } else {
      lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill );
    }
  } else {
    //generic draw
    lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_button_hslider( button_info *bp )
{
  char str[128];
  //generic draw
  lcd_draw_slider( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill, bp->x_pos );

}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void lcd_button_init()
{
  if( !did_init ) {
    did_init = 1;
    memset( ( uint8_t * ) _buttons, 0x00, MAX_BUTTONS * sizeof( button_info ) );
  }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void handle_button_evt( button_info *bp )
{
  printf( "\r\nbutton: <%s> released", bp->button_str );

  bp->button_prev_state = 0;
  bp->button_state = 0;


  int strl = strlen( bp->button_str );


  if( strl == 4 && strcmp( bp->button_str, "CLR " ) == 0 ) {
    lcd_refresh();
    return 0;
  }

  if( strl == 1 && strcmp( bp->button_str, "<" ) == 0 ) {
    int st = current_page;
    current_page--;
    if( current_page < 1 ) current_page = number_of_pages;
    printf( "\r\npage number %d", current_page );
    lcd_refresh();
  }
  if( strl == 1 && strcmp( bp->button_str, ">" ) == 0 ) {
    int st = current_page;
    current_page++;
    if( current_page > number_of_pages ) current_page = 1;
    printf( "\r\npage number %d", current_page );
    lcd_refresh();
  }
  if( strl == 6 && strcmp( bp->button_str, "TGHOLD" ) == 0 ) {
    tg_hold ^= 0x01;
    if( tg_hold ) {
      printf( "\r\nfollowing tg" );
    } else {
      printf( "\r\nunfollowing tg" );
    }
  }
  if( strl == 4 && strcmp( bp->button_str, "MUTE" ) == 0 ) {
    mute_audio ^= 0x01;
  }

  draw_button( bp );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int check_button_touch_evt( int x, int y, uint8_t state )
{

  button_info *bp;
  int i;

  for( i = 0; i < MAX_BUTTONS; i++ ) {
    bp = &_buttons[i];

    if( bp->button_type == BUTTON_TYPE_PRESS || bp->button_type == BUTTON_TYPE_TOGGLE ) {
      if( bp != NULL && state && ( current_page == bp->page_number || bp->global ) &&
          x > bp->x_pos && x < bp->x_pos + bp->width && y > bp->y_pos - bp->height && y < bp->y_pos ) {

        bp->button_prev_state = state;
        bp->button_state = state;
        printf( "\r\nbutton <%s> pressed state", bp->button_str );

        lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->bg_color, bp->fg_color, bp->do_fill );
        return 1;  //button pressed state
      }
    }
    else if( bp->button_type == BUTTON_TYPE_HSLIDER ) {
      if( bp != NULL && state && ( current_page == bp->page_number || bp->global ) &&
          y > bp->y_pos - bp->height && y < bp->y_pos ) {

        int old_xpos = bp->x_pos;
        bp->x_pos = x;
        bp->button_prev_state = state;
        bp->button_state = state;
        printf( "\r\nbutton <%s> pressed state", bp->button_str );

        lcd_draw_slider( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->bg_color, bp->bg_color, bp->do_fill, old_xpos );
        //TODO: fix this
        do_refresh=1;
        draw_lcd_page2();
        return 1;  //button pressed state
      }
    }
  }

  //released?
  int did_evt = 0;
  for( i = 0; i < MAX_BUTTONS; i++ ) {
    bp = &_buttons[i];
    if( bp != NULL && !state && bp->button_state ) {
      ( *bp->button_evt_handler )( bp ); //call the button handler code
      did_evt = 1;
      break;
    }
  }

  return did_evt;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
button_info *new_button_hslider( char *name, int x, int y, int font_scale, int page, int global, void ( *evt_handler ) )
{
  button_info *bp = &_buttons[_button_idx++];

  if( x > 319 ) x = 319;
  if( y > 239 ) y = 239;

  int scale = 1;
  if( font_scale == SCALE_1X ) scale = 1;
  if( font_scale == SCALE_2X ) scale = 2;
  if( font_scale == SCALE_3X ) scale = 3;
  if( font_scale == SCALE_4X ) scale = 4;
  if( font_scale == SCALE_5X ) scale = 5;
  int strl = strnlen( name, 63 );

  strncpy( bp->button_str, name, 63 );

  if( abs( bp->x_pos-x ) < 5 ) return;

  int old_xpos = bp->x_pos;
  bp->x_pos = x;

  bp->x_pos = x;
  bp->y_pos = y;
  bp->scale = font_scale;
  bp->width = 8 * scale * strl;
  bp->height = 16 * scale;
  bp->page_number = page;
  bp->fg_color = ILI9341_BLACK;
  bp->bg_color = ILI9341_LIGHTGREY;
  bp->do_fill = 1;
  bp->button_type = BUTTON_TYPE_HSLIDER;
  bp->global = global;

  bp->button_prev_state = BUTTON_STATE_IDLE;
  bp->button_state = BUTTON_STATE_IDLE;

  bp->button_evt_handler = evt_handler;

  if( current_page == bp->page_number || bp->global ) {
    lcd_draw_slider( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill, old_xpos );
  }

  return bp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
button_info *new_button_c1( char *name, int x, int y, int font_scale, int page, int global, void ( *evt_handler ) )
{
  button_info *bp = &_buttons[_button_idx++];

  if( x > 319 ) x = 319;
  if( y > 239 ) y = 239;

  int scale = 1;
  if( font_scale == SCALE_1X ) scale = 1;
  if( font_scale == SCALE_2X ) scale = 2;
  if( font_scale == SCALE_3X ) scale = 3;
  if( font_scale == SCALE_4X ) scale = 4;
  if( font_scale == SCALE_5X ) scale = 5;
  int strl = strnlen( name, 63 );

  strncpy( bp->button_str, name, 63 );
  bp->x_pos = x;
  bp->y_pos = y;
  bp->scale = font_scale;
  bp->width = 8 * scale * strl;
  bp->height = 16 * scale;
  bp->page_number = page;
  bp->fg_color = ILI9341_BLACK;
  bp->bg_color = ILI9341_LIGHTGREY;
  bp->do_fill = 1;
  bp->button_type = BUTTON_TYPE_PRESS;
  bp->global = global;

  bp->button_prev_state = BUTTON_STATE_IDLE;
  bp->button_state = BUTTON_STATE_IDLE;

  bp->button_evt_handler = evt_handler;

  if( current_page == bp->page_number || bp->global ) {
    lcd_draw_button( bp->button_str, bp->scale, bp->x_pos, bp->y_pos, bp->fg_color, bp->bg_color, bp->do_fill );
  }

  return bp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void redraw_buttons( void )
{
  int i;
  button_info *bp;

  for( i = 0; i < MAX_BUTTONS; i++ ) {
    bp = &_buttons[i];
    if( bp != NULL && bp->page_number == current_page || bp->global ) {
      draw_button( bp );
    }
  }
}
