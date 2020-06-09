#include <stdlib.h>
#include <string.h>
#include "SensorDataPackageHandler.h"
#include <stdio.h>
#include <lora_driver.h>
#include <math.h>

typedef struct SensorDataPackageHandler {
	uint16_t co2_to_pack;
	float temperature_to_pack;
	float humidity_to_pack;
} SensorDataPackageHandler;

SensorDataPackageHandler_t SensorDataPackageHandler_create(){
	
	SensorDataPackageHandler_t _sdph = calloc(sizeof(SensorDataPackageHandler), 1);
	
	if(NULL == _sdph){
		return NULL;
	}
	return _sdph;
}

void SensorDataPackageHandler_destroy(SensorDataPackageHandler_t self){
	
	if(NULL !=self){
		free(self);
	}
}

void set_dph_temperature(SensorDataPackageHandler_t self ,float temperature)
{
	self->temperature_to_pack = temperature;
}

void set_dph_humidity(SensorDataPackageHandler_t self ,float humidity)
{
	self->humidity_to_pack = humidity;
}

void set_dph_co2(SensorDataPackageHandler_t self, uint16_t co2)
{
	self->co2_to_pack = co2;
}


lora_payload_t assemble_payload(SensorDataPackageHandler_t self) {

	lora_payload_t uplink_payload;
	uint8_t temppayload = round(self->temperature_to_pack);
	uint8_t humpayload = round(self->humidity_to_pack);
	uint8_t co2payload = round(self->co2_to_pack);
	uplink_payload.bytes[0] = temppayload >> 8;
	uplink_payload.bytes[1] = temppayload & 0xFF;
	uplink_payload.bytes[2] = humpayload >> 8;
	uplink_payload.bytes[3] = humpayload & 0xFF;
	uplink_payload.bytes[4] = co2payload >> 8;
	uplink_payload.bytes[5] = co2payload & 0xFF;
	
	puts("package assembled");
	return uplink_payload;
	
}





