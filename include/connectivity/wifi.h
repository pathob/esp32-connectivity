#ifndef __WIFI_H__
#define __WIFI_H__

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_wifi_internal.h"
#include "esp_system.h"
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "connectivity/connectivity.h"

#define RSSI_LEVELS 4

#define WIFI_MAX_SSID_LENGTH 32
#define WIFI_MAX_PASS_LENGTH 64

typedef struct tm timeinfo_t;

// FreeRTOS event group to signal when we are connected & ready to make a request

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

uint8_t WIFI_sta_is_configured();

uint8_t WIFI_sta_is_connected();

esp_err_t WIFI_sta_connectivity_wait();

ip4_addr_t WIFI_sta_ip4_addr();

uint8_t WIFI_sta_rssi_level();

uint8_t WIFI_ap_is_configured();

uint8_t WIFI_ap_is_connected();

esp_err_t WIFI_ap_connectivity_wait();

#endif
