#include "IN219.h"
#include "http_client.h"
#include "i2c_manager.h"
#include "secret.h"
#include "wifi.h"
#include <ArduinoJson.h>
#include <esp_log.h>
#include <nvs_flash.h>

struct TaskParams {
  QueueHandle_t queue;
  IN219 *sensor;
  HTTPClient *client;
};

struct Data {
  char timestamp[32];
  float current;
};

esp_err_t init(Wifi &wifi, I2CManager &i2c_manager, IN219 &in219);
esp_err_t init_nvs();
esp_err_t get_time(char *timestamp, uint32_t size);