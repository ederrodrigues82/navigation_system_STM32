#ifndef __SENSOR_H
#define __SENSOR_H

#include "controller.h"

void read_bumpers(TIM_HandleTypeDef *htim);
void sensor_init(lawn_mower_status* law_mower); // Add prototype for sensor_init

#endif /* SENSOR_H */

