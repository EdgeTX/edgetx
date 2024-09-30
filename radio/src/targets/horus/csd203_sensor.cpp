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

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "stm32_i2c_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_exti_driver.h"

#include "hal.h"
#include "csd203_sensor.h"
#include "delays_driver.h"
#include "timers_driver.h"

#include "rtos.h"
#include "edgetx_types.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

/* CSD203 Regsistor map */
#define CONFIGURATION   0X00
/*Read Only*/
#define SHUNT_VOLTAGE   0X01
#define BUS_VOLTAGE     0X02
#define POWER           0X03
#define CURRENT         0x04
/*Read Only*/
#define CALIBRATION     0x05
#define MASKENABLE      0x06
#define ALERTLIMIT      0x07

#define ManufacturerID 	0xFE	//Manufacturer ID Register
#define DieID 			0xFF	//Die ID Register

#define ManIDCode 		0x4153	//CHIP ID 


/*Calibration Calculation parameter*/
/* This Parameter Gain*10000K*/
#define CalParam        51200
/* ↓ add your shunt here ↓   */
#define CSD_CONFIG_Rs_1mR						1
#define CSD_CONFIG_Rs_2mR						2
#define CSD_CONFIG_Rs_5mR						5
#define CSD_CONFIG_Rs_10mR					10
#define CSD_CONFIG_Rs_20mR					20
#define CSD_CONFIG_Rs_50mR					50
#define CSD_CONFIG_CurrentLsb1mA   	10
#define CSD_CONFIG_CurrentLsb2mA   	20
#define CSD_CONFIG_CurrentLsb5mA   	50
#define CSD_CONFIG_CurrentLsb10mA  	100
#define CSD_CONFIG_CurrentLsb20mA  	200
/*Calibration Calculation parameter*/

/* CSD203 Regsistor Config*/
#define CSD_CONFIG_RST 		 1
#define CSD_CONFIG_UnRST 	 0

#define CSD_CONFIG_Avg1			0
#define CSD_CONFIG_Avg4			1
#define CSD_CONFIG_Avg16		2
#define CSD_CONFIG_Avg64		3
#define CSD_CONFIG_Avg128		4
#define CSD_CONFIG_Avg256		5
#define CSD_CONFIG_Avg512		6
#define CSD_CONFIG_Avg1024		7

#define CSD_CONFIG_VBUS_CT1_102uS 	0
#define CSD_CONFIG_VBUS_CT1_204uS 	1
#define CSD_CONFIG_VBUS_CT1_332uS 	2
#define CSD_CONFIG_VBUS_CT1_588uS 	3
#define CSD_CONFIG_VBUS_CT1_1mS 	4
#define CSD_CONFIG_VBUS_CT1_2mS 	5

#define CSD_CONFIG_VShunt_CT1_102uS 	0
#define CSD_CONFIG_VShunt_CT1_204uS 1
#define CSD_CONFIG_VShunt_CT1_332uS 2
#define CSD_CONFIG_VShunt_CT1_588uS 3
#define CSD_CONFIG_VShunt_CT1_1mS 	4
#define CSD_CONFIG_VShunt_CT1_2mS 	5

#define CSD_CONFIG_ShuntBus_CON 7

