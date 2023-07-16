/**************************************************************************************************/
// Example: echobot
// Description:
//   Bot that response to any received text message with the same text received (echo messages).
//   It gives you a basic idea of how to receive and send messages.
// Created on: 21 apr. 2019
// Last modified date: 21 apr. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// Device libraries (ESP-IDF)
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Lightweight TCP-IP Stack library
#include "lwip/err.h"
#include "lwip/sys.h"

// Telegram libraries
#include "utlgbotlib.h"

/**************************************************************************************************/

/* Defines & Constants */

// WiFi Parameters
#define WIFI_SSID "mynet1234"
#define WIFI_PASS "password1234"
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 32
#define MAX_LENGTH_WIFI_PASS 64

// Telegram Bot Token (Get from Botfather)
#define TLG_TOKEN "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Component Tag identification
static const char TAG[] = "echobot";

// The event group allows multiple bits for each event, but we only care about two events:
// - we are connected to the AP with an IP
// - we failed to connect after the maximum amount of retries
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/**************************************************************************************************/

/* Functions Prototypes */

extern "C" { void app_main(void); }
void nvs_init(void);
void wifi_init_stat(void);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
    void* event_data);
bool is_wifi_connected();

/**************************************************************************************************/

/* Globals */

// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;

// Number of connection retries
static int s_retry_num = 0;

/**************************************************************************************************/

/* Main Function */

void app_main(void)
{
    // Create Bot object
    uTLGBot Bot(TLG_TOKEN);

    // Initialize Non-Volatile-Storage and WiFi station connection
    nvs_init();
    wifi_init_stat();

    // Main loop
    while(1)
    {
        // Check if device is not connected
        if(is_wifi_connected() == false)
        {
            // Wait 100ms and check again
            vTaskDelay(100/portTICK_PERIOD_MS);
            continue;
        }

        // Check and handle any received message
        while(Bot.getUpdates())
        {
            ESP_LOGI(TAG, "Message received from %s, echo it back...\n",
                Bot.received_msg.from.first_name);
            if(!Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text))
            {
                ESP_LOGI(TAG, "Send fail.\n");
                continue;
            }
            ESP_LOGI(TAG, "Send OK.\n\n");
        }

        // Wait 1s for next iteration
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

/**************************************************************************************************/

/* Functions */

// Initialize Non-Volatile-Storage
void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// Init WiFi interface
void wifi_init_stat(void)
{
    ESP_LOGI(TAG, "Initializing TCP-IP adapter...\n");

    s_wifi_event_group = xEventGroupCreate();

    // Start network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi with default config
    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    // Register Events
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // Prepare WiFi SSID and Password
    static wifi_config_t wifi_cfg;
    size_t len = strlen(WIFI_SSID);
    if (len > MAX_LENGTH_WIFI_SSID)
    {   len = MAX_LENGTH_WIFI_SSID;   }
    memcpy(wifi_cfg.sta.ssid, WIFI_SSID, len);
    len = strlen(WIFI_PASS);
    if (len > MAX_LENGTH_WIFI_PASS)
    {   len = MAX_LENGTH_WIFI_PASS;   }
    memcpy((void*)wifi_cfg.sta.ssid, (const void*)WIFI_PASS, len);
    //wifi_cfg.sta.thresholdwifi_cfg.sta.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    //wifi_cfg.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    // Set Station Mode and apply custom config
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "TCP-IP adapter successfuly initialized.\n");
}

// Check if WiFi connection and IP is available
bool is_wifi_connected()
{
    bool wifi_ready = false;

    // Waiting until either the connection is established (WIFI_CONNECTED_BIT)
    // or connection failed for the maximum number of retries (WIFI_FAIL_BIT).
    // The bits are set by event_handler()
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned,
    // hence we can test which event actually happened
    if (bits & WIFI_CONNECTED_BIT)
    {   wifi_ready = true;   }

    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", WIFI_SSID);
    }
    else
    {   ESP_LOGE(TAG, "Unexpected event");   }

    return wifi_ready;
}

/**************************************************************************************************/

/* WiFi Change Event Handler */

static void event_handler(void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data)
{
    // WiFi Interface started and ready
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {   esp_wifi_connect();   }

    // WiFi Disconnection
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG,"Disconnected from AP");
        if (s_retry_num < MAX_CONN_FAIL)
        {
            s_retry_num = s_retry_num + 1;
            ESP_LOGI(TAG, "Retrying to connect...");
            esp_wifi_connect();
        }
        else
        {   xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);   }
    }

    // Received IP
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**************************************************************************************************/
