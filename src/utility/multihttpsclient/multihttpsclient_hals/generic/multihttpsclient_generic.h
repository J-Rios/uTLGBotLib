/**************************************************************************************************/
// File: multihttpsclient_generic.h
// Description: Multiplatform HTTPS Client implementation for Generic systems (Windows and Linux).
// Created on: 11 may. 2019
// Last modified date: 02 dec. 2019
// Version: 1.0.1
/**************************************************************************************************/

#if defined(WIN32) || defined(_WIN32) || defined(__linux__)

/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENTGENERIC_H_
#define MULTIHTTPSCLIENTGENERIC_H_

/**************************************************************************************************/

/* Libraries */

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
        MultiHTTPSClient(char* cert_https_api_telegram_org);
        ~MultiHTTPSClient(void);
        void set_debug(const bool debug);
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
        char* _cert_https_api_telegram_org;
        mbedtls_net_context _server_fd;
        mbedtls_entropy_context _entropy;
        mbedtls_ctr_drbg_context _ctr_drbg;
        mbedtls_ssl_context _tls;
        mbedtls_ssl_config _tls_cfg;
        mbedtls_x509_crt _cacert;
        bool _connected;
        bool _debug;
        
        // Private Methods
        bool init(void);
        void release_tls_elements(void);
        size_t write(const char* request);
        bool read(char* response, const size_t response_len);
};

/**************************************************************************************************/

#endif

/**************************************************************************************************/

#endif
