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

  float _current_LSB = 0.024414f; // 400mA: 12.207 uA

  esp_err_t read_register(uint8_t reg, uint16_t &data);
  esp_err_t write_register(uint8_t reg, uint16_t data);
  esp_err_t configure();

  // Device address
  const uint8_t IN219_ADDRESS = 0x40;

  // Register map
  const uint8_t REG_CONFIG = 0x00;
  const uint8_t REG_CURRENT = 0x04;
  const uint8_t REG_CALIBRATION = 0x05;

  // Configuration bits
  const uint16_t BUS_VOLTAGE_RANGE = 0xDFFF; // 16V
  const uint16_t PGA = 0xEFFF;               // Gain = 1: 0xE7FF
  const uint16_t ADC_CONFIG =
      0x03B8; // 12-bit resolution with 68.1 ms averaging: 0x07F8
  const uint16_t CALIBRATION_VALUE =
      0x4190; // last bit is always 0 (400 mA: 0x8312)
};