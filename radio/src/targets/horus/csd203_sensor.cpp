/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "csd203_sensor.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "delays_driver.h"
#include "edgetx_types.h"
#include "hal.h"
#include "rtos.h"
#include "stm32_exti_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_i2c_driver.h"

// clang-format off
/* CSD203 Regsistor map */
#define CONFIGURATION                   0X00
/*Read Only*/
#define SHUNT_VOLTAGE                   0X01
#define BUS_VOLTAGE                     0X02
#define POWER                           0X03
#define CURRENT                         0x04
/*Read Only*/
#define CALIBRATION                     0x05
#define MASKENABLE                      0x06
#define ALERTLIMIT                      0x07

#define ManufacturerID               	0xFE	//Manufacturer ID Register
#define DieID 		                	0xFF	//Die ID Register

#define ManIDCode 		                0x4153	//CHIP ID

/*Calibration Calculation parameter*/
/* This Parameter Gain*10000K*/
#define CalParam                        51200
/* ↓ add your shunt here ↓   */
#define CSD_CONFIG_Rs_1mR			    1
#define CSD_CONFIG_Rs_2mR		    	2
#define CSD_CONFIG_Rs_5mR		    	5
#define CSD_CONFIG_Rs_10mR		    	10
#define CSD_CONFIG_Rs_20mR		    	20
#define CSD_CONFIG_Rs_50mR		    	50
#define CSD_CONFIG_CurrentLsb1mA    	10
#define CSD_CONFIG_CurrentLsb2mA    	20
#define CSD_CONFIG_CurrentLsb5mA    	50
#define CSD_CONFIG_CurrentLsb10mA    	100
#define CSD_CONFIG_CurrentLsb20mA  	    200
/*Calibration Calculation parameter*/

/* CSD203 Regsistor Config*/
#define CSD_CONFIG_RST 		 	    	1
#define CSD_CONFIG_UnRST 	 	    	0

#define CSD_CONFIG_Avg1			    	0
#define CSD_CONFIG_Avg4			    	1
#define CSD_CONFIG_Avg16		     	2
#define CSD_CONFIG_Avg64	    		3
#define CSD_CONFIG_Avg128		    	4
#define CSD_CONFIG_Avg256	    		5
#define CSD_CONFIG_Avg512   			6
#define CSD_CONFIG_Avg1024			    7

#define CSD_CONFIG_VBUS_CT1_1mS 	    4
#define CSD_CONFIG_VShunt_CT1_1mS   	4

#define CSD_CONFIG_ShuntBus_CON     	7

#define CSD_CONFIG_ADDR_A1_GND_A0_GND 	64
#define CSD_CONFIG_ADDR_A1_GND_VS_GND 	65
#define CSD_CONFIG_ADDR_A1_GND_SDA_GND 	66
#define CSD_CONFIG_ADDR_A1_GND_SCL_GND 	67
#define CSD_CONFIG_ADDR_A1_VS_A0_GND 	68
#define CSD_CONFIG_ADDR_A1_VS_A0_VS 	69
#define CSD_CONFIG_ADDR_A1_VS_A0_SDA 	70
#define CSD_CONFIG_ADDR_A1_VS_A0_SCL 	71
#define CSD_CONFIG_ADDR_A1_SDA_A0_GND 	72
#define CSD_CONFIG_ADDR_A1_SDA_A0_VS 	73
#define CSD_CONFIG_ADDR_A1_SDA_A0_SDA 	74
#define CSD_CONFIG_ADDR_A1_SDA_A0_SCL 	75
#define CSD_CONFIG_ADDR_A1_SCL_A0_GND 	76
#define CSD_CONFIG_ADDR_A1_SCL_A0_VS 	77
#define CSD_CONFIG_ADDR_A1_SCL_A0_SDA 	78
#define CSD_CONFIG_ADDR_A1_SCL_A0_SCL 	79

