#include "connectivity/connectivity.h"

static volatile uint8_t _connectivity_init_done = 0;
static EventGroupHandle_t _connectivity_event_group;

static void CONNECTIVITY_init() {
    if (!_connectivity_init_done) {
        _connectivity_event_group = xEventGroupCreate();
        _connectivity_init_done = 1;
    }
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