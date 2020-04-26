/**************************************************************************************************/
// File: multihttpsclient_generic.cpp
// Description: Multiplatform HTTPS Client implementation for Generic systems (Windows and Linux).
// Created on: 11 may. 2019
// Last modified date: 11 apr. 2020
// Version: 1.0.3
/**************************************************************************************************/

#if defined(WIN32) || defined(_WIN32) || defined(__linux__)

/**************************************************************************************************/

/* Libraries */

#include "multihttpsclient_generic.h"

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

#define PROGMEM
#define _yield()

// Initialize millis (just usefull for Generic)
clock_t _millis_t0 = clock();
#define _millis() (unsigned long)((clock() - ::_millis_t0)*1000.0/CLOCKS_PER_SEC)

#if defined(WIN32) || defined(_WIN32) // Windows
    #define _delay(x) do { Sleep(x); } while(0)
#elif defined(__linux__)
    #define _delay(x) do { usleep(x*1000); } while(0)
#endif

/**************************************************************************************************/

/* Constructor & Destructor */

// MultiHTTPSClient constructor, initialize and setup secure client with the certificate
MultiHTTPSClient::MultiHTTPSClient(void)
{
    _debug = false;
    _connected = false;
    _http_header[0] = '\0';
    _cert_https_server = NULL;

    init();
}

// MultiHTTPSClient destructor, free mbedtls resources
MultiHTTPSClient::~MultiHTTPSClient(void)
{
    // Release all mbedtls context
    release_tls_elements();
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
    set_cert((const char*)ca_pem_start);
}

// Setup Server Certificate
void MultiHTTPSClient::set_cert(const char* cert_https_server)
{
    _cert_https_server = cert_https_server;

    // Release all mbedtls context
    release_tls_elements();

    // Initialize again the mbedtls context
    init();
}

// Make HTTPS client connection to server
int8_t MultiHTTPSClient::connect(const char* host, uint16_t port)
{
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
    mbedtls_ssl_conf_read_timeout(&_tls_cfg, HTTP_WAIT_RESPONSE_TIMEOUT);
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
    if(_cert_https_server != NULL)
    {
        if((flags = mbedtls_ssl_get_verify_result(&_tls)) != 0)
        {
            char vrfy_buf[512];
            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
            _printf("[HTTPS] Warning: Invalid Server Certificate.\n%s\n", vrfy_buf);
            return -1;
        }
    }

    // Connection stablished and certificate verified
    _connected = true;
    return 1;
}

// HTTPS client disconnect from server
void MultiHTTPSClient::disconnect(void)
{
    // Close connection
    int ret = mbedtls_ssl_close_notify(&_tls);
    if((ret != 0) && (ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
        mbedtls_ssl_session_reset(&_tls);

    // Release all mbedtls context
    release_tls_elements();

    // Initialize again the mbedtls context
    init();

    _connected = false;
}

// Check if HTTPS client is connected
bool MultiHTTPSClient::is_connected(void)
{
    return _connected;
}

// Make and send a HTTP GET request
uint8_t MultiHTTPSClient::get(const char* uri, const char* host, char* response, 
        const size_t response_len, const unsigned long response_timeout)
{
    // Lets use response buffer for make the request first (for the sake of save memory)
    char* request = response;
    uint8_t rc = 0;

    // Create header request
    snprintf_P(request, HTTP_HEADER_MAX_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
        "User-Agent: MultiHTTPSClient\r\nAccept: text/html,application/xml,application/json" \
        "\r\n\r\n"), uri, host);

    // Send request
    _printf("HTTP GET request to send:\n%s", request);
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
    uint8_t rc = 0;

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

bool MultiHTTPSClient::init(void)
{
    static const char* entropy_generation_key = "tls_client\0";
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
    if(_cert_https_server != NULL)
    {
        ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char*)_cert_https_server, 
            strlen(_cert_https_server)+1);
        if(ret < 0)
        {
            printf("[HTTPS] Error: Cannot initialize HTTPS client. ");
            printf("mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
            return false;
        }
    }

    return true;
}

// Release all mbedtls context
void MultiHTTPSClient::release_tls_elements(void)
{
    mbedtls_net_free(&_server_fd);
    mbedtls_x509_crt_free(&_cacert);
    mbedtls_ssl_free(&_tls);
    mbedtls_ssl_config_free(&_tls_cfg);
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_entropy_free(&_entropy);
}

// HTTPS Write
size_t MultiHTTPSClient::write(const char* request)
{
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
}

// HTTPS Read
size_t MultiHTTPSClient::read(char* response, const size_t response_len)
{
    int ret;
_printf("Reading\n");
    ret = mbedtls_ssl_read(&_tls, (unsigned char*)response, response_len);
_printf("OK\n");

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

    return (size_t)ret;
}


// HTTP Read Response
uint8_t MultiHTTPSClient::read_response(char* response, const size_t response_max_len, 
        const unsigned long response_timeout)
{
    size_t rc = 0;

    rc = read(response, response_max_len);
    if(rc > 0)
        return 0;
    else
        return 1;
}

/**************************************************************************************************/

#endif
