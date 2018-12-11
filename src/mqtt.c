#include "connectivity/mqtt.h"

#define NVS_MQTT_STORAGE  "MQTT"
#define NVS_MQTT_HOST     "MQTT_HOST"
#define NVS_MQTT_PORT     "MQTT_PORT"
#define NVS_MQTT_USERNAME "MQTT_USERNAME"
#define NVS_MQTT_PASSWORD "MQTT_PASSWORD"
#define NVS_MQTT_CLIENTID "MQTT_CLIENTID"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t _mqtt_client;
static MQTT_callback_handler_t _mqtt_callback_handler;
static uint32_t _mqtt_connectivity_bit;

static char mqtt_host[64];
static uint16_t mqtt_port;
static char mqtt_username[64];
static char mqtt_password[64];
// static char mqtt_clientid[64];

static void MQTT_client_init();

static esp_err_t MQTT_event_handler(
    esp_mqtt_event_handle_t event);

static void MQTT_connectivity_set();

static void MQTT_connectivity_clear();

void MQTT_init(
    MQTT_callback_handler_t *mqtt_callback_handler)
{
    _mqtt_connectivity_bit = CONNECTIVITY_bit();

    if (mqtt_callback_handler != NULL) {
        memcpy(&_mqtt_callback_handler, mqtt_callback_handler, sizeof(MQTT_callback_handler_t));
    } else {
        MQTT_callback_handler_t mqtt_callback_handler = { NULL };   
        memcpy(&_mqtt_callback_handler, &mqtt_callback_handler, sizeof(MQTT_callback_handler_t));
    }

    if (_mqtt_callback_handler.init_handler) {
        _mqtt_callback_handler.init_handler();
    }

    MQTT_client_init();

    ESP_ERROR_CHECK(WIFI_sta_connectivity_wait());
    esp_mqtt_client_start(_mqtt_client);
}


static void MQTT_client_init()
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
    ret = nvs_open(NVS_MQTT_STORAGE, NVS_READWRITE, &nvs);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
        return;
    }

    esp_mqtt_client_config_t mqtt_client_config;
    memset(&mqtt_client_config, 0, sizeof(esp_mqtt_client_config_t));

    uint8_t nvs_written = 0;

    if (strlen(CONFIG_MQTT_HOST)) {
        ret = nvs_set_str(nvs, NVS_MQTT_HOST, CONFIG_MQTT_HOST);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        strcpy(mqtt_host, CONFIG_MQTT_HOST);
        nvs_written = 1;
    } else {
        size_t mqtt_host_len = sizeof(mqtt_host);
        ret = nvs_get_str(nvs, NVS_MQTT_HOST, mqtt_host, &mqtt_host_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
        }
    }

    if (CONFIG_MQTT_PORT) {
        ret = nvs_set_u16(nvs, NVS_MQTT_PORT, (uint16_t) CONFIG_MQTT_PORT);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        mqtt_port = CONFIG_MQTT_PORT;
        nvs_written = 1;
    } else {
        ret = nvs_get_u16(nvs, NVS_MQTT_PORT, &mqtt_port);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
            mqtt_port = 0;
        }
    }

    if (strlen(CONFIG_MQTT_USERNAME)) {
        ret = nvs_set_str(nvs, NVS_MQTT_USERNAME, CONFIG_MQTT_USERNAME);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        strcpy(mqtt_username, CONFIG_MQTT_USERNAME);
        nvs_written = 1;
    } else {
        size_t mqtt_username_len = sizeof(mqtt_username);
        ret = nvs_get_str(nvs, NVS_MQTT_USERNAME, mqtt_username, &mqtt_username_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
        }
    }

    if (strlen(CONFIG_MQTT_PASSWORD)) {
        ret = nvs_set_str(nvs, NVS_MQTT_PASSWORD, CONFIG_MQTT_PASSWORD);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing NVS", esp_err_to_name(ret));
        }

        strcpy(mqtt_password, CONFIG_MQTT_PASSWORD);
        nvs_written = 1;
    } else {
        size_t mqtt_password_len = sizeof(mqtt_password);
        ret = nvs_get_str(nvs, NVS_MQTT_PASSWORD, mqtt_password, &mqtt_password_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading NVS", esp_err_to_name(ret));
        }
    }

    if (nvs_written) {
        ret = nvs_commit(nvs);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Could not store new NVS data");
        }
    }

    // CONFIG_MQTT_CLIENTID can be done later as it has a good default value

    mqtt_client_config.host = mqtt_host;
    mqtt_client_config.username = mqtt_username;
    mqtt_client_config.password = mqtt_password;

    if (mqtt_port) {
        mqtt_client_config.port = mqtt_port;
    }

    mqtt_client_config.event_handle = MQTT_event_handler;
    _mqtt_client = esp_mqtt_client_init(&mqtt_client_config);

    nvs_close(nvs);
}

int MQTT_subscribe(
    const char *topic,
    int qos)
{
    if (!WIFI_sta_is_connected()) {
        ESP_LOGE(TAG, "Cannot subscribe to topic '%s', no Wi-Fi connection", topic);
        return 0;
    }

    return esp_mqtt_client_subscribe(_mqtt_client, topic, qos);
}

int MQTT_publish(
    const char *topic,
    const char *data,
    int len,
    int qos,
    int retain)
{
    if (!WIFI_sta_is_connected()) {
        ESP_LOGE(TAG, "Cannot publish to topic '%s', no Wi-Fi connection", topic);
        return 0;
    }

    return esp_mqtt_client_publish(_mqtt_client, topic, data, len, qos, retain);
}

static esp_err_t MQTT_event_handler(
    esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            MQTT_connectivity_set();

            if (_mqtt_callback_handler.connected_handler != NULL) {
                _mqtt_callback_handler.connected_handler(event);
            }
            break;
        }
        case MQTT_EVENT_DISCONNECTED: {
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            MQTT_connectivity_clear();

            if (_mqtt_callback_handler.disconnected_handler != NULL) {
                _mqtt_callback_handler.disconnected_handler(event);
            }
            break;
        }
        case MQTT_EVENT_SUBSCRIBED: {
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED");

            if (_mqtt_callback_handler.subscribed_handler != NULL) {
                _mqtt_callback_handler.subscribed_handler(event);
            }
            break;
        }
        case MQTT_EVENT_UNSUBSCRIBED: {
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED");

            if (_mqtt_callback_handler.unsubscribed_handler != NULL) {
                _mqtt_callback_handler.unsubscribed_handler(event);
            }
            break;
        }
        case MQTT_EVENT_PUBLISHED: {
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED");

            if (_mqtt_callback_handler.published_handler != NULL) {
                _mqtt_callback_handler.published_handler(event);
            }
            break;
        }
        case MQTT_EVENT_DATA: {
            ESP_LOGD(TAG, "MQTT_EVENT_DATA");

            if (_mqtt_callback_handler.data_handler != NULL) {
                _mqtt_callback_handler.data_handler(event);
            }
            break;
        }
        case MQTT_EVENT_ERROR: {
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        }
    }

    return ESP_OK;
}

void MQTT_connectivity_wait()
{
    CONNECTIVITY_wait(_mqtt_connectivity_bit);
}

static void MQTT_connectivity_set()
{
    CONNECTIVITY_set(_mqtt_connectivity_bit);
}

static void MQTT_connectivity_clear()
{
    CONNECTIVITY_clear(_mqtt_connectivity_bit);
}