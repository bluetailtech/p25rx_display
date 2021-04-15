

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

#include <complex.h>
#include <math.h>

#include "main.h"
#include "stm32h7xx_hal.h"

#define ARM_MATH_CM7 1
#include "arm_common_tables.h"
#include "arm_math.h"

#include "lcd.h"
#include "lcd_buttons.h"
#include "fonts.h"

#include "usb_main_inc.h"
#include "usbh_core.h"
#include "usbh_cdc.h"

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_tx;
void MX_I2S2_Init( void );
ALIGN_32BYTES( volatile uint16_t   audio_tx[256] __attribute__( ( section( ".AudioSection" ) ) ) ); //I2S2 audio buffer
volatile uint32_t out_s = 0;
volatile uint32_t out_e = 0;
volatile uint32_t is_playing;
volatile int audio_started;
volatile short out_buffer[AUDIO_OUT_BUFFER_SIZE];
static void MX_DMA_Init( void );
volatile uint16_t rand16;
static volatile float angle1;
static volatile float angle2;
static volatile int audio_dir;
volatile int usb_restart_count;
volatile int do_usb_start;
extern uint8_t volatile CDC_RX_Buffer[RX_BUFF_SIZE];


USBH_HandleTypeDef hUSBHost;
CDC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;
static void USBH_UserProcess( USBH_HandleTypeDef *phost, uint8_t id );
void GetDefaultConfiguration( void );

static void USBH_UserProcess( USBH_HandleTypeDef *phost, uint8_t id )
{
  switch( id ) {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    Appli_state = APPLICATION_DISCONNECT;

    break;

  case HOST_USER_CLASS_ACTIVE:
    GetDefaultConfiguration();
    Appli_state = APPLICATION_READY;
    break;

  case HOST_USER_CONNECTION:
    Appli_state = APPLICATION_START;
    break;

  default:
    break;
  }
}


UART_HandleTypeDef huart6;

int _mem_free( void );
void lcd_tick( void );

static int draw_mod;
static uint32_t ms_tick_time;

#define IN_BUFFER_N 32

int64_t __errno;

void SystemClock_Config( void );
static void MX_GPIO_Init( void );
static void MX_USART6_UART_Init( void );
static void MX_CRC_Init( void );

static void MPU_Config( void );
static uint32_t current_time;
static uint32_t led_time;
static uint32_t audio_test;
static uint8_t led_state;
static uint8_t audio_state;
static uint32_t clk_mhz;

static uint32_t lcd_tick_time;


//audio agc related
#define AUD_AGC_LEN 64
static volatile float audio_max[AUD_AGC_LEN];

static volatile float tmp_buffer_f[1024];
static volatile int audio_max_idx;
static volatile float aout_gain = 0.25f;
static volatile float aout_abs;
static volatile float aud_agc_max;
static volatile float gainfactor;
static volatile float gaindelta;
static volatile float maxbuf;

static int silent_cnt;
static int start_rx_cnt;
static volatile float volume = 0.25f;


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int main( void )
{
  MPU_Config();

  SCB_EnableICache();
  //SCB_DisableICache();

  SCB_EnableDCache();
  //SCB_DisableDCache();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART6_UART_Init();

  printf( "\r\n\r\nInstruction Cache Enabled" );
  printf( "\r\nData Cache Enabled" );
  printf( "\r\nHAL Initialized" );

  //HAL_Delay(1000);

  clk_mhz = SystemCoreClock / 1e6;
  printf( "\r\nSTM32H743 Running @ %lu MHz", clk_mhz );

  printf( "\r\nBluetail Technologies OS starting..." );


  int ii;

  int16_t *sptr;
  int ii_len;

  int free = _mem_free();
  printf( "\r\nheap mem free %d bytes\r\n", free );

  MX_GPIO_Init();
  MX_USART6_UART_Init();
  lcd_init();

  HAL_Delay( 100 );

#if 1
  //HAL_PWREx_EnableUSBReg();
  HAL_PWREx_EnableUSBVoltageDetector();
#else
  HAL_PWREx_DisableUSBVoltageDetector();
  HAL_PWREx_DisableUSBReg();
#endif
  HAL_Delay( 100 );

  printf( "\r\nUSB Init" );
  /* Init Host Library */
  USBH_Init( &hUSBHost, USBH_UserProcess, 0 );

  printf( "\r\nUSB Reg CDC" );
  /* Add Supported Class */
  USBH_RegisterClass( &hUSBHost, USBH_CDC_CLASS );


  /* Start Host Process */
  USBH_Start( &hUSBHost );

  printf( "\r\nUSB Start" );

  GetDefaultConfiguration();
  //USBH_ReEnumerate(&hUSBHost);
  //USBH_CDC_Stop(&hUSBHost);


  MX_DMA_Init();
  MX_I2S2_Init();
  update_i2s_clock();
  HAL_I2S_Transmit_DMA( &hi2s2, audio_tx, AUDIO_DMASIZE );

  //audio_started=1;
  //HAL_GPIO_WritePin( AUDIO_EN_PORT, AUDIO_EN, GPIO_PIN_SET );


  printf( "\r\nStarting Main Loop" );

  while( 1 ) {

    USBH_Process( &hUSBHost );

    lcd_tick();

    if(do_usb_start) {
      uint32_t prim = __get_PRIMASK();
      __disable_irq();
        do_usb_start=0;
        usb_start_rx();
      if( !prim ) {
        __enable_irq();
      }
    }

  }

}

