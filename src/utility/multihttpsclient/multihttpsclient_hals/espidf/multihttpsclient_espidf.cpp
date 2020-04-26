/**************************************************************************************************/
// File: multihttpsclient_espidf.cpp
// Description: Multiplatform HTTPS Client implementation for ESP32 ESPIDF Framework.
// Created on: 11 may. 2019
// Last modified date: 14 apr. 2020
// Version: 1.0.4
/**************************************************************************************************/

#if defined(ESP_IDF)

/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient_espidf.h"

/**************************************************************************************************/

/* Macros */

#ifndef MULTIHTTPSCLIENT_NO_DEBUG
    #define _print(x) do { if(_debug) printf("%s", x); } while(0)
    #define _println(x) do { if(_debug) printf("%s\n", x); } while(0)
    #define _printf(...) do { if(_debug) printf(__VA_ARGS__); } while(0)
#else
    #define _print(x)
    #define _println(x)
    #define _printf(...)
#endif

#define F(x) x
#define PSTR(x) x
#define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
#define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

#define _millis_setup()
#define _millis() (unsigned long)(esp_timer_get_time()/1000)
#define _delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)
#define _yield() do { taskYIELD(); } while(0)

#define PROGMEM

/**************************************************************************************************/

/* Constructor */

// MultiHTTPSClient constructor, initialize and setup secure client
MultiHTTPSClient::MultiHTTPSClient(void)
{
    _debug = false;
    _connected = false;
    _http_header[0] = '\0';
    _tls = NULL;
    _tls_cfg = NULL;
    set_cert(NULL, NULL);
}

/**************************************************************************************************/

/* Public Methods */

// Enable/Disable Debug Prints
void MultiHTTPSClient::set_debug(const bool debug)
{
    _debug = debug;
}

// Setup Server Certificate
void MultiHTTPSClient::set_cert(const uint8_t* ca_pem_start, const uint8_t* ca_pem_end)
{
    static esp_tls_cfg_t tls_cfg;

    tls_cfg.alpn_protos = NULL;
    tls_cfg.cacert_pem_buf = ca_pem_start;
    tls_cfg.cacert_pem_bytes = ca_pem_end - ca_pem_start;
    tls_cfg.non_block = true;
    _tls_cfg = &tls_cfg;
    _println(F("[HTTPS] Server Certificate setup."));
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
    uint8_t rc = 1;

    // Create header request
    snprintf_P(request, HTTP_HEADER_MAX_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
        "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
        "\r\n\r\n"), uri, host);

    // Send request
    _printf("HTTP GET request to send:\n%s\n", request);
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
    _printf("HTTP POST request to send:\n%s%s\n", _http_header, request_response);
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
size_t MultiHTTPSClient::read(char* response, const size_t response_len)
{
    ssize_t ret;

    ret = esp_tls_conn_read(_tls, response, response_len);

    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0;
    if(ret < 0)
    {
        _printf(F("[HTTPS] Client read error -0x%x\n"), -ret);
        return 0;
    }
    if(ret == 0)
    {
        _printf(F("[HTTPS] Lost connection while client was reading.\n"));
        return 0;
    }

    return ret;
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
