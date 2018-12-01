#include "connectivity/wifi.h"

#define NVS_WIFI_STORAGE  "WIFI"
#define NVS_WIFI_STA_SSID "WIFI_STA_SSID"
#define NVS_WIFI_STA_PASS "WIFI_STA_PASS"
#define NVS_WIFI_AP_SSID  "WIFI_AP_SSID"
#define NVS_WIFI_AP_PASS  "WIFI_AP_PASS"

static const char *TAG = "WIFI";

static const int8_t RSSI_MIN = -100;
static const int8_t RSSI_MAX = -55;

static WIFI_callbacks_t _wifi_callbacks;
static uint8_t _sta_is_connected = 0;
static ip4_addr_t _sta_ip4_addr;

static volatile EventBits_t _wifi_sta_connectivity_bit = 0;
static char _sta_ssid[WIFI_MAX_SSID_LENGTH] = { 0 };
static char _sta_pass[WIFI_MAX_PASS_LENGTH] = { 0 };

static volatile EventBits_t _wifi_ap_connectivity_bit = 0;

// Method declarations

static void WIFI_credentials_init();

static esp_err_t WIFI_sta_connectivity_set();

static esp_err_t WIFI_sta_connectivity_clear();

static int8_t WIFI_sta_rssi();

static void WIFI_config_sta();

static void WIFI_config_ap();

static esp_err_t event_handler(
    void *context,
    system_event_t *event);

// Method definitions

void WIFI_init(
    wifi_mode_t wifi_mode,
    WIFI_callbacks_t* wifi_callbacks)
{
    _wifi_sta_connectivity_bit = CONNECTIVITY_bit();
    _wifi_ap_connectivity_bit = CONNECTIVITY_bit();

    WIFI_credentials_init();
    tcpip_adapter_init();

    if (wifi_callbacks != NULL) {
        memcpy(&_wifi_callbacks, wifi_callbacks, sizeof(WIFI_callbacks_t));
    } else {
        WIFI_callbacks_t wifi_null_cb = { NULL, NULL };
        memcpy(&_wifi_callbacks, &wifi_null_cb, sizeof(WIFI_callbacks_t));
    }

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

static void WIFI_credentials_init()
{
    esp_err_t ret = nvs_flash_init();

    // TODO: NVS handling itself should probably moved elsewhere
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    } else if (ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE(TAG, "ESP_ERR_NVS_NEW_VERSION_FOUND");
    }

    nvs_handle nvs;
    ret = nvs_open(NVS_WIFI_STORAGE, NVS_READWRITE, &nvs);

    ESP_ERROR_CHECK( ret );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
        return;
    }

    if (strlen(CONFIG_WIFI_STA_SSID)) {
        ESP_LOGI(TAG, "Storing WIFI station SSID %s and password in NVS", CONFIG_WIFI_STA_SSID);

        ret = nvs_set_str(nvs, NVS_WIFI_STA_SSID, CONFIG_WIFI_STA_SSID);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        ret = nvs_set_str(nvs, NVS_WIFI_STA_PASS, CONFIG_WIFI_STA_PASS);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        ret = nvs_commit(nvs);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Could not store new NVS data");
        }

        // There is no need to read the just written data from NVS again
        strcpy(_sta_ssid, CONFIG_WIFI_STA_SSID);
        strcpy(_sta_pass, CONFIG_WIFI_STA_PASS);
    } else {
        ESP_LOGI(TAG, "Reading WIFI station credentials from NVS");

        size_t sta_ssid_len = sizeof(_sta_ssid);
        size_t sta_pass_len = sizeof(_sta_pass);

        ret = nvs_get_str(nvs, NVS_WIFI_STA_SSID, _sta_ssid, &sta_ssid_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
        }

        ret = nvs_get_str(nvs, NVS_WIFI_STA_PASS, _sta_pass, &sta_pass_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
        }
    }

    nvs_close(nvs);
}

esp_err_t WIFI_sta_connectivity_wait()
{
    return CONNECTIVITY_wait(_wifi_sta_connectivity_bit);
}

static esp_err_t WIFI_sta_connectivity_set()
{
    return CONNECTIVITY_set(_wifi_sta_connectivity_bit);
}

static esp_err_t WIFI_sta_connectivity_clear()
{
    return CONNECTIVITY_clear(_wifi_sta_connectivity_bit);
}

uint8_t WIFI_sta_is_connected()
{
    return _sta_is_connected;
}

ip4_addr_t WIFI_sta_ip4_addr()
{
    return _sta_ip4_addr;
}

static int8_t WIFI_sta_rssi()
{
    wifi_ap_record_t wifi_ap_record;
    esp_err_t esp_err = esp_wifi_sta_get_ap_info(&wifi_ap_record);

    if (!esp_err) {
        return wifi_ap_record.rssi;
    }

    return RSSI_MIN;
}

uint8_t WIFI_sta_rssi_level()
{
    int8_t rssi = WIFI_sta_rssi();

    if (rssi <= RSSI_MIN) {
        return 0;
    } else if (rssi >= RSSI_MAX) {
        return RSSI_LEVELS - 1;
    }

    float inputRange = (RSSI_MAX - RSSI_MIN);
    float outputRange = (RSSI_LEVELS - 1);
    return (int)((float)(rssi - RSSI_MIN) * outputRange / inputRange);
}

static void WIFI_config_sta()
{
    if (strlen(_sta_ssid)) {
        wifi_sta_config_t wifi_sta_config;
        memset(&wifi_sta_config, 0, sizeof(wifi_sta_config_t));
        memcpy(wifi_sta_config.ssid, _sta_ssid, strlen(_sta_ssid)+1);

        if (strlen(_sta_pass)) {
            memcpy(wifi_sta_config.password, _sta_pass, strlen(_sta_pass)+1);
        }

        wifi_config_t wifi_config;
        memset(&wifi_config, 0, sizeof(wifi_config_t));
        memcpy(&wifi_config.sta, &wifi_sta_config, sizeof(wifi_sta_config_t));

        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_LOGI(TAG, "Setting WiFi station configuration SSID %s...", wifi_config.sta.ssid);
    }
}

static void WIFI_config_ap()
{
    if (strlen(CONFIG_WIFI_AP_SSID)) {
        wifi_auth_mode_t wifi_auth_mode = WIFI_AUTH_WPA2_PSK;

        if (strlen(CONFIG_WIFI_AP_PASS) == 0) {
            wifi_auth_mode = WIFI_AUTH_OPEN;
        }

        wifi_config_t wifi_config = {
            .ap = {
                .ssid = CONFIG_WIFI_AP_SSID,
                .ssid_len = strlen(CONFIG_WIFI_AP_SSID),
                .password = CONFIG_WIFI_AP_PASS,
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
            _sta_is_connected = 1;
            _sta_ip4_addr = event->event_info.got_ip.ip_info.ip;

            WIFI_sta_connectivity_set();
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
            _sta_is_connected = 0;
            memset(&_sta_ip4_addr, 0, sizeof(ip4_addr_t));
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

            WIFI_sta_connectivity_clear();
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
