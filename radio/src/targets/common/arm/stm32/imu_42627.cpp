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
#include "hal/i2c_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "imu_42627.h"

#include "imu4.h"

#include "rtos.h"
#include "edgetx_types.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

#define mSDO_LEVEL 0

#if mSDO_LEVEL
	#define MPU3050_I2C_WR_ADDR 0xD2  //3050
	#define MPU3050_I2C_RD_ADDR 0xD3
	
	#define MPU6050_I2C_WR_ADDR 0xD2  //6050
	#define MPU6050_I2C_RD_ADDR 0xD3
#else
	//#define MPU3050_I2C_WR_ADDR 0xD0
	//#define MPU3050_I2C_RD_ADDR 0xD1
	
	#define MPU6050_I2C_WR_ADDR 0xD2  //40607 40608
	#define MPU6050_I2C_RD_ADDR 0xD3
	
	#define MPU6050_DVR			0x68  //0x68<<1=0xD0 
	
#endif

#define MPU3050_ADDRESS         0x69        /* MPU3050  <<1=d2 */

#define MPU3050_PWR_MGM_POS     6
#define MPU3050_PWR_MGM_MASK    0x40

#define MPU3050_AUTO_DELAY      1000

#define MPU3050_MIN_VALUE       -32768
#define MPU3050_MAX_VALUE       32767


/*mpu3050 */
#define MPU42627_ID            0x75         //0x68+bit0
#define MPU3050_ID             0x00         //0x68+bit0

#define MPU3050_PID            0x00         // id，
#define MPU3050_OFFSET_XH      0x0C 
#define MPU3050_OFFSET_XL      0x0D         //X
#define MPU3050_OFFSET_YH      0x0E 
#define MPU3050_OFFSET_YL      0x0F         //Y
#define MPU3050_OFFSET_ZH      0x10 
#define MPU3050_OFFSET_ZL      0x11         //Z
#define MPU3050_FIFO_EN        0x12         //FIFO 0x00 
#define MPU3050_AUX_VDD        0x13         // 0x00（vlogic） 0x04（vdd）
#define MPU3050_AUX_ADD        0x14         //7D7~D0）D8：clkout
#define MPU3050_SAMP_DIV       0x15         //0x00   Fsam=Fin/（divider+1） Fin=1k or 8k
#define MPU3050_DLPF           0x16         // 000 00 000
#define MPU3050_INT            0x17         //0x00 
#define MPU3050_AUX_RADD       0x18         //0x00
#define MPU3050_INTS           0x1A         //
#define MPU3050_TEMP_H         0x1B         //
#define MPU3050_TEMP_L         0x1C 
#define MPU3050_XOUTH          0x1D         //x
#define MPU3050_XOUTL          0x1E 
#define MPU3050_YOUTH          0x1F         //y
#define MPU3050_YOUTL          0x20 
#define MPU3050_ZOUTH          0x21         //z
#define MPU3050_ZOUTL          0x22 
#define MPU3050_AXOUTH         0x23         //x
#define MPU3050_AXOUTL         0x24
#define MPU3050_AYOUTH         0x25         //y
#define MPU3050_AYOUTL         0x26 
#define MPU3050_AZOUTH         0x27         //z
#define MPU3050_AZOUTL         0x28 
#define MPU3050_FIFO_CH        0x3A         //FIFO
#define MPU3050_FIFO_CL        0x3B 
#define MPU3050_FIFO_DATA      0x3C         //FIFO
#define MPU3050_USER_CON       0x3D         //
#define MPU3050_POW_MGM        0x3E         //

#define MPU3050_XYZOUT         0x9D         //x

