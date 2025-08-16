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

#include "edgetx.h"
#include "imu_filter.h"
#include <math.h>
#include <string.h>

// Filter parameters
#define COMPLEMENTARY_ALPHA     0.92f   // Complementary filter coefficient
#define LP_FILTER_ALPHA         0.9f    // Low-pass filter coefficient
#define SAMPLE_TIME_S           0.005f  // Gyro is read every 5ms as part of execMixerFrequentActions()
#define RAD_TO_DEG              57.2958f
#define DEG_TO_RAD              0.0174533f

// Filter state variables
static IMU_FilteredData_t filtered_data = {0};
static float prev_accel[3] = {0};
static float prev_gyro[3] = {0};
static uint8_t filter_initialized = 0;

// Low-pass filter implementation
float low_pass_filter(float new_value, float prev_value, float alpha) {
    return alpha * prev_value + (1.0f - alpha) * new_value;
}

// Calculate roll and pitch from accelerometer data
void calculate_accel_angles(float ax, float ay, float az, float *roll, float *pitch) {
    // Normalize accelerometer data
    float norm = sqrtf(ax*ax + ay*ay + az*az);
    if (norm == 0.0f) return;

    ax /= norm;
    ay /= norm;
    az /= norm;

    // Calculate angles
    *roll = atan2f(ay, sqrtf(ax*ax + az*az));
    *pitch = atan2f(-ax, sqrtf(ay*ay + az*az));
}

// Complementary filter for attitude estimation
void complementary_filter(IMU_RawData_t *raw_data, float dt) {
    float accel_roll, accel_pitch;

    // Calculate angles from accelerometer
    calculate_accel_angles(raw_data->accel_x, raw_data->accel_y, raw_data->accel_z,
                          &accel_roll, &accel_pitch);

    if (!filter_initialized) {
        // Initialize filter with accelerometer angles
        filtered_data.roll = accel_roll;
        filtered_data.pitch = accel_pitch;
        filtered_data.yaw = 0.0f;
        filter_initialized = 1;
    } else {
        // Integrate gyroscope data
        float gyro_roll = filtered_data.roll + raw_data->gyro_x * dt;
        float gyro_pitch = filtered_data.pitch + raw_data->gyro_y * dt;
        float gyro_yaw = filtered_data.yaw + raw_data->gyro_z * dt;

        // Apply complementary filter
        filtered_data.roll = COMPLEMENTARY_ALPHA * gyro_roll +
                            (1.0f - COMPLEMENTARY_ALPHA) * accel_roll;
        filtered_data.pitch = COMPLEMENTARY_ALPHA * gyro_pitch +
                             (1.0f - COMPLEMENTARY_ALPHA) * accel_pitch;
        filtered_data.yaw = gyro_yaw; // No magnetometer correction in this example

        // Keep yaw within -π to π range
        while (filtered_data.yaw > M_PI) filtered_data.yaw -= 2.0f * M_PI;
        while (filtered_data.yaw < -M_PI) filtered_data.yaw += 2.0f * M_PI;
    }
}

// Apply low-pass filter to raw sensor data
void apply_lowpass_filter(IMU_RawData_t *raw_data) {
    // Filter accelerometer data
    filtered_data.accel_x = low_pass_filter(raw_data->accel_x, prev_accel[0], LP_FILTER_ALPHA);
    filtered_data.accel_y = low_pass_filter(raw_data->accel_y, prev_accel[1], LP_FILTER_ALPHA);
    filtered_data.accel_z = low_pass_filter(raw_data->accel_z, prev_accel[2], LP_FILTER_ALPHA);

    // Filter gyroscope data
    filtered_data.gyro_x = low_pass_filter(raw_data->gyro_x, prev_gyro[0], LP_FILTER_ALPHA);
    filtered_data.gyro_y = low_pass_filter(raw_data->gyro_y, prev_gyro[1], LP_FILTER_ALPHA);
    filtered_data.gyro_z = low_pass_filter(raw_data->gyro_z, prev_gyro[2], LP_FILTER_ALPHA);

    // Update previous values
    prev_accel[0] = filtered_data.accel_x;
    prev_accel[1] = filtered_data.accel_y;
    prev_accel[2] = filtered_data.accel_z;

    prev_gyro[0] = filtered_data.gyro_x;
    prev_gyro[1] = filtered_data.gyro_y;
    prev_gyro[2] = filtered_data.gyro_z;
}

// Main IMU filter processing function
void process_imu_data(IMU_RawData_t *raw_data) {
    // Apply low-pass filtering to raw data
    apply_lowpass_filter(raw_data);

    // Apply complementary filter for attitude estimation
    complementary_filter(raw_data, SAMPLE_TIME_S);
}

// Get filtered IMU data
IMU_FilteredData_t* get_filtered_imu_data(void) {
    return &filtered_data;
}

// Reset filter state
void reset_imu_filter(void) {
    memset(&filtered_data, 0, sizeof(filtered_data));
    memset(prev_accel, 0, sizeof(prev_accel));
    memset(prev_gyro, 0, sizeof(prev_gyro));
    filter_initialized = 0;
}

// Advanced Kalman filter structure (optional implementation)
typedef struct {
    float Q_angle;     // Process noise variance for accelerometer
    float Q_bias;      // Process noise variance for gyro bias
    float R_measure;   // Measurement noise variance
    float angle;       // Estimated angle
    float bias;        // Estimated bias
    float rate;        // Unbiased rate
    float P[2][2];     // Error covariance matrix
} kalman_state_t;

// Kalman filter implementation (more advanced option)
float kalman_filter(kalman_state_t *state, float new_angle, float new_rate, float dt) {
    // Prediction step
    state->rate = new_rate - state->bias;
    state->angle += dt * state->rate;

    // Update error covariance
    state->P[0][0] += dt * (dt * state->P[1][1] - state->P[0][1] - state->P[1][0] + state->Q_angle);
    state->P[0][1] -= dt * state->P[1][1];
    state->P[1][0] -= dt * state->P[1][1];
    state->P[1][1] += state->Q_bias * dt;

    // Innovation
    float S = state->P[0][0] + state->R_measure;
    float K[2] = {state->P[0][0] / S, state->P[1][0] / S};

    float y = new_angle - state->angle;

    // Update step
    state->angle += K[0] * y;
    state->bias += K[1] * y;

    // Update error covariance
    float P00_temp = state->P[0][0];
    float P01_temp = state->P[0][1];

    state->P[0][0] -= K[0] * P00_temp;
    state->P[0][1] -= K[0] * P01_temp;
    state->P[1][0] -= K[1] * P00_temp;
    state->P[1][1] -= K[1] * P01_temp;

    return state->angle;
}