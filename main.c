#include <stdio.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <unistd.h>

#include <hal_defs.h>
#include <ihal.h>

#include <ATMEGA_FreeRTOS.h>

#include <FreeRTOSTraceDriver.h>
#include <stdio_driver.h>
#include <serial.h>
#include <hih8120.h>
#include <ihal.h>
#include <lora_driver.h>
#include "event_groups.h"
#include "HumTempSensor.h"
#include "LorawanDriver.h"
#include "SensorDataPackageHandler.h"
#include "Co2Sensor.h"
#include <display_7seg.h>
#include "message_buffer.h"
#include "Configuration.h"
#include "Servo.h"

HumTempSensor_t HtSensor;
Co2Sensor_t co2sensor;
SensorDataPackageHandler_t package_handler;
Configuration_t configuration;

hih8120DriverReturnCode_t rc;
e_LoRa_return_code_t lc;
mh_z19_return_code_t mc;
MessageBufferHandle_t uplink_buffer;


EventGroupHandle_t eGroup;
TaskHandle_t xHandle = NULL;
EventBits_t uxBits;

#define CO2_BIT			( 1 << 0 )
#define LORA_BIT		( 1 << 4 )
#define TEMP_HUM_BIT	( 1 << 1 )
#define ASSEMBLE_BIT	( 1 << 2 )
#define ASSEMBLE_BIT_2	( 1 << 3 )
#define SERVO_BIT		( 1 << 5 )

void co2_task( void *pvParameters );
void temp_hum_task( void *pvParameters );
void set_up_lora_task( void *pvParameters );
void assemble_payload_task( void *pvParameters );
void send_and_receive_task( void *pvParameters );
void temperature_controll_task(void *pvParameters );


void initialiseSystem()
{
	//Creating ADT's responsible for holding data and communicating with sensors
	HtSensor = humptempsensor_create(); 
	package_handler = SensorDataPackageHandler_create();
	co2sensor = co2sensor_create();
	configuration = configuration_create();


	
	//Initializing all drivers, initialization returns a code
	rc = inithtdrivers(HtSensor);
	if(rc != HIH8120_OK)
	{
		puts("humidity and temperature driver was not initialized, all sent data will be equivalent to 0");
	}
	initservodriver();
	if(mc != MHZ19_OK)
	{
		puts("servo driver was not initialized, can not operate window");
	}
	trace_init();
	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdioCreate(ser_USART0);
	hal_create(5);
	lora_driver_create(LORA_USART, NULL);

	
	
	create_tasks();
	
	//Creates FreeRTOS event groups, it it cant be created the program shut downs as it cannot operate without it
	eGroup = xEventGroupCreate();
	if (eGroup == NULL) {
		printf("Event Group can not be created.\n, program shutdown");
		exit(0);
	}
	
	//Creating message buffer that will transmit payload to LoRaWaN task, program cannot operate without it
	const size_t xMessageBufferSizeBytes = 255;
	uplink_buffer = xMessageBufferCreate(xMessageBufferSizeBytes);
	if (eGroup == NULL) {
		printf("Message buffer can not be created.\n, program shutdown");
		exit(0);
	}
	
	
}