#define ICM42627_RA_TEMP_OUT_H      0x1D
#define ICM42627_RA_TEMP_OUT_L       0x1E
#define ICM42627_RA_ACCEL_XOUT_H     0x1F
#define ICM42627_RA_ACCEL_XOUT_L     0x20
#define ICM42627_RA_ACCEL_YOUT_H     0x21
#define ICM42627_RA_ACCEL_YOUT_L     0x22
#define ICM42627_RA_ACCEL_ZOUT_H     0x23
#define ICM42627_RA_ACCEL_ZOUT_L     0x24

#define ICM42627_RA_GYRO_XOUT_H      0x25
#define ICM42627_RA_GYRO_XOUT_L      0x26
#define ICM42627_RA_GYRO_YOUT_H      0x27
#define ICM42627_RA_GYRO_YOUT_L      0x28
#define ICM42627_RA_GYRO_ZOUT_H      0x29
#define ICM42627_RA_GYRO_ZOUT_L      0x2A

// BANK 0

#define ICM40608_DRIVCE_CONFIG    		0x11
#define ICM40608_DRIVER_CONFIG    		0x13

#define ICM40608_PWR_MGMT0				0X4E

#define ICM40608_RA_GYRO_CONFIG      	0x4F
#define ICM40608_RA_ACCEL_CONFIG      	0x50

#define ICM40608_RA_GYRO_CONFIG1      	0x51
#define ICM40608_RA_ACCEL_CONFIG0      	0x52
#define ICM40608_RA_ACCEL_CONFIG1      	0x53
#define ICM40608_TMST_CONFIG      		0x54

#define ICM40608_WHOAMI      			0x75
#define ICM40608_BANKSELECT      		0x76

// BANK 1
#define ICM40608_GYRO_CONFIG_STATIC2     0x0B
#define ICM40608_GYRO_CONFIG_STATIC3     0x0C
#define ICM40608_GYRO_CONFIG_STATIC4     0x0D
#define ICM40608_GYRO_CONFIG_STATIC5     0x0E
#define ICM40608_GYRO_CONFIG_STATIC6     0x0F
#define ICM40608_GYRO_CONFIG_STATIC7     0x10
#define ICM40608_GYRO_CONFIG_STATIC8     0x11
#define ICM40608_GYRO_CONFIG_STATIC9     0x12
#define ICM40608_GYRO_CONFIG_STATIC10    0x13
#define ICM40608_GYRO_CONFIG_STATIC3     0x0C

// BANK 2
#define ICM40608_ACCEL_CONFIG_STATIC2    0x03
#define ICM40608_ACCEL_CONFIG_STATIC3    0x04
#define ICM40608_ACCEL_CONFIG_STATIC4    0x05

//#define ICM40608_ACCEL_CONFIG_STATIC2    0x03

#define ICM40608_REG_BANK_SEL			0X76

enum icm40608_gyro_fsr_e {
    ICM40608_FSR_2000DPS = 0,
    ICM40608_FSR_1000DPS,
    ICM40608_FSR_500DPS,
    ICM40608_FSR_250DPS,
    NUM_ICM40608_GYRO_FSR
};
enum icm40608_gyro_ord_e {
	
	ICM40608_gyroORD_Reserved1 = 0,
	ICM40608_gyroORD_Reserved2,
	ICM40608_gyroORD_Reserved3,
    ICM40608_gyroORD_8K,
    ICM40608_gyroORD_4K,
    ICM40608_gyroORD_2K,
    ICM40608_gyroORD_1K,
    ICM40608_gyroORD_200HZ,
};

enum icm40608_accel_fsr_e {
    ICM40608_FSR_18G = 0,
    ICM40608_FSR_8G,
    ICM40608_FSR_4G,
    ICM40608_FSR_2G,
    NUM_ICM40608_ACCEL_FSR
};

enum icm40608_accel_ord_e {
	
	ICM40608_accelORD_Reserved1 = 0,
	ICM40608_accelORD_Reserved2,
	ICM40608_accelORD_Reserved3,
    ICM40608_accelORD_8K,
    ICM40608_accelORD_4K,
    ICM40608_accelORD_2K,
    ICM40608_accelORD_1K,
    ICM40608_accelORD_200HZ,
};