/////////////////////////////////////////////
// delay for 3 clock cycles * ulCount
/////////////////////////////////////////////
void __attribute__( ( naked ) ) DelayClk3( unsigned long ulCount )
{
  __asm( "    subs    r0, #1\n"
         "    bne     DelayClk3\n"
         "    bx      lr" );
}
/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_ms_ni( int delay )
{
  delay_us_ni( 1500 * delay );
}
/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_ms( int delay )
{
  HAL_Delay( delay );
}
/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_us_ni( int delay )
{
  DelayClk3( delay * 134 );
}
/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_us( int delay )
{
  DelayClk3( delay * 134 );
  //delay_us_ni( delay );
  //DelayClk3( delay );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void lcd_tick()
{
  uint32_t prim;

  current_time = HAL_GetTick();

  if( ( current_time - lcd_tick_time ) > 50 || pen_down ) {
    lcd_tick_time = current_time;

    lcd_test_tick();

  }

  if( current_time - ms_tick_time > 0 ) {

    #if 0
  uint32_t prim = __get_PRIMASK();
  __disable_irq();
    usb_restart_count++;
    if(usb_restart_count>8000) {
      usb_restart_count=0;
      usb_restart();
    }
  if( !prim ) {
    __enable_irq();
  }
    #else
      usb_restart_count++;
      if(usb_restart_count>5000 && lcd_is_usb_connected && do_usb_start==0) {
        usb_restart_count=0;
        //USBH_CDC_Receive( &hUSBHost, CDC_RX_Buffer, 256 );

        uint32_t prim = __get_PRIMASK();
        __disable_irq();
          do_usb_start=1;
        if( !prim ) {
          __enable_irq();
        }

      }
    #endif

    ms_tick_time = current_time;
    lcd_tick_ms();

    if( is_playing ) {
      silent_cnt = 0;
    } else {
      //turn off speaker/headphone power
      if( silent_cnt++ > 180000 ) { //180 seconds
        HAL_GPIO_WritePin( AUDIO_EN_PORT, AUDIO_EN, GPIO_PIN_RESET );
      }
    }

  }

  if( ++start_rx_cnt % 3000 ==0) {
    usb_tick();
  }

#if 0
  if( ++start_rx_cnt == 6000 ) {
    HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_RESET );
    printf( "\r\nUSB VBUS off" );
    HAL_Delay( 100 );
    HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_SET );
    printf( "\r\nUSB VBUS on" );
  }
#endif



}

