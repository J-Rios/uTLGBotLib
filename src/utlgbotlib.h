/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbotlib.h
// Description: Lightweight library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 20 mar. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef UTLGBOTLIB_H_
#define UTLGBOTLIB_H_

/**************************************************************************************************/

/* Defines & Macros */

// Set to true or false to enable/disable FreeRTOS safe use of the input pin through multiples Tasks
//#define FREERTOS_MUTEX true

/**************************************************************************************************/

/* Libraries */

#ifdef ARDUINO
    #include <Arduino.h>
    #include <WiFiClientSecure.h>
#else /* ESP-IDF */
    /*#include "lwip/err.h"
    #include "lwip/sockets.h"
    #include "lwip/sys.h"
    #include "lwip/netdb.h"
    #include "lwip/dns.h"*/
    #include "esp_tls.h"
#endif

#include <inttypes.h>
#include <stdint.h>

/**************************************************************************************************/

/* Constants */

// Telegram Server address and address lenght
#define TELEGRAM_SERVER "https://api.telegram.org"
#define TELEGRAM_HOST "api.telegram.org"
#define TELEGRAM_SERVER_LENGTH 28

// Bot token max lenght (Note: Actual token lenght is 46, but it seems was increased in the past, 
// so we set it to 64)
#define TOKEN_LENGTH 64

// Telegram API address lenght
#define TELEGRAM_API_LENGTH (TELEGRAM_SERVER_LENGTH + TOKEN_LENGTH)

// Telegram HTTPS Server Port
#define HTTPS_PORT 443

// Maximum HTTP GET and POST data lenght
#define HTTP_MAX_URI_LENGTH 128
#define HTTP_MAX_BODY_LENGTH 1024
#define HTTP_MAX_GET_LENGTH HTTP_MAX_URI_LENGTH + 128
#define HTTP_MAX_POST_LENGTH HTTP_MAX_URI_LENGTH + HTTP_MAX_BODY_LENGTH
#define HTTP_MAX_RES_LENGTH 5121

// HTTP response wait timeout (ms)
#define HTTP_WAIT_RESPONSE_TIMEOUT 3000

/**************************************************************************************************/

/* Telegram API Commands and Contents */

// Commands
#define API_CMD_GET_ME "getMe"
#define API_CMD_SEND_MSG "sendMessage"

// Content of sendMessage
/*
#define API_CONT_SEND_MSG \
    "{" \
        "chat_id=%d," \
        "text=%s," \
        "parse_mode=%s," \
        "disable_web_page_preview=false," \
        "disable_notification=false," \
        "reply_to_message_id=%d," \
        "reply_markup=" \
    "}"
*/

/**************************************************************************************************/

/* Library Data Types */


/**************************************************************************************************/

class uTLGBot
{
    public:
        uTLGBot(const char* token);
        uint8_t connect(void);
        void disconnect(void);
        bool is_connected(void);
        uint8_t getMe(void);
        uint8_t sendMessage(const int64_t chat_id, const char* text, const char* parse_mode="", 
            bool disable_web_page_preview=false, bool disable_notification=false, 
            uint64_t reply_to_message_id=0);

    private:
        #ifdef ARDUINO
            WiFiClientSecure* _client;
        #else /* ESP-IDF */
            esp_tls_cfg_t* _tls_cfg;
            struct esp_tls* _tls;
        #endif
        char _token[TOKEN_LENGTH];
        char _tlg_api[TELEGRAM_API_LENGTH];
        char _response[HTTP_MAX_RES_LENGTH];
        bool _connected;

        uint8_t tlg_get(const char* command, char* response, const size_t response_len);
        uint8_t tlg_post(const char* command, const char* body, const size_t body_len, 
            char* response, const size_t response_len);

        void https_client_init(void);
        bool https_client_connect(const char* host, int port);
        void https_client_disconnect(void);
        bool https_client_is_connected(void);
        size_t https_client_write(const char* request);
        bool https_client_read(char* response, const size_t response_len);
        uint8_t https_client_get(const char* uri, const char* host, char* response, 
            const size_t response_len, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t https_client_post(const char* uri, const char* host, const char* body, 
            const uint64_t body_len, char* response, const size_t response_len, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

        bool cstr_read_until_word(char* str, const char* word, char* readed, const bool preserve);
};

/**************************************************************************************************/

#endif