enum icm40608_GYRO_CONFIG1_e {
	
	GYRO_UI_FILT_ORD1stOrder = 0,
	GYRO_UI_FILT_ORD2stOrder,
	GYRO_UI_FILT_ORD3stOrder,
	GYRO_UI_FILT_ORD_Reserved3,
	
	GYRO_DEC2_M2_ORD_ORD1stOrder = 0,
	GYRO_DEC2_M2_ORD_ORD2stOrder,
	GYRO_DEC2_M2_ORD_ORD3stOrder,
	GYRO_DEC2_M2_ORD_ORD_Reserved3,
	
};

enum icm40608_GYRO_ACCEL_CONFIG0_e {
	
	ACCEL_UI_FILT_BW0 = 0,	// BW=ODR/2
	ACCEL_UI_FILT_BW1,		// BW=max(400Hz, ODR)/4 (default)
	ACCEL_UI_FILT_BW2,		// BW=max(400Hz, ODR)/5
	ACCEL_UI_FILT_BW3,		// BW=max(400Hz, ODR)/8
	ACCEL_UI_FILT_BW4,		// BW=max(400Hz, ODR)/10
	ACCEL_UI_FILT_BW5,		// BW=max(400Hz, ODR)/16
	ACCEL_UI_FILT_BW6,		// BW=max(400Hz, ODR)/20
	ACCEL_UI_FILT_BW7,		// BW=max(400Hz, ODR)/40
	
	ACCEL_UI_FILT_BW8,		// Reserved
	ACCEL_UI_FILT_BW9,
	ACCEL_UI_FILT_BW10,
	ACCEL_UI_FILT_BW11,
	ACCEL_UI_FILT_BW12,
	ACCEL_UI_FILT_BW13,	// Reserved
	ACCEL_UI_FILT_BW14,
	ACCEL_UI_FILT_BW15,
	
	GYRO_UI_FILT_BW0 = 0,	// BW=ODR/2
	GYRO_UI_FILT_BW1,		// BW=max(400Hz, ODR)/4 (default)
	GYRO_UI_FILT_BW2,		// BW=max(400Hz, ODR)/5
	GYRO_UI_FILT_BW3,		// BW=max(400Hz, ODR)/8
	GYRO_UI_FILT_BW4,		// BW=max(400Hz, ODR)/10
	GYRO_UI_FILT_BW5,		// BW=max(400Hz, ODR)/16
	GYRO_UI_FILT_BW6,		// BW=max(400Hz, ODR)/20
	GYRO_UI_FILT_BW7,		// BW=max(400Hz, ODR)/40
	
	GYRO_UI_FILT_BW8,		// Reserved
	GYRO_UI_FILT_BW9,
	GYRO_UI_FILT_BW10,
	GYRO_UI_FILT_BW11,
	GYRO_UI_FILT_BW12,
	GYRO_UI_FILT_BW13,		// Reserved
	GYRO_UI_FILT_BW14,
	GYRO_UI_FILT_BW15,
	
};

enum icm40608_ACCEL_CONFIG1_e {
	
	ACCEL_UI_FILT_ORD1stOrder = 0,
	ACCEL_UI_FILT_ORD2stOrder,
	ACCEL_UI_FILT_ORD3stOrder,
	ACCEL_UI_FILT_ORD_Reserved3,
	
	ACCEL_DEC2_M2_ORD_ORD1stOrder = 0,
	ACCEL_DEC2_M2_ORD_ORD2stOrder,
	ACCEL_DEC2_M2_ORD_ORD3stOrder,
	ACCEL_DEC2_M2_ORD_ORD_Reserved3,
};

enum icm40608_BANKSEL_e 
{	
	Bank0 = 0,
	Bank1,
	Bank2,
	Bank3,
	Bank4,
};


