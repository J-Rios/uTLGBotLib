/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbotlib.h
// Description: Lightweight library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 19 mar. 2019
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

/*#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"*/
#include "esp_tls.h"

/**************************************************************************************************/

/* Constants */

// Telegram Server address and address lenght
#define TELEGRAM_SERVER "https://api.telegram.org"
#define TELEGRAM_HOST "api.telegram.org"
#define TELEGRAM_SERVER_LENGTH 28

// Bot token max lenght (note: token lenght is 46, but it is increasing, so we set it to 64)
#define TOKEN_LENGTH 64

// Telegram API address lenght
#define TELEGRAM_API_LENGTH (TELEGRAM_SERVER_LENGTH + TOKEN_LENGTH)

// Telegram HTTPS Server Port
#define HTTPS_PORT 443

/**************************************************************************************************/

/* Library Data Types */

/**************************************************************************************************/

/* Telegram Certificate */

extern const uint8_t tlg_api_ca_pem_start[] asm("_binary_res_certs_apitelegramorg_crt_start");
extern const uint8_t tlg_api_ca_pem_end[] asm("_binary_res_certs_apitelegramorg_crt_end");

/**************************************************************************************************/

class uTLGBot
{
    public:
        uTLGBot(const char* token);
        uint8_t connect(void);
        void disconnect(void);
        bool is_connected(void);

    private:
        esp_tls_cfg_t* _tls_cfg;
        struct esp_tls* _tls;
        char _token[TOKEN_LENGTH];
        char _tlg_api[TELEGRAM_API_LENGTH];
        bool _connected;

        void https_client_init(void);
        bool https_client_connect(const char* host, int port);
        void https_client_disconnect(void);
        bool https_client_is_connected(void);
};

/**************************************************************************************************/

#endif
