#pragma once

#include "esp_http_client.h"
#include <string>

class HTTPClient {
public:
  HTTPClient(const std::string &url);
  ~HTTPClient();

  esp_err_t post(const std::string &json);

private:
  esp_http_client_handle_t client;
};