/*---- MPU 'FIFO_EN'(0x12) ----*/
#define BIT_TEMP_OUT                0x80
#define BIT_GYRO_XOUT               0x40
#define BIT_GYRO_YOUT               0x20
#define BIT_GYRO_ZOUT               0x10
#define BIT_ACCEL_XOUT              0x08
#define BIT_ACCEL_YOUT              0x04
#define BIT_ACCEL_ZOUT              0x02
#define BIT_AUX_1OUT                0x01

/*---- MPU 'AUX_VDDIO'  (0x13) ----*/
#define BIT_AUX_OUT                0x04
//#define BIT_AUX_2OUT                0x02
//#define BIT_AUX_3OUT                0x01

/*---- MPU 'AUX_ADD' (0x14) ----*/
#define BIT_CLKOUT_EN               0x80
#define BITS_AUX_ADD                ADXL345_ADDRESS

/*---- MPU 'DLPF_FS_SYNC' register (16) ----*/
#define BITS_EXT_SYNC_SET           0xE0
#define BITS_FS_SEL                 0x18
#define BITS_DLPF_CFG               0x07

/*---- MPU 'DLPF_FS_SYNC' register (16) ----*/
#define BITS_EXT_SYNC_NONE          0x00
#define BITS_EXT_SYNC_TEMP          0x20
#define BITS_EXT_SYNC_GYROX         0x40
#define BITS_EXT_SYNC_GYROY         0x60
#define BITS_EXT_SYNC_GYROZ         0x80
#define BITS_EXT_SYNC_ACCELX        0xA0
#define BITS_EXT_SYNC_ACCELY        0xC0
#define BITS_EXT_SYNC_ACCELZ        0xE0
#define BITS_EXT_SYNC_MASK          0xE0
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_MASK                0x18
#define BITS_DLPF_CFG_256HZ_NOLPF2  0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07

/*---- MPU 'INT_CFG' register (17) ----*/
#define BIT_ACTL                    0x80
#define BIT_OPEN                    0x40
#define BIT_LATCH_INT_EN            0x20
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_MPU_RDY_EN              0x04
#define BIT_DMP_INT_EN              0x02
#define BIT_RAW_RDY_EN              0x01

/*---- MPU 'INT_STATUS' register (1A) ----*/
#define BIT_MPU_RDY                 0x04
#define BIT_DMP_INT                 0x02
#define BIT_RAW_RDY                 0x01

/*---- MPU 'BANK_SEL' register (37) ----*/
#define BITS_MEM_SEL                0x0f

/*---- MPU 'USER_CTRL' register (3D) ----*/
#define BIT_DMP_EN                  0x80
#define BIT_FIFO_EN                 0x40
#define BIT_AUX_IF_EN               0x20
#define BIT_AUX_RD_LENG             0x10
#define BIT_AUX_IF_RST              0x08
#define BIT_DMP_RST                 0x04
#define BIT_FIFO_RST                0x02
#define BIT_GYRO_RST                0x01

/*---- MPU 'PWR_MGM' register (3E) ----*/
#define BIT_H_RESET                 0x80
#define BIT_SLEEP                   0x40
#define BIT_STBY_XG                 0x20
#define BIT_STBY_YG                 0x10
#define BIT_STBY_ZG                 0x08
#define BITS_CLKSEL                 0x07

//#define MPU3050_SCALE   131   //  14.375 (LSB/(deg/sec))
//#define MPU3050_SCALE   65.5   //  14.375 (LSB/(deg/sec))
//#define MPU3050_SCALE   32.8   //  14.375 (LSB/(deg/sec)) 1000度
#define MPU3050_SCALE   16.4

bool IMUSTATUS=false;
bool IMURUNFLAG=false;

uint8_t IMU42627ID=0;
bool IMUReadWriteStatusFlag = false;

