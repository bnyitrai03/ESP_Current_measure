#include "wifi.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "string.h"
#include <cstring>

constexpr auto *TAG = "WiFi";

EventGroupHandle_t Wifi::_wifi_event_group = nullptr;
bool Wifi::_connected = false;

Wifi::Wifi() {
  esp_log_level_set("wifi_init", ESP_LOG_WARN);
  esp_log_level_set("wifi", ESP_LOG_WARN);

  _wifi_event_group = xEventGroupCreate();
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  if (esp_wifi_init(&cfg) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize the WiFi stack!");
  }

  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &Wifi::eventHandler, NULL, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &Wifi::eventHandler, NULL, NULL);
}

Wifi::~Wifi() {
  _connected = false;
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
}

esp_err_t Wifi::connect(const char *ssid, const char *password) {
  esp_err_t ret;
  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password));

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  ret = esp_wifi_start();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WiFi station!");
    return ret;
  }

  ESP_LOGI(TAG, "Waiting for WiFi connection...");

  EventBits_t bits =
      xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE,
                          pdFALSE, pdMS_TO_TICKS(15000));

  if (bits & WIFI_CONNECTED_BIT) {
    _connected = true;
    ESP_LOGI(TAG, "WiFi Connected!");
  } else {
    ESP_LOGE(TAG, "WiFi couldn't connect!");
    ret = ESP_FAIL;
  }

  return ret;
}

esp_err_t Wifi::sync_time() {
  ESP_LOGI(TAG, "Syncing time with NTP server...");
  // Set timezone to Budapest
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();
  // Configure SNTP
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  esp_netif_sntp_init(&config);
  esp_err_t ret = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(20000));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure NTP server!");
  }
  return ret;
}

void Wifi::eventHandler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED && _connected) {
    ESP_LOGI(TAG, "Retry connecting to the AP");
    esp_wifi_connect();
    xEventGroupClearBits(_wifi_event_group, WIFI_CONNECTED_BIT);
    vTaskDelay(pdMS_TO_TICKS(100));
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}