

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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>

#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD

#define AUDIO_EN_PORT GPIOC
#define AUDIO_EN GPIO_PIN_8

#define USB_EN_PORT GPIOD
#define USB_EN GPIO_PIN_5

#define FLASH_CS_PORT GPIOA
#define FLASH_CS GPIO_PIN_1

#define SD_CS_PORT GPIOA
#define SD_CS GPIO_PIN_2

#ifdef __cplusplus
extern "C" {
#endif
void _Error_Handler( char *, int );

#define AUDIO_DMASIZE 128
#define AUDIO_OUT_BUFFER_SIZE 65536 //must be power of 2

void _Error_Handler( char *file, int line );
void delay_us( int delay );
void delay_ms( int delay );
void delay_us_ni( int delay );
void delay_ms_ni( int delay );
void update_i2s_clock();
int main( void );
void SystemClock_Config( void );
void SystemClock_Config( void );
extern int64_t __errno;
void lcd_tick( void );
void lcd_tick();
int _mem_free( void );
void GetDefaultConfiguration( void );
extern volatile short out_buffer[AUDIO_OUT_BUFFER_SIZE];
extern volatile int audio_started;
extern volatile uint32_t is_playing;
extern volatile uint32_t out_e;
extern volatile uint32_t out_s;
void __attribute__( ( naked ) )DelayClk3( unsigned long ulCount );
void MX_I2S2_Init( void );
void set_volume(int vol);
float update_gain_s16( int16_t *audio, int len, float target, float rate );
float update_gain_f32( float *audio, int len, float target, float rate );
extern int usb_restart_count;
extern volatile int did_usb_start;

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif
