idf_component_register(SRCS "wifi.cpp" "http_client.cpp" "i2c_manager.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi esp_event esp_netif esp_http_client esp_driver_i2c)
