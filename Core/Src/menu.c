//menu using UART1
//1- Movement setup

#include <stdio.h>
#include "menu.h"
#include "actuators.h"


void menu(char command)
{
    switch (command)
    {
        case '0':
        	printf("\r\nSelect your option\r\n");
        	printf("0- show the menu again \r\n");
        	printf("1- Set forward movement, 10 pulses \r\n");
        	printf("2- Set backward movement, 10 pulses \r\n");
        	printf("3- Print right wheel status\r\n");
        	printf("9- Stop movement\r\n");
        	break;

        case '1':
        	set_right_wheel(FORWARD, 10);
			break;
        case '2':
			set_right_wheel(BACKWARD, 10);
			break;
        case '3':
        	print_wheel_status(ENCODER_RIGHT);
			break;
        case '9':
        	set_right_wheel(STOP, 0);
        	break;

        default:
            printf("Unknown command: %c, press 0 to show the menu again\r\n", command);
            break;
    }
}
