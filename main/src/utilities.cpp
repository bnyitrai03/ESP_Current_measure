#include "utilities.h"
#include <esp_log.h>

constexpr auto *TAG = "Utilities";

esp_err_t init(Wifi &wifi, I2CManager &i2c_manager, IN219 &in219) {
  if (wifi.connect(SSID, PASSWORD) != ESP_OK) {
    return ESP_FAIL;
  }

  if (wifi.sync_time() != ESP_OK) {
    return ESP_FAIL;
  }

  if (i2c_manager.init() != ESP_OK) {
    return ESP_FAIL;
  }

  if (in219.init() != ESP_OK) {
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t init_nvs() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    ret = nvs_flash_init();
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to init flash!");
      return ret;
    }
  }
  if (ret != ESP_OK && ret != ESP_ERR_NVS_NEW_VERSION_FOUND &&
      ret != ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_LOGE(TAG, "Failed to init flash!");
  }
  return ret;
}

esp_err_t get_time(char *timestamp, uint32_t size) {
  struct timeval tv;
  struct tm timeinfo;

  // Get current time with microsecond precision
  if (gettimeofday(&tv, NULL) != 0) {
    ESP_LOGE(TAG, "Failed to get time of day");
    return ESP_FAIL;
  }

  localtime_r(&tv.tv_sec, &timeinfo);
  uint32_t milliseconds = tv.tv_usec / 1000;

  if (snprintf(timestamp, size, "%02d:%02d:%02d.%03ld", timeinfo.tm_hour,
               timeinfo.tm_min, timeinfo.tm_sec, milliseconds) < 0) {
    ESP_LOGE(TAG, "Error formatting timestamp");
    return ESP_FAIL;
  }

  return ESP_OK;
}