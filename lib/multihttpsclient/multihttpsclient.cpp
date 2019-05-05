/**************************************************************************************************/
// File: multihttpsclient.h
// Description: Basic Multiplatform HTTPS Client (Implement network HALs for differents devices).
// Created on: 04 may. 2019
// Last modified date: 04 may. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient.h"

/**************************************************************************************************/

/* Macros */

#if defined(ARDUINO) // ESP32 Arduino Framework
    #define _print(x) do { Serial.print(x); } while(0)
    #define _println(x) do { Serial.println(x); } while(0)
    #define _printf(...) do { Serial.printf(__VA_ARGS__); } while(0)
    #define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

    #define _millis_setup() 
    #define _millis() millis()
    #define _delay(x) delay(x)
#elif defined(ESP_IDF) // ESP32 ESPIDF Framework
    #define _millis_setup() 
    #define _millis() (unsigned long)(esp_timer_get_time()/1000)
    #define _delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)
    #define _print(x) do { printf("%s", x); } while(0)
    #define _println(x) do { printf("%s", x); printf("\n"); } while(0)
    #define _printf(...) do { printf(__VA_ARGS__); } while(0)

    #define F(x) x
    #define PSTR(x) x
    #define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
    #define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)
    
    #define PROGMEM 
#else // Generic devices (intel, amd, arm) and OS (windows, Linux)
    #define _print(x) do { printf("%s", x); } while(0)
    #define _println(x) do { printf("%s", x); printf("\n"); } while(0)
    #define _printf(...) do { printf(__VA_ARGS__); } while(0)

    #define F(x) x
    #define PSTR(x) x
    #define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
    #define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

    #define PROGMEM

    // Initialize millis (just usefull for Generic)
    clock_t _millis_t0 = clock();
    #define _millis() (unsigned long)((clock() - ::_millis_t0)*1000.0/CLOCKS_PER_SEC)

    #if defined(WIN32) || defined(_WIN32) // Windows
        #define _delay(x) do { Sleep(x); } while(0)
    #elif defined(__linux__)
        #define _delay(x) do { usleep(x*1000); } while(0)
    #endif
#endif

/**************************************************************************************************/

/* Constructor & Destructor */

// MultiHTTPSClient constructor, initialize and setup secure client with the certificate
#if defined(ESP_IDF)
    MultiHTTPSClient::MultiHTTPSClient(const uint8_t* tlg_api_ca_pem_start, const uint8_t* tlg_api_ca_pem_end)
    {
        _connected = false;
        _tlg_api_ca_pem_start = tlg_api_ca_pem_start;
        _tlg_api_ca_pem_end = tlg_api_ca_pem_end;

        init();
    }
#else
    MultiHTTPSClient::MultiHTTPSClient(char* cert_https_api_telegram_org)
    {
        _connected = false;
        _cert_https_api_telegram_org = cert_https_api_telegram_org;

        init();
    }
#endif

// MultiHTTPSClient destructor, free mbedtls resources
#if !defined(ARDUINO) && !defined(ESP_IDF) // Just in Windows or Linux
    MultiHTTPSClient::~MultiHTTPSClient(void)
    {
        // Release all mbedtls context
        release_tls_elements();
    }
#endif

/**************************************************************************************************/

/* Public Methods */

