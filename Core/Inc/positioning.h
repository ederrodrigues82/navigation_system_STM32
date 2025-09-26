#ifndef __POSITIONING_H
#define __POSITIONING_H

#define BNO080_BUFFER_SIZE 20 //buffer size of movement sensor
#define BNO080_ADDR (0x4A << 1)  // = 0x94 ADDR pulled down
#define FILTER_SIZE 10  // Filter used on accelerometer, you can adjust this value
#define DT 100 // Period o accelerometer measure in miliseconds

#include "controller.h" // Required for lawn_mower_status
extern I2C_HandleTypeDef hi2c1;
extern uint8_t bno080_rx_buffer[BNO080_BUFFER_SIZE];
extern uint8_t rx_data;

int positioning_init(lawn_mower_status* law_mower);
void BNO080_SetFeatureCommand(uint8_t reportID, uint16_t interval);
void BNO080_Receive(void);
void BNO080_ParseInputReport(uint8_t *data);
void BNO080_activate(void);
float HighPassFilter(float current_input, float previous_output, float alpha);
float FilteredAccel(float *buffer, float newValue);
void QuaternionToEuler(float w, float x, float y, float z, float *roll, float *pitch, float *yaw);
void UpdateVelocityPosition(float dt);

#endif /* POSITIONING_H */
