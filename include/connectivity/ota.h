#ifndef __OTA_H__
#define __OTA_H__

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_types.h"

#include "nvs_flash.h"
#include "cJSON.h"

#include "wifi.h"

void OTA_init();

#endif
