#pragma once
#include <stdio.h>
#include <lora_driver.h>


bool network_setup();

e_LoRa_return_code_t lorawan_send(lora_payload_t uplink_payload_t);

uint16_t send_and_receive(MessageBufferHandle_t received_buffer);