//Responsible for the robot control

#include <stdbool.h>
#include <controller.h>

static lawn_mower_status law_mower;

void init_struct (lawn_mower_status law_mower) {
	motion_control_init(&law_mower);
	positioning_init(&law_mower);
	sensors_init(&lawer_mower); //TODO implement this function, create sensors.c
	battery_power_init(&lawer_mower); //TODO implement this function
	system_status_init(&lawer_mower); //TODO implement this function
}

void start_operation(void) {
	// check the hardware
	// Allways start looking a right obstacle
	// map the edge movement
};
void pause_operation(uint8_t reason) {
	//log the reason and stop the movement
}

void stop_operation(void) {
	//put the robot on hibernate state
}

void return_to_base(void) {
	// return to base, end the job or recharge needed
}

void implement_movement(void) {//add a struct as parameter
	//based on movement plan add movement vectors and start the movement
}
void add_movement_vector(float angle, int lenght){
	//add the vector in a circular buffer
}

void bypasses_obstacle(void) {
	// do the bypass obstacle managing the actuators and sensors
}

void warning_operation(uint8_t fail){
	//log fail
	// pause the movement
}
