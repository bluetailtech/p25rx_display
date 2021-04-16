

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
//#include "xy_plot.h"
#include <stdlib.h>

int do_clear;
int do_refresh = 1;
int xy_clr_mod;

static int ms_time;

static int tg_test;
static int tg_on;

static int mod_time = 1000;
static int current_tg;

#define LCD_PAGE1 1

int usb_rx_timeout;

static int8_t xy_data[304];
static uint8_t sysinfo[320];


static int synced;
static double current_freq;
static double freq;
static int rssi;
static int tg_active;
static int prev_tg_active;
static int blks_sec;
static uint8_t current_desc[32];
static uint16_t rfss_id;
static uint16_t site_id;
static int sys_id;
static int wacn;
static int is_phase2 = -1;
static int nac;
static int p25_follow;
int current_talkgroup;
static uint8_t str[64];
static char sys_name[12];

static volatile int do_update_sysinfo;
static int init_txt;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_page1_init()
{

  lcd_button_init();

  //global buttons show on all pages
  new_button_c1( "TGHOLD", 115, 50, SCALE_2X, LCD_PAGE1, BUT_GLOBAL_ON, &handle_button_evt );
  new_button_c1( "<", 17, 60, SCALE_3X, LCD_PAGE1, BUT_GLOBAL_ON, &handle_button_evt );
  new_button_c1( ">", 290, 60, SCALE_3X, LCD_PAGE1, BUT_GLOBAL_ON, &handle_button_evt );
  new_button_c1( "MUTE", 235, 15, SCALE_1X, LCD_PAGE1, BUT_GLOBAL_ON, &handle_button_evt );


};

/////////////////////////////////////////////
/////////////////////////////////////////////
void lcd_tick_ms( void )
{

  //TODO: fix
  return;

}

