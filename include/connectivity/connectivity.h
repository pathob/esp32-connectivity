#ifndef __CONNECTIVITY_CONNECTION_BITS__
#define __CONNECTIVITY_CONNECTION_BITS__

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "soc/soc.h"

// are we connected...
#define WIFI_STA_CONNECTED BIT0
#define WIFI_AP_CONNECTED  BIT1
#define SNTP_TIME_SET      BIT2
#define MQTT_CONNECTED     BIT3

extern EventGroupHandle_t CONNECTIVITY_event_group;

void CONNECTIVITY_device_id(
    char **device_id);

void CONNECTIVITY_wait(
    const EventBits_t bits);

void CONNECTIVITY_set(
    const EventBits_t bits);

void CONNECTIVITY_clear(
    const EventBits_t bits);

#endif