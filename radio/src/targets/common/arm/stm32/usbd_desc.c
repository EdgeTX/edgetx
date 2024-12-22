/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : App/usbd_desc.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB device descriptors.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

/* USER CODE BEGIN INCLUDE */
#include "hal/usb_driver.h"
#include "usb_descriptor.h"
#include "stamp.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @addtogroup USBD_DESC
  * @{
  */

/** @defgroup USBD_DESC_Private_TypesDefinitions USBD_DESC_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Defines USBD_DESC_Private_Defines
  * @brief Private defines.
  * @{
  */

#define USBD_VID_STM                        0x0483    // STM Vendor ID
#define USBD_VID_PID_CODES                  0x1209    // https://pid.codes

#define USBD_LANGID_STRING                  0x409
#define USBD_MANUFACTURER_STRING            "OpenTX"
#define USB_SIZ_STRING_SERIAL               0x1A

#if defined(BOOT)
#define USBD_MSC_PRODUCT_FS_STRING          USB_NAME " Bootloader"
#else
#define USBD_MSC_PRODUCT_FS_STRING          USB_NAME " Mass Storage"
#endif

#define USBD_MSC_VID                        USBD_VID_STM
#define USBD_MSC_PID                        0x5720
#define USBD_MSC_CONFIGURATION_FS_STRING    "MSC Config"
#define USBD_MSC_INTERFACE_FS_STRING        "MSC Interface"

#define USBD_HID_VID                        USBD_VID_PID_CODES
#define USBD_HID_PID                        0x4F54     // OpenTX assigned PID
#define USBD_HID_PRODUCT_FS_STRING          USB_NAME " Joystick"
#define USBD_HID_CONFIGURATION_FS_STRING    "HID Config"
#define USBD_HID_INTERFACE_FS_STRING        "HID Interface"

#define USBD_CDC_VID                        USBD_VID_STM
#define USBD_CDC_PID                        0x5740     // do not change, this ID is used by the ST USB driver for Windows
#define USBD_CDC_PRODUCT_FS_STRING          USB_NAME " Serial Port"
#define USBD_CDC_CONFIGURATION_FS_STRING    "VSP Config"
#define USBD_CDC_INTERFACE_FS_STRING        "VSP Interface"

#define USBD_DFU_VID                        USBD_VID_STM
#define USBD_DFU_PID                        0xDF11
#define USBD_DFU_PRODUCT_STRING             USB_NAME " DFU"
#define USBD_DFU_CONFIGURATION_STRING       "DFU Config"
#define USBD_DFU_INTERFACE_STRING           "DFU Interface"

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/** @defgroup USBD_DESC_Private_Macros USBD_DESC_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_FunctionPrototypes USBD_DESC_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_FunctionPrototypes USBD_DESC_Private_FunctionPrototypes
  * @brief Private functions declaration for FS.
  * @{
  */

uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
#if (USBD_LPM_ENABLED == 1)
uint8_t * USBD_FS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Variables USBD_DESC_Private_Variables
  * @brief Private variables.
  * @{
  */

USBD_DescriptorsTypeDef FS_Desc =
{
  USBD_FS_DeviceDescriptor
, USBD_FS_LangIDStrDescriptor
, USBD_FS_ManufacturerStrDescriptor
, USBD_FS_ProductStrDescriptor
, USBD_FS_SerialStrDescriptor
, USBD_FS_ConfigStrDescriptor
, USBD_FS_InterfaceStrDescriptor
#if (USBD_LPM_ENABLED == 1)
, USBD_FS_USR_BOSDescriptor
#endif /* (USBD_LPM_ENABLED == 1) */
};
#if 0
#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/** USB standard device descriptor. */
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
  0x12,                       /*bLength */
  USB_DESC_TYPE_DEVICE,       /*bDescriptorType*/
#if (USBD_LPM_ENABLED == 1)
  0x01,                       /*bcdUSB */ /* changed to USB version 2.01
                                             in order to support LPM L1 suspend
                                             resume test of USBCV3.0*/
#else
  0x00,                       /*bcdUSB */
#endif /* (USBD_LPM_ENABLED == 1) */
  0x02,
  0x00,                       /*bDeviceClass*/
  0x00,                       /*bDeviceSubClass*/
  0x00,                       /*bDeviceProtocol*/
  USB_MAX_EP0_SIZE,           /*bMaxPacketSize*/
  LOBYTE(USBD_VID),           /*idVendor*/
  HIBYTE(USBD_VID),           /*idVendor*/
  LOBYTE(USBD_PID_FS),        /*idProduct*/
  HIBYTE(USBD_PID_FS),        /*idProduct*/
  0x00,                       /*bcdDevice rel. 2.00*/
  0x02,
  USBD_IDX_MFC_STR,           /*Index of manufacturer  string*/
  USBD_IDX_PRODUCT_STR,       /*Index of product string*/
  USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
  USBD_MAX_NUM_CONFIGURATION  /*bNumConfigurations*/
};
#endif
/* USB_DeviceDescriptor */
/** BOS descriptor. */
#if (USBD_LPM_ENABLED == 1)
#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN uint8_t USBD_FS_BOSDesc[USB_SIZ_BOS_DESC] __ALIGN_END =
{
  0x5,
  USB_DESC_TYPE_BOS,
  0xC,
  0x0,
  0x1,  /* 1 device capability*/
        /* device capability*/
  0x7,
  USB_DEVICE_CAPABITY_TYPE,
  0x2,
  0x2,  /* LPM capability bit set*/
  0x0,
  0x0,
  0x0
};
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Variables USBD_DESC_Private_Variables
  * @brief Private variables.
  * @{
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */

/** USB lang identifier descriptor. */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
     USB_LEN_LANGID_STR_DESC,
     USB_DESC_TYPE_STRING,
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING)
};

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

