#ifndef __CONNECTIVITY_CONNECTION_BITS__
#define __CONNECTIVITY_CONNECTION_BITS__

#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "soc/soc.h"

extern EventGroupHandle_t CONNECTIVITY_event_group;

EventBits_t CONNECTIVITY_bit();

void CONNECTIVITY_device_id(
    char *device_id);

esp_err_t CONNECTIVITY_wait(
    const EventBits_t bits);

esp_err_t CONNECTIVITY_set(
    const EventBits_t bits);

esp_err_t CONNECTIVITY_clear(
    const EventBits_t bits);

uint8_t CONNECTIVITY_get(
    const EventBits_t bits);

#endif