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
#include "delays_driver.h"
#include "imu_42627.h"
#include "imu4.h"
#include "CI1302.h"

#include "rtos.h"
#include "edgetx_types.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

float x_Gyro_angle=0;
float y_Gyro_angle=0;
float z_Gyro_angle=0;

s16 NewGyro_Gx=0;		//gyro x
s16 NewGyro_Gy=0;
s16 NewGyro_Gz=0;

s16 NewAccel_Ax=0;		//acc x
s16	NewAccel_Ay=0;		//acc y
s16 NewAccel_Az=0;		//acc z

s16 AccelSum_Ax=0;
s16 AccelSum_Ay=0;
s16 AccelSum_Az=0;

u16 AccelGyroJust=0;

s32 AccelAdd_X=0;
s32 AccelAdd_Y=0;
s32 AccelAdd_Z=0x2000;  	//=8192

s16 BalanceAccelGyroX=0;	//x,y
s16 BalanceAccelGyroY=0;

float x_angle=0;
float y_angle=0;

u8 gyro_InsMode=0;


u16 GetAccelCheck(u16 v)
{
    u16 a=0,b=0x4000,c,d=v;

    for(;;)
    {
        c=a+b;
        a=a>>1;
        if(d>=c)
        {
            d-=c;a+=b;
        }
        b=b>>2;
        if(b==0)
        {
            if(a<d)
            {
                d=a;
                d+=1;
                return d;
            }
            return a;
        }//if(b==0)
    }//for(;;)
}

s16 GetAccelCheck1(s32 v)
{
	s16 a;
	s16 b;
	s16 c;
	s16 d;
	s16 e;
	s16 f;

	a=0x1922;
	b=0x4b66;
	d=0x0648;
	e=0x1f6a;
	f=0x0001;

	if((s16)(v)<0)
	{
		c=-(s16)v;
	}
	else
	{
	 	c=v;
	}
	f=f+c;

	if((s16)(v>>16)<0)
	{
		c=(((s32)((s16)(v>>16)+f)<<0x0d))/((s16)(f-(s16)(v>>16)));	//ok
	}
	else
	{//
		c=((s32)((s16)(v>>16)-f)<<0x0d)/(((s16)(v>>16)+f));	//ok
		b=a;
	}

	b=b+((((((d*((((s32)c*(s32)c)>> 0x0d)))>>0x0d))-e)*c)>>0x0d);

	if((s16)v<0)
	{
		return -b;
	}
	else
	{//
		return b;
	}
}

void IMU4_GyroAccelRotate(void)
{
	s16 acctal_y_0_1=0,acctal_y_2_3=0,acctal_y_4_5=0,acctal_y_a_b=0;

	s16 at;
	s16 tgx,tgy,tgz;

	s32 a,b;

	//***********************************************x for(
	if(NewGyro_Gx<0)tgx=-NewGyro_Gx;
	else tgx=NewGyro_Gx;

	if(tgx>6)acctal_y_0_1=NewGyro_Gx/5.68;	//1ms

	a=AccelSum_Ax;
	a*=15;
	b=NewAccel_Ax;
	a+=b;

	a=a>>4;
	AccelSum_Ax=a;

	a=NewAccel_Ax;
	b=a;
	a=a*b;

	a=a/0x1A44;

	at=a;
	acctal_y_a_b+=at;

	//***********************************************y
	if(NewGyro_Gy<0)tgy=-NewGyro_Gy;
	else tgy=NewGyro_Gy;

	if(tgy>6)acctal_y_2_3=NewGyro_Gy/5.68;	//4ms

	a=AccelSum_Ay;
	a*=15;
	b=NewAccel_Ay;
	a+=b;

	a=a>>4;
	AccelSum_Ay=a;

	a=NewAccel_Ay;		//accy * accy
	b=a;
	a=a*b;

	a=a/0x1A44;

	at=a;
	acctal_y_a_b+=at;

	//***********************************************z

	if(NewGyro_Gz<0)tgz=-NewGyro_Gz;
	else tgz=NewGyro_Gz;

	if(tgz>6)acctal_y_4_5=NewGyro_Gz/5.68;	//4ms

	a=AccelSum_Az;
	a*=15;
	b=NewAccel_Az;
	a+=b;

	a=a>>4;
	AccelSum_Az=a;

	a=NewAccel_Az;
	b=a;
	a=a*b;

	a=a/0x1A44;

	at=a;
	acctal_y_a_b+=at;
	//*****************************************************
	//*****************************************************
	acctal_y_a_b=GetAccelCheck(acctal_y_a_b);

	at=AccelGyroJust;
	at=at*7;
	at+=acctal_y_a_b;

	at=at>>3;			//at/=8;
	AccelGyroJust=at;

	//-------------------
	a=AccelAdd_X;
	b=acctal_y_2_3;
	a=a*b;

	a=a>>0x0F;

	b=AccelAdd_Z;
	b=b+a;
	AccelAdd_Z=b;

	a=acctal_y_2_3;
	a=a*b;

	a=a>>0x0F;

	b=AccelAdd_X;
	b=b-a;
	AccelAdd_X=b;

	//------------------
	b=AccelAdd_Z;
	a=acctal_y_0_1;

	a=a*b;
	a=a>>0x0f;

	b=AccelAdd_Y;
	b=b+a;
	AccelAdd_Y=b;

	a=acctal_y_0_1;

	a=a*b;
	a=a>>0x0f;

	b=AccelAdd_Z;
	b=b-a;
	AccelAdd_Z=b;

	//-----------------
	b=AccelAdd_Y;
	a=acctal_y_4_5;
	a=a*b;
	a=a>>0x0f;

	b=AccelAdd_X;
	b=b+a;
	AccelAdd_X=b;

	a=acctal_y_4_5;
	a=a*b;
	a=a>>0x0f;

	b=AccelAdd_Y;
	b=b-a;
	AccelAdd_Y=b;
	//****************************************
}

