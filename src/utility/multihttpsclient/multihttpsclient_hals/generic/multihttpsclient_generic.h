/**************************************************************************************************/
// File: multihttpsclient_generic.h
// Description: Multiplatform HTTPS Client implementation for Generic systems (Windows and Linux).
// Created on: 11 may. 2019
// Last modified date: 14 apr. 2020
// Version: 1.0.4
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
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

// MBEDTLS library
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"

/**************************************************************************************************/

/* Constants */

// HTTP response wait timeout (ms)
#define HTTP_WAIT_RESPONSE_TIMEOUT 5000

// HTTP response between bytes receptions timeout (ms)
#define HTTP_RESPONSE_BETWEEN_BYTES_TIMEOUT 500

// HTTP Request header max length
#define HTTP_HEADER_MAX_LENGTH 256

/**************************************************************************************************/

class MultiHTTPSClient
{
    public:
        // Public Methods
        MultiHTTPSClient();
        ~MultiHTTPSClient();
        void set_debug(const bool debug);
        void set_cert(const char* cert_https_server);
        void set_cert(const uint8_t* ca_pem_start, const uint8_t* ca_pem_end);
        int8_t connect(const char* host, uint16_t port);
        void disconnect();
        bool is_connected();
        uint8_t get(const char* uri, const char* host, char* response, const size_t response_len,
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t post(const char* uri, const char* host, char* request_response,
                const size_t request_len, const size_t request_response_max_size,
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

    private:
        // Private Attributtes
        char _http_header[HTTP_HEADER_MAX_LENGTH];
        const char* _cert_https_server;
        mbedtls_net_context _server_fd;
        mbedtls_entropy_context _entropy;
        mbedtls_ctr_drbg_context _ctr_drbg;
        mbedtls_ssl_context _tls;
        mbedtls_ssl_config _tls_cfg;
        mbedtls_x509_crt _cacert;
        bool _connected;
        bool _debug;

        // Private Methods
        bool init();
        void release_tls_elements();
        size_t write(const char* request);
        size_t read(char* response, const size_t response_len);
        uint8_t read_response(char* response, const size_t response_max_len,
        const unsigned long response_timeout);
};

/**************************************************************************************************/

#endif

/**************************************************************************************************/

#endif
