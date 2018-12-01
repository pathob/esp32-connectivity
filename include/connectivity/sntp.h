#ifndef __CONNECTIVITY_SNTP_H__
#define __CONNECTIVITY_SNTP_H__

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_wifi_internal.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/apps/sntp.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "connectivity/connectivity.h"
#include "connectivity/wifi.h"

void SNTP_init();

esp_err_t SNTP_connectivity_wait();

#endif