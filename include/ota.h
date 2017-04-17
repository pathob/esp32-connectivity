#ifndef __OTA_H__
#define __OTA_H__

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_err.h"

#include "nvs_flash.h"

#include "wifi.h"

#define OTA_HOST_IP    CONFIG_OTA_HOST_IP
#define OTA_HOST_PORT  CONFIG_OTA_HOST_PORT
#define OTA_IMAGE_FILE CONFIG_OTA_IMAGE_FILE
#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

void OTA_task(void *pvParameter);

#endif
