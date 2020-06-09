#include <stdlib.h>
#include <string.h>
#include <ATMEGA_FreeRTOS.h>
#include <ihal.h>
#include <lora_driver.h>
#include "message_buffer.h"
#include "LorawanDriver.h"
#include "lora_driver.h"
#define LORA_appEUI "CEC19A451F80259C"
#define LORA_appKEY "7E3886820D8A21A19330ADC3D2028344"


bool network_setup() {
	if (lora_driver_rn2483_factory_reset() != LoRA_OK)
	{
		puts("factory reset failed");
	}
	
	if (lora_driver_configure_to_eu868() != LoRA_OK)
	{
		puts("eu686 configuration failed");
	}
	
	static char dev_eui[17]; // It is static to avoid it to occupy stack space in the task
	
	if (lora_driver_get_rn2483_hweui(dev_eui) != LoRA_OK)
	{
		puts("getting hweui failed");
	}
	
	if (lora_driver_set_otaa_identity(LORA_appEUI,LORA_appKEY,dev_eui) != LoRA_OK)
	{
		puts("setting identity failed");
	}
	
	if (lora_driver_save_mac() != LoRA_OK)
	{
		puts("saving config failed");
	}
	
	if (lora_driver_join(LoRa_OTAA) == LoRa_ACCEPTED)
	{
		puts("managed to join the network");
		return true;
	}
	return false;
	
}


e_LoRa_return_code_t lorawan_send(lora_payload_t uplink_payload) {
	e_LoRa_return_code_t lc;
	lora_driver_sent_upload_message(false, &uplink_payload);
	return lc;
}

uint16_t send_and_receive(MessageBufferHandle_t received_buffer)
{
	size_t xDataBytes;
	MessageBufferHandle_t payload_received;
	uint16_t co2_to_set = 0;
	lora_payload_t payload_to_send;
	e_LoRa_return_code_t lrc;
	xDataBytes = xMessageBufferReceive(received_buffer, (void*)payload_to_send.bytes, sizeof(payload_to_send), 200);
	payload_to_send.port_no = 1;
	payload_to_send.len = 6;
	lrc = lora_driver_sent_upload_message(false, &payload_to_send);
	//the return code specifies whether data was sent correctly and whether there is any downlink
	if ((lrc == LoRa_MAC_TX_OK  ))
	{
		puts("sent successfully, no down link");
		xStreamBufferReset(received_buffer);
	}

	else if(lrc == LoRa_MAC_RX)
	{
		xDataBytes = xMessageBufferReceive(payload_received, (void*)co2_to_set, sizeof(co2_to_set), 200);
		if (xDataBytes > 0)
		{
			puts("sent successfully, with down link");
			return co2_to_set; 
		}
	}
	else
	{
		puts("couldn't send the data, please contact the administrator");
	}
	
	return co2_to_set;
}
	






