
#ifndef _ILI9341H_LCD
#define _ILI9341H_LCD

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
#include "lcd_regs_ili9341.h"
#include "lcd_buttons.h"
#include "fonts.h"

/* This file was automatically generated.  Do not edit! */
void lcd_tick_ms( void );
void lcd_refresh( void );
void draw_page_number( void );
void __lcd_drawcircle( uint16_t color, int x0, int y0, int radius );
void lcd_drawcircle( uint16_t color, int x0, int y0, int radius, int fill );
void lcd_draw_rect( int x, int y, int w, int h, uint16_t color );
void lcd_draw_rect_hl( int x, int y, int w, int h );
void lcd_draw_str_h( char *str, int font_size, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg );
void lcd_draw_str_scaled( char *str, int scale, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg );
void lcd_draw_button( char *str, int scale, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg );
void lcd_draw_slider( char *str, int scale, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg, int oldx );
void lcd_draw_str_v( char *str, int font_size, int x, int y, uint16_t fcolor, uint16_t bcolor, int fillbg );
void lcd_drawline( uint16_t color, int x0, int y0, int x1, int y1 );
void lcd_draw_pixel( int16_t x, int16_t y, uint16_t color );
void lcd_fill( uint16_t color );
void lcd_fill_rect( int x, int y, int w, int h, uint16_t color );
void _lcd_fill( int x, int y, int w, int h, uint16_t color );
void lcd_spi_write8( uint16_t val );
void lcd_fill_rand_rect();
void lcd_cs_high();
void lcd_spi_write16( uint16_t val );
void lcd_write_command( uint8_t lcd_cmd );
void lcd_set_draw_port( uint16_t x1, uint16_t y1, uint16_t w, uint16_t h );
void lcd_set_scroll_def( uint16_t top, uint16_t sa, uint16_t bottom );
void lcd_scroll_to( uint16_t y );
void lcd_set_rotation( uint8_t rot );
void lcd_send_command( uint8_t lcd_cmd, uint8_t *lcd_data, int lcd_data_len );
void lcd_init();
void lcd_test_tick( void );
uint16_t get_touch_pos( uint8_t cmd );
int check_touch_screen_irqn( void );
extern uint16_t lcd_rotation;
extern uint16_t lcd_height;
extern uint16_t lcd_width;
extern const uint8_t initcmd[];
void usb_send_command(char *command_str);

extern int do_clear;
extern int do_refresh;
extern int xy_clr_mod;

extern uint16_t lcd_rotation;
extern uint16_t lcd_height;
extern uint16_t lcd_width;

extern uint16_t lcd_width;
extern uint16_t lcd_height;
extern uint16_t lcd_rotation;
extern uint8_t pen_down;
extern int lcd_is_usb_connected;
extern volatile int update_usb_connect;
extern volatile int usb_rx_count;
extern int current_talkgroup;

//LCD PORT A
#define LCD_T_CS_PORT GPIOA
#define LCD_T_CS GPIO_PIN_3
#define LCD_CS_PORT GPIOA
#define LCD_CS GPIO_PIN_4
#define LCD_SCK_PORT GPIOA
#define LCD_SCK GPIO_PIN_5
#define LCD_MISO_PORT GPIOA
#define LCD_MISO GPIO_PIN_6
#define LCD_MOSI_PORT GPIOA
#define LCD_MOSI GPIO_PIN_7

//LCD PORT E
#define LCD_RESET_PORT GPIOE
#define LCD_RESET GPIO_PIN_7
#define LCD_DC_PORT GPIOE
#define LCD_DC GPIO_PIN_8
#define LCD_LED_PORT GPIOE
#define LCD_LED GPIO_PIN_9
#define LCD_T_CLK_PORT GPIOE
#define LCD_T_CLK GPIO_PIN_10
#define LCD_T_DOUT_PORT GPIOE
#define LCD_T_DOUT GPIO_PIN_11
#define LCD_T_DIN_PORT GPIOE
#define LCD_T_DIN GPIO_PIN_12
#define LCD_T_IRQ_PORT GPIOE
#define LCD_T_IRQ GPIO_PIN_13

//XPT2046 touch screen controller commands
#define T_CMD_READX 0xd0  //addr 5, 12-bit conv, irq enabled, auto-convert on transfer
#define T_CMD_READY 0x90  //addr 1, 12-bit conv, irq enabled, auto-convert on transfer

#define ILI9341_BLACK 0x0000
#define ILI9341_NAVY 0x000F
#define ILI9341_DARKGREEN 0x03E0
#define ILI9341_DARKCYAN 0x03EF
#define ILI9341_MAROON 0x7800
#define ILI9341_PURPLE 0x780F
#define ILI9341_OLIVEGREEN 0x7BE0
#define ILI9341_LIGHTGREY 0xC618
#define ILI9341_DARKGREY 0x7BEF
#define ILI9341_DARKERGREY 0x4208
#define ILI9341_BLUE 0x001F
#define ILI9341_GREEN 0x07E0
#define ILI9341_CYAN 0x07FF
#define ILI9341_RED 0xF800
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW 0xFFE0
#define ILI9341_WHITE 0xFFFF
#define ILI9341_ORANGE 0xFD20
#define ILI9341_GREENYELLOW 0xAFE5
#define ILI9341_PINK 0xFC18

#define BUT_GLOBAL_ON 1
#define BUT_GLOBAL_OFF 0

#endif
