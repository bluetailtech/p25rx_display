/**
  ******************************************************************************
  * File Name          : stm32h7xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

extern DMA_HandleTypeDef hdma_spi2_tx;
extern void _Error_Handler( char *, int );
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit( void )
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( MemoryManagement_IRQn, 0, 0 );
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( BusFault_IRQn, 0, 0 );
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( UsageFault_IRQn, 0, 0 );
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SVCall_IRQn, 0, 0 );
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( DebugMonitor_IRQn, 0, 0 );
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( PendSV_IRQn, 0, 0 );
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

void HAL_CRC_MspInit( CRC_HandleTypeDef *hcrc )
{

  if( hcrc->Instance == CRC ) {
    /* USER CODE BEGIN CRC_MspInit 0 */

    /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
    /* USER CODE BEGIN CRC_MspInit 1 */

    /* USER CODE END CRC_MspInit 1 */
  }

}

void HAL_CRC_MspDeInit( CRC_HandleTypeDef *hcrc )
{

  if( hcrc->Instance == CRC ) {
    /* USER CODE BEGIN CRC_MspDeInit 0 */

    /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
    /* USER CODE BEGIN CRC_MspDeInit 1 */

    /* USER CODE END CRC_MspDeInit 1 */
  }

}

void HAL_UART_MspInit( UART_HandleTypeDef *huart )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( huart->Instance == USART6 ) {

    /* Peripheral clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
    HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

  }

}

void HAL_UART_MspDeInit( UART_HandleTypeDef *huart )
{

  if( huart->Instance == USART6 ) {
    __HAL_RCC_USART6_CLK_DISABLE();

    HAL_GPIO_DeInit( GPIOG, GPIO_PIN_9 | GPIO_PIN_14 );

  }

}
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void HAL_I2S_MspInit( I2S_HandleTypeDef *hi2s )
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};


  if( hi2s->Instance == SPI2 ) {
    /* USER CODE BEGIN SPI2_MspInit 0 */

    /* USER CODE END SPI2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2S2 GPIO Configuration
    PB12     ------> I2S2_WS
    PB13     ------> I2S2_CK
    PB15     ------> I2S2_SDO
    PC6     ------> I2S2_MCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    /* I2S2 DMA Init */
    /* SPI2_TX Init */
    hdma_spi2_tx.Instance = DMA1_Stream1;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if( HAL_DMA_Init( &hdma_spi2_tx ) != HAL_OK ) {
      _Error_Handler( __FILE__, __LINE__ );
    }

    __HAL_LINKDMA( hi2s, hdmatx, hdma_spi2_tx );

    /* I2S2 interrupt Init */
    HAL_NVIC_SetPriority( SPI2_IRQn, 2, 2 );
    HAL_NVIC_EnableIRQ( SPI2_IRQn );


    /* USER CODE BEGIN SPI2_MspInit 1 */

    /* USER CODE END SPI2_MspInit 1 */
  }


}

/**
* @brief I2S MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2s: I2S handle pointer
* @retval None
*/
void HAL_I2S_MspDeInit( I2S_HandleTypeDef *hi2s )
{
  if( hi2s->Instance == SPI2 ) {
    __HAL_RCC_SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15 );
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_6 );
  }

}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

}
