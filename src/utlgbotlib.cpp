/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbot.h
// Description: Lightweight Library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 19 mar. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Libraries */

#include "utlgbotlib.h"
#include "tlgcert.h"

/**************************************************************************************************/

/* Macros */

#ifdef ARDUINO
    #define _print(x) do { Serial.print(x); } while(0)
    #define _println(x) do { Serial.println(x); } while(0)
    #define _printf(...) do { Serial.printf(__VA_ARGS__); } while(0)

    #define _millis() millis()
    #define _delay(x) do { delay(x); } while(0)
#else /* ESP-IDF */
    #define _millis() (unsigned long)(esp_timer_get_time()/1000)
    #define _print(x) do { printf(x); } while(0)
    #define _println(x) do { printf(x); printf("\n"); } while(0)
    #define _printf(...) do { printf(__VA_ARGS__); } while(0)

    #define F(x) x
    #define PSTR(x) x
    #define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
    
    #define _delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)
    #define PROGMEM
#endif

/**************************************************************************************************/

/* Constructor */

// TLGBot constructor, initialize and setup secure client with telegram cert and get the token
uTLGBot::uTLGBot(const char* token)
{
    https_client_init();
    
    snprintf(_token, TOKEN_LENGTH, "%s", token);
    snprintf(_tlg_api, TELEGRAM_API_LENGTH, "/bot%s", token);
    _connected = false;
}

/**************************************************************************************************/

/* Public Methods */

// Connect to Telegram server
uint8_t uTLGBot::connect(void)
{
    _println(F("[Bot] Connecting to telegram server..."));

    if(https_client_is_connected())
    {
        _println(F("[Bot] Already connected to server."));
        return true;
    }

    if(!https_client_connect(TELEGRAM_HOST, HTTPS_PORT))
    {
        _println(F("[Bot] Conection fail."));
        return false;
    }

    _println(F("[Bot] Successfully connected."));

    return true;
}

// Disconnect from Telegram server
void uTLGBot::disconnect(void)
{
    _println(F("[Bot] Disconnecting from telegram server..."));

    if(!https_client_is_connected())
    {
        _println(F("[Bot] Already disconnected from server."));
        return;
    }

    https_client_disconnect();

    _println(F("[Bot] Successfully disconnected."));
}

// Check for Bot connection to server status
bool uTLGBot::is_connected(void)
{
    return https_client_is_connected();
}

/**************************************************************************************************/

/* Private Methods - HAL functions (Implement network functionality for differents devices) */

// Initialize HTTPS client
void uTLGBot::https_client_init(void)
{
    #ifdef ARDUINO
        _client = new WiFiClientSecure();
        //_client->setCACert(cert_https_api_telegram_org);
    #else /* ESP-IDF */
        _tls = NULL;
        /*_tls_cfg.cacert_pem_buf = server_root_cert_pem_start;
        _tls_cfg.cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start;*/
        /*_tls_cfg.cacert_pem_buf = cert_https_api_telegram_org;
        _tls_cfg.cacert_pem_bytes = strlen((char*)cert_https_api_telegram_org);*/
        unsigned int cert_size = tlg_api_ca_pem_end - tlg_api_ca_pem_start;
        static esp_tls_cfg_t tls_cfg = {
            .alpn_protos = NULL,
            .cacert_pem_buf = tlg_api_ca_pem_start,
            .cacert_pem_bytes = cert_size,
            .non_block = false,
        };
        _tls_cfg = &tls_cfg;
    #endif
}

// Make HTTPS client connection to server
bool uTLGBot::https_client_connect(const char* host, int port)
{
    #ifdef ARDUINO
        return _client->connect(host, port);
    #else /* ESP-IDF */
        _tls = esp_tls_conn_new(host, strlen(host), port, _tls_cfg);
        if(_tls == NULL)
            _connected = false;
        else
            _connected = true;
        return _connected;
    #endif
}

// HTTPS client disconnect from server
void uTLGBot::https_client_disconnect(void)
{
    #ifdef ARDUINO
        _client->stop();
    #else
        esp_tls_conn_delete(_tls);
        _connected = false;
    #endif
}

// Check if HTTPS client is connected
bool uTLGBot::https_client_is_connected(void)
{
    #ifdef ARDUINO
        return _client->connected();
    #else /* ESP-IDF */
        // Note: ESP-IDF 3.1 doesn't has connection state in ESP-TLS
        /*if(_tls != NULL)
        {
            if(_tls->conn_state == ESP_TLS_DONE)
                return true;
        }
        return false;*/
        return _connected;
    #endif
}
