/**************************************************************************************************/
// File: multihttpsclient_espidf.h
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 16 jul. 2023
// Version: 1.1.0
/**************************************************************************************************/

#if defined(ESP_IDF)

/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENTESPIDF_H_
#define MULTIHTTPSCLIENTESPIDF_H_

/**************************************************************************************************/

/* Libraries */

#include "esp_tls.h"

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

/**************************************************************************************************/

/* Constants */

// HTTP connection timeout
#define HTTP_CONNECT_TIMEOUT 5000

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
        void set_debug(const bool debug);
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
        esp_tls_t* _tls;
        esp_tls_cfg_t* _tls_cfg;
        bool _connected;
        bool _debug;

        // Private Methods
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
