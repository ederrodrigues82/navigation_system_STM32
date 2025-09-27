// Configure and transmit message to BNO080
#include <main.h>
#include "positioning.h"
#include "stm32f1xx_hal.h"

float accelX_buffer[FILTER_SIZE] = {0};
float accelY_buffer[FILTER_SIZE] = {0};
float accelZ_buffer[FILTER_SIZE] = {0};
uint8_t accel_index = 0;

typedef struct {
    float accelX, accelY, accelZ;
    float velX, velY, velZ;
    float posX, posY, posZ;
    float roll, pitch, yaw;
} BNO080_Data_t;

BNO080_Data_t bno080_data;

int positioning_init(lawn_mower_status* law_mower) {
	law_mower->pos[X] = bno080_data.posX;
	law_mower->pos[Y] = bno080_data.posY;
	law_mower->pos[Z] = bno080_data.posZ;
	// The following lines were incorrect, assigning velocity to position. Removing them.
	// law_mower->pos[X] = bno080_data.velX;
	// law_mower->pos[Y] = bno080_data.velY;
	// law_mower->pos[Z] = bno080_data.velZ;
	law_mower->accel[X] = bno080_data.accelX;
	law_mower->accel[Y] = bno080_data.accelY;
	law_mower->accel[Z] = bno080_data.accelZ;

	// Gyro data is not directly stored in BNO080_Data_t in this setup, initializing to 0.0f.
	law_mower->gyro[X] = 0.0f;
	law_mower->gyro[Y] = 0.0f;
	law_mower->gyro[Z] = 0.0f;

	law_mower->euler_angles[ROLL] = bno080_data.roll;
	law_mower->euler_angles[PITCH] = bno080_data.pitch;
	law_mower->euler_angles[YAW] = bno080_data.yaw;
	return 0;
}

void BNO080_SetFeatureCommand(uint8_t reportID, uint16_t interval)
{
    uint8_t packet[17] = {0};

    packet[0] = 0xFD; // Start byte
    packet[1] = 0x0F; // Packet length (LSB)
    packet[2] = 0x00; // Packet length (MSB)
    packet[3] = 0xFD; // Channel (always 0xFD)
    packet[4] = 0x05; // Command: Set Feature Command
    packet[5] = reportID; // e.g., 0x01 for Accelerometer
    packet[6] = 0x00; // Feature flags
    packet[7] = interval & 0xFF; // Report interval LSB
    packet[8] = (interval >> 8) & 0xFF; // MSB
    // The rest are defaults or zeros...

    HAL_I2C_Master_Transmit(&hi2c1, BNO080_ADDR, packet, 17, HAL_MAX_DELAY);
}

//Activate accelerometer and rotation vector
void BNO080_activate(void) {
	BNO080_SetFeatureCommand(0x01, DT); // Accelerometer, 100 ms
	BNO080_SetFeatureCommand(0x04, DT); // Linear Acceleration
	BNO080_SetFeatureCommand(0x05, DT); // Rotation Vector, 100 ms
}

void BNO080_Receive(void)
{
    HAL_I2C_Master_Receive_DMA(&hi2c1, BNO080_ADDR, bno080_rx_buffer, sizeof(bno080_rx_buffer));
}

//Parse BNO080 data, accelerometer and rotation vector
void BNO080_ParseInputReport(uint8_t *data) {
    static float prevAccelX = 0, prevAccelY = 0, prevAccelZ = 0;

    uint8_t reportID = data[4];
    if (reportID == 0x04) {
        int16_t x = (data[5] | (data[6] << 8));
        int16_t y = (data[7] | (data[8] << 8));
        int16_t z = (data[9] | (data[10] << 8));

        float rawX = x * 0.0001f;
        float rawY = y * 0.0001f;
        float rawZ = z * 0.0001f;

        float filteredX = FilteredAccel(accelX_buffer, rawX);
        float filteredY = FilteredAccel(accelY_buffer, rawY);
        float filteredZ = FilteredAccel(accelZ_buffer, rawZ);

        prevAccelX = bno080_data.accelX;
        prevAccelY = bno080_data.accelY;
        prevAccelZ = bno080_data.accelZ;

        bno080_data.accelX = HighPassFilter(filteredX, prevAccelX, 0.9f);
		bno080_data.accelY = HighPassFilter(filteredY, prevAccelY, 0.9f);
		bno080_data.accelZ = HighPassFilter(filteredZ, prevAccelZ, 0.9f);
		UpdateVelocityPosition(DT);

    } else if (reportID == 0x05) {
        int16_t i = (data[5] | (data[6] << 8));
        int16_t j = (data[7] | (data[8] << 8));
        int16_t k = (data[9] | (data[10] << 8));
        int16_t real = (data[11] | (data[12] << 8));
        float quatI = i * 0.0001f;
        float quatJ = j * 0.0001f;
        float quatK = k * 0.0001f;
        float quatReal = real * 0.0001f;
        QuaternionToEuler(
                quatReal,
                quatI,
                quatJ,
                quatK,
                &bno080_data.roll,
                &bno080_data.pitch,
                &bno080_data.yaw
            );
    }
}

//High pass filter to improve the sensor signal
float HighPassFilter(float current_input, float previous_output, float alpha) {
    return alpha * (previous_output + current_input - previous_output);
}

//Removes high-frequency noise well
float FilteredAccel(float *buffer, float newValue)
{

	buffer[accel_index] = newValue;

    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        sum += buffer[i];
    }

    // Wrap index for next write
    accel_index = (accel_index + 1) % FILTER_SIZE;

    return sum / FILTER_SIZE;
}

//Quartenion to Euler convertion
/**
 * Converts a quaternion (w, x, y, z) to Euler angles (roll, pitch, yaw).
 * Angles are in radians.**/
void QuaternionToEuler(float w, float x, float y, float z, float *roll, float *pitch, float *yaw)
{
    // Roll (x-axis rotation)
    *roll = atan2f(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (w * y - z * x);
    if (fabsf(sinp) >= 1.0f)
        *pitch = copysignf(M_PI / 2.0f, sinp);  // use 90 degrees if out of range
    else
        *pitch = asinf(sinp);

    // Yaw (z-axis rotation)
    *yaw = atan2f(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
}

void UpdateVelocityPosition(float dt) {
    dt = DT / 1000; // convert dt to miliseconds
	bno080_data.velX += bno080_data.accelX * dt;
    bno080_data.velY += bno080_data.accelY * dt;
    bno080_data.velZ += bno080_data.accelZ * dt;

    bno080_data.posX += bno080_data.velX * dt;
    bno080_data.posY += bno080_data.velY * dt;
    bno080_data.posZ += bno080_data.velZ * dt;
}
