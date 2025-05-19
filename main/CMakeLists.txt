idf_component_register(SRCS "src/main.cpp" "src/utilities.cpp"
                       PRIV_INCLUDE_DIRS "include"
                       PRIV_REQUIRES communication esp_psram nvs_flash IN219)
