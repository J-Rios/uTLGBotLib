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
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

// Custom libraries
#include "utlgbotlib.h"

/**************************************************************************************************/

// WiFi Parameters
#define WIFI_SSID "mynet1234"
#define WIFI_PASS "password1234"
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63

// Telegram Bot Token (Get from Botfather)
#define TLG_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

/**************************************************************************************************/

/* Functions Prototypes */

extern "C" { void app_main(void); }
void nvs_init(void);
void wifi_init_stat(void);
static esp_err_t network_event_handler(void *ctx, system_event_t *e);

/**************************************************************************************************/

/* Globals */
volatile bool wifi_connected = false;
volatile bool wifi_has_ip = false;

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
        if(!wifi_connected || !wifi_has_ip)
        {
            // Wait 100ms and check again
            vTaskDelay(100/portTICK_PERIOD_MS);
            continue;
        }

        // Check and handle any received message
        while(Bot.getUpdates())
        {
            printf("Message received from %s, echo it back...\n", Bot.received_msg.from.first_name);
            if(!Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text))
            {
                printf("Send fail.\n");
                continue;
            }
            printf("Send OK.\n\n");
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
    static wifi_init_config_t wifi_init_cfg;
    static wifi_config_t wifi_cfg;

    printf("Initializing TCP-IP adapter...\n");

    tcpip_adapter_init();

    wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Set TCP-IP event handler callback
    ESP_ERROR_CHECK(esp_event_loop_init(network_event_handler, NULL));

    // Create and launch WiFi Station
    memcpy(wifi_cfg.sta.ssid, WIFI_SSID, MAX_LENGTH_WIFI_SSID+1);
    memcpy(wifi_cfg.sta.password, WIFI_PASS, MAX_LENGTH_WIFI_PASS+1);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("TCP-IP adapter successfuly initialized.\n");
}

/**************************************************************************************************/

/* WiFi Change Event Handler */

static esp_err_t network_event_handler(void *ctx, system_event_t *e)
{
    static uint8_t conn_fail_retries = 0;

    switch(e->event_id)
    {
        case SYSTEM_EVENT_STA_START:
            printf("WiFi Station interface Up.\n");
            printf("Connecting...\n");
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            printf("WiFi connected.\n");
            printf("Waiting for IP...\n");
            wifi_connected = true;
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            printf("WiFi IPv4 received: %s\n", ip4addr_ntoa(&e->event_info.got_ip.ip_info.ip));
            wifi_has_ip = true;
            break;

        case SYSTEM_EVENT_STA_LOST_IP:
            printf("WiFi IP lost.\n");
            wifi_has_ip = false;
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            if(wifi_connected)
            {
                printf("WiFi disconnected\n");
                conn_fail_retries = 0;
            }
            else
            {
                printf("Can't connect to AP, trying again...\n");
                conn_fail_retries = conn_fail_retries + 1;
            }
            wifi_has_ip = false;
            wifi_connected = false;
            if(conn_fail_retries < MAX_CONN_FAIL)
                esp_wifi_connect();
            else
            {
                printf("WiFi connection fail %d times.\n", MAX_CONN_FAIL);
                printf("Rebooting the system...\n\n");
                esp_restart();
            }
            break;

        case SYSTEM_EVENT_STA_STOP:
            printf("WiFi interface stopped\n");
            conn_fail_retries = 0;
            wifi_has_ip = false;
            wifi_connected = false;
            break;

        default:
            break;
    }
    
    return ESP_OK;
}

/**************************************************************************************************/
