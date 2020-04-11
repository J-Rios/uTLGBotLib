/**************************************************************************************************/
// File: multihttpsclient_arduino.h
// Description: Multiplatform HTTPS Client implementation for ESP32 Arduino Framework.
// Created on: 11 may. 2019
// Last modified date: 11 apr. 2020
// Version: 1.0.3
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
        uint8_t post(const char* uri, const char* host, const char* body, const uint64_t body_len, 
                char* response, const size_t response_len, 
                const unsigned long response_timeout=HTTP_WAIT_RESPONSE_TIMEOUT);

    private:
        // Private Attributtes
        WiFiClientSecure* _client;
        char* _cert_https_api_telegram_org;
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