#define CSD_CONFIG_ADDR_A1_GND_A0_GND 64
#define CSD_CONFIG_ADDR_A1_GND_VS_GND 65
#define CSD_CONFIG_ADDR_A1_GND_SDA_GND 66
#define CSD_CONFIG_ADDR_A1_GND_SCL_GND 67
#define CSD_CONFIG_ADDR_A1_VS_A0_GND 68
#define CSD_CONFIG_ADDR_A1_VS_A0_VS 69
#define CSD_CONFIG_ADDR_A1_VS_A0_SDA 70
#define CSD_CONFIG_ADDR_A1_VS_A0_SCL 71
#define CSD_CONFIG_ADDR_A1_SDA_A0_GND 72
#define CSD_CONFIG_ADDR_A1_SDA_A0_VS 73
#define CSD_CONFIG_ADDR_A1_SDA_A0_SDA 74
#define CSD_CONFIG_ADDR_A1_SDA_A0_SCL 75
#define CSD_CONFIG_ADDR_A1_SCL_A0_GND 76
#define CSD_CONFIG_ADDR_A1_SCL_A0_VS 77
#define CSD_CONFIG_ADDR_A1_SCL_A0_SDA 78
#define CSD_CONFIG_ADDR_A1_SCL_A0_SCL 79
/******************/
/*CSD Alert Option*/
/******************/
/*Vshunt Voltage Over Voltage*/
#define CSD_ALERT_VShunt_OVA_ON 	1
#define CSD_ALERT_VShunt_OVA_OFF 	0
/*Vshunt Voltage Under Voltage*/
#define CSD_ALERT_VShunt_UVA_ON 	1
#define CSD_ALERT_VShunt_UVA_OFF 	0
/*VBus Voltage Over Voltage*/
#define CSD_ALERT_VBUS_OVA_ON 	1
#define CSD_ALERT_VBUS_OVA_OFF 	0
/*VBus Voltage Under Voltage*/
#define CSD_ALERT_VBUS_UVA_ON 	1
#define CSD_ALERT_VBUS_UVA_OFF 	0
/* Power Over Limit */
#define CSD_ALERT_Power_Over_ON 	1
#define CSD_ALERT_Power_Over_OFF 	0
/* ADC Coversion Ready */
#define CSD_ALERT_CoversionReady_ON 	1
#define CSD_ALERT_CoversionReady_OFF 	0
#define CSD_ALERT_CNVR_FLAG_ON 	1
#define CSD_ALERT_CNVR_FLAG_OFF 	0
/*  ALERT Function Flag and Alert latch Work Together */
#define CSD_ALERT_ALERT_FLAG_ON 	1
#define CSD_ALERT_ALERT_FLAG_OFF 	0
#define CSD_ALERT_ALERT_Latch_ON 	1
#define CSD_ALERT_ALERT_Latch_OFF 	0
/*Power over flow */
#define CSD_ALERT_OVF_ON 	1
#define CSD_ALERT_OVF_OFF 	0
/*Alert Pin */
#define CSD_ALERT_APOL_ActiveLow 	1
#define CSD_ALERT_APOL_ActiveHigh 0

/*CSD Basic Configuration struct*/
typedef struct{
	uint8_t DeviceADDR;
	uint8_t RST;
	uint8_t Average;
	uint8_t VBUS_Conv_Time;
	uint8_t VShunt_Conv_Time;
	uint8_t Mode;
	uint16_t CurrentLSB;
	uint16_t Rshunt;
}CSD_CONFIG;
/*CSD Basic Configuration struct*/

/*CSD ALERT Configuration struct*/
typedef struct{
	uint8_t DeviceADDR;
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
}CSD_ALERT;
/*CSD ALERT Configuration struct*/


/*Initial CSD203 Config*/
void CSD203_Alert(CSD_ALERT *CSD_ALT);
void CSD203_Init(CSD_CONFIG *CSD203_CFG);
/* Read Vbus Voltage*/
uint16_t CSD203_ReadVbus(CSD_CONFIG *CSD203_CFG);
/*Read Rshunt*/
uint16_t CSD203_ReadRshunt(CSD_CONFIG *CSD203_CFG);
/*Read Power*/
uint16_t CSD203_ReadPower(CSD_CONFIG *CSD203_CFG);
/*Read Current*/
uint16_t CSD203_ReadCurrent(CSD_CONFIG *CSD203_CFG);

void IIC_DUT_W(uint8_t addr,uint8_t reg,uint16_t data);
uint16_t IIC_DUT_R(uint8_t addr,uint8_t reg);


bool CSD203MainInitFlag = false;
bool CSD203InInitFlag = false;
bool CSD203ExtInitFlag = false;

