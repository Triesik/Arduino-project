#pragma once
#include <stdio.h>

typedef struct Configuration* Configuration_t;

Configuration_t configuration_create();

void configuration_destroy(Configuration_t self);

void set_max_temperature(Configuration_t self, uint16_t max_co2);

uint16_t get_max_temperature_setting(Configuration_t self);

void set_temperature_current(Configuration_t self, uint16_t co2);

void operate_window();


