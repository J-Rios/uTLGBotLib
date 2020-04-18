/**************************************************************************************************/
// File: multihttpsclient_espidf.h
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 14 apr. 2020
// Version: 1.0.4
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
        MultiHTTPSClient(const uint8_t* tlg_api_ca_pem_start, const uint8_t* tlg_api_ca_pem_end);
        void set_debug(const bool debug);
        int8_t connect(const char* host, uint16_t port);
        void disconnect(void);
        bool is_connected(void);
        uint8_t get(const char* uri, const char* host, char* response, const size_t response_len, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);
        uint8_t post(const char* uri, const char* host, char* request_response, 
                const size_t request_len, const size_t request_response_max_size, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

    private:
        // Private Attributtes
        char _http_header[HTTP_HEADER_MAX_LENGTH];
        const uint8_t* _tlg_api_ca_pem_start;
        const uint8_t* _tlg_api_ca_pem_end;
        struct esp_tls* _tls;
        esp_tls_cfg_t* _tls_cfg;
        bool _connected;
        bool _debug;
        
        // Private Methods
        bool init(void);
        void release_tls_elements(void);
        size_t write(const char* request);
        size_t read(char* response, const size_t response_len);
        uint8_t read_response(char* response, const size_t response_max_len, 
                const unsigned long response_timeout);
};

/**************************************************************************************************/

#endif

/**************************************************************************************************/

#endif