#if defined ( __ICCARM__ )      /* !< IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
  USB_SIZ_STRING_SERIAL,
  USB_DESC_TYPE_STRING,
};

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Functions USBD_DESC_Private_Functions
  * @brief Private functions.
  * @{
  */

/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  UNUSED(speed);

  int vid, pid;

  switch (getSelectedUsbMode()) {
    case USB_JOYSTICK_MODE:
      vid = USBD_HID_VID;
      pid = USBD_HID_PID;
      break;

    case USB_SERIAL_MODE:
      vid = USBD_CDC_VID;
      pid = USBD_CDC_PID;
      break;

    case USB_MASS_STORAGE_MODE:
      vid = USBD_MSC_VID;
      pid = USBD_MSC_PID;
      break;

    case USB_DFU_MODE:
      vid = USBD_DFU_VID;
      pid = USBD_DFU_PID;
      break;

    default:
      vid = 0;
      pid = 0;
  }

  /* version has to be bcd coded */
  uint8_t version_minor = ((VERSION_MINOR / 10) << 4) | (VERSION_MINOR % 10);
  uint8_t version_major = ((VERSION_MAJOR / 10) << 4) | (VERSION_MAJOR % 10);

  /* USB Standard Device Descriptor */
  __ALIGN_BEGIN const uint8_t USBD_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
    {
      USB_LEN_DEV_DESC,           /*bLength */
      USB_DESC_TYPE_DEVICE,       /*bDescriptorType*/
      0x00,                       /*bcdUSB USB specification release number in binary-coded decimal*/
      0x02,
      0x00,                       /*bDeviceClass*/
      0x00,                       /*bDeviceSubClass*/
      0x00,                       /*bDeviceProtocol*/
      USB_MAX_EP0_SIZE,           /*bMaxPacketSize*/
      LOBYTE(vid),                /*idVendor*/
      HIBYTE(vid),                /*idVendor*/
      LOBYTE(pid),                /*idProduct*/
      HIBYTE(pid),                /*idProduct*/
      version_minor,              /*bcdDevice device release number in binary-coded decimal*/
      version_major,
      USBD_IDX_MFC_STR,           /*Index of manufacturer string*/
      USBD_IDX_PRODUCT_STR,       /*Index of product string*/
      USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
      USBD_MAX_NUM_CONFIGURATION  /*bNumConfigurations*/
    }; /* USB_DeviceDescriptor */

  *length = sizeof(USBD_DeviceDesc);
  memcpy(USBD_StrDesc, USBD_DeviceDesc, *length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  UNUSED(speed);
  *length = sizeof(USBD_LangIDDesc);
  memcpy(USBD_StrDesc, USBD_LangIDDesc, *length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  switch (getSelectedUsbMode()) {
    case USB_JOYSTICK_MODE:
      USBD_GetString ((uint8_t*)USBD_HID_PRODUCT_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_SERIAL_MODE:
      USBD_GetString ((uint8_t*)USBD_CDC_PRODUCT_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_MASS_STORAGE_MODE:
      USBD_GetString ((uint8_t*)USBD_MSC_PRODUCT_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_DFU_MODE:
      USBD_GetString ((uint8_t*)USBD_DFU_PRODUCT_STRING, USBD_StrDesc, length);
      break;
  }
  return USBD_StrDesc;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  *length = USB_SIZ_STRING_SERIAL;

  /* Update the serial number string descriptor with the data from the unique
   * ID */
  Get_SerialNum();

  return (uint8_t *)USBD_StringSerial;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  switch (getSelectedUsbMode()) {
    case USB_JOYSTICK_MODE:
      USBD_GetString ((uint8_t*)USBD_HID_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_SERIAL_MODE:
      USBD_GetString ((uint8_t*)USBD_CDC_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_MASS_STORAGE_MODE:
      USBD_GetString ((uint8_t*)USBD_MSC_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_DFU_MODE:
      USBD_GetString ((uint8_t*)USBD_DFU_CONFIGURATION_STRING, USBD_StrDesc, length);
      break;
  }
  return USBD_StrDesc;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  switch (getSelectedUsbMode()) {
    case USB_JOYSTICK_MODE:
      USBD_GetString ((uint8_t*)USBD_HID_INTERFACE_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_SERIAL_MODE:
      USBD_GetString ((uint8_t*)USBD_CDC_INTERFACE_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_MASS_STORAGE_MODE:
      USBD_GetString ((uint8_t*)USBD_MSC_INTERFACE_FS_STRING, USBD_StrDesc, length);
      break;
    case USB_DFU_MODE:
      USBD_GetString ((uint8_t*)USBD_DFU_INTERFACE_STRING, USBD_StrDesc, length);
      break;
  }
  return USBD_StrDesc;
}

#if (USBD_LPM_ENABLED == 1)
/**
  * @brief  Return the BOS descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  UNUSED(speed);
  *length = sizeof(USBD_FS_BOSDesc);
  return (uint8_t*)USBD_FS_BOSDesc;
}
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @brief  Create the serial number string descriptor
  * @param  None
  * @retval None
  */
static void Get_SerialNum(void)
{
  uint32_t deviceserial0;
  uint32_t deviceserial1;
  uint32_t deviceserial2;

  deviceserial0 = *(uint32_t *) DEVICE_ID1;
  deviceserial1 = *(uint32_t *) DEVICE_ID2;
  deviceserial2 = *(uint32_t *) DEVICE_ID3;

  deviceserial0 += deviceserial2;

  if (deviceserial0 != 0)
  {
    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
  }
}

/**
  * @brief  Convert Hex 32Bits value into char
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
  uint8_t idx = 0;

  for (idx = 0; idx < len; idx++)
  {
    if (((value >> 28)) < 0xA)
    {
      pbuf[2 * idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[2 * idx + 1] = 0;
  }
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