/******************/
/*CSD Alert Option*/
/******************/
/*Vshunt Voltage Over Voltage*/
#define CSD_ALERT_VShunt_OVA_ON 		1
#define CSD_ALERT_VShunt_OVA_OFF 		0
/*Vshunt Voltage Under Voltage*/
#define CSD_ALERT_VShunt_UVA_ON 		1
#define CSD_ALERT_VShunt_UVA_OFF 		0
/*VBus Voltage Over Voltage*/
#define CSD_ALERT_VBUS_OVA_ON 			1
#define CSD_ALERT_VBUS_OVA_OFF 			0
/*VBus Voltage Under Voltage*/
#define CSD_ALERT_VBUS_UVA_ON 			1
#define CSD_ALERT_VBUS_UVA_OFF 			0
/* Power Over Limit */
#define CSD_ALERT_Power_Over_ON 		1
#define CSD_ALERT_Power_Over_OFF 		0
/* ADC Coversion Ready */
#define CSD_ALERT_CoversionReady_ON 	1
#define CSD_ALERT_CoversionReady_OFF 	0
#define CSD_ALERT_CNVR_FLAG_ON 			1
#define CSD_ALERT_CNVR_FLAG_OFF 		0
/*  ALERT Function Flag and Alert latch Work Together */
#define CSD_ALERT_ALERT_FLAG_ON 		1
#define CSD_ALERT_ALERT_FLAG_OFF 		0
#define CSD_ALERT_ALERT_Latch_ON 		1
#define CSD_ALERT_ALERT_Latch_OFF 		0
/*Power over flow */
#define CSD_ALERT_OVF_ON 				1
#define CSD_ALERT_OVF_OFF 				0
/*Alert Pin */
#define CSD_ALERT_APOL_ActiveLow 		1
#define CSD_ALERT_APOL_ActiveHigh 		0
// clang-format on

/*CSD Basic Configuration struct*/
typedef struct {
  uint8_t DeviceADDR;
  uint8_t RST;
  uint8_t Average;
  uint8_t VBUS_Conv_Time;
  uint8_t VShunt_Conv_Time;
  uint8_t Mode;
  uint16_t CurrentLSB;
  uint16_t Rshunt;
} CSD_CONFIG;
/*CSD Basic Configuration struct*/

/*CSD ALERT Configuration struct*/
typedef struct {
  uint8_t VShunt_OVA;
  uint8_t VShuntUVA;
  uint8_t VBUS_OVA;
  uint8_t VBUS_UVA;
  uint8_t Power_OVER_Limit;
  uint8_t CNVR;
  uint8_t ALERT_FLAG;
  uint8_t CNVR_FLAG;
  uint8_t OVF;
  uint8_t APOL;
  uint8_t ALERT_Latch;
} CSD_ALERT;
/*CSD ALERT Configuration struct*/

void CSD203_Init(CSD_CONFIG *CSD203_CFG);            // Initialise CSD203 config
uint16_t CSD203_ReadVbus(CSD_CONFIG *CSD203_CFG);    // Read Vbus Voltage
uint16_t CSD203_ReadRshunt(CSD_CONFIG *CSD203_CFG);  // Read Rshunt
uint16_t CSD203_ReadPower(CSD_CONFIG *CSD203_CFG);   // Read Power
uint16_t CSD203_ReadCurrent(CSD_CONFIG *CSD203_CFG);  // Read Current

void IIC_DUT_W(uint8_t addr, uint8_t reg, uint16_t data);
uint16_t IIC_DUT_R(uint8_t addr, uint8_t reg);

bool CSD203MainInitFlag = false;
bool CSD203InInitFlag = false;
bool CSD203ExtInitFlag = false;

bool IICReadStatusFlag = false;

static uint16_t csd203extvbus = 0;

static CSD_CONFIG CSD203_MainSensorCFG;
static CSD_CONFIG CSD203_InSensorCFG;
static CSD_CONFIG CSD203_ExtSensorCFG;

bool I2C_CSD203_WriteRegister(uint8_t I2C_ADDR, uint8_t reg, uint8_t *buf,
                              uint8_t len)
{
  uint8_t uAddrAndBuf[6];
  uAddrAndBuf[0] = (uint8_t)(reg & 0x00FF);

  if (len > 0) {
    for (int i = 0; i < len; i++) {
      uAddrAndBuf[i + 1] = buf[i];
    }
  }

  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, I2C_ADDR, uAddrAndBuf, len + 1, 100) <
      0) {
    TRACE("I2C B1 ERROR: WriteRegister failed");
    return false;
  }
  return true;
}

