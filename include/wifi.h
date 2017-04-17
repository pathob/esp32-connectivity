#ifndef __WIFI_H__
#define __WIFI_H__

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_log.h"

#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_STA_SSID CONFIG_WIFI_STA_SSID
#define WIFI_STA_PASS CONFIG_WIFI_STA_PASS

#define WIFI_AP_PASS  CONFIG_WIFI_AP_PASS
#define WIFI_AP_SSID  CONFIG_WIFI_AP_SSID

// are we connected with an AP
#define WIFI_STA_CONNECTED_BIT BIT0

// FreeRTOS event group to signal when we are connected & ready to make a request
EventGroupHandle_t WIFI_event_group;

/**
 * Currently only callback for station mode
 */
typedef struct WIFI_callbacks_t {
    void (*wifi_connected_callback)(void);
    void (*wifi_disconnected_callback)(void);
} WIFI_callbacks_t;

/**
 * Init WIFI
 *
 * Pass NULL to set no function callbacks or
 * pass a WIFI_callbacks_t to specify function callbacks
 */
void WIFI_init(WIFI_callbacks_t *wifi_cb);

#endif