//Seize IIC and avoid conflicts during IIC operations
bool IICReadStatusFlag = false;

bool IICReadWriteStatusFlag = false;

uint8_t voiceSwitch=1;

bool ExtModuleprotect=0;				//0=enable 1=disable

static uint16_t csd203extvbus=0;
static int16_t csd203extCurrent=0;
static int16_t realtimeextoldCurrent=0;
static int16_t realtimeextCurrent=0;

static uint16_t csd203mains2vbus=0;
static uint16_t csd203mains1vbus=0;

static int16_t csd203mainCurrent=0;
static int16_t csd203oldmainCurrent=0;

static int16_t csd203intCurrent=0;

static CSD_CONFIG CSD203_MainSensorCFG;
static CSD_CONFIG CSD203_InSensorCFG;
static CSD_CONFIG CSD203_ExtSensorCFG;

static CSD_ALERT CSD203_ExtAlertCFG;

static bool trigger_alert_status=false;


#define mixer_timer_select	1		//0=mixer loop   1=timer loop

//---------------------------------------------------

extern void EXTERNALMODULEOFF(void);

static void Trigger_ExtAlert()
{
  	EXTERNALMODULEOFF();
	trigger_alert_status=true;
}

void init_extalert()
{
  TRACE("init_EXTALERT_heartbeat");

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  
  pinInit.Pin = EXTALERT_TRIGGER_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP;

  LL_GPIO_Init(EXTALERT_TRIGGER_GPIO, &pinInit);
  
  LL_SYSCFG_SetEXTISource(EXTALERT_TRIGGER_EXTI_PORT, EXTALERT_TRIGGER_EXTI_SYS_LINE);
  
  stm32_exti_enable(EXTALERT_TRIGGER_EXTI_LINE, LL_EXTI_TRIGGER_FALLING,  //LL_EXTI_TRIGGER_FALLING LL_EXTI_TRIGGER_RISING
                    Trigger_ExtAlert);
}

bool I2C_CSD203_WriteRegister(uint8_t I2C_ADDR,uint8_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uAddrAndBuf[6];
  //uAddrAndBuf[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uAddrAndBuf[0] = (uint8_t)(reg & 0x00FF);

  if (len > 0) {
    for (int i = 0; i < len; i++) {
      uAddrAndBuf[i + 1] = buf[i];
    }
  }

  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, I2C_ADDR, uAddrAndBuf, len + 1,100) < 0) 
  {
    TRACE("I2C B1 ERROR: WriteRegister failed");
    return false;
  }
  return true;
}

