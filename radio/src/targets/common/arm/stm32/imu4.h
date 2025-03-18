

#ifndef __IMU4_IMU_H
#define __IMU4_IMU_H

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef int16_t		s16;
typedef int32_t		s32;

/*
typedef char 	   s8;
typedef int 	   s16;
typedef long 	   s32;

typedef unsigned char 	   u8;
typedef unsigned int 	   u16;
typedef unsigned long 	   u32;
*/
//10£¬

//#define  WORD_LO(xxx)  ((byte) ((word)(xxx) & 255))
//#define  WORD_HI(xxx)  ((byte) ((word)(xxx) >> 8))

//#define  LONG_LO(xxx)  ((s16) ((long)(xxx) & 65535))
//#define  LONG_HI(xxx)  ((s16) ((long)(xxx) >> 16))

//read
extern s16 tNewGyro_Gx;		//gyro x
extern s16 tNewGyro_Gy;
extern s16 tNewGyro_Gz;

extern s16 NewGyro_Gx;		//gyro x
extern s16 NewGyro_Gy;		//gyro y
extern s16 NewGyro_Gz;		//gyro z
    
extern s16 NewAccel_Ax;		//acc x 
extern s16 NewAccel_Ay;		//acc y
extern s16 NewAccel_Az;		//acc z


extern s16 AccelSum_Ax;		//
extern s16 AccelSum_Ay;
extern s16 AccelSum_Az;

extern u16 AccelGyroJust;

extern s32 AccelAdd_X;
extern s32 AccelAdd_Y;
extern s32 AccelAdd_Z;

extern s16 AccelGyroParHX;
extern s16 AccelGyroParHY;

extern s16 AccelGyroParVX;
extern s16 AccelGyroParVY;

extern s16 BalanceAccelGyroX;
extern s16 BalanceAccelGyroY;

extern s16 tBalanceAccelGyroX;	//x,y
extern s16 tBalanceAccelGyroY;

extern u8 HVJustFlag;			//
extern u8 ROLLJustMode;			//
extern u8 ROLLJustFlag;			//

u8 MPU6050_getMotionImu(u8 mode);		//
void IMU4_getValues(u8 *m); 			//

#endif
