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

/**************************************************************************************************/

/* Constructor */

// uTLGBot constructor, initialize and setup secure client with telegram cert and get the token
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
    printf("[Bot] Connecting to telegram server...\n");

    if(https_client_is_connected())
    {
        printf("[Bot] Already connected to server.\n");
        return true;
    }

    if(!https_client_connect(TELEGRAM_HOST, HTTPS_PORT))
    {
        printf("[Bot] Conection fail.\n");
        return false;
    }

    printf("[Bot] Successfully connected.\n");

    return true;
}

// Disconnect from Telegram server
void uTLGBot::disconnect(void)
{
    printf("[Bot] Disconnecting from telegram server...\n");

    if(!https_client_is_connected())
    {
        printf("[Bot] Already disconnected from server.\n");
        return;
    }

    https_client_disconnect();

    printf("[Bot] Successfully disconnected.\n");
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
}

// Make HTTPS client connection to server
bool uTLGBot::https_client_connect(const char* host, int port)
{
    _tls = esp_tls_conn_new(host, strlen(host), port, _tls_cfg);
    if(_tls == NULL)
        _connected = false;
    else
        _connected = true;
    return _connected;
}

// HTTPS client disconnect from server
void uTLGBot::https_client_disconnect(void)
{
    esp_tls_conn_delete(_tls);
    _connected = false;
}

// Check if HTTPS client is connected
bool uTLGBot::https_client_is_connected(void)
{
    // Note: ESP-IDF 3.1 doesn't has connection state in ESP-TLS
    /*if(_tls != NULL)
    {
        if(_tls->conn_state == ESP_TLS_DONE)
            return true;
    }
    return false;*/
    return _connected;
}
