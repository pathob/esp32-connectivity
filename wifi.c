#include <wifi.h>

static WIFI_callbacks_t wifi_callbacks;
static const char *TAG = "WIFI";

static void WIFI_init_sta(void);
static void WIFI_init_ap(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);

void WIFI_init(WIFI_callbacks_t* wifi_cb)
{
    tcpip_adapter_init();
    if (wifi_cb != NULL) {
        memcpy(&wifi_callbacks, wifi_cb, sizeof(WIFI_callbacks_t));
    } else {
        WIFI_callbacks_t wifi_null_cb = { NULL, NULL };
        memcpy(&wifi_callbacks, &wifi_null_cb, sizeof(WIFI_callbacks_t));
    }

    // WIFI_init_sta();
    WIFI_init_ap();
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void WIFI_init_sta(void)
{
    WIFI_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&config) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi station configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
}

static void WIFI_init_ap(void)
{
    WIFI_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&config) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
	    .ap = {
	        .ssid = WIFI_AP_SSID,
	        .password = WIFI_AP_PASS,
	        .ssid_len = 10,
	        .channel = 0,
	        .authmode = WIFI_AUTH_WPA2_PSK,
	        .ssid_hidden = 0,
	        .max_connection = 4,
	        .beacon_interval = 100
	    }
	};
    ESP_LOGI(TAG, "Setting WiFi access point configuration SSID %s...", wifi_config.sta.ssid);
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
        if (wifi_callbacks.wifi_connected_callback != NULL) {
            wifi_callbacks.wifi_connected_callback();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        // workaround: ESP32 WiFi libs currently don't auto-reassociate
        esp_wifi_connect();
        xEventGroupClearBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
        if (wifi_callbacks.wifi_disconnected_callback != NULL) {
            wifi_callbacks.wifi_disconnected_callback();
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}
