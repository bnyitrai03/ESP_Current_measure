#include "secret.h"
#include "utilities.h"
#include <ArduinoJson.h>
#include <esp_log.h>

constexpr auto *TAG = "Main app";

// ***************** FUNCTION AND DATA PROTOTYPES **************
void read_task(void *pvParameter);
void post_task(void *pvParameter);
// *********************************************************

// *********************** MAIN APP ***********************
extern "C" void app_main(void) {
  if (init_nvs() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init NVS!");
    return;
  }

  Wifi wifi;
  I2CManager i2c_manager;
  IN219 in219(i2c_manager);
  if (init(wifi, i2c_manager, in219) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init components!");
    return;
  }
  HTTPClient http_client(SERVER_URL);

  const int QUEUE_LENGTH = 20000;
  StaticQueue_t *queue_struct = (StaticQueue_t *)heap_caps_malloc(
      sizeof(StaticQueue_t), MALLOC_CAP_SPIRAM);
  uint8_t *queue_storage = (uint8_t *)heap_caps_malloc(
      QUEUE_LENGTH * sizeof(Data), MALLOC_CAP_SPIRAM);
  QueueHandle_t queue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(Data),
                                           queue_storage, queue_struct);
  if (queue == NULL) {
    ESP_LOGE(TAG, "Failed to create queue");
    return;
  }

  // Create tasks
  TaskParams read_params = {
      .queue = queue, .sensor = &in219, .client = nullptr};
  BaseType_t res =
      xTaskCreate(&read_task, "read_task", 8096, &read_params, 5, NULL);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create sensor reader task");
    return;
  }
  TaskParams post_params = {
      .queue = queue, .sensor = nullptr, .client = &http_client};
  res = xTaskCreate(&post_task, "post_task", 8096, &post_params, 8, NULL);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create HTTP post task");
    return;
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
// *********************************************************

// *********************** TASKS ***************************
void read_task(void *pvParameter) {
  TaskParams *params = (TaskParams *)pvParameter;
  QueueHandle_t queue = params->queue;
  IN219 *sensor = params->sensor;
  Data data;

  while (true) {
    if (sensor->read_current(data.current) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read current!");
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (get_time(data.timestamp, sizeof(data.timestamp)) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to get time!");
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (xQueueSend(queue, &data, 10) != pdTRUE) {
      ESP_LOGE(TAG, "Failed to send message to queue!");
    }
    ESP_LOGI(TAG, "Current: %.2f mA, Timestamp: %s", data.current,
             data.timestamp);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void post_task(void *pvParameter) {
  TaskParams *params = (TaskParams *)pvParameter;
  QueueHandle_t queue = params->queue;
  HTTPClient *client = params->client;
  Data data;
  JsonDocument doc;
  std::string message;

  while (true) {
    if (xQueueReceive(queue, &data, portMAX_DELAY) == pdTRUE) {
      doc.clear();
      doc["timestamp"] = data.timestamp;
      doc["current"] = data.current;
      serializeJson(doc, message);
      if (client->post(message) == ESP_OK) {
        ESP_LOGI(TAG, "Data sent successfully");
      }
    }
  }
}

// *********************************************************