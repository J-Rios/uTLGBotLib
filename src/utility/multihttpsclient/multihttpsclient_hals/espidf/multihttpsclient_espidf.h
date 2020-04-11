/**************************************************************************************************/
// File: multihttpsclient_espidf.h
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 11 apr. 2020
// Version: 1.0.3
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

// HTTP response wait timeout (ms)
#define HTTP_WAIT_RESPONSE_TIMEOUT 5000

// HTTP response between bytes receptions timeout (ms)
#define HTTP_RESPONSE_BETWEEN_BYTES_TIMEOUT 500

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
        uint8_t post(const char* uri, const char* host, const char* body, const uint64_t body_len, 
                char* response, const size_t response_len, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

    private:
        // Private Attributtes
        const uint8_t* _tlg_api_ca_pem_start;
        const uint8_t* _tlg_api_ca_pem_end;
        struct esp_tls* _tls;
        esp_tls_cfg_t* _tls_cfg;
        bool _connected;
        bool _debug;
        
        // Private Methods
        bool init(void);
        void release_tls_elements(void);
        uint8_t read_response(char* response, const size_t response_max_len, 
                const unsigned long response_timeout);
        size_t write(const char* request);
        size_t read(char* response, const size_t response_len);
};

/**************************************************************************************************/

#endif

/**************************************************************************************************/

#endif
