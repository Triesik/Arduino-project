#pragma once
#include <mh_z19.h>
#include <stdio.h>

typedef struct Co2Sensor* Co2Sensor_t;

Co2Sensor_t Co2Sensor_create();

mh_z19_return_code_t initco2driver(Co2Sensor_t self);

void co2sensor_destroy(Co2Sensor_t self);

mh_z19_return_code_t co2_meassure(Co2Sensor_t self);

uint16_t co2get_measurment(Co2Sensor_t self);

void co2_set(Co2Sensor_t self, uint16_t co2_to_set);