void create_tasks(void)
{

	xTaskCreate(co2_task, (const portCHAR *)"Task1", configMINIMAL_STACK_SIZE, NULL, 3, NULL );

	xTaskCreate(temp_hum_task, (const portCHAR *)"Task2", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	
	xTaskCreate(set_up_lora_task, (const portCHAR *)"Task3", configMINIMAL_STACK_SIZE, NULL, 4, &xHandle );
	
	xTaskCreate(assemble_payload_task, (const portCHAR *)"Task4", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	
	xTaskCreate(send_and_receive_task, (const portCHAR *)"Task5", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	
	xTaskCreate(temperature_controll_task, (const portCHAR *)"Task5", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	
}

void set_up_lora_task (void *pvParameters)
{
	
	bool lora_setup_status;
	
	puts("lorwan initialization");
	lora_driver_reset_rn2483(1);
	vTaskDelay(2);
	lora_driver_reset_rn2483(0);
	vTaskDelay(150);
	lora_driver_rn2483_factory_reset();
	lora_driver_flush_buffers();
	lora_setup_status = network_setup();
	if(lora_setup_status == true)
	{
		xEventGroupSetBits(eGroup, CO2_BIT);
	}
	else
	{
		puts("lora initialization failed, program shutdown. Please contact the administrator");
		exit(0);
	}
	vTaskDelete(xHandle);
}

void co2_task( void *pvParameters )
{		
	
	for(;;)
	{
			
		uxBits = xEventGroupWaitBits(eGroup, CO2_BIT , pdTRUE, pdTRUE, portMAX_DELAY);
		if ((uxBits & (CO2_BIT)) == (CO2_BIT))
		{
			co2_meassure(co2sensor);
			xEventGroupSetBits(eGroup, TEMP_HUM_BIT | ASSEMBLE_BIT);
		}
	}
}

void temp_hum_task( void *pvParameters )
{
		
	for(;;)
	{
		uxBits = xEventGroupWaitBits(eGroup, TEMP_HUM_BIT, pdTRUE, pdFALSE, 100);
		if ((uxBits & (TEMP_HUM_BIT)) == (TEMP_HUM_BIT))
		{
			temphum_meassure(HtSensor);
			xEventGroupSetBits(eGroup, ASSEMBLE_BIT_2);
		}
	}
}


	
void assemble_payload_task (void *pvParameters)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();	
	for(;;)
	{
		lora_payload_t lora_payload;
		
		uxBits = xEventGroupWaitBits(eGroup, ASSEMBLE_BIT | ASSEMBLE_BIT_2, pdTRUE, pdTRUE, 100);
		if ((uxBits & (ASSEMBLE_BIT | ASSEMBLE_BIT_2)) == (ASSEMBLE_BIT | ASSEMBLE_BIT_2))
		{
			//Set all variables inside SensorDataPackageHandler
			set_dph_temperature(package_handler, tempget_measurment(HtSensor));
			set_dph_humidity(package_handler, humget_measurment(HtSensor));
			set_dph_co2(package_handler, co2get_measurment(co2sensor));
			
			//Call SDBH method to assemble the lora payload
			lora_payload = assemble_payload(package_handler);
			
			//send payload with message buffer
			xMessageBufferSend(uplink_buffer, (void*)lora_payload.bytes, sizeof(lora_payload.bytes), 100);
			//signalize the servo that all resources are safe to use and it can work independently for 5 minutes
			xEventGroupSetBits(eGroup, SERVO_BIT);
			vTaskDelay(2000);
			//restart the cycle, signalize the servo task that the resources are no longer safe to use
			xEventGroupClearBits(eGroup, SERVO_BIT);
			vTaskDelay(500);
			xEventGroupSetBits(eGroup, CO2_BIT);
		}
	}

}

void send_and_receive_task (void *pvParameters)
{
	
	for(;;)
	{
		BaseType_t isEmpty;
		uint16_t received_temperature;
		isEmpty = xStreamBufferIsEmpty(uplink_buffer);
		if (isEmpty == pdFALSE)
		{
			received_temperature = (configuration ,send_and_receive(uplink_buffer));
			xStreamBufferReset(uplink_buffer);
			vTaskDelay(200);
			if(received_temperature > 0)
			{
				set_max_temperature(configuration ,received_temperature);
			}

		}
		
	}
}

void temperature_controll_task (void *pvParameters)
{
	
	for(;;)
	{
		uxBits = xEventGroupWaitBits(eGroup, SERVO_BIT, pdFALSE, pdFALSE, 100);
		if ((uxBits & (SERVO_BIT)) == (SERVO_BIT))
		{
			temphum_meassure(HtSensor);
			set_temperature_current(configuration, tempget_measurment(HtSensor));
			operate_window(configuration);
			vTaskDelay(1000);
		}
		
		
	}
}



int main(void)
{
	initialiseSystem();
	printf("STARTED\n ");
	vTaskStartScheduler();
	
}

