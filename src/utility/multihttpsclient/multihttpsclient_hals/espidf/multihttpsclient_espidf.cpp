/**************************************************************************************************/
// File: multihttpsclient_espidf.cpp
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 02 dec. 2019
// Version: 1.0.1
/**************************************************************************************************/

#if defined(ESP_IDF)

/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient_espidf.h"

/**************************************************************************************************/

/* Macros */

#define _millis_setup() 
#define _millis() (unsigned long)(esp_timer_get_time()/1000)
#define _delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)
#define _print(x) do { if(_debug) printf("%s", x); } while(0)
#define _println(x) do { if(_debug) printf("%s\n", x); } while(0)
#define _printf(...) do { if(_debug) printf(__VA_ARGS__); } while(0)

#define F(x) x
#define PSTR(x) x
#define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
#define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

#define PROGMEM 

/**************************************************************************************************/

/* Constructor */

// MultiHTTPSClient constructor, initialize and setup secure client with the certificate
MultiHTTPSClient::MultiHTTPSClient(const uint8_t* tlg_api_ca_pem_start, 
    const uint8_t* tlg_api_ca_pem_end)
{
    _debug = false;
    _connected = false;
    _tlg_api_ca_pem_start = tlg_api_ca_pem_start;
    _tlg_api_ca_pem_end = tlg_api_ca_pem_end;

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
    unsigned long t0, t1;
    int conn_status;

    // Reserve memory for TLS (Warning, here we are dynamically reserving some memory in HEAP)
    _tls = (esp_tls*)calloc(1, sizeof(esp_tls_t));
    if(!_tls)
    {
        _println(F("[HTTPS] Error: Cannot reserve memory for TLS."));
        return false;
    }

    t0 = _millis();
    conn_status = 0;
    while(conn_status == 0)
    {
        t1 = _millis();

        // Check for overflow
        // Note: Due Arduino millis() return an unsigned long instead specific size type, 
        // lets just handle overflow by reseting counter (this time the timeout can 
        // be < 2*expected_timeout)
        if(t1 < t0)
        {
            t0 = 0;
            continue;
        }

        // Check for timeout
        if(t1-t0 >= HTTP_CONNECT_TIMEOUT)
        {
            _println(F("[HTTPS] Error: Can't connect to server (connection timeout)."));
            break;
        }

        // Check connection
        conn_status = esp_tls_conn_new_async(host, strlen(host), port, _tls_cfg, _tls);
        if(conn_status == 0) // Connection in progress
            continue;
        else if(conn_status == -1) // Connection Fail
        {
            _println(F("[HTTPS] Error: Can't connect to server (connection fail)."));
            break;
        }
        else if(conn_status == 1) // Connection Success
            break;

        // Release CPU usage
        _delay(10);
    }

    _connected = is_connected();
    return _connected;
}

// HTTPS client disconnect from server
void MultiHTTPSClient::disconnect(void)
{
    if(_tls != NULL)
    {
        esp_tls_conn_delete(_tls);
        _tls = NULL;
    }
    _connected = false;
}

// Check if HTTPS client is connected
bool MultiHTTPSClient::is_connected(void)
{
    if(_tls != NULL)
    {
        if(_tls->conn_state == ESP_TLS_DONE)
            _connected = true;
        else
            _connected = false;
    }
    else
        _connected = false;

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
    _printf(F("HTTP request to send: %s\n"), request);
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
    _printf(F("HTTP request to send: %s\n"), request);
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

    _printf(F("[HTTPS] Response: %s\n\n"), response);
    
    return 0;
}

/**************************************************************************************************/

/* Private Methods */

bool MultiHTTPSClient::init(void)
{
    _tls = NULL;
    static esp_tls_cfg_t tls_cfg;
    tls_cfg.alpn_protos = NULL;
    tls_cfg.cacert_pem_buf = _tlg_api_ca_pem_start,
    tls_cfg.cacert_pem_bytes = _tlg_api_ca_pem_end - _tlg_api_ca_pem_start,
    tls_cfg.non_block = true,
    _tls_cfg = &tls_cfg;

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
    size_t written_bytes = 0;
    int ret;
    
    do
    {
        ret = esp_tls_conn_write(_tls, request + written_bytes, strlen(request) - 
            written_bytes);
        if(ret > 0)
            written_bytes += ret;
        else if(ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            _printf(F("[HTTPS] Client write error 0x%x\n"), ret);
            break;
        }
    } while(written_bytes < strlen(request));

    return written_bytes;
}

// HTTPS Read
bool MultiHTTPSClient::read(char* response, const size_t response_len)
{
    int ret;

    ret = esp_tls_conn_read(_tls, response, response_len);

    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return false;

    if(ret < 0)
    {
        _printf(F("[HTTPS] Client read error -0x%x\n"), -ret);
        return false;
    }
    if(ret == 0)
    {
        _printf(F("[HTTPS] Lost connection while client was reading.\n"));
        return false;
    }
    
    if(ret > 0)
        return true;
    return false;
}

/**************************************************************************************************/

#endif
