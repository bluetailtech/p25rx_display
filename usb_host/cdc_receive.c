
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

#include "usb_main_inc.h"
#include "lcd.h"
#include "fonts.h"
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"

void process_usb_rx( uint8_t c );
extern void update_const( uint8_t *iq_const );
extern void process_sysinfo( uint8_t *sysinfo );

uint8_t volatile CDC_RX_Buffer[RX_BUFF_SIZE];
uint8_t volatile CDC_TX_Buffer[RX_BUFF_SIZE];
static void usb_rx( void );

static volatile int rx_state;
static volatile int pcm_idx;
static volatile int16_t audio_val;
volatile int voice_mod;

static volatile int16_t pcm[320];
static volatile int pcm_cnt;

static volatile uint8_t iq_const[320];
static volatile int iq_cnt;

static volatile uint8_t sysinfo[320];
static volatile int sysinfo_cnt;

static volatile int init_cnt;
static int did_tx;

volatile int did_usb_start;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void usb_start_rx()
{
  USBH_CDC_Receive( &hUSBHost, CDC_RX_Buffer, 256 );
  did_tx=0;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void usb_tick(void) {
  uint32_t prim;
  if(did_tx==0) {

    prim = __get_PRIMASK();
    __disable_irq();
      init_cnt = 0;
    if( !prim ) {
      __enable_irq();
    }

    usb_start_tx();
    did_tx=1;
  }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void usb_send_command(char *command_str) {
  uint32_t prim;

//  if(init_cnt>0 && init_cnt<7) return;

  //prim = __get_PRIMASK();
  //__disable_irq();

  strcpy( CDC_TX_Buffer, command_str );
  int tx_len = strlen( CDC_TX_Buffer );
  USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );

  delay_ms_ni(10);

  //if( !prim ) {
   // __enable_irq();
  //}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void usb_start_tx()
{

  if(init_cnt>0 && init_cnt<7) return;

#if 1
  strcpy( CDC_TX_Buffer, "en_voice_send 1\r\n\0" );
  int tx_len = strlen( CDC_TX_Buffer );
  USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );

#endif
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void USBH_CDC_ReceiveCallback( USBH_HandleTypeDef *phost )
{
  usb_rx();
  USBH_CDC_Receive( &hUSBHost, CDC_RX_Buffer, 256 );
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void USBH_CDC_TransmitCallback( USBH_HandleTypeDef *phost )
{
  init_cnt++;
  if( init_cnt == 1 ) {
    strcpy( CDC_TX_Buffer, "en_display 1\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
#if 1
  else if( init_cnt == 2 ) {
    strcpy( CDC_TX_Buffer, "led_mode 0\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
  else if( init_cnt == 3 ) {
    strcpy( CDC_TX_Buffer, "en_display 1\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
  else if( init_cnt == 4 ) {
    strcpy( CDC_TX_Buffer, "en_display 1\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
  else if( init_cnt == 5 ) {
    strcpy( CDC_TX_Buffer, "en_voice_send 1\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
  else if( init_cnt == 6 ) {
    strcpy( CDC_TX_Buffer, "en_voice_send 1\r\n\0" );
    int tx_len = strlen( CDC_TX_Buffer );
    USBH_CDC_Transmit( &hUSBHost, CDC_TX_Buffer, tx_len );
    delay_ms_ni(10);
  }
#endif
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
static void usb_rx( void )
{
  uint16_t size;
  int i;

  size = USBH_CDC_GetLastReceivedDataSize( &hUSBHost );


  //easier to process 1 byte at a time with a state machine
  for( i = 0; i < size; i++ ) {
    process_usb_rx( CDC_RX_Buffer[i] );
  }

  //draw received blocks on lcd page 3
  usb_rx_count++;
  update_usb_connect = 1;

  //printf( "%s", ptr );
  //lcd_draw_str_scaled( usb_rx, SCALE_2X,15, 220, ILI9341_WHITE, ILI9341_BLACK, 1 );        //8x16, 4x

}

////////////////////////////////////////////////////////////////////////////////////
//#define BL_OP_MAGIC 0xd35467A6
//#define VOICE8K_MAGIC 0xb25f9c71
//#define CONST_MAGIC 0xb25b12e4
//#define TDMA_MAGIC 0xb259ef72
//#define AUDIO_FLUSH_MAGIC 0xb2987231
//#define SYSINFO_MAGIC 0xb2517015
////////////////////////////////////////////////////////////////////////////////////
void process_usb_rx( uint8_t c )
{
  uint32_t prim;
  int i;

  switch( rx_state ) {
  case  0 :
    if( c == 0xb2 ) rx_state++;
    break;
  case  1 :
    if( c == 0x5f || c == 0x98 || c == 0x5b || c == 0x51 ) rx_state++;
    else rx_state = 0;
    break;
  case  2 :
    if( c == 0x9c || c == 0x72 || c == 0x12 || c == 0x70 ) rx_state++;
    else rx_state = 0;
    break;
  case  3 :
        prim = __get_PRIMASK();
        __disable_irq();
        usb_restart_count=0;
        if( !prim ) {
          __enable_irq();
        }

    if( c == 0x71 ) { //voice audio
      rx_state = 4;
      pcm_idx = 0;
      pcm_cnt = 0;

      if(!mute_audio) {
        HAL_GPIO_WritePin( AUDIO_EN_PORT, AUDIO_EN, GPIO_PIN_SET );
      }
    } else if( c == 0xe4 ) { //iq constellation
      rx_state = 5;
      iq_cnt = 0;
    } else if( c == 0x15 ) { //sysinfo
      rx_state = 6;
      sysinfo_cnt = 0;
    } else if( c == 0x31 ) { //audio flush
      rx_state = 0;
      voice_mod = 0;

      if( !is_playing ) {
        //audio flush
        prim = __get_PRIMASK();
        __disable_irq();
        is_playing = 1;
        audio_started = 1;
        if( !prim ) {
          __enable_irq();
        }
      }
    } else {
      rx_state = 0;
    }
    break;


  case  4 :   //VOICE BLK INCOMING

    if( ( pcm_idx & 0x01 ) == 0x00 ) {
      audio_val = ( uint16_t ) c;
    } else {
      audio_val |= ( uint16_t ) c << 8;
      pcm[pcm_cnt++] = ( int16_t ) audio_val;
    }

    if( pcm_idx++ == 319 ) {


      update_gain_s16( pcm, 160, 22000.0f, 0.02f ); //rate of 0.06f is 6x normal because of 8kHz sample rate  (vs 48kHz sample rate)

      if(out_s==out_e) voice_mod=0;

      //prim = __get_PRIMASK();
      //__disable_irq();
      for( i = 0; i < 160; i++ ) {
        out_buffer[out_e++] = ( int16_t ) pcm[i];
        out_e &= AUDIO_OUT_BUFFER_SIZE - 1;
      }
      //if( !prim ) {
      // __enable_irq();
      //}

      if( voice_mod++ > 10 ) {
        prim = __get_PRIMASK();
        __disable_irq();
        is_playing = 1;
        audio_started = 1;
        if( !prim ) {
          __enable_irq();
        }
        voice_mod = 0;
      }
      rx_state = 0;

    }
    break;

  case  5 :   //IQ CONST
    iq_const[iq_cnt++] = c;
    if( iq_cnt == 320 ) {
      prim = __get_PRIMASK();
      __disable_irq();
        rx_state = 0;
        update_const( iq_const );
      if( !prim ) {
        __enable_irq();
      }
    }
    break;

  case  6 :   //SYSINFO
    sysinfo[sysinfo_cnt++] = c;
    if( sysinfo_cnt == 140 ) {

      prim = __get_PRIMASK();
      __disable_irq();
        process_sysinfo( sysinfo );
        rx_state = 0;
      if( !prim ) {
        __enable_irq();
      }
    }
    break;

  default :
    rx_state = 0;
    break;
  }

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void usb_printf( const char *format, ... )
{
//  uint8_t usb_rx[128];
// sprintf(usb_rx, "%s", format);
  //lcd_draw_str_scaled( usb_rx, SCALE_2X,15, 220, ILI9341_WHITE, ILI9341_BLACK, 1 );        //8x16, 4x
}
