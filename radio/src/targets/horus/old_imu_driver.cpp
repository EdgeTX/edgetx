#define OUTX_L_G  0x22

#define ALPHA 0.98
#define DT    0.01
#define SCALE_FACT_ACC  0.005
#define SCALE_FACT_IMU 0.0078 // 250deg/s / 32000

float roll=0;
float pitch=0;

void imuCompute()
{
  uint8_t gyr_acc_raw[12];

  int8_t err = I2C_readBytes(IMU_I2C_ADDRESS, OUTX_L_G, 12, gyr_acc_raw);

  int16_t gx = (int16_t)(gyr_acc_raw[1] << 8 | gyr_acc_raw[0]);
  int16_t gy = (int16_t)(gyr_acc_raw[3] << 8 | gyr_acc_raw[2]);
  int16_t gz = (int16_t)(gyr_acc_raw[5] << 8 | gyr_acc_raw[4]);

  int16_t ax = (int16_t)(gyr_acc_raw[7]  << 8 | gyr_acc_raw[6]);
  int16_t ay = (int16_t)(gyr_acc_raw[9]  << 8 | gyr_acc_raw[8]);
  int16_t az = (int16_t)(gyr_acc_raw[11] << 8 | gyr_acc_raw[10]);

  // integrate gyro
  roll  -= gx * SCALE_FACT_IMU * DT;
  pitch += gy * SCALE_FACT_IMU * DT;

  int32_t magn = abs(ax) + abs(ay) + abs(az);
  if (magn > 8192 && magn < 32768) {

    if (az < 0) az = -az;
        
    float rollAcc  = atan2f((float)ay,(float)az) * 57.3 /* 180/PI */;
    float pitchAcc = atan2f((float)ax,(float)az) * 57.3 /* 180/PI */;

    roll  = roll  * ALPHA + rollAcc  * (1.0-ALPHA);
    pitch = pitch * ALPHA + pitchAcc * (1.0-ALPHA);
  }
}
