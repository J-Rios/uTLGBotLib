/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbotlib.h
// Description: Lightweight library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 26 apr. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef UTLGBOTLIB_H_
#define UTLGBOTLIB_H_

/**************************************************************************************************/

/* Check Build System */

#if !defined(ESPIDF) && !defined(ARDUINO) && !defined(WIN32) && !defined(_WIN32) && \
    !defined(__linux__)
    #error Unsupported system (Supported: Windows, Linux and ESP32)
#endif

/**************************************************************************************************/

/* Libraries */

#if defined(ARDUINO) // ESP32 Arduino Framework
    #include <Arduino.h>
    #include <WiFiClientSecure.h>
#elif defined(IDF_VER) // ESP32 ESPIDF Framework
    #include "esp_tls.h"
#else // Generic devices (intel, amd, arm) and OS (windows, Linux)
    #if defined(WIN32) || defined(_WIN32) // Windows
        #include <windows.h>
    #endif
    #include <stdio.h>
    #include <time.h>
    #include <unistd.h>

    // MBEDTLS library
    #include "mbedtls/net.h"
    #include "mbedtls/ssl.h"
    #include "mbedtls/entropy.h"
    #include "mbedtls/ctr_drbg.h"
    #include "mbedtls/certs.h"
    #include "mbedtls/debug.h"
    #include "mbedtls/error.h"
#endif

//#define __STDC_LIMIT_MACROS // Could be needed for C++, and it must be before inttypes include
//#define __STDC_CONSTANT_MACROS // Could be needed for C++, and it must be before inttypes include
#define __STDC_FORMAT_MACROS  // Could be needed for C++, and it must be before inttypes include
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "jsmn.h"

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

// HTTP TLS connection timeout (ms)
#define HTTP_CONNECT_TIMEOUT 5000

// HTTP response wait timeout (ms)
#define HTTP_WAIT_RESPONSE_TIMEOUT 3000

// Maximum HTTP GET and POST data lenght
#define HTTP_MAX_URI_LENGTH 128
#define HTTP_MAX_BODY_LENGTH 1024
#define HTTP_MAX_GET_LENGTH HTTP_MAX_URI_LENGTH + 128
#define HTTP_MAX_POST_LENGTH HTTP_MAX_URI_LENGTH + HTTP_MAX_BODY_LENGTH
#define HTTP_MAX_RES_LENGTH 5121

// Telegram getUpdate Long Poll value (s)
#define TLG_LONG_POLL 10

// JSON Max values length
#define MAX_JSON_STR_LEN 1024
#define MAX_JSON_SUBVAL_STR_LEN 512
#define MAX_JSON_ELEMENTS 128
#define MAX_JSON_SUBELEMENTS 64

// Telegram data types Max values length
#define MAX_USER_LENGTH 32
#define MAX_USERNAME_LENGTH 32
#define MAX_LANGUAGE_CODE_LENGTH 8
#define MAX_CHAT_TYPE_LENGTH 16
#define MAX_CHAT_TITLE_LENGTH 32
#define MAX_CHAT_DESCRIPTION_LENGTH 128
#define MAX_URL_LENGTH 64
#define MAX_STICKER_NAME 32
#define MAX_TEXT_LENGTH 1024

/**************************************************************************************************/

/* Telegram API Commands and Contents */

// Commands
#define API_CMD_GET_ME "getMe"
#define API_CMD_SEND_MSG "sendMessage"
#define API_CMD_GET_UPDATES "getUpdates"

/**************************************************************************************************/

/* Telegram Data Types (Not all of them are implemented) */