bool I2C_42627_WriteRegister(uint8_t I2C_ADDR,uint8_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uAddrAndBuf[6];
  //uAddrAndBuf[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uAddrAndBuf[0] = (uint8_t)(reg & 0x00FF);

  if (len > 0) {
    for (int i = 0; i < len; i++) {
      uAddrAndBuf[i + 1] = buf[i];
    }
  }

  if (stm32_i2c_master_tx(I2C_Bus_1, I2C_ADDR, uAddrAndBuf, len + 1,100) < 0) 
  {
    TRACE("I2C B1 ERROR: WriteRegister failed");
    return false;
  }
  return true;
}

bool I2C_42627_ReadRegister(uint8_t I2C_ADDR,uint8_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uRegAddr[2];
  //uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uRegAddr[0] = (uint8_t)(reg & 0x00FF);

  if (stm32_i2c_master_tx(I2C_Bus_1, I2C_ADDR, uRegAddr, 1, 100) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister write reg address failed");
    return false;
  }

  if (stm32_i2c_master_rx(I2C_Bus_1, I2C_ADDR, buf, len, 100) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister read reg address failed");
    return false;
  }
  return true;
}

void ITG3200_Write(uint8_t reg,uint16_t data)
{
	uint8_t buf[2];

	buf[1]=data>>8;
	buf[0]=data&0xff;
	
	IMUReadWriteStatusFlag=I2C_42627_WriteRegister(MPU6050_I2C_WR_ADDR>>1,reg, buf, 1);
}

uint8_t ITG3200_Read(uint8_t reg)
{
	uint8_t buf[2];

	IMUReadWriteStatusFlag=I2C_42627_ReadRegister(MPU6050_I2C_WR_ADDR>>1,reg, buf, 1);

    return buf[0];
}
bool ITG3200_ReadImu(uint8_t *buf)
{
	IMUReadWriteStatusFlag=I2C_42627_ReadRegister(MPU6050_I2C_WR_ADDR>>1,ICM42627_RA_TEMP_OUT_H, buf, 14);

    return true;
}

