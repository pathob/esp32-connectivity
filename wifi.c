#include "wifi.h"

static const char *TAG = "WIFI";

static void WIFI_config_sta();

static void WIFI_config_ap();

static esp_err_t event_handler(
    void *context,
    system_event_t *event);

static WIFI_callbacks_t _wifi_callbacks;

void WIFI_init(
    wifi_mode_t wifi_mode,
    WIFI_callbacks_t* wifi_callbacks)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK( ret );

    tcpip_adapter_init();

    if (wifi_callbacks != NULL) {
        memcpy(&_wifi_callbacks, wifi_callbacks, sizeof(WIFI_callbacks_t));
    } else {
        WIFI_callbacks_t wifi_null_cb = { NULL, NULL };
        memcpy(&_wifi_callbacks, &wifi_null_cb, sizeof(WIFI_callbacks_t));
    }

    WIFI_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&config) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(wifi_mode) );

    if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA) {
        WIFI_config_sta();
    }

    if (wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA) {
        WIFI_config_ap();
    }

    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void WIFI_config_sta()
{
    if (strlen(WIFI_STA_SSID)) {
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = WIFI_STA_SSID,
                .password = WIFI_STA_PASS,
            }
        };

        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_LOGI(TAG, "Setting WiFi station configuration SSID %s...", wifi_config.sta.ssid);
    }
}

static void WIFI_config_ap()
{
    if (strlen(WIFI_AP_SSID)) {
        wifi_config_t wifi_config = {
            .ap = {
                .ssid = WIFI_AP_SSID,
                .password = WIFI_AP_PASS,
                .ssid_len = 10,
                .channel = 0,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .ssid_hidden = 0,
                .max_connection = 4,
                .beacon_interval = 100,
            }
        };

        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
        ESP_LOGI(TAG, "Setting WiFi access point configuration SSID %s...", wifi_config.ap.ssid);
    }
}

static esp_err_t event_handler(
    void *context,
    system_event_t *event)
{
    esp_err_t ret = ESP_OK;

    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ret = esp_wifi_connect();
            break;
        }
        case SYSTEM_EVENT_STA_CONNECTED: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
            break;
        }
        case SYSTEM_EVENT_STA_GOT_IP: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            xEventGroupSetBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
            if (_wifi_callbacks.wifi_connected_callback) {
                _wifi_callbacks.wifi_connected_callback();
            }
            break;
        }
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_AUTHMODE_CHANGE");
            break;
        }
        case SYSTEM_EVENT_STA_LOST_IP: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_LOST_IP");
            break;
        }
        case SYSTEM_EVENT_STA_STOP: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_STOP");
            break;
        }
        case SYSTEM_EVENT_STA_DISCONNECTED: {
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            // workaround: ESP32 WiFi libs currently don't auto-reassociate
            esp_wifi_connect();
            xEventGroupClearBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
            if (_wifi_callbacks.wifi_disconnected_callback) {
                _wifi_callbacks.wifi_disconnected_callback();
            }
            break;
        }
        default: {
            break;
        }
    }

    if (ret) {
        ESP_LOGE(TAG, "%s failed, error code = %x\n", __func__, ret);
    }

    return ret;
}
