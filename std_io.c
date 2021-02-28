

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
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "std_io.h"
#include "lcd.h"
#include "usb_main_inc.h"

extern UART_HandleTypeDef huart6;
uint8_t net_buffer[MAX_NET_BUFFER];
uint8_t printf_buf[MAX_PRINTF_BUFFER];

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void printf_uart( char *net_buffer )
{

  //TODO: FIX make this an option
  return;

  if( net_buffer[0] != 0 ) {
    HAL_UART_Transmit( &huart6, net_buffer, strlen( ( char * )net_buffer ), HAL_MAX_DELAY );
    net_buffer[0] = 0;
  }

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int printf( const char *format, ... )
{

  uint16_t ret = 0;
  va_list args;



  va_start( args, format );

  ret = vsnprintf( printf_buf, sizeof( printf_buf ) - 1, format, args );
  printf_buf[MAX_PRINTF_BUFFER - 1] = 0;

  int len1 = strlen( net_buffer );
  int len2 = strlen( printf_buf );


  if( len1 + len2 < MAX_NET_BUFFER ) {
    strcat( net_buffer, printf_buf );
    //net_buffer[len1+len2]=0x00;
    net_buffer[MAX_NET_BUFFER - 1] = 0;
  }

  va_end( args );


#if 0
  //usb_rx_count++;
  update_usb_connect = 1;
  if( net_buffer[0] != 0 ) {
    net_buffer[63] = 0;
    lcd_fill_rect( 8, 150 - 32, 16 * 20, 32, ILI9341_BLACK );
    lcd_draw_str_scaled( net_buffer, SCALE_2X, 25, 150, ILI9341_ORANGE, ILI9341_BLACK, 0 );       //8x16, 2x
  }
#endif


  printf_uart( net_buffer );



  return ret; //length sent to device
}
