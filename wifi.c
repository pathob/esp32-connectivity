#include "wifi.h"

static const char *TAG = "WIFI";

static void WIFI_config_sta();

static void WIFI_config_ap();

static void SNTP_obtain_time();

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

    if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA) {
        SNTP_obtain_time();
    }
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
        wifi_auth_mode_t wifi_auth_mode = WIFI_AUTH_WPA2_PSK;

        if (strcmp(WIFI_AP_PASS, "") == 0) {
            wifi_auth_mode = WIFI_AUTH_OPEN;
        }

        wifi_config_t wifi_config = {
            .ap = {
                .ssid = WIFI_AP_SSID,
                .ssid_len = strlen(WIFI_AP_SSID),
                .password = WIFI_AP_PASS,
                .authmode = wifi_auth_mode,
                .channel = 0,
                .ssid_hidden = 0,
                .max_connection = 4,
                .beacon_interval = 100,
            }
        };

        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
        ESP_LOGI(TAG, "Setting WiFi access point configuration SSID %s...", wifi_config.ap.ssid);
    }
}

static void SNTP_obtain_time()
{
    xEventGroupWaitBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT, false, true, portMAX_DELAY);

    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);
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
