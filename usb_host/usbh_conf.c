
/************************ (C) COPYRIGHT STMicroelectronics *********/

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
#include "usbh_core.h"
#include "stm32h7xx_hal_hcd.h"
#include "main.h"

HCD_HandleTypeDef hhcd;

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void HAL_HCD_MspInit( HCD_HandleTypeDef *hhcd )
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Configure USB FS GPIOs */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Configure DM DP Pins */
  GPIO_InitStruct.Pin = ( GPIO_PIN_11 | GPIO_PIN_12 );
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  /* Configure ID pin */
  //GPIO_InitStruct.Pin = GPIO_PIN_5;
  //GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  //GPIO_InitStruct.Pull = GPIO_PULLUP;
  //GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure VBUS Pin */
  //GPIO_InitStruct.Pin = GPIO_PIN_9;
  //GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  //GPIO_InitStruct.Pull = GPIO_NOPULL;
  //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Initialize the IO */
  //BSP_IO_Init();

  /* Configure POWER_SWITCH IO pin */
  //BSP_IO_ConfigPin(OTG_FS1_POWER_SWITCH_PIN, IO_MODE_OUTPUT);
  //USB power  PD10
  HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_SET );
  GPIO_InitStruct.Pin = USB_EN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( USB_EN_PORT, &GPIO_InitStruct );

  /* Enable USB FS Clocks */
  __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();

  /* Set USBFS Interrupt priority */
  HAL_NVIC_SetPriority( OTG_FS_IRQn, 0x0F, 0 );

  /* Enable USBFS Interrupt */
  HAL_NVIC_EnableIRQ( OTG_FS_IRQn );
}

/**
  * @brief  DeInitializes the HCD MSP.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_MspDeInit( HCD_HandleTypeDef *hhcd )
{
  __HAL_RCC_USB2_OTG_FS_CLK_DISABLE();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void HAL_HCD_SOF_Callback( HCD_HandleTypeDef *hhcd )
{
  USBH_LL_IncTimer( hhcd->pData );
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void HAL_HCD_Connect_Callback( HCD_HandleTypeDef *hhcd )
{
  USBH_LL_Connect( hhcd->pData );
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void HAL_HCD_Disconnect_Callback( HCD_HandleTypeDef *hhcd )
{
  USBH_LL_Disconnect( hhcd->pData );
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void HAL_HCD_HC_NotifyURBChange_Callback( HCD_HandleTypeDef *hhcd,
    uint8_t chnum,
    HCD_URBStateTypeDef urb_state )
{
  /* To be used with OS to sync URB state with the global state machine */
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_Init( USBH_HandleTypeDef *phost )
{
  printf( "\r\nUSB_LL_Init" );


  /* Set the LL Driver parameters */
  hhcd.Instance = USB2_OTG_FS;
  hhcd.Init.Host_channels = 11;
  hhcd.Init.dma_enable = 0;
  hhcd.Init.low_power_enable = 0;
  hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;
  hhcd.Init.Sof_enable = 0;
  hhcd.Init.speed = HCD_SPEED_FULL;
  hhcd.Init.vbus_sensing_enable = 0;
  hhcd.Init.lpm_enable = 0;

  /* Link the driver to the stack */
  hhcd.pData = phost;
  phost->pData = &hhcd;

  /* Initialize the LL Driver */
  HAL_HCD_Init( &hhcd );

  USBH_LL_SetTimer( phost, HAL_HCD_GetCurrentFrame( &hhcd ) );

  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_DeInit( USBH_HandleTypeDef *phost )
{
  HAL_HCD_DeInit( phost->pData );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_Start( USBH_HandleTypeDef *phost )
{
  HAL_HCD_Start( phost->pData );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_Stop( USBH_HandleTypeDef *phost )
{
  HAL_HCD_Stop( phost->pData );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_SpeedTypeDef USBH_LL_GetSpeed( USBH_HandleTypeDef *phost )
{
  USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

  switch( HAL_HCD_GetCurrentSpeed( phost->pData ) ) {
  case 0:
    speed = USBH_SPEED_HIGH;
    break;

  case 1:
    speed = USBH_SPEED_FULL;
    break;

  case 2:
    speed = USBH_SPEED_LOW;
    break;

  default:
    speed = USBH_SPEED_FULL;
    break;
  }
  return speed;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_ResetPort( USBH_HandleTypeDef *phost )
{
  HAL_HCD_ResetPort( phost->pData );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
uint32_t USBH_LL_GetLastXferSize( USBH_HandleTypeDef *phost, uint8_t pipe )
{
  return HAL_HCD_HC_GetXferCount( phost->pData, pipe );
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_OpenPipe( USBH_HandleTypeDef *phost,
                                     uint8_t pipe,
                                     uint8_t epnum,
                                     uint8_t dev_address,
                                     uint8_t speed,
                                     uint8_t ep_type, uint16_t mps )
{
  HAL_HCD_HC_Init( phost->pData, pipe, epnum, dev_address, speed, ep_type, mps );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_ClosePipe( USBH_HandleTypeDef *phost, uint8_t pipe )
{
  HAL_HCD_HC_Halt( phost->pData, pipe );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_SubmitURB( USBH_HandleTypeDef *phost,
                                      uint8_t pipe,
                                      uint8_t direction,
                                      uint8_t ep_type,
                                      uint8_t token,
                                      uint8_t *pbuff,
                                      uint16_t length, uint8_t do_ping )
{
  HAL_HCD_HC_SubmitRequest( phost->pData,
                            pipe,
                            direction, ep_type, token, pbuff, length, do_ping );
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_URBStateTypeDef USBH_LL_GetURBState( USBH_HandleTypeDef *phost,
    uint8_t pipe )
{
  return ( USBH_URBStateTypeDef ) HAL_HCD_HC_GetURBState( phost->pData, pipe );
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_DriverVBUS( USBH_HandleTypeDef *phost, uint8_t state )
{

  if( state==0 ) {
    HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_RESET );
  }
  else {
    HAL_GPIO_WritePin( USB_EN_PORT, USB_EN, GPIO_PIN_SET );
  }


  //HAL_Delay( 500 ); //500



  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
USBH_StatusTypeDef USBH_LL_SetToggle( USBH_HandleTypeDef *phost, uint8_t pipe,
                                      uint8_t toggle )
{
  if( hhcd.hc[pipe].ep_is_in ) {
    hhcd.hc[pipe].toggle_in = toggle;
  } else {
    hhcd.hc[pipe].toggle_out = toggle;
  }
  return USBH_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
uint8_t USBH_LL_GetToggle( USBH_HandleTypeDef *phost, uint8_t pipe )
{
  uint8_t toggle = 0;

  if( hhcd.hc[pipe].ep_is_in ) {
    toggle = hhcd.hc[pipe].toggle_in;
  } else {
    toggle = hhcd.hc[pipe].toggle_out;
  }
  return toggle;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void USBH_Delay( uint32_t Delay )
{
  HAL_Delay( Delay );
}
