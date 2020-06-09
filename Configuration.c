#pragma once
#include <mh_z19.h>
#include "Configuration.h"
#include <stdio.h>
#include "Servo.h"



typedef struct Configuration {
	uint16_t max_temperature;
	uint16_t current_temperature;
} Configuration;


Configuration_t configuration_create(){
	
	Configuration_t config = calloc(sizeof(Configuration), 1);
	
	if(NULL == config){
		return NULL;
	}
	return config;
}

void configuration_destroy(Configuration_t self){
	
	if(NULL !=self){
		free(self);
	}
}


void set_max_temperature(Configuration_t self, uint16_t temperature)
{
	self->max_temperature = temperature;
}

uint16_t get_max_temperature_setting(Configuration_t self) {
	return self->max_temperature;
}

void set_temperature_current(Configuration_t self, uint16_t temperature)
{
	self->current_temperature = temperature;
}

void operate_window(Configuration_t self)
{
	if(self->max_temperature > 0)
	{
		if(self->max_temperature >= self->current_temperature)
		{
			servo_open();
		}
		else if(self->max_temperature < self->current_temperature)
		{
			servo_close();
		}
	}
}


