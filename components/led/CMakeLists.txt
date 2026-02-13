idf_component_register(
    SRCS "led.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_netif esp_wifi log vfs
)
