#ifndef __CONNECTIVITY_CONNECTION_BITS__
#define __CONNECTIVITY_CONNECTION_BITS__

#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "soc/soc.h"

extern EventGroupHandle_t CONNECTIVITY_event_group;

uint32_t CONNECTIVITY_bit();

void CONNECTIVITY_device_id(
    char **device_id);

void CONNECTIVITY_wait(
    const EventBits_t bits);

void CONNECTIVITY_set(
    const EventBits_t bits);

void CONNECTIVITY_clear(
    const EventBits_t bits);

#endif