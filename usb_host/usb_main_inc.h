/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MAIN_H
#define __USB_MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "usbh_core.h"
#include "usbh_cdc.h"
#include "stm32h7xx_hal_hcd.h"

typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START,
  APPLICATION_READY,
  APPLICATION_RUNNING,
  APPLICATION_DISCONNECT,
} CDC_ApplicationTypeDef;
extern USBH_HandleTypeDef hUSBHost;
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
