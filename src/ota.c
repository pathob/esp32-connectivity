#include "connectivity/ota.h"

static const char *TAG = "OTA";

extern const uint8_t s3_amazonaws_com_pem_start[] asm("_binary_s3_amazonaws_com_pem_start");
extern const uint8_t s3_amazonaws_com_pem_end[]   asm("_binary_s3_amazonaws_com_pem_end");

#ifndef CONFIG_OTA_UPDATE_INTERVAL
#define CONFIG_OTA_UPDATE_INTERVAL 1
#endif

static const uint32_t _update_interval_ms = CONFIG_OTA_UPDATE_INTERVAL * 60000;

static void OTA_task(
    void *pvParameter);

static void OTA_start();

static void OTA_github_update_check(
    const char *repo_slug,
    const char *tag);

void parse_releases(
    const char *data,
    const char *current_version);

static uint8_t OTA_current_version_uptodate(
    const char *current_version,
    const char *available_version);

void OTA_update(
    const char *update_url,
    const char *cert_pem);;

void OTA_init()
{
#ifdef CONFIG_OTA_CHECK_AFTER_BOOT_BLOCKING
    OTA_start();
#endif
    xTaskCreate(&OTA_task, "OTA_task", 4096, NULL, 10, NULL);
}

static void OTA_task(
    void *pvParameter)
{
    ESP_LOGI(TAG, "Start task");
#if defined(CONFIG_OTA_CHECK_AFTER_BOOT) && !defined(CONFIG_OTA_CHECK_AFTER_BOOT_BLOCKING)
    OTA_start();
#endif

    while (1) {
        vTaskDelay(_update_interval_ms / portTICK_PERIOD_MS);
        OTA_start();
    }
}

static void OTA_start()
{
#if defined(GITHUB_REPO_SLUG) && defined(GITHUB_TAG)
    OTA_github_update_check(GITHUB_REPO_SLUG, GITHUB_TAG);
#endif
}

static void OTA_github_update_check(
    const char *repo_slug,
    const char *current_version)
{
    esp_err_t err;

    ESP_LOGI(TAG, "Repo Slug: %s", repo_slug);
    ESP_LOGI(TAG, "Tag: %s", current_version);

    const char *api_github_format = "https://api.github.com/repos/%s/releases/latest";
    char *api_url = calloc(strlen(api_github_format) + strlen(repo_slug), sizeof(char));
    sprintf(api_url, api_github_format, repo_slug);

    ESP_LOGI(TAG, "API URL: %s", api_url);

    esp_http_client_config_t http_config_api = {
        .url = api_url,
    };

    esp_http_client_handle_t http_client_api = esp_http_client_init(&http_config_api);

    if ((err = esp_http_client_open(http_client_api, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return;
    }

    int status = esp_http_client_get_status_code(http_client_api);
    int content_length =  esp_http_client_fetch_headers(http_client_api);

    ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d", status, content_length);

    char *buffer = calloc(content_length + 1, sizeof(char));
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }

    esp_http_client_read(http_client_api, buffer, content_length);
    esp_http_client_close(http_client_api);
    esp_http_client_cleanup(http_client_api);

    cJSON *json = cJSON_Parse(buffer);
    
    if (json == NULL) {
        ESP_LOGE(TAG, "Cannot parse JSON data");
        goto _end;
    }

    const cJSON *tag = cJSON_GetObjectItemCaseSensitive(json, "tag_name");
    if (cJSON_IsString(tag) && (tag->valuestring != NULL)) {
        ESP_LOGI(TAG, "Found tag %s in latest release", tag->valuestring);
        
        if (OTA_current_version_uptodate(current_version, tag->valuestring)) {
            goto _end;
        }
    } else {
        ESP_LOGE(TAG, "Could not parse tag in response");
        goto _end;
    }

    const cJSON *asset = NULL;
    const cJSON *assets = cJSON_GetObjectItemCaseSensitive(json, "assets");

    cJSON_ArrayForEach(asset, assets) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(asset, "name");

        if (cJSON_IsString(name) && (name->valuestring != NULL)) {
            ESP_LOGI(TAG, "Found asset name %s", name->valuestring);
            if (strcmp("app.bin", name->valuestring) != 0) {
                goto _continue;
            }
        }

        const cJSON *browser_download_url = cJSON_GetObjectItemCaseSensitive(asset, "browser_download_url");
        
        if (cJSON_IsString(browser_download_url) && (browser_download_url->valuestring != NULL)) {
            ESP_LOGI(TAG, "Found download url %s", browser_download_url->valuestring);

            esp_http_client_config_t http_config_url = {
                .url = browser_download_url->valuestring,
                .disable_auto_redirect = 1,
                .buffer_size = 1024,
            };

            char *binary_url = NULL;
            esp_http_client_handle_t http_client_url = esp_http_client_init(&http_config_url);
            esp_http_client_perform(http_client_url);

            if ((err = esp_http_client_get_location(http_client_url, &binary_url)) == 0) {
                ESP_LOGI(TAG, "Could not get real download URL");
                goto _end;
            }

            ESP_LOGI(TAG, "Would update from URL %s", binary_url);

            OTA_update(binary_url, (char*) s3_amazonaws_com_pem_start);
            esp_http_client_cleanup(http_client_url);
            goto _end;
        }
_continue:
        ; // Need empty statement after label
    }

_end:
    free(buffer);
    cJSON_Delete(json);
}

void OTA_update(
    const char *update_url,
    const char *cert_pem)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "Start updating from URL %s", update_url);

    esp_http_client_config_t http_config_ota = {
        .url = update_url,
        .cert_pem = cert_pem,
        .buffer_size = 1024,
    };

    ret = esp_https_ota(&http_config_ota);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware update failed");
    }
}

static uint8_t OTA_current_version_uptodate(
    const char *current_version,
    const char *available_version)
{
    int cur[3], ava[3];
    sscanf(current_version, "%d.%d.%d", &cur[0], &cur[1], &cur[2]);
    sscanf(available_version, "%d.%d.%d", &ava[0], &ava[1], &ava[2]);

    for (uint8_t i = 0; i < 3; i++) {
        if (cur[i] > ava[i])
            return 1;
        else if (cur[i] < ava[i])
            return 0;
    }

    return 1;
}