bool I2C_CSD203_ReadRegister(uint8_t I2C_ADDR, uint8_t reg, uint8_t *buf,
                             uint8_t len)
{
  uint8_t uRegAddr[2];
  // uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uRegAddr[0] = (uint8_t)(reg & 0x00FF);

  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, I2C_ADDR, uRegAddr, 1, 100) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister write reg address failed");
    return false;
  }

  if (stm32_i2c_master_rx(TOUCH_I2C_BUS, I2C_ADDR, buf, len, 100) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister read reg address failed");
    return false;
  }
  return true;
}

void IIC_DUT_W(uint8_t addr, uint8_t reg, uint16_t data)
{
  uint8_t buf[2];

  buf[0] = data >> 8;
  buf[1] = data & 0xff;

  I2C_CSD203_WriteRegister(addr, reg, buf, 2);
}

uint16_t IIC_DUT_R(uint8_t addr, uint8_t reg)
{
  uint8_t buf[2];

  I2C_CSD203_ReadRegister(addr, reg, buf, 2);

  return buf[0] << 8 | buf[1];
}

void CSD203_Init(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  Data |= (CSD203_CFG->RST) << 15;
  Data |= (CSD203_CFG->Average) << 9;
  Data |= (CSD203_CFG->VBUS_Conv_Time) << 5;
  Data |= (CSD203_CFG->VShunt_Conv_Time) << 2;
  Data |= CSD203_CFG->Mode;

  ADDR = (CSD203_CFG->DeviceADDR);

  IIC_DUT_W(ADDR, CONFIGURATION, Data);

  Data = CalParam / ((CSD203_CFG->CurrentLSB) * (CSD203_CFG->Rshunt));
  IIC_DUT_W(ADDR, CALIBRATION, Data);
}

/*Read Vbus*/
uint16_t CSD203_ReadVbus(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, BUS_VOLTAGE);
  return Data;
}

/*Read Rshunt*/
uint16_t CSD203_ReadRshunt(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, SHUNT_VOLTAGE);
  return Data;
}

/*Read Power*/
uint16_t CSD203_ReadPower(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, POWER);
  return Data;
}

uint16_t CSD203_ReadCurrent(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, CURRENT);
  return Data;
}

uint16_t CSD203_ReadCONFIGURATION(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, CONFIGURATION);
  return Data;
}

uint16_t CSD203_ReadManufacturerID(CSD_CONFIG *CSD203_CFG)
{
  uint16_t Data = 0, ADDR = 0;
  ADDR = (CSD203_CFG->DeviceADDR);
  Data = IIC_DUT_R(ADDR, ManufacturerID);
  return Data;
}