void IMU4_GyroAccelRotate1(void)
{
	s16 aa;
	s32 a,b,c;

	if(AccelGyroJust>=0x0062&&AccelGyroJust<=0x0066)
	{
		aa=0x012C;
	}
	else if((AccelGyroJust>0x0066&&AccelGyroJust<=0x0069)||(AccelGyroJust<0x0062&&AccelGyroJust>=0x005f))
	{
		aa=0x0190;
	}
	else if((AccelGyroJust>0x0069&&AccelGyroJust<=0x006E)||(AccelGyroJust<0x005f&&AccelGyroJust>=0x005a))
	{
		aa=0x0258;
	}
	else if((AccelGyroJust>0x006e&&AccelGyroJust<=0x0073)||(AccelGyroJust<0x005a&&AccelGyroJust>=0x0055))
	{
		aa=0x0320;
	}
	else
	{
		aa=0;
	}

	if(aa<=0)
	{//end
		return;
	}
	//---------------------------------
	a=AccelAdd_X;
	b=AccelSum_Ax;
	c=aa-1;
	a=a*c;

	a=a+b;
	c=aa;
	a=a/c;
	AccelAdd_X=a;
	//-----------
	a=AccelAdd_Y;
	b=AccelSum_Ay;
	c=aa-1;
	a=a*c;

	a=a+b;
	c=aa;
	a=a/c;
	AccelAdd_Y=a;
	//-----------
	a=AccelAdd_Z;
	b=AccelSum_Az;
	c=aa-1;
	a=a*c;

	a=a+b;
	c=aa;
	a=a/c;
	AccelAdd_Z=a;
}

void IMU4_GyroAccelRotate2(u8 mode)
{
	s16 aa,ab,ac,ad,ae,af;
	s32 a,b,c,d,e;

	a=AccelAdd_Y;

	a=a*a;
	aa=a>>0x0F;

	b=AccelAdd_Z;
	b=b*b;
	ab=b>>0x0f;

	c=AccelAdd_Z;
	ac=c>>2;

	d=AccelAdd_Y;
	ad=d>>2;

	d=((s32)ac<<16)+ad;

	d=GetAccelCheck1(d);

	d=d*0x0064;
	BalanceAccelGyroX=d/0x008F;

	//*********************************
	ae=aa+ab;
	ae=GetAccelCheck(ae);
	ae=ae*0x00B5;
	ae=ae>>2;

	e=AccelAdd_X;
	af=e>>2;

	d=((s32)ae<<16)+af;

	d=GetAccelCheck1(d);
	d=-d;
	d=d*0x0064;

	BalanceAccelGyroY=d/0x008F;

	x_angle=BalanceAccelGyroX/100;
	y_angle=-BalanceAccelGyroY/100;
}
//*********************************************************

int16_t NewGyro_Gxbase=0;
int16_t NewGyro_Gybase=0;
int16_t NewGyro_Gzbase=0;

