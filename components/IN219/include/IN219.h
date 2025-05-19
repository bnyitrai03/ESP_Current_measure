#pragma once

#include "i2c_manager.h"

class IN219 {
public:
  IN219(I2CManager &i2c_manager);

  esp_err_t init();
  esp_err_t read_current(float &current);

private:
  I2CManager &_i2c_manager;
  uint8_t _address;
  float _current_lsb;
  float _voltage_lsb;
  float _power_lsb;

  esp_err_t read_register(uint8_t reg, uint16_t &data);
  esp_err_t write_register(uint8_t reg, uint16_t data);
  esp_err_t read_calibration_data();
};