// User: https://core.telegram.org/bots/api#user
typedef struct tlg_type_user
{
    int64_t id;
    bool is_bot;
    char first_name[MAX_USER_LENGTH];
    char last_name[MAX_USER_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char language_code[MAX_LANGUAGE_CODE_LENGTH];
} tlg_type_user;

// Chat: https://core.telegram.org/bots/api#chat
typedef struct tlg_type_chat
{
    int64_t id;
    char type[MAX_CHAT_TYPE_LENGTH];
    char title[MAX_CHAT_TITLE_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char first_name[MAX_USER_LENGTH];
    char last_name[MAX_USER_LENGTH];
    bool all_members_are_administrators;
    //tlg_chatphoto_entity photo; // Uninplemented
    //char description[MAX_CHAT_DESCRIPTION_LENGTH]; // Uninplemented
    //char invite_link[MAX_URL_LENGTH]; // Uninplemented
    //tlg_type_message pinned_message; // Uninplemented
    //char sticker_set_name[MAX_STICKER_NAME]; // Uninplemented
    //bool can_set_sticker_set; // Uninplemented
} tlg_type_chat;

// Message: https://core.telegram.org/bots/api#message
typedef struct tlg_type_message
{
    int64_t message_id;
    tlg_type_user from;
    uint32_t date;
    tlg_type_chat chat;
    char text[MAX_TEXT_LENGTH];
    //tlg_type_user forward_from;
    //tlg_type_chat forward_from_chat;
    //int32_t forward_from_message_id;
    //...
} tlg_type_message;

/**************************************************************************************************/

class uTLGBot
{
    public:
        // Public Attributtes
        tlg_type_message received_msg;

        // Public Methods
        uTLGBot(const char* token);
        #if !defined(ARDUINO) && !defined(ESPIDF) // Windows or Linux
            ~uTLGBot(void);
        #endif
        uint8_t connect(void);
        void disconnect(void);
        bool is_connected(void);
        uint8_t getMe(void);
        uint8_t sendMessage(const int64_t chat_id, const char* text, const char* parse_mode="", 
            bool disable_web_page_preview=false, bool disable_notification=false, 
            uint64_t reply_to_message_id=0);
        uint8_t getUpdates(void);

    private:
        // Private Attributtes
        #if defined(ARDUINO) // ESP32 Arduino Framework
            WiFiClientSecure* _client;
        #elif defined(IDF_VER) // ESP32 ESPIDF Framework
            struct esp_tls* _tls;
            esp_tls_cfg_t* _tls_cfg;
        #else // Windows and Linux
            mbedtls_net_context _server_fd;
            mbedtls_entropy_context _entropy;
            mbedtls_ctr_drbg_context _ctr_drbg;
            mbedtls_ssl_context _tls;
            mbedtls_ssl_config _tls_cfg;
            mbedtls_x509_crt _cacert;
        #endif
        char _token[TOKEN_LENGTH];
        char _tlg_api[TELEGRAM_API_LENGTH];
        char _response[HTTP_MAX_RES_LENGTH];
        jsmntok_t _json_elements[MAX_JSON_ELEMENTS];
        jsmntok_t _json_subelements[MAX_JSON_SUBELEMENTS];
        char _json_value_str[MAX_JSON_STR_LEN];
        char _json_subvalue_str[MAX_JSON_SUBVAL_STR_LEN];
        size_t _last_received_msg;

        // Private Methods - High
        uint8_t tlg_get(const char* command, char* response, const size_t response_len, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t tlg_post(const char* command, const char* body, const size_t body_len, 
            char* response, const size_t response_len, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        
        // Private Methods - Low (HAL specifics)
        bool https_client_init(void);
        int8_t https_client_connect(const char* host, int port);
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

        // Private Methods - Auxiliar Functions
        void clear_msg_data(void);
        uint32_t json_parse_str(const char* json_str, const size_t json_str_len, 
            jsmntok_t* json_tokens, const uint32_t json_tokens_len);
        uint32_t json_has_key(const char* json_str, jsmntok_t* json_tokens, 
            const uint32_t num_tokens, const char* key);
        void json_get_element_string(const char* json_str, jsmntok_t* token, char* converted_str, 
            const uint32_t converted_str_len);
        uint8_t json_get_key_value(const char* key, const char* json_str, jsmntok_t* tokens, 
            const uint32_t num_tokens, char* converted_str, const uint32_t converted_str_len);
        bool cstr_read_until_word(char* str, const char* word, char* readed, const bool preserve);
};

/**************************************************************************************************/

#endif
