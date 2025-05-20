#include "IN219.h"
#include "esp_log.h"
#include "esp_random.h"

constexpr auto *TAG = "IN219";

IN219::IN219(I2CManager &i2c_manager)
    : _i2c(i2c_manager), _device_handle(nullptr) {}

esp_err_t IN219::init() {
  i2c_master_bus_handle_t bus_handle = _i2c.get_bus_handle();
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = I2C_DEVICE_ADDRESS_NOT_USED,
      .scl_speed_hz = 400000,
  };

  esp_err_t err =
      i2c_master_bus_add_device(bus_handle, &dev_cfg, &_device_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add device to I2C bus: %d", err);
    return err;
  }

  // Probe device to check if it's connected
  if (_i2c.probe(IN219_ADDRESS) != ESP_OK) {
    return ESP_ERR_NOT_FOUND;
  }

  if (configure() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure IN219");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t IN219::read_current(float &current) {
  if (_device_handle == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  uint16_t raw_value;
  esp_err_t err = read_register(REG_CURRENT, raw_value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read current register: %s", esp_err_to_name(err));
    return err;
  }

  int16_t raw_value_signed = static_cast<int16_t>(raw_value);
  current = raw_value_signed * _current_LSB; // current in mA
  return ESP_OK;
}

esp_err_t IN219::read_register(uint8_t reg, uint16_t &data) {
  if (_device_handle == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t write_address = (IN219_ADDRESS << 1);    // LSB = 0 for write
  uint8_t read_address = (IN219_ADDRESS << 1 | 1); // LSB = 1 for read

  uint8_t write_data[2] = {write_address, reg};
  uint8_t read_data[2] = {};

  i2c_operation_job_t read[] = {
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 2}},
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = &read_address, .total_bytes = 1}},
      {.command = I2C_MASTER_CMD_READ,
       .read = {.ack_value = I2C_ACK_VAL,
                .data = &read_data[0],
                .total_bytes = 1}},
      {.command = I2C_MASTER_CMD_READ,
       .read = {.ack_value = I2C_NACK_VAL,
                .data = &read_data[1],
                .total_bytes = 1}},
      {.command = I2C_MASTER_CMD_STOP},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, read, 7, 1000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read register: %s", esp_err_to_name(err));
    return err;
  }

  data = (static_cast<uint16_t>(read_data[0]) << 8) | read_data[1];
  return ESP_OK;
}

esp_err_t IN219::write_register(uint8_t reg, uint16_t data) {
  if (_device_handle == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t address = (IN219_ADDRESS << 1); // LSB = 0 for write

  uint8_t write_data[4] = {address, reg, static_cast<uint8_t>(data >> 8),
                           static_cast<uint8_t>(data & 0xFF)};

  i2c_operation_job_t write[] = {
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 4}},
      {.command = I2C_MASTER_CMD_STOP},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, write, 3, 1000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register: %s", esp_err_to_name(err));
    return err;
  }
  return ESP_OK;
}

esp_err_t IN219::configure() {
  // Set the configuration register
  uint16_t config = 0;
  esp_err_t err = read_register(REG_CONFIG, config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure IN219: %s", esp_err_to_name(err));
    return err;
  }
  config &= BUS_VOLTAGE_RANGE & PGA;
  err = write_register(REG_CONFIG, config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write configuration register: %s",
             esp_err_to_name(err));
    return err;
  }

  // Set calibration value
  err = write_register(REG_CALIBRATION, CALIBRATION_VALUE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set calibration value: %s", esp_err_to_name(err));
    return err;
  }

  return ESP_OK;
}