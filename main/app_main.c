#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/**
 * Losant Credentials
 * put your device ID, access key, and access
 * secret here to be used throughout the application. 
*/
#define LOSANT_DEVICE_ID "Your Losant Device ID"
#define LOSANT_ACCESS_KEY "Your Losant Access Key"
#define LOSANT_ACCESS_SECRET "Your Losant Access Secret"

/**
 * LOGGING_TAG is a variable used throughout for logging
 * purposes. You can customize the value of the variable.
*/
static const char *LOGGING_TAG = "ESP32_GETTING_STARTED";

/**
 * @sendMessage 
 * 
 * This function creates the Losant device state topic for your 
 * device. 
 * 
 * It then publishes a messages to that topic. 
 * 
 * pvParameters and vTaskDelay are used for FreeRTOS task management
*/
void sendMessage(void *pvParameters)
{
    esp_mqtt_client_handle_t client = *((esp_mqtt_client_handle_t *)pvParameters);

    // create topic variable
    char topic[128];

    //set topic value using sprintf to the losant state topic for our device
    sprintf(topic, "losant/%s/state", LOSANT_DEVICE_ID);

    // Using FreeRTOS task management, forever loop, and send state to the topic
    for (;;)
    {
        esp_mqtt_client_publish(client, topic, "{\"data\": {\"message\": \"hello from ESP32\",\"number\": 14}}", 0, 1, 0);

        vTaskDelay(pdMS_TO_TICKS(5000)); // wait 5 seconds
    }
}

/**
 * On device connect, using the MQTT events provided by the esp-idf
 * this subscribes to the command topic with MQTT_EVENT_CONNECTED
 * 
 * When a command is received, the MQTT_EVENT_DATA is triggered.
 * 
 * */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;

    // define Losant device Command topic
    char command_topic[128];
    sprintf(command_topic, "losant/%s/command", LOSANT_DEVICE_ID);

    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // on connect, subscribe to the command topic
        msg_id = esp_mqtt_client_subscribe(client, command_topic, 0);
        ESP_LOGI(LOGGING_TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(LOGGING_TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    default:
        ESP_LOGI(LOGGING_TAG, "Other event id:%d", event->event_id);
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(LOGGING_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
    ESP_ERROR_CHECK(example_connect());

    /**
     * MQTT client configuration, the variables DO NOT need to be updated
     * you can add more config information from:
     * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mqtt.html 
    */
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://broker.losant.com",
        .client_id = LOSANT_DEVICE_ID,
        .username = LOSANT_ACCESS_KEY,
        .password = LOSANT_ACCESS_SECRET};

    // establish mqtt client information, then start mqtt client
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    // Create sendMessage task.
    TaskHandle_t xHandle = NULL;
    xTaskCreate(sendMessage, "READ", 2048, &client, tskIDLE_PRIORITY, &xHandle);
}