int32_t NewGyro_GxOld=0;
int32_t NewGyro_GyOld=0;
int32_t NewGyro_GzOld=0;

int16_t NewGyro_Gzbaseflag=0;
int16_t NewGyro_Gzbasestep=0;
float gyroinitbasevalue=0.05;

bool gyroinitflag=false;

extern volatile rotenc_t rotencValue;

void MouseCall(void)
{
	static float x_Gyro_angleold=0;
	static int16_t mousecount=0;

	if(++mousecount>=40)
	{//20*5ms  100ms
		mousecount=0;
		float x=x_Gyro_angleold-x_Gyro_angle;

		if(x>2)rotencValue+=1;
		else if(x<-2)rotencValue-=1;

		x_Gyro_angleold=x_Gyro_angle;
	}
}

void IMU4_GetGyroValues(void)
{
	float x,y,z;//,ax,ay;

	float dt=0.005; 	//for 200hz
	//float dt=0.01; 	//for 100hz  10ms

	float Scale=16.4;
	static float x_angleold=0;
	static int16_t waitct=200*3;	//wait 3s
	static float angle=1;

	x=NewGyro_Gx;
	y=NewGyro_Gy;
	z=NewGyro_Gz;

	x/=Scale;
	y/=Scale;
	z/=Scale;

	x=(x*dt);
	y=(y*dt);
	z=(z*dt);

	x_Gyro_angle+=x;
	y_Gyro_angle-=y;
	z_Gyro_angle+=z;

	if(NewGyro_Gzbaseflag>=200)
	{
		//Offset correction
		if(x_Gyro_angle>0.02)x_Gyro_angle-=0.02;
		else if(x_Gyro_angle<-0.02)x_Gyro_angle+=0.02;
		else x_Gyro_angle=0;

		if(y_Gyro_angle>0.02)y_Gyro_angle-=0.02;
		else if(y_Gyro_angle<-0.02)y_Gyro_angle+=0.02;
		else y_Gyro_angle=0;

		if(z_Gyro_angle>0.04)z_Gyro_angle-=0.04;
		else if(z_Gyro_angle<-0.04)z_Gyro_angle+=0.04;
		else z_Gyro_angle=0;

		//MouseCall();
	}
	else if(waitct==0)
	{
		dt=x_angle-x_angleold;
		if(dt<angle&&dt>-angle)
		{
			if(++NewGyro_Gzbaseflag>=200)
			{
				NewGyro_Gxbase=NewGyro_Gx;
				NewGyro_Gybase=NewGyro_Gy;
				NewGyro_Gzbase=NewGyro_Gz;

				gyroinitflag=true;
			}
		}
		else
		{//0.05ms
			angle=angle+0.001;
			x_angleold=x_angle;
			NewGyro_Gzbaseflag=0;
		}
	}
	else
	{
		waitct--;
	}
}
//*********************************************************

u8 MPU6050_getMotionImu(u8 *data)
{
  // endianness correction
  for (uint8_t i = 1; i < 7; i++) {
    u8 c1 = data[i * 2];
    u8 c2 = data[i * 2 + 1];

    data[i * 2] = c2;
    data[i * 2 + 1] = c1;
  }

  imu_cmdpck_t *m = (imu_cmdpck_t *)data;

  NewAccel_Ax = m->IMU_ACCEL_Y;
  NewAccel_Ay = m->IMU_ACCEL_X;
  NewAccel_Az = -m->IMU_ACCEL_Z;

  NewGyro_Gx = m->IMU_GYRO_Y;
  NewGyro_Gy = m->IMU_GYRO_X;
  NewGyro_Gz = -m->IMU_GYRO_Z;

  NewGyro_GxOld = NewGyro_Gx;
  NewGyro_GyOld = NewGyro_Gy;
  NewGyro_GzOld = NewGyro_Gz;

  NewGyro_Gx -= NewGyro_Gxbase;
  NewGyro_Gy -= NewGyro_Gybase;
  NewGyro_Gz -= NewGyro_Gzbase;

  return 0;
}

void IMU4_getValues(uint8_t *m)
{
  // static uint16_t loopcount=0;

  MPU6050_getMotionImu(m);

  IMU4_GetGyroValues();

  IMU4_GyroAccelRotate();

  IMU4_GyroAccelRotate1();
  IMU4_GyroAccelRotate2(0);
}