bool I2C_CSD203_ReadRegister(uint8_t I2C_ADDR,uint8_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uRegAddr[2];
  //uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
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

void IIC_DUT_W(uint8_t addr,uint8_t reg,uint16_t data)
{
	uint8_t buf[2];

	buf[0]=data>>8;
	buf[1]=data&0xff;
	
	IICReadWriteStatusFlag=I2C_CSD203_WriteRegister(addr,reg, buf, 2);
}

uint16_t IIC_DUT_R(uint8_t addr,uint8_t reg)
{
	uint8_t buf[2];

	IICReadWriteStatusFlag=I2C_CSD203_ReadRegister(addr,reg, buf, 2);

    return buf[0]<<8|buf[1];
}

void CSD203_Init(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	Data|=(CSD203_CFG->RST)<<15;
	Data|=(CSD203_CFG->Average)<<9;
	Data|=(CSD203_CFG->VBUS_Conv_Time)<<5;
	Data|=(CSD203_CFG->VShunt_Conv_Time)<<2;
	Data|=CSD203_CFG->Mode;
	
	ADDR=(CSD203_CFG->DeviceADDR);
	
	IIC_DUT_W(ADDR,CONFIGURATION,Data);
	if(IICReadWriteStatusFlag==true)
	{
		IIC_DUT_W(ADDR,CONFIGURATION,Data);
	}
	Data=CalParam/((CSD203_CFG->CurrentLSB)*(CSD203_CFG->Rshunt));
	IIC_DUT_W(ADDR,CALIBRATION,Data);
	if(IICReadWriteStatusFlag==true)
	{
		IIC_DUT_W(ADDR,CALIBRATION,Data);
	}
	//printf("CALIBRATION Register :%x\r\n",Data);
}
void CSD203_InitAlert(CSD_ALERT *CSD203_ALERTCFG)
{
	uint16_t Data=0,ADDR=0;

	ADDR=(CSD203_ALERTCFG->DeviceADDR);
	
	//#define MAXCURRENT		100	//max LIMIT mA
	#define MAXCURRENT		3600	//max LIMIT mA
	#define LIMITCURRENT	((MAXCURRENT*CSD_CONFIG_Rs_10mR)/2.5)	//1200mA

	Data=LIMITCURRENT;
	IIC_DUT_W(ADDR,ALERTLIMIT,Data);
	if(IICReadWriteStatusFlag==true)
	{
		IIC_DUT_W(ADDR,ALERTLIMIT,Data);
	}
	
	Data|=CSD203_ALERTCFG->ALERT_Latch;
	Data|=CSD203_ALERTCFG->APOL<<1;
	Data|=CSD203_ALERTCFG->ALERT_FLAG<<4;
	Data|=CSD203_ALERTCFG->VShunt_OVA<<15;

	IIC_DUT_W(ADDR,MASKENABLE,Data);
	if(IICReadWriteStatusFlag==true)
	{
		IIC_DUT_W(ADDR,MASKENABLE,Data);

		IIC_DUT_R(ADDR,MASKENABLE);
	}
}
/*Read Vbus*/
uint16_t CSD203_ReadVbus(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,BUS_VOLTAGE);
	return Data;
}
/*Read Rshunt*/
uint16_t CSD203_ReadRshunt(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,SHUNT_VOLTAGE);
	return Data;
}
/*Read Power*/
uint16_t CSD203_ReadPower(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,POWER);
	return Data;
}

uint16_t CSD203_ReadCurrent(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,CURRENT);
	return Data;
}

uint16_t CSD203_ReadCONFIGURATION(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,CONFIGURATION);
	return Data;
}
uint16_t CSD203_ReadManufacturerID(CSD_CONFIG *CSD203_CFG)
{
	uint16_t Data=0,ADDR=0;
	ADDR=(CSD203_CFG->DeviceADDR);
	Data=IIC_DUT_R(ADDR,ManufacturerID);
	return Data;
}
void IICcsd203init(void)
{
#if !defined(HARDWARE_TOUCH)
  if (stm32_i2c_init(IIC_I2C_BUS, IIC_I2C_CLK_RATE) < 0) {
    //TRACE("GT911 ERROR: stm32_i2c_init failed");
    return;
  }
#endif
}
void ReIICcsd203init(void)
{
	if (stm32_i2c_deinit(TOUCH_I2C_BUS) < 0)
    	TRACE("I2C B1 ReInit - I2C DeInit failed");
  	//if (stm32_i2c_init(TOUCH_I2C_BUS, I2C_B1_CLK_RATE) < 0) {
    	//TRACE("GT911 ERROR: stm32_i2c_init failed");
    //	return;
  //	}
}