/////////////////////////////////////////////////////////////////////////////////////////////
//_Error_Handler( __FILE__, __LINE__ );
/////////////////////////////////////////////////////////////////////////////////////////////
void SystemClock_Config( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply( PWR_LDO_SUPPLY );
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE0 );

  while( !__HAL_PWR_GET_FLAG( PWR_FLAG_VOSRDY ) ) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG( RCC_PLLSOURCE_HSE );
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;

  RCC_OscInitStruct.PLL.PLLN = 120; //480 MHz with 8MHz ref.  USB 48 MHz clock depends on this

  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 20;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3 | RCC_PERIPHCLK_SPI2
      | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 18;
  PeriphClkInitStruct.PLL2.PLL2P = 1;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 6144;
  PeriphClkInitStruct.PLL3.PLL3M = 1;
  PeriphClkInitStruct.PLL3.PLL3N = 18;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 1;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 6144;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  /** Enable USB Voltage detector
  */
  HAL_PWREx_EnableUSBVoltageDetector();
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
static void MX_USART6_UART_Init( void )
{

  huart6.Instance = USART6;
  huart6.Init.BaudRate = 2000000;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  //huart6.Init.Prescaler = UART_PRESCALER_DIV1;
  //huart6.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  //huart6.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  //huart6.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if( HAL_UART_Init( &huart6 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
static void MX_GPIO_Init( void )
{


  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();


//LCD PORT A
//#define LCD_T_CS_PORT GPIOA
//#define LCD_T_CS GPIO_PIN_3
//#define LCD_CS_PORT GPIOA
//#define LCD_CS GPIO_PIN_4
//#define LCD_SCK_PORT GPIOA
//#define LCD_SCK GPIO_PIN_5
//#define LCD_MISO_PORT GPIOA
//#define LCD_MISO GPIO_PIN_6
//#define LCD_MOSI_PORT GPIOA
//#define LCD_MOSI GPIO_PIN_7

//LCD PORT E
//#define LCD_RESET_PORT GPIOE
//#define LCD_RESET GPIO_PIN_7
//#define LCD_DC_PORT GPIOE
//#define LCD_DC GPIO_PIN_8
//#define LCD_LED_PORT GPIOE
//#define LCD_LED GPIO_PIN_9
//#define LCD_T_CLK_PORT GPIOE
//#define LCD_T_CLK GPIO_PIN_10
//#define LCD_T_DOUT_PORT GPIOE
//#define LCD_T_DOUT GPIO_PIN_11
//#define LCD_T_DIN_PORT GPIOE
//#define LCD_T_DIN GPIO_PIN_12
//#define LCD_T_IRQ_PORT GPIOE
//#define LCD_T_IRQ GPIO_PIN_13


  //PORT A   LCD PINS
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_1 | GPIO_PIN_2 ;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  //PORT E   LCD PINS
  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

  //lcd inputs GPIOE
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

  //lcd inputs GPIOA
  GPIO_InitStruct.Pin = GPIO_PIN_6 ; //lcd miso
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  HAL_GPIO_WritePin( LCD_SCK_PORT, LCD_SCK, GPIO_PIN_RESET );//lcd SCK
  HAL_GPIO_WritePin( LCD_CS_PORT, LCD_CS, GPIO_PIN_SET );//lcd cs
  HAL_GPIO_WritePin( LCD_RESET_PORT, LCD_RESET, GPIO_PIN_RESET ); //lcd reset
  HAL_GPIO_WritePin( LCD_LED_PORT, LCD_LED, GPIO_PIN_SET );//lcd led
  HAL_GPIO_WritePin( LCD_T_CS_PORT, LCD_T_CS, GPIO_PIN_SET );

  HAL_GPIO_WritePin( FLASH_CS_PORT, FLASH_CS, GPIO_PIN_SET );
  HAL_GPIO_WritePin( SD_CS_PORT, SD_CS, GPIO_PIN_SET );


  //USB power  PD5
  HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_SET );//ON
  GPIO_InitStruct.Pin = USB_EN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( USB_EN_PORT, &GPIO_InitStruct );

  //AUDIO amp enable
  HAL_GPIO_WritePin( AUDIO_EN_PORT, AUDIO_EN, GPIO_PIN_RESET );
  GPIO_InitStruct.Pin = AUDIO_EN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( AUDIO_EN_PORT, &GPIO_InitStruct );

}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
static void MPU_Config( void )
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = audio_tx;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion( &MPU_InitStruct );


  /* Enable the MPU */
  HAL_MPU_Enable( MPU_PRIVILEGED_DEFAULT );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
