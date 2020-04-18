/**************************************************************************************************/
// File: multihttpsclient_arduino.cpp
// Description: Multiplatform HTTPS Client implementation for ESP32 Arduino Framework.
// Created on: 11 may. 2019
// Last modified date: 14 apr. 2020
// Version: 1.0.4
/**************************************************************************************************/

#if defined(ARDUINO)

/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient_arduino.h"

/**************************************************************************************************/

/* Macros */

#ifndef MULTIHTTPSCLIENT_NO_DEBUG
    #define _print(x) do { if(_debug) Serial.print(x); } while(0)
    #define _println(x) do { if(_debug) Serial.println(x); } while(0)
    #define _printf(...) do { if(_debug) Serial.printf(__VA_ARGS__); } while(0)
#else
    #define _print(x)
    #define _println(x)
    #define _printf(...)
#endif

#define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

#define _millis_setup()
#define _millis() millis()
#define _delay(x) delay(x)
#define _yield() yield()

/**************************************************************************************************/

/* Constructor */

// MultiHTTPSClient constructor, initialize and setup secure client with the certificate
MultiHTTPSClient::MultiHTTPSClient(char* cert_https_api_telegram_org)
{
    _debug = false;
    _connected = false;
    _http_header[0] = '\0';
    _client = NULL;
    _cert_https_api_telegram_org = cert_https_api_telegram_org;

    init();
}

/**************************************************************************************************/

/* Public Methods */

// Enable/Disable Debug Prints
void MultiHTTPSClient::set_debug(const bool debug)
{
    _debug = debug;
}

// Make HTTPS client connection to server
int8_t MultiHTTPSClient::connect(const char* host, uint16_t port)
{
    int8_t conn_result = _client->connect(host, port);
    if(conn_result)
        _connected = true;
    return conn_result;
}

// HTTPS client disconnect from server
void MultiHTTPSClient::disconnect(void)
{
    _client->stop();
    _connected = false;
}

// Check if HTTPS client is connected
bool MultiHTTPSClient::is_connected(void)
{
    _connected = _client->connected();
    return _connected;
}

// Make and send a HTTP GET request
uint8_t MultiHTTPSClient::get(const char* uri, const char* host, char* response, 
        const size_t response_len, const unsigned long response_timeout)
{
    // Lets use response buffer for make the request first (for the sake of save memory)
    char* request = response;
    uint8_t rc = 1;

    // Create header request
    snprintf_P(request, HTTP_HEADER_MAX_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
        "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
        "\r\n\r\n"), uri, host);

    // Send request
    _println(F("HTTP GET request to send: "));
    _println(request);
    _println();
    if(write(request) != strlen(request))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] GET request successfully sent."));
    memset(response, '\0', response_len);

    // Wait and read response
    _println(F("[HTTPS] Waiting for response..."));
    rc = read_response(response, response_len, response_timeout);
    _printf("[HTTPS] Response: %s\n\n", response);
    
    return rc;
}

// Make and send a HTTP POST request
// Provide HTTP body in request_response argument
// Argument request_response will be modified and returned as request response
uint8_t MultiHTTPSClient::post(const char* uri, const char* host, char* request_response, 
        const size_t request_len, const size_t request_response_max_size, 
        const unsigned long response_timeout)
{
    uint8_t rc = 1;

    // Create header request
    snprintf_P(_http_header, HTTP_HEADER_MAX_LENGTH, PSTR("POST %s HTTP/1.1\r\nHost: %s\r\n" \
        "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
        "\r\nContent-Type: application/json\r\nContent-Length: %" PRIu64 "\r\n\r\n"), uri, 
        host, (uint64_t)request_len);

    // Send request
    _println(F("HTTP POST request to send: "));
    _println(_http_header);
    _println(request_response);
    _println();
    if(write(_http_header) != strlen(_http_header))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    if(write(request_response) != strlen(request_response))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] POST request successfully sent."));
    memset(request_response, '\0', request_response_max_size);

    // Wait and read response
    _println(F("[HTTPS] Waiting for response..."));
    rc = read_response(request_response, request_response_max_size, response_timeout);
    _printf("[HTTPS] Response: %s\n\n", request_response);

    return rc;
}

/**************************************************************************************************/

/* Private Methods */

bool MultiHTTPSClient::init(void)
{
    _client = new WiFiClientSecure();

// Let's do not use Server authenticy verification with Arduino for simplify Makers live
#ifdef ESP8266
    // ESP8266 doesn't have a hardware element for SSL/TLS acceleration, so it is really slow
    // Let's reconfigure software watchdog timer to 8s for avoid server connection issues
    // Let's ignore server authenticy verification and trust to get a fast response ¯\_(ツ)_/¯
    //ESP.wdtDisable();
    //ESP.wdtEnable(8000U);
    //_client->setFingerprint(_cert_https_api_telegram_org);
    _client->setInsecure();
#else
    // ESP32 has a hardware element for SSL/TLS acceleration, so it could be use
    //_client->setCACert(_cert_https_api_telegram_org);
    //_client->setFingerprint(_cert_https_api_telegram_org);
#endif

    return true;
}

// Release all mbedtls context
void MultiHTTPSClient::release_tls_elements(void)
{
    /* Not release in microcontrollers */
}

// HTTPS Write
size_t MultiHTTPSClient::write(const char* request)
{
    return _client->print(request);
}

// HTTPS Read
size_t MultiHTTPSClient::read(char* response, const size_t response_len)
{
    char c;
    size_t i = 0;

    while(_client->available())
    {
        c = _client->read();
        if(i < response_len-1)
        {
            response[i] = c;
            i = i + 1;
        }

        _yield();
    }

    return i;
}

// HTTP Read Response
uint8_t MultiHTTPSClient::read_response(char* response, const size_t response_max_len, 
        const unsigned long response_timeout)
{
    unsigned long t0 = 0, t1 = 0, t2 = 0;
    size_t num_bytes_read = 0;
    size_t total_bytes_read = 0;
    size_t response_len = response_max_len;

    t0 = _millis();
    while(true)
    {
        t1 = _millis();

        // Check for overflow
        // Note: Due Arduino millis() return an unsigned long instead specific size type, lets just 
        // handle overflow by reseting counter (this time the timeout can be < 2*expected_timeout)
        if(t1 < t0)
        {
            t0 = 0;
            continue;
        }

        // Check for timeout
        if(t1-t0 >= response_timeout)
        {
            _println(F("[HTTPS] Error: No response from server (timeout)."));
            return 2; // Timeout response
        }

        // Check for response
        num_bytes_read = read(response, response_len);
        total_bytes_read = total_bytes_read + num_bytes_read;
        if(total_bytes_read >= response_max_len)
        {
            _println(F("[HTTPS] Response read buffer full."));
            return 3;
        }
        if(num_bytes_read == 0)
        {
            // Check for timeout without any incomming byte
            if(t2 != 0)
            {
                t1 = _millis();
                if(t1 < t2)
                    t2 = t1;
                if(t1-t2 >= HTTP_RESPONSE_BETWEEN_BYTES_TIMEOUT)
                {
                    // Assume full reception
                    _println(F("[HTTPS] Response successfully received."));
                    break;
                }
            }
        }
        else
        {
            _println(F("[HTTPS] Something partially received:"));
            _println(response);
            response = response + num_bytes_read;
            response_len = response_len - num_bytes_read;
            t2 = _millis();
        }

        _yield();
    }

    return 0;
}

/**************************************************************************************************/

#endif
