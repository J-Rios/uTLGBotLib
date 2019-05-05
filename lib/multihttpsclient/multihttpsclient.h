/**************************************************************************************************/
// File: multihttpsclient.h
// Description: Basic Multiplatform HTTPS Client (Implement network HALs for differents devices).
// Created on: 04 may. 2019
// Last modified date: 04 may. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENT_H_
#define MULTIHTTPSCLIENT_H_

/**************************************************************************************************/

/* Check Build System */

#if !defined(ARDUINO) && !defined(ESP_IDF) && !defined(WIN32) && !defined(_WIN32) && \
!defined(__linux__)
    #error Unsupported system (Supported: Windows, Linux and ESP32)
#endif

/**************************************************************************************************/

/* Libraries */

#if defined(ARDUINO) // ESP32 Arduino Framework
    #include <Arduino.h>
    #include <WiFiClientSecure.h>
#elif defined(ESP_IDF) // ESP32 ESPIDF Framework
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

/**************************************************************************************************/

/* Constants */

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
#define HTTP_MAX_RES_LENGTH 4096

/**************************************************************************************************/

class MultiHTTPSClient
{
    public:
        // Public Methods
        #if defined(ESP_IDF)
            MultiHTTPSClient(const uint8_t* tlg_api_ca_pem_start, 
                const uint8_t* tlg_api_ca_pem_end);
        #else
            MultiHTTPSClient(char* cert_https_api_telegram_org);
        #endif
        #if !defined(ARDUINO) && !defined(ESP_IDF) // Windows or Linux
            ~MultiHTTPSClient(void);
        #endif
        int8_t connect(const char* host, uint16_t port);
        void disconnect(void);
        bool is_connected(void);
        uint8_t get(const char* uri, const char* host, char* response, const size_t response_len, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t post(const char* uri, const char* host, const char* body, const uint64_t body_len, 
                char* response, const size_t response_len, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

    private:
        // Private Attributtes
        #if defined(ARDUINO) // ESP32 Arduino Framework
            WiFiClientSecure* _client;
            char* _cert_https_api_telegram_org;
        #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
            const uint8_t* _tlg_api_ca_pem_start;
            const uint8_t* _tlg_api_ca_pem_end;
            struct esp_tls* _tls;
            esp_tls_cfg_t* _tls_cfg;
        #else // Windows and Linux
            char* _cert_https_api_telegram_org;
            mbedtls_net_context _server_fd;
            mbedtls_entropy_context _entropy;
            mbedtls_ctr_drbg_context _ctr_drbg;
            mbedtls_ssl_context _tls;
            mbedtls_ssl_config _tls_cfg;
            mbedtls_x509_crt _cacert;
        #endif
        bool _connected;
        
        // Private Methods
        bool init(void);
        void release_tls_elements(void);
        size_t write(const char* request);
        bool read(char* response, const size_t response_len);
};

/**************************************************************************************************/

#endif
