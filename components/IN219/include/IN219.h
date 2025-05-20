#pragma once

#include "i2c_manager.h"

class IN219 {
public:
  IN219(I2CManager &i2c_manager);

  esp_err_t init();
  esp_err_t read_current(float &current);

private:
  I2CManager &_i2c;
  i2c_master_dev_handle_t _device_handle;

  float _current_LSB = 0.025f; // 25 uA/bit

  esp_err_t read_register(uint8_t reg, uint16_t &data);
  esp_err_t write_register(uint8_t reg, uint16_t data);
  esp_err_t configure();

  // Device address
  const uint8_t IN219_ADDRESS = 0x40;

  // Register map
  const uint8_t REG_CONFIG = 0x00;
  const uint8_t REG_CURRENT = 0x04;
  const uint8_t REG_CALIBRATION = 0x05;

  // Configuration register bits
  const uint16_t BUS_VOLTAGE_RANGE = 0xDFFF; // 16V
  const uint16_t PGA = 0xEFFF;               // Gain

  // Calibration value for 800 mA max
  const uint16_t CALIBRATION_VALUE = (0x4000 << 1); // last bit is always 0
};