#ifndef __WIFI_H__
#define __WIFI_H__

#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_wifi_internal.h"
#include "esp_system.h"
#include "esp_log.h"

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "apps/sntp/sntp.h"

#define WIFI_STA_SSID CONFIG_WIFI_STA_SSID
#define WIFI_STA_PASS CONFIG_WIFI_STA_PASS

#define WIFI_AP_PASS  CONFIG_WIFI_AP_PASS
#define WIFI_AP_SSID  CONFIG_WIFI_AP_SSID

// are we connected with an AP
#define WIFI_STA_CONNECTED_BIT BIT0

// FreeRTOS event group to signal when we are connected & ready to make a request
EventGroupHandle_t WIFI_event_group;

/**
 * WIFI packet injection API
 */
esp_err_t esp_wifi_80211_tx(
        wifi_interface_t ifx,
        const void *buffer,
        int len,
        bool en_sys_seq);

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
void WIFI_init(
    wifi_mode_t wifi_mode,
    WIFI_callbacks_t *wifi_callbacks);

#endif