void initCSD203(void)
{
    CSD203MainInitFlag=false;
	CSD203InInitFlag=false;
	CSD203ExtInitFlag=false;

    //TRACE("202 test ...");		//A0=CLK A1=SDA  RadioSky
	
	CSD203_MainSensorCFG.RST=CSD_CONFIG_RST;
	CSD203_MainSensorCFG.Average=CSD_CONFIG_Avg1;
	CSD203_MainSensorCFG.VBUS_Conv_Time=CSD_CONFIG_VBUS_CT1_1mS;
	CSD203_MainSensorCFG.VShunt_Conv_Time=CSD_CONFIG_VShunt_CT1_1mS;
	CSD203_MainSensorCFG.Rshunt=CSD_CONFIG_Rs_10mR;
	CSD203_MainSensorCFG.CurrentLSB=CSD_CONFIG_CurrentLsb1mA;
	CSD203_MainSensorCFG.Mode=7;
	CSD203_MainSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_VS_A0_GND;				//100 0100
	//CSD203_MainSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GND;

	CSD203_InSensorCFG.RST=CSD_CONFIG_RST;
	CSD203_InSensorCFG.Average=CSD_CONFIG_Avg1;
	CSD203_InSensorCFG.VBUS_Conv_Time=CSD_CONFIG_VBUS_CT1_1mS;
	CSD203_InSensorCFG.VShunt_Conv_Time=CSD_CONFIG_VShunt_CT1_1mS;
	CSD203_InSensorCFG.Rshunt=CSD_CONFIG_Rs_10mR;
	CSD203_InSensorCFG.CurrentLSB=CSD_CONFIG_CurrentLsb1mA;
	CSD203_InSensorCFG.Mode=7;
	CSD203_InSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_VS_A0_VS;				//100 0101
	//CSD203_InSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GNDu;

	CSD203_ExtSensorCFG.RST=CSD_CONFIG_RST;
	CSD203_ExtSensorCFG.Average=CSD_CONFIG_Avg1;
	CSD203_ExtSensorCFG.VBUS_Conv_Time=CSD_CONFIG_VBUS_CT1_588uS;
	CSD203_ExtSensorCFG.VShunt_Conv_Time=CSD_CONFIG_VShunt_CT1_102uS;
	CSD203_ExtSensorCFG.Rshunt=CSD_CONFIG_Rs_10mR;
	CSD203_ExtSensorCFG.CurrentLSB=CSD_CONFIG_CurrentLsb1mA;
	CSD203_ExtSensorCFG.Mode=7;
	CSD203_ExtSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_VS_GND;				//100 0001
	//CSD203_ExtSensorCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_A0_GND;
	
	CSD203_Init(&CSD203_MainSensorCFG);
	delay_ms(1);
	uint16_t cfg=CSD203_ReadManufacturerID(&CSD203_MainSensorCFG);
	if(cfg==ManIDCode)
	{
		CSD203_ReadCurrent(&CSD203_MainSensorCFG);
		CSD203MainInitFlag=true;
	}

	delay_ms(5);
	CSD203_Init(&CSD203_InSensorCFG);
	delay_ms(1);
	cfg=CSD203_ReadManufacturerID(&CSD203_InSensorCFG);
	if(cfg==ManIDCode)
	{
		CSD203_ReadCurrent(&CSD203_InSensorCFG);
		CSD203InInitFlag=true;
	}
	
	delay_ms(5);
	CSD203_Init(&CSD203_ExtSensorCFG);
	delay_ms(1);
	cfg=CSD203_ReadManufacturerID(&CSD203_ExtSensorCFG);
	if(cfg==ManIDCode)
	{	
		CSD203_ReadCurrent(&CSD203_ExtSensorCFG);
		CSD203ExtInitFlag=true;

		//ALERT LIMIT
		CSD203_ExtAlertCFG.DeviceADDR=CSD_CONFIG_ADDR_A1_GND_VS_GND;	//100 0001
		CSD203_ExtAlertCFG.APOL=CSD_ALERT_APOL_ActiveHigh; 				//CSD_ALERT_APOL_ActiveHigh CSD_ALERT_APOL_ActiveLow
		CSD203_ExtAlertCFG.ALERT_FLAG=CSD_ALERT_ALERT_FLAG_ON;
		CSD203_ExtAlertCFG.VShunt_OVA=CSD_ALERT_VShunt_OVA_ON;
		CSD203_ExtAlertCFG.ALERT_Latch=CSD_ALERT_ALERT_Latch_ON;

		CSD203_InitAlert(&CSD203_ExtAlertCFG);	//alert init

	#if defined(RADIO_V16)	//Only V16 has hardware interrupt pins
		init_extalert();	//external alert int init
	#endif
	}
}
uint16_t getCSD203BatteryVoltage(void)
{//1000=1000mV
	return csd203mains2vbus;
}
uint16_t getcsd203BatteryS1Voltage(void)
{//1000=1000mV
	return csd203mains1vbus;
}

