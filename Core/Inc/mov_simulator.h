#define FORWARD             1
#define BACKWARD            2
#define STOP                0
#define ROTATING_CLOCK      3
#define ROTATING_COUNTER    4

void tick(void);
int set_movement(int direction, int count);