// Make HTTPS client connection to server
int8_t MultiHTTPSClient::connect(const char* host, uint16_t port)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
        int8_t conn_result = _client->connect(host, port);
        if(conn_result)
            _connected = true;
        return conn_result;
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
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
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        int ret;

        // Start connection
        char str_port[6];
        snprintf(str_port, 6, "%d", port);
        if((ret = mbedtls_net_connect(&_server_fd, host, str_port, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            _printf("[HTTPS] Error: Can't connect to server. ");
            _printf("Start connection fail (mbedtls_net_connect returned %d).\n", ret);
            return 0;
        }

        // Set SSL/TLS configuration
        if((ret = mbedtls_ssl_config_defaults(&_tls_cfg, MBEDTLS_SSL_IS_CLIENT, 
            MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            _printf("[HTTPS] Error: Can't connect to server ");
            _printf("Default SSL/TLS configuration fail ");
            _printf("(mbedtls_ssl_config_defaults returned %d).\n", ret);
            return 0;
        }
        mbedtls_ssl_conf_authmode(&_tls_cfg, MBEDTLS_SSL_VERIFY_OPTIONAL);
        mbedtls_ssl_conf_ca_chain(&_tls_cfg, &_cacert, NULL);
        mbedtls_ssl_conf_rng(&_tls_cfg, mbedtls_ctr_drbg_random, &_ctr_drbg);
        //mbedtls_ssl_conf_dbg(&_tls_cfg, my_debug, stdout);

        // SSL/TLS Server, Hostname and Bio setup
        if((ret = mbedtls_ssl_setup( &_tls, &_tls_cfg)) != 0)
        {
            _printf("[HTTPS] Error: Can't connect to server ");
            _printf("SSL/TLS setup fail (mbedtls_ssl_setup returned %d).\n", ret);
            return 0;
        }
        if((ret = mbedtls_ssl_set_hostname(&_tls, host)) != 0)
        {
            _printf("[HTTPS] Error: Can't connect to server. ");
            _printf("Hostname setup fail (mbedtls_ssl_set_hostname returned %d).\n", ret);
            return 0;
        }
        mbedtls_ssl_set_bio(&_tls, &_server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        // Perform SSL/TLS Handshake
        while((ret = mbedtls_ssl_handshake(&_tls)) != 0)
        {
            if((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                _printf("[HTTPS] Error: Can't connect to server ");
                _printf("SSL/TLS handshake fail (mbedtls_ssl_handshake returned -0x%x).\n", -ret);
                return 0;
            }
        }

        // Verify server certificate
        uint32_t flags;
        if((flags = mbedtls_ssl_get_verify_result(&_tls)) != 0)
        {
            char vrfy_buf[512];
            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
            _printf("[HTTPS] Warning: Invalid Server Certificate.\n%s\n", vrfy_buf);
            return -1;
        }

        // Connection stablished and certificate verified
        _connected = true;
        return 1;
    #endif
}

// HTTPS client disconnect from server
void MultiHTTPSClient::disconnect(void)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
        _client->stop();
        _connected = false;
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
        if(_tls != NULL)
        {
            esp_tls_conn_delete(_tls);
            _tls = NULL;
        }
        _connected = false;
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        // Close connection
        int ret = mbedtls_ssl_close_notify(&_tls);
        if((ret != 0) && (ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
            mbedtls_ssl_session_reset(&_tls);

        // Release all mbedtls context
        release_tls_elements();

        // Initialize again the mbedtls context
        init();

        _connected = false;
    #endif
}

// Check if HTTPS client is connected
bool MultiHTTPSClient::is_connected(void)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
        _connected = _client->connected();
        return _connected;
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
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
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        return _connected;
    #endif
}

// Make and send a HTTP GET request
uint8_t MultiHTTPSClient::get(const char* uri, const char* host, char* response, 
        const size_t response_len, const unsigned long response_timeout)
{
    unsigned long t0, t1;
    char request[HTTP_MAX_GET_LENGTH];

    // Clear response buffer and create request
    // Note that we use specific header values for Telegram requests
    memset(response, '\0', response_len);
    snprintf_P(request, HTTP_MAX_GET_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
            "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
            "\r\n\r\n"), uri, host);

    // Send request
    #if defined(ARDUINO) // ESP32 Arduino Framework
        _print(F("HTTP request to send: "));
        _println(request);
        _println();
    #else
        _printf(F("HTTP request to send: %s\n"), request);
    #endif
    if(write(request) != strlen(request))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] GET request successfully sent."));

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
    unsigned long t0, t1;
    char request[HTTP_MAX_POST_LENGTH];

    // Clear response buffer and create request
    // Note that we use specific header values for Telegram requests
    memset(response, '\0', response_len);
    snprintf_P(request, HTTP_MAX_POST_LENGTH, PSTR("POST %s HTTP/1.1\r\nHost: %s\r\n" \
               "User-Agent: ESP32\r\nAccept: text/html,application/xml,application/json" \
               "\r\nContent-Type: application/json\r\nContent-Length: %" PRIu64 "\r\n\r\n%s"), uri, 
               host, body_len, body);

    // Send request
    #if defined(ARDUINO) // ESP32 Arduino Framework
        _print(F("HTTP request to send: "));
        _println(request);
        _println();
    #else
        _printf(F("HTTP request to send: %s\n"), request);
    #endif
    if(write(request) != strlen(request))
    {
        _println(F("[HTTPS] Error: Incomplete HTTP request sent (sent less bytes than expected)."));
        return 1;
    }
    _println(F("[HTTPS] POST request successfully sent."));

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

    //_printf(F("[HTTPS] Response: %s\n\n"), response);
    
    return 0;
}

/**************************************************************************************************/

/* Private Methods */

bool MultiHTTPSClient::init(void)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
        _client = new WiFiClientSecure();
        //_client->setCACert(_cert_https_api_telegram_org);
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
        _tls = NULL;
        static esp_tls_cfg_t tls_cfg;
        tls_cfg.alpn_protos = NULL;
        tls_cfg.cacert_pem_buf = _tlg_api_ca_pem_start,
        tls_cfg.cacert_pem_bytes = _tlg_api_ca_pem_end - _tlg_api_ca_pem_start,
        tls_cfg.non_block = true,
        _tls_cfg = &tls_cfg;
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        static const char* entropy_generation_key = "tsl_client\0";
        int ret = 1;

        // Initialization
        mbedtls_net_init(&_server_fd);
        mbedtls_ssl_init(&_tls);
        mbedtls_ssl_config_init(&_tls_cfg);
        mbedtls_x509_crt_init(&_cacert);
        mbedtls_ctr_drbg_init(&_ctr_drbg);
        mbedtls_entropy_init(&_entropy);
        if((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy, 
            (const unsigned char*)entropy_generation_key, strlen(entropy_generation_key))) != 0)
        {
            printf("[HTTPS] Error: Cannot initialize HTTPS client. ");
            printf("mbedtls_ctr_drbg_seed returned %d\n", ret);
            return false;
        }

        // Load Certificate
        ret = mbedtls_x509_crt_parse(&_cacert, 
            (const unsigned char*)_cert_https_api_telegram_org, 
            strlen(_cert_https_api_telegram_org)+1);
        if(ret < 0)
        {
            printf("[HTTPS] Error: Cannot initialize HTTPS client. ");
            printf("mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
            return false;
        }
    #endif

    return true;
}

// Release all mbedtls context
void MultiHTTPSClient::release_tls_elements(void)
{
    // Just in Windows or Linux
    #if !defined(ARDUINO) && !defined(ESP_IDF)
        mbedtls_net_free(&_server_fd);
        mbedtls_x509_crt_free(&_cacert);
        mbedtls_ssl_free(&_tls);
        mbedtls_ssl_config_free(&_tls_cfg);
        mbedtls_ctr_drbg_free(&_ctr_drbg);
        mbedtls_entropy_free(&_entropy);
    #endif
}

// HTTPS Write
size_t MultiHTTPSClient::write(const char* request)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
        return _client->print(request);
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
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
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        size_t written_bytes = 0;
        int ret;

        written_bytes = strlen(request);
        while((ret = mbedtls_ssl_write(&_tls, (const unsigned char*)request, written_bytes)) <= 0)
        {
            if((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                _printf(F("[HTTPS] Client write error -0x%x\n"), -ret);
                return 0;
            }
        }
        written_bytes = ret;

        return written_bytes;
    #endif
}

// HTTPS Read
bool MultiHTTPSClient::read(char* response, const size_t response_len)
{
    #if defined(ARDUINO) // ESP32 Arduino Framework
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
    #elif defined(ESP_IDF) // ESP32 ESPIDF Framework
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
    #else // Generic devices (intel, amd, arm) and OS (windows, Linux)
        int ret;

        ret = mbedtls_ssl_read(&_tls, (unsigned char*)response, response_len);

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
    #endif
}
