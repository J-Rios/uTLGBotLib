/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbotlib.h
// Description: Lightweight library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 12 apr. 2020
// Version: 1.0.3
/**************************************************************************************************/

/* Include Guard */

#ifndef UTLGBOTLIB_H_
#define UTLGBOTLIB_H_

/**************************************************************************************************/

/* Libraries Configurations */

// If uTLGBot library without debug was set, disable debug in Multihttpsclient library too
#ifdef UTLGBOT_NO_DEBUG
    #define MULTIHTTPSCLIENT_NO_DEBUG
#endif

// Set default and limit memory usage level
#ifndef UTLGBOT_MEMORY_LEVEL
    #define UTLGBOT_MEMORY_LEVEL 5
#endif
#if UTLGBOT_MEMORY_LEVEL < 0
    #undef UTLGBOT_MEMORY_LEVEL
    #define UTLGBOT_MEMORY_LEVEL 0
#endif
#if UTLGBOT_MEMORY_LEVEL > 5
    #undef UTLGBOT_MEMORY_LEVEL
    #define UTLGBOT_MEMORY_LEVEL 5
#endif

// Integer types macros
//#define __STDC_LIMIT_MACROS // Could be needed for C++, and it must be before inttypes include
//#define __STDC_CONSTANT_MACROS // Could be needed for C++, and it must be before inttypes include
#define __STDC_FORMAT_MACROS  // Could be needed for C++, and it must be before inttypes include

/**************************************************************************************************/

/* Libraries Inclusion */

#if defined(ARDUINO) // Arduino Framework
    #include <Arduino.h>
#endif

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "utility/multihttpsclient/multihttpsclient.h"
#include "utility/jsmn/jsmn.h"
#include "tlgcert.h"

/**************************************************************************************************/

/* Constants */

// Telegram HTTPS Server Port
#define HTTPS_PORT 443

// Telegram Server address and address lenght
#define TELEGRAM_SERVER "https://api.telegram.org"
#define TELEGRAM_HOST "api.telegram.org"
#define TELEGRAM_SERVER_LENGTH 28

// Bot token max lenght (Note: Actual token lenght is 46, but it seems was increased in the past, 
// so we set it to 64)
#define TOKEN_LENGTH 64

// Telegram API address lenght
#define TELEGRAM_API_LENGTH (TELEGRAM_SERVER_LENGTH + TOKEN_LENGTH)

// Default Telegram getUpdate Long Poll value (s)
#define DEFAULT_TELEGRAM_LONG_POLL_S 1

// Telegram data types Max values length
#define MAX_ID_LENGTH 24
#define MAX_USER_LENGTH 32
#define MAX_USERNAME_LENGTH 32
#define MAX_LANGUAGE_CODE_LENGTH 8
#define MAX_CHAT_TYPE_LENGTH 16
#define MAX_CHAT_TITLE_LENGTH 32
#define MAX_CHAT_DESCRIPTION_LENGTH 128
#define MAX_URL_LENGTH 64
#define MAX_STICKER_NAME 32
#define MAX_TEXT_LENGTH 4097 // Yes, it is 4097 instead 4096 (telegram big brain)

// Memory usage level apply
#undef MAX_TEXT_LENGTH
#if UTLGBOT_MEMORY_LEVEL == 0
    #warning "Info: uTLGBotLib memory level 0 select."
    #define MAX_TEXT_LENGTH 128
#elif UTLGBOT_MEMORY_LEVEL == 1
    #warning "Info: uTLGBotLib memory level 1 select."
    #define MAX_TEXT_LENGTH 256
#elif UTLGBOT_MEMORY_LEVEL == 2
    #warning "Info: uTLGBotLib memory level 2 select."
    #define MAX_TEXT_LENGTH 512
#elif UTLGBOT_MEMORY_LEVEL == 3
    #warning "Info: uTLGBotLib memory level 3 select."
    #define MAX_TEXT_LENGTH 1024
#elif UTLGBOT_MEMORY_LEVEL == 4
    #warning "Info: uTLGBotLib memory level 4 select."
    #define MAX_TEXT_LENGTH 2048
#elif UTLGBOT_MEMORY_LEVEL == 5
    #warning "Info: uTLGBotLib memory level 5 select."
    #define MAX_TEXT_LENGTH 4097