void _Error_Handler( char *file, int line )
{
  while( 1 ) {
  }
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
#ifdef  USE_FULL_ASSERT
void assert_failed( uint8_t *file, uint32_t line )
{
}
#endif /* USE_FULL_ASSERT */
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void MX_I2S2_Init( void )
{

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  //hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  //hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.FirstBit = I2S_FIRSTBIT_MSB;
  hi2s2.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  hi2s2.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  hi2s2.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_ENABLE;
  if( HAL_I2S_Init( &hi2s2 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void HAL_I2S_TxHalfCpltCallback( I2S_HandleTypeDef *hi2s )
{
  handle_i2s_double_buffer( &audio_tx[0] );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void HAL_I2S_TxCpltCallback( I2S_HandleTypeDef *hi2s )
{
  handle_i2s_double_buffer( &audio_tx[AUDIO_DMASIZE / 2] );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void handle_i2s_double_buffer( int16_t *audio_tx_ptr )
{

  int vi;
  int i;

  int do_stop = 0;

  if( audio_started == 0 ) {
    is_playing = 0;
    uint8_t *ptr = ( uint8_t * ) audio_tx;

    for( i = 0; i < AUDIO_DMASIZE / 4; i++ ) {
      *audio_tx_ptr++ = ( uint16_t ) 0;
      *audio_tx_ptr++ = ( uint16_t ) 0;
    }
    return;
  }


  for( vi = 0; vi < AUDIO_DMASIZE / 4; vi++ ) {

    if( ( is_playing && out_s != out_e ) ) {

      if(!mute_audio) {
        *audio_tx_ptr++ = ( int16_t )( out_buffer[out_s] *  volume );
        *audio_tx_ptr++ = ( int16_t )( out_buffer[out_s++] *  volume );
      }
      else {
        out_s++;
        *audio_tx_ptr++ = 0; 
        *audio_tx_ptr++ = 0; 
      }
      out_s &= AUDIO_OUT_BUFFER_SIZE - 1;

    } else {
      do_stop = 1;

      *audio_tx_ptr++ = 0;   //silence
      *audio_tx_ptr++ = 0;
    }
  }


  if( do_stop ) {
    is_playing = 0;
    do_stop = 0;
  }

}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void update_i2s_clock()
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3 | RCC_PERIPHCLK_SPI2
      | RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 1;     //PLL2 clocks the audio I2S,
  //PeriphClkInitStruct.PLL2.PLL2N = 122;   //audio playback rate for 8kHz
  PeriphClkInitStruct.PLL2.PLL2N = 118;   //slower playback
  PeriphClkInitStruct.PLL2.PLL2P = 4;
  PeriphClkInitStruct.PLL2.PLL2Q = 4;
  PeriphClkInitStruct.PLL2.PLL2R = 4;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 1;

  PeriphClkInitStruct.PLL3.PLL3M = 1;
  PeriphClkInitStruct.PLL3.PLL3N = 23;
  PeriphClkInitStruct.PLL3.PLL3P = 8;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 7987;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;

  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }


}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void MX_DMA_Init( void )
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority( DMA1_Stream1_IRQn, 0, 0 );
  HAL_NVIC_EnableIRQ( DMA1_Stream1_IRQn );

}

//////////////////////////////////////////////////////////////////////////////////
// audio agc
//////////////////////////////////////////////////////////////////////////////////
float update_gain_s16( int16_t *audio, int len, float target, float rate )
{
  int i;

  for( i = 0; i < len; i++ ) {
    tmp_buffer_f[i] = ( float ) audio[i];
  }

  update_gain_f32(tmp_buffer_f, len, target, rate);

  for( i = 0; i < len; i++ ) {
    audio[i] = ( int16_t ) tmp_buffer_f[i];
  }

}
//////////////////////////////////////////////////////////////////////////////////
// audio agc
//#define AUD_AGC_LEN 8192
//////////////////////////////////////////////////////////////////////////////////
float update_gain_f32( float *audio, int len, float target, float rate )
{
  #define MAX_GAIN 4.0f

  int i, n;

  float hf_audio[160];

  if( audio_max_idx > AUD_AGC_LEN-1 ) audio_max_idx = 0;

  // detect max level after compression
  aud_agc_max = 0;
  for( n = 0; n < len; n++ ) {
    if( fabs( audio[n] ) > aud_agc_max ) aud_agc_max = fabs( audio[n] );
  }
  audio_max[audio_max_idx++] = aud_agc_max;
  if( audio_max_idx > AUD_AGC_LEN-1 ) audio_max_idx = 0;

  aud_agc_max = 0;
  for( n = 0; n < AUD_AGC_LEN; n++ ) {
    if( audio_max[n] > aud_agc_max ) aud_agc_max = audio_max[n];
  }

  //calculate gain
  if( aud_agc_max > 0.0f ) {
    gainfactor = ( target / aud_agc_max );
  } else {
    gainfactor = MAX_GAIN; 
  }

  if( gainfactor < aout_gain ) {
    aout_gain = gainfactor;
    gaindelta = 0.0f;
  } else {
    if( gainfactor > MAX_GAIN ) gainfactor = MAX_GAIN; 
    gaindelta = gainfactor - aout_gain;
    if( gaindelta > rate * aout_gain ) {
      gaindelta = rate * aout_gain;
    }
  }
  gaindelta /= ( float ) 160.0f;

  if( aout_gain < 0.01f ) aout_gain = 0.01f;
  if( aout_gain > 30.0f ) aout_gain = 30.0f;

  float redo = 1.0f;
  for( n = 0; n < len; n++ ) {
    audio[n] *= aout_gain + ( gaindelta * ( float )n );
  }
  aout_gain += 160.0f * gaindelta;
  return aout_gain;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void set_volume(int vol) {

  uint32_t prim = __get_PRIMASK();
  __disable_irq();
  volume = (float) ( (float) vol / 100.0f );
  if( !prim ) {
    __enable_irq();
  }
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void usb_restart() {

  //HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_RESET );
  //HAL_Delay( 500 );
  //HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_SET );
  //HAL_Delay( 100 );


  USBH_CDC_Stop(&hUSBHost);
  if(!did_usb_start) {
    USBH_DeInit( &hUSBHost );
    HAL_PWREx_DisableUSBVoltageDetector();
  }

  HAL_Delay( 100 );

  if(!did_usb_start) {
    HAL_PWREx_EnableUSBVoltageDetector();
    USBH_Init( &hUSBHost, USBH_UserProcess, 0 );
    USBH_RegisterClass( &hUSBHost, USBH_CDC_CLASS );
  }
  USBH_Start( &hUSBHost );
}
