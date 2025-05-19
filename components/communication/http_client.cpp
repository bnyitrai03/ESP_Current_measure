#include "http_client.h"
#include "esp_log.h"

constexpr auto *TAG = "HTTP Client";

HTTPClient::HTTPClient(const std::string &url) {
  esp_http_client_config_t config = {
      .url = url.c_str(),
      .method = HTTP_METHOD_POST,
      .disable_auto_redirect = true,
      .transport_type = HTTP_TRANSPORT_OVER_TCP,
      .buffer_size = 512,
  };

  client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to initialize HTTP client");
    return;
  }
  if (esp_http_client_set_header(client, "Content-Type", "application/json") !=
      ESP_OK) {
    ESP_LOGE(TAG, "Failed to set header");
  }
}

HTTPClient::~HTTPClient() {
  if (client) {
    esp_http_client_cleanup(client);
  }
}

esp_err_t HTTPClient::post(const std::string &jsonBody) {
  esp_err_t err = esp_http_client_set_post_field(client, jsonBody.c_str(),
                                                 jsonBody.length());
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set post field: %s", esp_err_to_name(err));
    return err;
  }

  err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
  }
  return err;
}
