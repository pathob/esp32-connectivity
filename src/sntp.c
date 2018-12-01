#include "connectivity/sntp.h"

static const char *TAG = "SNTP";

static volatile EventBits_t _sntp_connectivity_bit = 0;

static esp_err_t SNTP_connectivity_set();

void SNTP_init()
{
    // TODO: Besser start task

    _sntp_connectivity_bit = CONNECTIVITY_bit();
    ESP_ERROR_CHECK(WIFI_sta_connectivity_wait());

    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // wait for time to be set
    time_t now = 0;
    timeinfo_t timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry < retry_count) {
        char strftime_buf[64];

        // Set timezone to Eastern Standard Time and print local time
        setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
        tzset();
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

        SNTP_connectivity_set();
    } else {
        ESP_LOGE(TAG, "Setting time failed");
    }
}

esp_err_t SNTP_connectivity_wait()
{
    return CONNECTIVITY_wait(_sntp_connectivity_bit);
}

static esp_err_t SNTP_connectivity_set()
{
    return CONNECTIVITY_set(_sntp_connectivity_bit);
}