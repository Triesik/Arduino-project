#include <stdio.h>
#include <lora_driver.h>

typedef struct SensorDataPackageHandler* SensorDataPackageHandler_t;

SensorDataPackageHandler_t co2sensor_create();

void set_dph_temperature(SensorDataPackageHandler_t self ,float temperature);


void set_dph_humidity(SensorDataPackageHandler_t self ,float humidity);


void set_dph_co2(SensorDataPackageHandler_t self, uint16_t co2);

lora_payload_t assemble_payload(SensorDataPackageHandler_t self);