int16_t getcsd203MainCurrent(void)
{//1000=1000mA
	if(csd203mainCurrent<0)return -csd203mainCurrent;
	return (int16_t)csd203mainCurrent;
}
int16_t getcsd203OldMainCurrent(void)
{//1000=1000mA
	return (int16_t)csd203oldmainCurrent;
}

uint16_t getcsd203extVoltage(void)
{//1000=1000mV
	return csd203extvbus;
}
int16_t getcsd203extCurrent(void)
{//1000=1000mA
	if(csd203extCurrent<0)return 0;
	return (int16_t)csd203extCurrent;
}
int16_t getcsd203IntCurrent(void)
{//1000=1000mA
	if(csd203intCurrent<0)return 0;
	return (int16_t)csd203intCurrent;
}

bool Getcsd203iicStatus(void)
{//get iic status
	return IICReadStatusFlag;
}

extern void AUDIOExtWarn(void);
extern void AUDIOExtWarncut(void);

extern uint16_t MaxAbnormalCurrent();
extern uint8_t getextModuleprotect();

static uint8_t EXTMODULE_WarningFlag=0;
/*
Comment: Resolve the abnormal situation of 
voice input when using TIMER loop tasks
*/
void Getcsdloop(void)
{
#if mixer_timer_select		//0=mixer loop   1=timer loop
	if(EXTMODULE_WarningFlag==1)
	{
		EXTMODULE_WarningFlag=0;
		AUDIOExtWarncut();
	}
	if(EXTMODULE_WarningFlag==2)
	{
		EXTMODULE_WarningFlag=0;
		AUDIOExtWarn();
	}
#endif
}

