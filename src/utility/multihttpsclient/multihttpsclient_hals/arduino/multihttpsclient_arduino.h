/**************************************************************************************************/
// File: multihttpsclient_arduino.h
// Description: Multiplatform HTTPS Client implementation for ESP32 Arduino Framework.
// Created on: 11 may. 2019
// Last modified date: 14 apr. 2020
// Version: 1.0.4
/**************************************************************************************************/

#if defined(ARDUINO)

/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENTARDUINO_H_
#define MULTIHTTPSCLIENTARDUINO_H_

/**************************************************************************************************/

/* Libraries */

#include <Arduino.h>
#include <WiFiClientSecure.h>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

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
        MultiHTTPSClient(char* cert_https_api_telegram_org);
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
        WiFiClientSecure* _client;
        char* _cert_https_api_telegram_org;
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
