

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
#include "stm32h7xx.h"
#include "stm32h7xx_it.h"



extern DMA_HandleTypeDef hdma_spi2_tx;
extern I2S_HandleTypeDef hi2s2;
extern HCD_HandleTypeDef hhcd;

void SysTick_Handler( void )
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}



//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void OTG_FS_IRQHandler( void )
{
  HAL_HCD_IRQHandler( &hhcd );
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void SPI2_IRQHandler( void )
{
  HAL_I2S_IRQHandler( &hi2s2 );
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void DMA1_Stream1_IRQHandler( void )
{
  HAL_DMA_IRQHandler( &hdma_spi2_tx );
}