void readCSD203(void)
{//1ms
	static uint16_t GetSenSordelay=0;
	static uint16_t GetSenSorStep=0;

	static uint16_t GetSenSorerrct=0;
	static uint16_t GetSenSorerrcount=0;

	static uint16_t SenSorErrorcount=0;

	static uint32_t wait_delay=0,AUDIOExtWarnCount=0;

	uint16_t volt;
	int16_t current;

#if mixer_timer_select		//0=mixer loop   1=timer loop

#else
	#define GetCSD203FQ		5		//wait ms

	if ((uint32_t)(timersGetMsTick() - wait_delay) <= GetCSD203FQ)return;
	wait_delay=timersGetMsTick();
#endif

	//RTRACE("C");

	if(GetSenSorerrcount<2&&++GetSenSordelay>=1)GetSenSordelay=0;
	else{
		if(GetSenSorerrcount>=2)GetSenSorerrcount++;
		if(GetSenSorerrcount>=(500*5))GetSenSorerrcount=0;
		return;
	}

	if(IICReadStatusFlag == true)return;
	IICReadStatusFlag=true;

	if(trigger_alert_status==true)
	{
		trigger_alert_status=false;
		
		if(SenSorErrorcount==0)
		{
			//AUDIOExtWarncut();
			EXTMODULE_WarningFlag=1;

			SenSorErrorcount=1000;
		}

		IIC_DUT_R(CSD_CONFIG_ADDR_A1_GND_VS_GND,MASKENABLE);
	}

	if(CSD203ExtInitFlag==true){
		current=CSD203_ReadCurrent(&CSD203_ExtSensorCFG)/1;
		//int16_t RShunt = CSD203_ReadRshunt(&CSD203_ExtSensorCFG);
		int16_t MASKFLAG = IIC_DUT_R(CSD_CONFIG_ADDR_A1_GND_VS_GND,MASKENABLE);
		
		if(IICReadWriteStatusFlag==true){
			realtimeextoldCurrent=realtimeextCurrent;
			realtimeextCurrent=current;

			//#define MaxAbnormalCurrent	2000	//500=500mA
			//#define MaxAbnormalCurrent g_eeGeneral.extmaxcurrent

			if(SenSorErrorcount)SenSorErrorcount--;
			else if(getextModuleprotect()==0&&(realtimeextoldCurrent>=(MaxAbnormalCurrent()*0.5)&&current>=MaxAbnormalCurrent()||MASKFLAG&0X10))
			{//warning cut
				EXTERNALMODULEOFF();

			#if mixer_timer_select		//0=mixer loop   1=timer 1ms loop
				EXTMODULE_WarningFlag=1;
			#else
				AUDIOExtWarncut();
			#endif

				realtimeextCurrent=0;

				SenSorErrorcount=1000;
			}
			else if(AUDIOExtWarnCount==0&&getextModuleprotect()==0&&realtimeextCurrent>=(MaxAbnormalCurrent()*0.8))
			{//warning
				AUDIOExtWarnCount=9000;

			#if mixer_timer_select		//0=mixer loop   1=timer 1ms loop
				EXTMODULE_WarningFlag=2;
			#else
				AUDIOExtWarn();
			#endif
			}
			else
			{
				if(AUDIOExtWarnCount)
					AUDIOExtWarnCount--;
			}
			if(current<0)current=csd203extCurrent*0.9;
			csd203extCurrent=(csd203extCurrent*9+current*1)/10;
		}//
	}

	if(IICReadWriteStatusFlag==false){//err
		if(++GetSenSorerrct>=2){
			if(GetSenSorerrcount==0){
				ReIICcsd203init();
			}
			//RTRACE("ReIICc\r\n");
			GetSenSorerrcount++;
		}
		IICReadStatusFlag = false;
		return;
	}
	GetSenSorerrct=0;
	GetSenSorerrcount=0;

	if(GetSenSorStep==5&&CSD203MainInitFlag==true){
		current=CSD203_ReadCurrent(&CSD203_MainSensorCFG)/1;
		if(IICReadWriteStatusFlag==true){
			//if(current<0)current=csd203mainCurrent*0.9;
			csd203mainCurrent=(csd203mainCurrent*8+current*2)/10;
			if(current>160)
				csd203oldmainCurrent=csd203mainCurrent;
		}
		volt=CSD203_ReadVbus(&CSD203_MainSensorCFG)*1.25;
		if(IICReadWriteStatusFlag==true){
			csd203mains1vbus=(csd203mains1vbus*9+volt*1)/10;
		}
	}
	else if(GetSenSorStep==10&&CSD203InInitFlag==true){
		current=CSD203_ReadCurrent(&CSD203_InSensorCFG)/1;
		if(IICReadWriteStatusFlag==true){
			if(current<0)current=csd203intCurrent*0.9;
			csd203intCurrent=(csd203intCurrent*8+current*2)/10;
		}
		volt=CSD203_ReadVbus(&CSD203_InSensorCFG)*1.25;
		if(IICReadWriteStatusFlag==true){
			csd203extvbus=(csd203extvbus*9+volt*1)/10;
		}
	}
	else if(GetSenSorStep==15&&CSD203ExtInitFlag==true){
		//CSD203_ReadCurrent(&CSD203_ExtSensorCFG);
		volt=(CSD203_ReadVbus(&CSD203_ExtSensorCFG)*1.25);
		if(IICReadWriteStatusFlag==true){
			csd203mains2vbus=(csd203mains2vbus*9+volt*1)/10;
		}
		//RTRACE("Vbat=%d Ebat=%d Current=%d\r",csd203extvbus,ExtVbus,csd203extCurrent);
	}
	if(++GetSenSorStep>=20)GetSenSorStep=0;
	if(IICReadWriteStatusFlag==false){//err
		ReIICcsd203init();
		//RTRACE("ReIICc2\r\n");
	}	
	IICReadStatusFlag = false;
}