/////////////////////////////////////////////
/////////////////////////////////////////////
void draw_lcd_page1()
{

  uint32_t stime = HAL_GetTick();
  uint32_t etime;
  uint32_t elapsed;
  int i;

  //clear area under xy plot
  if( xy_clr_mod++ % 3 == 0 ) {
    lcd_fill_rect( 3, 64, 85, 77, ILI9341_BLACK );
    //draw grid for xy plot
    lcd_drawline( ILI9341_GREEN, 3, 100, 85, 100 );
    lcd_drawline( ILI9341_GREEN, 45, 60, 45, 141 );
  }

  if(!init_txt) {
    //lcd_fill_rect( 8, 220 - 32, 16 * 21, 32, ILI9341_BLACK );
    //lcd_draw_str_scaled( "Connecting...", SCALE_2X, 8, 220, ILI9341_WHITE, ILI9341_BLACK, 1 );       //8x16, 4x
    lcd_draw_str_scaled( "Connect to P25RX..", SCALE_1X, 4, 190, ILI9341_ORANGE, ILI9341_BLACK, 1 );      //8x16, 2x
    init_txt=1;
  }

#if 0
  //draw xy plot
  int *xyptr = xy_plot;
  for( i = 0; i < xy_plot_len; i++ ) {
    int x = *xyptr++;
    int y = *xyptr++;

    x /= 2;
    y /= 2;

    double rzo = rand() / ( double )RAND_MAX;
    rzo *= 16.0;
    x += rzo;

    rzo = rand() / ( double )RAND_MAX;
    rzo *= 16.0;
    y += rzo;
#else
  //draw xy plot
  int8_t *xyptr = xy_data;

  for( i = 0; i < 126; i++ ) {
    int x = ( int )( ( float ) * xyptr++ / 1.25f );
    int y = ( int )( ( float ) * xyptr++ / 1.25f );


    if( x > 30 ) x = 30;
    if( x < -30 ) x = -30;
    if( y > 30 ) y = 30;
    if( y < -30 ) y = -30;

#endif

#if 0
    lcd_drawcircle( ILI9341_YELLOW, x - 85, y + 35, 2, 1 );
#else
    lcd_draw_pixel( x + 46, y + 100, ILI9341_YELLOW );
#endif
  }


  if(do_update_sysinfo) {
    do_update_sysinfo=0;
    update_sysinfo();
  }



  //TODO: fix
  return;


  if( !do_refresh ) goto show_elapsed;



show_elapsed:

  etime = HAL_GetTick();
  elapsed = etime - stime;
  //printf( "\r\nelapsed %d", elapsed );
}

////////////////////////////////////////////////////////////////////////////////////
// err_hz_f
// est_hz_f
// rf_gain_db
// quad_agc->g
// synced (int)
////////////////////////////////////////////////////////////////////////////////////
void update_const( uint8_t *iq_const )
{
  memcpy( ( uint8_t * ) xy_data, &iq_const[20], 126 * 2 );
}


////////////////////////////////////////////////////////////////////////////////////
  /*
  int synced = p25_is_synced();
  if(synced>0) synced=1;
  if(p25_is_tdu()) synced=-2;

  if(config->sa_mode==MODE_DMR) {
    synced=0;
    if(dmr_is_synced()) synced=1;
  }

  if(config->sa_mode==MODE_NXDN4800) {
    synced=0;
    if(nxdn_synced()) synced=1;
  }

  memcpy(&send_buffer[4], (uint8_t *) &synced, 4);

  memcpy(&send_buffer[8], (uint8_t *) current_loc, 32);
  memcpy(&send_buffer[40], (uint8_t *) current_desc, 32);

  memcpy(&send_buffer[72], (uint8_t *) &wacn_id, 4); //uint32_t
  memcpy(&send_buffer[76], (uint8_t *) &sys_id, 4); //uint32_t
  memcpy(&send_buffer[80], (uint8_t *) &rf_id, 2); //uint16_t
  memcpy(&send_buffer[82], (uint8_t *) &site_id, 2); //uint16_t
  memcpy(&send_buffer[84], (uint8_t *) &config->p25_sys_nac, 4); //int
  memcpy(&send_buffer[88], (uint8_t *) &config->frequency, 8); //double
  int rssi = get_latest_valid_rssi();
  memcpy(&send_buffer[96], (uint8_t *) &rssi, 4); //int
  memcpy(&send_buffer[100], (uint8_t *) &voice_bk, 4); //int
  memcpy(&send_buffer[104], (uint8_t *) &is_phase2, 4); //int
  memcpy(&send_buffer[108], (uint8_t *) &tsbk_ps, 4); //int
  memcpy(&send_buffer[112], (uint8_t *) &config->p25_sys_nac, 4); //int
  memcpy(&send_buffer[116], (uint8_t *) &current_talkgroup, 4); //int
  memcpy(&send_buffer[120], (uint8_t *) &config->p25_follow, 4); //int
  memcpy(&send_buffer[124], (uint8_t *) &tg_active, 4); //int
  memcpy(&send_buffer[128], (uint8_t *) config->sys_name, 12);

  send_binary_blocking((uint8_t *) send_buffer, 128+4); 
  */
////////////////////////////////////////////////////////////////////////////////////
void process_sysinfo( uint8_t *data )
{

  if(do_update_sysinfo) return;

  memcpy(sysinfo, data, 140);
  do_update_sysinfo=1;

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void update_sysinfo() {
  int i1;
  uint16_t u1;

  if( current_page != 1 ) return;

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[112], 4 );
  if( i1 != current_talkgroup || do_refresh ) {
    current_talkgroup = i1;
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[116], 4 );
  if( i1 != p25_follow || do_refresh ) {
    p25_follow = i1;
    if(p25_follow) {
      tg_hold=1;
    }
    else {
      tg_hold=0;
    }
    redraw_buttons();
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[0], 4 );
  if( i1 != synced || do_refresh ) {
    synced = i1;
    //SIG led
    if( synced == 1 ) lcd_drawcircle( ILI9341_GREEN, 72, 35, 10, 1 );
    else lcd_drawcircle( ILI9341_DARKERGREY, 72, 35, 10, 1 );
  }

  memcpy( ( uint8_t * ) &freq, ( uint8_t * ) &sysinfo[84], 8 );

  if( !test_doubles_equal( freq, current_freq ) || do_refresh ) {
    current_freq = freq;

    sprintf( str, "Frequency %3.6f MHz", current_freq );
    lcd_draw_str_h( str, _FONT8x8, 105, 70, ILI9341_WHITE, ILI9341_BLACK, 1 );

    memcpy( sys_name, &sysinfo[124], 12 );
    int sys_name_len = strlen(sys_name);
    if(sys_name_len<1) sys_name_len=1;
    if(sys_name_len>11) sys_name_len=11;

    lcd_draw_str_scaled( sys_name, SCALE_2X, 4, 190, ILI9341_ORANGE, ILI9341_BLACK, 1 );      //8x16, 2x
    //test fonts
    lcd_draw_str_v( "IQ SYMBOLS", _FONT8x16, 100, 62, ILI9341_WHITE, ILI9341_BLACK, 1 );        //8x16
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[100], 4 );
  if( i1 != is_phase2 || do_refresh ) {
    is_phase2 = i1;
    if( is_phase2 ) strcpy( str, "P25 Phase II" );
    else strcpy( str, "P25 Phase I " );

    lcd_draw_str_scaled( str, SCALE_2X, 105, 155, ILI9341_YELLOW, ILI9341_BLACK, 1 );       //8x16, 3x
  }


  int did_sys_id=0;
  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[72], 4 );
  if( i1 != sys_id || do_refresh ) {
    sys_id = i1;
    did_sys_id=1;
  }
  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[68], 4 );


  if( i1 != wacn || did_sys_id || do_refresh ) {
    wacn = i1;


    sprintf( str, "SYS:0x%05X-0x%03X", wacn , sys_id );
    lcd_draw_str_scaled( str, SCALE_1X, 176, 182, ILI9341_ORANGE, ILI9341_BLACK, 1 );      //8x16, 2x
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[92], 4 );
  if( i1 != rssi || do_refresh ) {
    rssi = i1;

    sprintf( str, "RSSI %3d dBm", rssi );
    lcd_draw_str_h( str, _FONT8x8, 105, 118, ILI9341_WHITE, ILI9341_BLACK, 1 );
  }

  memcpy( ( uint8_t * ) &u1, ( uint8_t * ) &sysinfo[76], 2 );
  if( u1 != rfss_id || do_refresh ) {
    rfss_id = u1;

    sprintf( str, "RFSS ID %d   ", rfss_id );
    lcd_draw_str_h( str, _FONT8x8, 105, 102, ILI9341_WHITE, ILI9341_BLACK, 1 );
  }

  memcpy( ( uint8_t * ) &u1, ( uint8_t * ) &sysinfo[78], 2 );
  if( u1 != site_id || do_refresh ) {
    site_id = u1;

    sprintf( str, "SITE ID %d   ", site_id );
    lcd_draw_str_h( str, _FONT8x8, 105, 86, ILI9341_WHITE, ILI9341_BLACK, 1 );
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[104], 4 );
  if( i1 != blks_sec || do_refresh ) {
    blks_sec = i1;

    sprintf( str, "BLKS/SEC %d ", blks_sec );
    lcd_draw_str_h( str, _FONT8x8, 225, 118, ILI9341_WHITE, ILI9341_BLACK, 1 );
  }

  memcpy( ( uint8_t * ) &i1, ( uint8_t * ) &sysinfo[108], 4 );
  if( i1 != nac || do_refresh ) {
    nac = i1;

    sprintf( str, "NAC 0x%03X", nac );
    lcd_draw_str_h( str, _FONT8x8, 225, 86, ILI9341_WHITE, ILI9341_BLACK, 1 );
  }

  memset( current_desc, 0x00, 32 );
  strncpy( current_desc, ( uint8_t * ) &sysinfo[36], 16 );



  memcpy( ( uint8_t * ) &tg_active, ( uint8_t * ) &sysinfo[120], 4 );
  tg_active ^= 0x01;

  if( tg_active != prev_tg_active) { 

    prev_tg_active = tg_active;

    if( tg_active ) {
      lcd_drawcircle( ILI9341_YELLOW, 245, 35, 10, 1 );
      sprintf(str, "%s  TG %d", current_desc, current_talkgroup);
      lcd_draw_str_scaled( str, SCALE_2X, 8, 220, ILI9341_WHITE, ILI9341_BLACK, 1 );       //8x16, 4x
    } else {
      lcd_drawcircle( ILI9341_DARKERGREY, 245, 35, 10, 1 );

      //clear tg desc
      lcd_fill_rect( 8, 220 - 32, 16 * 21, 32, ILI9341_BLACK );
      memset( current_desc, 0x00, 32 );
    }
  }


  do_refresh = 0;


}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
#define precision 7
int test_doubles_equal( double d1, double d2 )
{
  return fabs( d1 - d2 ) < pow( 10, -precision );
}
