/**************************************************************************************************/
// File: multihttpsclient_espidf.h
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 02 dec. 2019
// Version: 1.0.1
/**************************************************************************************************/

#if defined(ESP_IDF)

/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENTESPIDF_H_
#define MULTIHTTPSCLIENTESPIDF_H_

/**************************************************************************************************/

/* Libraries */

#include "esp_tls.h"

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
        size_t write(const char* request);
        bool read(char* response, const size_t response_len);
};

/**************************************************************************************************/

#endif

/**************************************************************************************************/

#endif
