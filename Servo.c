/*
 * ServoJ13.c
 *
 * Created: 20. 5. 2020 21:46:09
 *  Author: Wakda
 */ 
#include <stdlib.h>
#include <string.h>
#include "Servo.h"
#include <rcServo.h>


void initservodriver()
{
	rcServoCreate();
}

void servo_open(){
	uint8_t servoNo = 0;
	
	rcServoSet(servoNo, 100);
}

void servo_close(){
	uint8_t servoNo = 0;
	
	rcServoSet(servoNo, 0);
}