#pragma once
#include <mh_z19.h>
#include "Co2Sensor.h"
#include <stdio.h>



typedef struct Co2Sensor {
	uint16_t co2;
} Co2Sensor;


Co2Sensor_t co2sensor_create(){
	
	Co2Sensor_t _twoSensor = calloc(sizeof(Co2Sensor), 1);
	
	if(NULL == _twoSensor){
		return NULL;
	}
	return _twoSensor;
}

//initializes co2 sensor drivers
mh_z19_return_code_t initco2driver(Co2Sensor_t self){
	mh_z19_return_code_t  rc;
	mh_z19_create(ser_USART3, NULL);
	return rc;
}


mh_z19_return_code_t co2_meassure(Co2Sensor_t self)
{
	mh_z19_return_code_t rc;
	rc = mh_z19_take_meassuring();
	mh_z19_get_co2_ppm(self->co2);
	return rc;
}


void co2sensor_destroy(Co2Sensor_t self){
	
	if(NULL !=self){
		free(self);
	}
}


uint16_t co2get_measurment(Co2Sensor_t self) {
	return self->co2;
}

void co2_set(Co2Sensor_t self, uint16_t co2_to_set)
{
	self->co2 = co2_to_set;
}




