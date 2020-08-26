#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"

static const char *TAG = "MQTT_EXAMPLE";

// function to publish message to mqtt topic
void sendMessage(void *pvParameters)
{
    esp_mqtt_client_handle_t client = *((esp_mqtt_client_handle_t *)pvParameters);

    for (;;)
    {

        esp_mqtt_client_publish(client, "losant/<device id>/state", "{\"data\": {\"message\": \"hello from ESP32\",\"number\": 14}}", 0, 1, 0);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        msg_id = esp_mqtt_client_subscribe(client, "losant/<device id>/command", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
    ESP_ERROR_CHECK(example_connect());

    // you can add more config information from:
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mqtt.html
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://broker.losant.com",
        .client_id = "<device id>",
        .username = "<access_key>",
        .password = "<access_secret>"};

    // establish mqtt client information, then start mqtt client
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    // Create sensor task.
    TaskHandle_t xHandle = NULL;
    xTaskCreate(sendMessage, "READ", 2048, &client, tskIDLE_PRIORITY, &xHandle);

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
