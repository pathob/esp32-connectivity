#include "connectivity/connectivity.h"

static const char *TAG = "CONNECTIVITY";

static volatile uint32_t _connectivity_current_bit = 0;

static volatile uint8_t _connectivity_init_done = 0;
static EventGroupHandle_t _connectivity_event_group;
static char _connectivity_device_id[12];

static void CONNECTIVITY_init() {
    if (!_connectivity_init_done) {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        sprintf(_connectivity_device_id, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(TAG, "Device ID: %.12s", _connectivity_device_id);

        _connectivity_event_group = xEventGroupCreate();
        _connectivity_init_done = 1;
    }
}

EventBits_t CONNECTIVITY_bit()
{
    return BIT(_connectivity_current_bit++);
}

void CONNECTIVITY_device_id(
    char *device_id)
{
    CONNECTIVITY_init();
    memcpy(device_id, &_connectivity_device_id, sizeof(char) * 12);
}

esp_err_t CONNECTIVITY_wait(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    if (bits == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    xEventGroupWaitBits(_connectivity_event_group, bits, false, true, portMAX_DELAY);
    return ESP_OK;
}

esp_err_t CONNECTIVITY_set(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    if (bits == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    xEventGroupSetBits(_connectivity_event_group, bits);
    return ESP_OK;
}

esp_err_t CONNECTIVITY_clear(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    if (bits == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    xEventGroupClearBits(_connectivity_event_group, bits);
    return ESP_OK;
}

uint8_t CONNECTIVITY_get(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    if (xEventGroupGetBits(_connectivity_event_group) & bits) {
        return 1;
    }
    return 0;
}
