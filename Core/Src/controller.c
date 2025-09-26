//Responsible for the robot control

#include "string.h"
#include "math.h"
//#include "delay.h"  // You must implement delay_ms()
#include <stdbool.h>
#include <controller.h>
#include <actuators.h>

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

// === State Machine Definitions ===

static MowerState current_state = STATE_ALIGN_TO_EDGE;

// === Movement Control ===
void move_forward(int count) {
    // TODO implement to set velocity
	set_right_wheel(FORWARD, count);
	set_left_wheel(FORWARD, count);
}

void move_backward(int count) {
	// TODO implement to set velocity
	set_right_wheel(BACKWARD, count);
	set_left_wheel(BACKWARD, count);
}

void turn_clock(int count) {
	set_wheel(WHEEL_RIGHT, ROTATING_CLOCK, count);
	set_wheel(WHEEL_LEFT, ROTATING_COUNTER, count);
}

void turn_counter(int count) {
	set_wheel(WHEEL_RIGHT, ROTATING_COUNTER, count);
	set_wheel(WHEEL_LEFT, ROTATING_CLOCK, count);
}

//void turn_clock_soft(void) {
//	set_right_wheel(ROTATING_CLOCK), 15);
//	set_left_wheel(ROTATING_, 15);
//}
//
//void turn_counter_soft(void) {
//	set_right_wheel(ROTATING_COUNTER), 15);
//	set_left_wheel(ROTATING_COUNTER, 15);
//}

void stop_movement(lawn_mower_status *status) {
	set_right_wheel(STOP, 0);
	set_left_wheel(STOP, 0);
}

// === Behavior Functions ===

void align_to_right_edge(lawn_mower_status *status) {
    // Align until right bumper hits an obstacle
    while (!(*status->bumpers[2])) {  // RIGHT = index 2
        move_forward(status);
    }

    stop_movement(status);
    strcpy(status->direction, "RIGHT_EDGE_ALIGNED");
}

void follow_edge(lawn_mower_status *status) {
//    uint8_t right_ir = *status->irda_distance[3]; // RIGHT IRDA
	switch (status->edge_sensor) {
		case EDGE_SENSOR_CUT:
			get_touch();
			break;
		case EDGE_SENSOR_HARD_TOUCH:
			get_away();
			break;
		case EDGE_SENSOR_OFF:
//			error(ERROR_EDGE_SENSOR_OFF); //TODO Implement this error
			break;
		case EDGE_SENSOR_BLOCKED:
//			error(ERROR_EDGE_SENSOR_BLOCKED);  //TODO Implement this error
			break;
		case EDGE_SENSOR_TOUCH:
//			Do nothing
			break;
	}

//    if (*status->bumpers[FRONT_RIGHT]) {
//        turn_left(status);
//    } else if (right_ir > 25) { // Too far from edge
//        turn_right(status);
//    } else {
//        move_forward(status);
//    }
}

void get_touch() {
	turn_clock(SOFT_CLOCK);

}

bool area_closed(lawn_mower_status *status) {
    static float start_x = -1.0f, start_y = -1.0f;
    static uint32_t start_time = 0;

    if (start_x < 0.0f) {
        start_x = *status->pos[0];
        start_y = *status->pos[1];
        start_time = status->uptime_ms;
    }

    float dx = *status->pos[0] - start_x;
    float dy = *status->pos[1] - start_y;
    float distance = sqrtf(dx * dx + dy * dy);

    return (distance < 20.0f && status->uptime_ms - start_time > 15000);
}

bool check_bumper_hit(lawn_mower_status *status) {
    for (int i = 0; i < 8; i++) {
        if (*status->bumpers[i]) return true;
    }
    return false;
}

void perform_zigzag(lawn_mower_status *status) {
    static bool zig_left = true;
    static uint32_t last_switch = 0;

    if (status->uptime_ms - last_switch > 3000) {
        zig_left = !zig_left;
        last_switch = status->uptime_ms;
    }

    if (zig_left) {
        turn_left_soft(status);
    } else {
        turn_right_soft(status);
    }

    move_forward(status);
}

void avoid_obstacle(lawn_mower_status *status) {
    stop_movement(status);
    move_backward(status);
    delay_ms(500); //TODO implement a delay function without block the process

    if (*status->bumpers[3]) {  // LEFT
        turn_right(status);
    } else {
        turn_left(status);
    }

    delay_ms(600); //TODO implement a delay function without block the process
    move_forward(status);
}

// === Main State Machine Entry ===
void mower_main_loop(lawn_mower_status *status) {
    switch (current_state) {
//        case STATE_ALIGN_TO_EDGE:
//            align_to_right_edge(status);
//            current_state = STATE_FOLLOW_EDGE;
//            break;

        case STATE_FOLLOW_EDGE:
            follow_edge(status);
            if (area_closed(status)) {
                current_state = STATE_CLOSED_AREA_DETECTED;
            }
            break;

        case STATE_CLOSED_AREA_DETECTED:
            stop_movement(status);
            current_state = STATE_ZIGZAG;
            break;

        case STATE_ZIGZAG:
            if (check_bumper_hit(status)) {
                current_state = STATE_AVOID_OBSTACLE;
            } else {
                perform_zigzag(status);
            }
            break;

        case STATE_AVOID_OBSTACLE:
            avoid_obstacle(status);
            current_state = STATE_ZIGZAG;
            break;
    }
}

// Initialize the FIFO
void mov_fifo_init(MovementFIFO *mov_fifo) {
    mov_fifo->head = 0;
    mov_fifo->tail = 0;
    mov_fifo->count = 0;
}

// Push an item into the FIFO
bool mov_fifo_push(MovementFIFO *mov_fifo, movement item) {
    if (mov_fifo->count == FIFO_SIZE) {
        // FIFO full
        return false;
    }
    mov_fifo->buffer[mov_fifo->tail] = item;
    mov_fifo->tail = (mov_fifo->tail + 1) % FIFO_SIZE;
    mov_fifo->count++;
    return true;
}

// Pop an item from the FIFO
bool mov_fifo_pop(MovementFIFO *mov_fifo, movement *item) {
    if (mov_fifo->count == 0) {
        // FIFO empty
        return false;
    }
    *item = mov_fifo->buffer[mov_fifo->head];
    mov_fifo->head = (mov_fifo->head + 1) % FIFO_SIZE;
    mov_fifo->count--;
    return true;
}

