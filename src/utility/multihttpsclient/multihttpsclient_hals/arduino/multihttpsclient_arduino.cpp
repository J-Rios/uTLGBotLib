/**************************************************************************************************/
// File: multihttpsclient_arduino.cpp
// Description: Multiplatform HTTPS Client implementation for ESP32 Arduino Framework.
// Created on: 11 may. 2019
// Last modified date: 02 dec. 2019
// Version: 1.0.1
/**************************************************************************************************/

#if defined(ARDUINO)

/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient_arduino.h"

/**************************************************************************************************/

/* Macros */

#define _print(x) do { if(_debug) Serial.print(x); } while(0)
#define _println(x) do { if(_debug) Serial.println(x); } while(0)
#define _printf(...) do { if(_debug) Serial.printf(__VA_ARGS__); } while(0)
#define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

#define _millis_setup() 
#define _millis() millis()
#define _delay(x) delay(x)

/**************************************************************************************************/

/* Constructor */

// MultiHTTPSClient constructor, initialize and setup secure client with the certificate
MultiHTTPSClient::MultiHTTPSClient(char* cert_https_api_telegram_org)
{
    _debug = false;
    _connected = false;
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
    unsigned long t0, t1;

    // Clear response buffer and create request
    // Note that we use specific header values for Telegram requests
    snprintf_P(request, response_len, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
            "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
            "\r\n\r\n"), uri, host);

    // Send request
    //_print(F("HTTP request to send: "));
    //_println(request);
    //_println();
    if(write(request) != strlen(request))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] GET request successfully sent."));
    memset(response, '\0', response_len);

    // Wait and read response
    _println(F("[HTTPS] Waiting for response..."));
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
            _println(F("[HTTPS] Error: No response from server (wait response timeout)."));
            return 2;
        }

        // Check for response
        if(read(response, response_len))
        {
            _println(F("[HTTPS] Response successfully received."));
            break;
        }

        // Release CPU usage
        _delay(10);
    }

    //_printf(F("[HTTPS] Response: %s\n\n"), response);
    
    return 0;
}

// Make and send a HTTP POST request
uint8_t MultiHTTPSClient::post(const char* uri, const char* host, const char* body, 
        const uint64_t body_len, char* response, const size_t response_len, 
        const unsigned long response_timeout)
{
    // Lets use response buffer for make the request first (for the sake of save memory)
    char* request = response;
    unsigned long t0, t1;

    // Clear response buffer and create request
    // Note that we use specific header values for Telegram requests
    snprintf_P(request, response_len, PSTR("POST %s HTTP/1.1\r\nHost: %s\r\n" \
               "User-Agent: ESP32\r\nAccept: text/html,application/xml,application/json" \
               "\r\nContent-Type: application/json\r\nContent-Length: %" PRIu64 "\r\n\r\n%s"), uri, 
               host, body_len, body);

    // Send request
    //_print(F("HTTP request to send: "));
    //_println(request);
    //_println();
    if(write(request) != strlen(request))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] POST request successfully sent."));
    memset(response, '\0', response_len);

    // Wait and read response
    _println(F("[HTTPS] Waiting for response..."));
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
            return 2;
        }

        // Check for response
        if(read(response, response_len))
        {
            _println(F("[HTTPS] Response successfully received."));
            break;
        }

        // Release CPU usage
        _delay(10);
    }

    //_printf("[HTTPS] Response: %s\n\n", response);
    
    return 0;
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
    ESP.wdtDisable();
    ESP.wdtEnable(8000U);
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
bool MultiHTTPSClient::read(char* response, const size_t response_len)
{
    char c;
    size_t i = 0;

    if(!_client->available())
        return false;
    while(_client->available())
    {
        c = _client->read();
        if(i < response_len-1)
        {
            response[i] = c;
            i = i + 1;
        }
    }
    return true;
}

/**************************************************************************************************/

#endif