void IICimu42627init(void)
{
	if (i2c_init(I2C_Bus_1) < 0) {
    return;  //err
  }
}
void imu42627Init(void)
{
	ITG3200_Write(ICM40608_DRIVCE_CONFIG,0x01); 		//Reset MPU
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_DRIVCE_CONFIG,0x01); 	//Reset MPU
	delay_ms(150);
	
	ITG3200_Write(ICM40608_DRIVCE_CONFIG,0x01); 		//Reset MPU
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_DRIVCE_CONFIG,0x01); //Reset MPU
	delay_ms(150);
	
	ITG3200_Write(ICM40608_PWR_MGMT0,0x0F); 			//
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_PWR_MGMT0,0x0F); 		//
	delay_ms(150);
	ITG3200_Write(ICM40608_PWR_MGMT0,0x2F);
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_PWR_MGMT0,0x2F);
	delay_ms(150);

	//0=200HZ
	ITG3200_Write(ICM40608_RA_GYRO_CONFIG,ICM40608_FSR_2000DPS<<5|ICM40608_gyroORD_200HZ); //250HZ 1K/(1+3)=250hZ=4.0MS
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_GYRO_CONFIG,ICM40608_FSR_2000DPS<<5|ICM40608_gyroORD_200HZ);
	delay_us(100);
	ITG3200_Write(ICM40608_RA_GYRO_CONFIG,ICM40608_FSR_2000DPS<<5|ICM40608_gyroORD_200HZ); //250HZ 1K/(1+3)=250hZ=4.0MS
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_GYRO_CONFIG,ICM40608_FSR_2000DPS<<5|ICM40608_gyroORD_200HZ);
	delay_us(100);
	ITG3200_Write(ICM40608_RA_ACCEL_CONFIG,ICM40608_FSR_4G<<5|ICM40608_accelORD_200HZ); //250HZ 1K/(1+3)=250hZ=4.0MS
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_ACCEL_CONFIG,ICM40608_FSR_4G<<5|ICM40608_accelORD_200HZ);
	delay_us(100);
	ITG3200_Write(ICM40608_RA_ACCEL_CONFIG,ICM40608_FSR_4G<<5|ICM40608_accelORD_200HZ); //250HZ 1K/(1+3)=250hZ=4.0MS
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_ACCEL_CONFIG,ICM40608_FSR_4G<<5|ICM40608_accelORD_200HZ);
	//BANK0
	delay_us(100);
	ITG3200_Write(ICM40608_RA_GYRO_CONFIG1,GYRO_UI_FILT_ORD3stOrder<<2|GYRO_DEC2_M2_ORD_ORD3stOrder); //设置 GYRO_UI_FILT_ORD
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_GYRO_CONFIG1,GYRO_UI_FILT_ORD3stOrder<<2|GYRO_DEC2_M2_ORD_ORD3stOrder); 
	delay_us(100);
	ITG3200_Write(ICM40608_RA_ACCEL_CONFIG0,ACCEL_UI_FILT_BW2<<4|GYRO_UI_FILT_BW2); //ACCEL_UI_FILT_BW
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_ACCEL_CONFIG0,ACCEL_UI_FILT_BW2<<4|GYRO_UI_FILT_BW2); 

	delay_us(100);
	ITG3200_Write(ICM40608_RA_ACCEL_CONFIG1,ACCEL_UI_FILT_ORD3stOrder<<3|ACCEL_DEC2_M2_ORD_ORD3stOrder<<1); //设置 ACCEL_UI_FILT_ORD
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_RA_ACCEL_CONFIG1,ACCEL_UI_FILT_ORD3stOrder<<3|ACCEL_DEC2_M2_ORD_ORD3stOrder<<1);
	//BANK1
	delay_us(100);
	ITG3200_Write(ICM40608_BANKSELECT,Bank1); //BANK
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_BANKSELECT,Bank1); //BANK

	delay_us(100);
	ITG3200_Write(ICM40608_GYRO_CONFIG_STATIC2,0x00); //GYRO_AAF_DIS DISABLE
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_GYRO_CONFIG_STATIC2,0x00); //GYRO_AAF_DIS DISABLE
	
	//BANK0
	delay_us(100);
	ITG3200_Write(ICM40608_BANKSELECT,Bank0); //ACCEL_UI_FILT_ORD
	if(IMUReadWriteStatusFlag==false)
		ITG3200_Write(ICM40608_BANKSELECT,Bank0); //ACCEL_UI_FILT_ORD
	delay_ms(238);
	IMU42627ID=ITG3200_Read(MPU42627_ID);
	if(IMUReadWriteStatusFlag==false)
		IMU42627ID=ITG3200_Read(MPU42627_ID);
	delay_ms(238);
	IMU42627ID=ITG3200_Read(MPU42627_ID);
	if(IMUReadWriteStatusFlag==false)
		IMU42627ID=ITG3200_Read(MPU42627_ID);

	if(IMU42627ID==0x20)
		IMUSTATUS=true;
}

uint8_t imudata[16];

#if defined(CSD203_SENSOR)
extern bool IICReadStatusFlag;
#endif

uint8_t GetIMUID(void)
{
	return IMU42627ID;
}

void GetIMU42627(uint16_t loop)
{
	// static uint16_t errct=0;

	if(IMU42627ID==0x20)
	{
		if(loop==4)
		{
			if(IICReadStatusFlag == true)return;
			IICReadStatusFlag=true;

			ITG3200_ReadImu(imudata);

			IICReadStatusFlag=false;
		}
		else if(loop==0)
		{
			if(IMUReadWriteStatusFlag==true)
			{
				IMU4_getValues(imudata); 	//
			}
			// else{
			// 	RTRACE("err=%d",errct);
			// }
		}
	}
}
