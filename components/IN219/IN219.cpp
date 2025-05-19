#include "IN219.h"
#include "esp_log.h"
#include "esp_random.h"

constexpr auto *TAG = "IN219";

IN219::IN219(I2CManager &i2c_manager)
    : _i2c_manager(i2c_manager), _address(0x40) {
  _current_lsb = 0.001;
  _voltage_lsb = 0.001;
  _power_lsb = 0.01;
}

esp_err_t IN219::init() { return ESP_OK; }

esp_err_t IN219::read_current(float &current) {
  float min_current = 0.0f;
  float max_current = 500.0f;
  uint32_t rand_val = esp_random();
  float random_fraction =
      static_cast<float>(rand_val) / static_cast<float>(UINT32_MAX);
  current = min_current + random_fraction * (max_current - min_current);
  return ESP_OK;
}