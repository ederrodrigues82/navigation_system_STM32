//Simulate the encoders signals
//Simulate the movement back and forward
//Simulate the rotate movement


#include <main.h>
#include <stdio.h>
#include <actuators.h> // Required for Movement_direction enum (STOP, FORWARD, ROTATING_CLOCK)
#include <mov_simulator.h>
/* USER CODE END Includes */

#define CLOCK 0
#define COUNTER_CLOCK 1

static uint8_t simulator_direction = STOP;

volatile int count_right = 0;
volatile int count_left = 0;
volatile int limit_right = 0;
volatile int limit_left = 0;

static void simulate_encoder_pulse(uint8_t direction);

int set_simulate_movement(int direction, int count)
{
	simulator_direction = direction;
	LOG_INFO("Simulating: Moving %d pulses", count);
    return 1;
}

void tick(void)
{
    if (simulator_direction != STOP)
    {
    	if ((simulator_direction == FORWARD) || (simulator_direction == ROTATING_CLOCK)) {
    		simulate_encoder_pulse(CLOCK);
    	} else {
    		simulate_encoder_pulse(COUNTER_CLOCK);
    	}
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // TOGGLE LED INDICATOR
    }
}

static void simulate_encoder_pulse(uint8_t direction)
{
    //TODO os pinos estã desabilitados antes de colocar para funcionar corrigir isso
	// Sequência quadratura
//    static const uint8_t steps[4][2] = {
//        {0, 0},
//        {1, 0},
//        {1, 1},
//        {0, 1}
//    };

    static int step_index = 0;

    if (direction == 0) // clock direction
    	step_index = (step_index + 3) % 4;  // -1 com wraparound
    else                // counter clock direction
        step_index = (step_index + 1) % 4;

    //HAL_GPIO_WritePin(GPIOC, MOV_SIM_CHANNEL_A_Pin, steps[step_index][0]);  // Channel A
    //HAL_GPIO_WritePin(GPIOC, MOV_SIM_CHANNEL_B_Pin, steps[step_index][1]);  // Channel B
}
