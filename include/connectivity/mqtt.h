#ifndef __CONNECTIVITY_MQTT_H__
#define __CONNECTIVITY_MQTT_H__

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "mqtt_client.h"
#include "mqtt_config.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "connectivity/connectivity.h"
#include "connectivity/wifi.h"

#include "sdkconfig.h"

typedef enum {
    MQTT_QOS_AT_MOST_ONCE = 0x0,
    MQTT_QOS_AT_LEAST_ONCE,
    MQTT_QOS_EXACTLY_ONCE,
    MQTT_QOS_MAX
} MQTT_qos_t;

typedef struct MQTT_callback_handler_t {
    void (*init_handler)(void);
    void (*connected_handler)(esp_mqtt_event_handle_t);
    void (*disconnected_handler)(esp_mqtt_event_handle_t);
    void (*subscribed_handler)(esp_mqtt_event_handle_t);
    void (*unsubscribed_handler)(esp_mqtt_event_handle_t);
    void (*published_handler)(esp_mqtt_event_handle_t);
    void (*data_handler)(esp_mqtt_event_handle_t);
} MQTT_callback_handler_t;

void MQTT_init();

int MQTT_subscribe(
    const char *topic,
    MQTT_qos_t qos);

int MQTT_unsubscribe(
    const char *topic);

int MQTT_publish(
    const char *topic,
    const char *data,
    int len,
    MQTT_qos_t qos,
    int retain);

void MQTT_connectivity_wait();

#endif