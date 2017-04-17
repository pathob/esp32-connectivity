#include <wifi.h>

// static WIFI_callbacks_t *wifi_callbacks = NULL;
static const char *TAG = "WIFI";

static void WIFI_init_station(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);

void WIFI_init(WIFI_callbacks_t* wifi_cb)
{
    if (wifi_cb == NULL) {
        // printf("NULL!!!!\n");
    }
    // *wifi_callbacks = *wifi_cb;

    WIFI_init_station();
    // TODO: Init AP
}

static void WIFI_init_station(void)
{
    tcpip_adapter_init();
    WIFI_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
        // TODO: Check code
        /*
        if (wifi_callbacks != NULL) {
            wifi_callbacks->wifi_connected_callback();
        }
        */
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        // workaround: ESP32 WiFi libs currently don't auto-reassociate
        esp_wifi_connect();
        xEventGroupClearBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT);
        // TODO: Check code
        /*
        if (wifi_callbacks.wifi_disconnected_callback != NULL) {
            wifi_callbacks.wifi_disconnected_callback();
        }
        */
        break;
    default:
        break;
    }
    return ESP_OK;
}
