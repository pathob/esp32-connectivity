#include "connectivity/connectivity.h"

static const char *TAG = "CONNECTIVITY";

static volatile uint8_t _connectivity_init_done = 0;
static volatile uint32_t _connectivity_current_bit = 0;
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

uint32_t CONNECTIVITY_bit()
{
    return BIT(_connectivity_current_bit++);
}

void CONNECTIVITY_device_id(
    char **device_id)
{
    CONNECTIVITY_init();
    *device_id = &_connectivity_device_id;
}

void CONNECTIVITY_wait(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    xEventGroupWaitBits(_connectivity_event_group, bits, false, true, portMAX_DELAY);
}

void CONNECTIVITY_set(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    xEventGroupSetBits(_connectivity_event_group, bits);
}

void CONNECTIVITY_clear(
    const EventBits_t bits)
{
    CONNECTIVITY_init();
    xEventGroupClearBits(_connectivity_event_group, bits);
}