void initCSD203(void)
{
  CSD203MainInitFlag = false;
  CSD203InInitFlag = false;
  CSD203ExtInitFlag = false;

  // TRACE("202 test ...");		//A0=CLK A1=SDA  RadioSky

  CSD203_MainSensorCFG.RST = CSD_CONFIG_RST;
  CSD203_MainSensorCFG.Average = CSD_CONFIG_Avg16;
  CSD203_MainSensorCFG.VBUS_Conv_Time = CSD_CONFIG_VBUS_CT1_1mS;
  CSD203_MainSensorCFG.VShunt_Conv_Time = CSD_CONFIG_VShunt_CT1_1mS;
  CSD203_MainSensorCFG.Rshunt = CSD_CONFIG_Rs_10mR;
  CSD203_MainSensorCFG.CurrentLSB = CSD_CONFIG_CurrentLsb1mA;
  CSD203_MainSensorCFG.Mode = 7;
  CSD203_MainSensorCFG.DeviceADDR = CSD_CONFIG_ADDR_A1_VS_A0_GND;  // 100 0100
  // CSD203_MainSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GND;

  CSD203_InSensorCFG.RST = CSD_CONFIG_RST;
  CSD203_InSensorCFG.Average = CSD_CONFIG_Avg16;
  CSD203_InSensorCFG.VBUS_Conv_Time = CSD_CONFIG_VBUS_CT1_1mS;
  CSD203_InSensorCFG.VShunt_Conv_Time = CSD_CONFIG_VShunt_CT1_1mS;
  CSD203_InSensorCFG.Rshunt = CSD_CONFIG_Rs_10mR;
  CSD203_InSensorCFG.CurrentLSB = CSD_CONFIG_CurrentLsb1mA;
  CSD203_InSensorCFG.Mode = 7;
  CSD203_InSensorCFG.DeviceADDR = CSD_CONFIG_ADDR_A1_VS_A0_VS;  // 100 0101
  // CSD203_InSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GND;

  CSD203_ExtSensorCFG.RST = CSD_CONFIG_RST;
  CSD203_ExtSensorCFG.Average = CSD_CONFIG_Avg16;
  CSD203_ExtSensorCFG.VBUS_Conv_Time = CSD_CONFIG_VBUS_CT1_1mS;
  CSD203_ExtSensorCFG.VShunt_Conv_Time = CSD_CONFIG_VShunt_CT1_1mS;
  CSD203_ExtSensorCFG.Rshunt = CSD_CONFIG_Rs_10mR;
  CSD203_ExtSensorCFG.CurrentLSB = CSD_CONFIG_CurrentLsb1mA;
  CSD203_ExtSensorCFG.Mode = 7;
  CSD203_ExtSensorCFG.DeviceADDR = CSD_CONFIG_ADDR_A1_GND_VS_GND;  // 100 0001
  // CSD203_ExtSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GND;

  CSD203_Init(&CSD203_MainSensorCFG);
  delay_ms(1);
  uint16_t cfg = CSD203_ReadManufacturerID(&CSD203_MainSensorCFG);
  if (cfg == ManIDCode) {
    CSD203_ReadCurrent(&CSD203_MainSensorCFG);
    CSD203MainInitFlag = true;
  }

  delay_ms(5);
  CSD203_Init(&CSD203_InSensorCFG);
  delay_ms(1);
  cfg = CSD203_ReadManufacturerID(&CSD203_InSensorCFG);
  if (cfg == ManIDCode) {
    CSD203_ReadCurrent(&CSD203_InSensorCFG);
    CSD203InInitFlag = true;
  }

  delay_ms(5);
  CSD203_Init(&CSD203_ExtSensorCFG);
  delay_ms(1);
  cfg = CSD203_ReadManufacturerID(&CSD203_ExtSensorCFG);
  if (cfg == ManIDCode) {
    CSD203_ReadCurrent(&CSD203_ExtSensorCFG);
    CSD203ExtInitFlag = true;
  }
}

uint16_t getCSD203BatteryVoltage(void)
{  // 1000=1000mV
  return csd203extvbus;
}

void readCSD203(void)
{  // 5ms
  static uint16_t GetSenSorStep = 0;

  if (IICReadStatusFlag == true) return;

  IICReadStatusFlag = true;
  if (GetSenSorStep == 0 && CSD203MainInitFlag == true) {
    CSD203_ReadCurrent(&CSD203_MainSensorCFG);
    CSD203_ReadVbus(&CSD203_MainSensorCFG);
  } else if (GetSenSorStep == 1 && CSD203InInitFlag == true) {
    CSD203_ReadCurrent(&CSD203_InSensorCFG);
    CSD203_ReadVbus(&CSD203_InSensorCFG);
  } else if (GetSenSorStep == 2 && CSD203ExtInitFlag == true) {
    CSD203_ReadCurrent(&CSD203_ExtSensorCFG);
    csd203extvbus = (CSD203_ReadVbus(&CSD203_ExtSensorCFG) * 1.25);
    // TRACE("Vbat=%d\r",csd203extvbus);
  }
  IICReadStatusFlag = false;
  if (++GetSenSorStep >= 3) GetSenSorStep = 0;
}