#else
    #warning "Info: uTLGBotLib invalid memory level selected."
    #define MAX_TEXT_LENGTH 4097
#endif

// Maximum HTTP GET and POST data lenght
#define HTTP_MAX_URI_LENGTH 128
#define HTTP_MAX_RES_LENGTH MAX_TEXT_LENGTH + 1024

// JSON Max values length
#define MAX_JSON_STR_LEN MAX_TEXT_LENGTH
#define MAX_JSON_SUBVAL_STR_LEN 512
#define MAX_JSON_ELEMENTS 64
#define MAX_JSON_SUBELEMENTS 32

// Others
#define MAX_KEYBOARD_MARKUP_LENGTH 128
#define MAX_TMP_BUFFER_LENGTH MAX_KEYBOARD_MARKUP_LENGTH*2

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
    char id[MAX_ID_LENGTH];
    bool is_bot;
    char first_name[MAX_USER_LENGTH];
    char last_name[MAX_USER_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char language_code[MAX_LANGUAGE_CODE_LENGTH];
} tlg_type_user;

// Chat: https://core.telegram.org/bots/api#chat
typedef struct tlg_type_chat
{
    char id[MAX_ID_LENGTH];
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
        uTLGBot(const char* token, const bool dont_keep_connection=false);
        #if defined(WIN32) || defined(_WIN32) || defined(__linux__) // Native (Windows, Linux)
            ~uTLGBot(void);
        #endif
        void set_debug(const uint8_t debug_level);
        void set_token(const char* token);
        void set_polling_timeout(const uint8_t seconds);
        char* get_token(void);
        uint8_t get_polling_timeout(void);
        uint8_t connect(void);
        void disconnect(void);
        bool is_connected(void);
        uint8_t getMe(void);
        uint8_t sendMessage(const char* chat_id, const char* text, const char* parse_mode="", 
            bool disable_web_page_preview=false, bool disable_notification=false, 
            uint64_t reply_to_message_id=0, const char* reply_markup="");
        uint8_t sendReplyKeyboardMarkup(const char* chat_id, const char* text, 
            const char* keyboard);
        uint8_t getUpdates(void);

    private:
        // Private Attributtes
        MultiHTTPSClient* _client;
        uint8_t _long_poll_timeout;
        char _token[TOKEN_LENGTH];
        char _tlg_api[TELEGRAM_API_LENGTH];
        char _buffer[HTTP_MAX_RES_LENGTH];
        jsmntok_t _json_elements[MAX_JSON_ELEMENTS];
        jsmntok_t _json_subelements[MAX_JSON_SUBELEMENTS];
        char _json_value_str[MAX_JSON_STR_LEN];
        char _json_subvalue_str[MAX_JSON_SUBVAL_STR_LEN];
        char json_keyboard[MAX_KEYBOARD_MARKUP_LENGTH];
        uint64_t _last_received_msg;
        bool _dont_keep_connection;
        uint8_t _debug_level;

        // Private Methods
        uint8_t tlg_get(const char* command, char* response, const size_t response_len, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t tlg_post(const char* command, char* request_response, const size_t request_len, 
            const size_t request_response_max_size, 
            const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

        void clear_msg_data(void);
        void cant_create_send_msg(const char* msg);
        uint32_t json_parse_str(const char* json_str, const size_t json_str_len, 
            jsmntok_t* json_tokens, const uint32_t json_tokens_len);
        uint32_t json_has_key(const char* json_str, jsmntok_t* json_tokens, 
            const uint32_t num_tokens, const char* key);
        void json_get_element_string(const char* json_str, jsmntok_t* token, char* converted_str, 
            const uint32_t converted_str_len);
        uint8_t json_get_key_value(const char* key, const char* json_str, jsmntok_t* tokens, 
            const uint32_t num_tokens, char* converted_str, const uint32_t converted_str_len);
        int32_t cstr_get_substr_pos_end(char* str, const size_t str_len, const char* substr, 
            const size_t substr_len);
        void cstr_rm_char(char* str, const size_t str_len, const char c_remove);
        bool cstr_strncat(char* dest, const size_t dest_max_size, const char* src, 
            const size_t src_len);
};

/**************************************************************************************************/

#endif
