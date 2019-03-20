/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbot.h
// Description: Lightweight Library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 20 mar. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Libraries */

#include "utlgbotlib.h"
#include "tlgcert.h"

/**************************************************************************************************/

/* Macros */

#ifdef ARDUINO
    #define _print(x) do { Serial.print(x); } while(0)
    #define _println(x) do { Serial.println(x); } while(0)
    #define _printf(...) do { Serial.printf(__VA_ARGS__); } while(0)

    #define _millis() millis()
    #define _delay(x) do { delay(x); } while(0)
#else /* ESP-IDF */
    #define _millis() (unsigned long)(esp_timer_get_time()/1000)
    #define _print(x) do { printf(x); } while(0)
    #define _println(x) do { printf(x); printf("\n"); } while(0)
    #define _printf(...) do { printf(__VA_ARGS__); } while(0)

    #define F(x) x
    #define PSTR(x) x
    #define snprintf_P(...) do { snprintf(__VA_ARGS__); } while(0)
    
    #define _delay(x) do { vTaskDelay(x/portTICK_PERIOD_MS); } while(0)
    #define PROGMEM
#endif

/**************************************************************************************************/

/* Constructor */

// TLGBot constructor, initialize and setup secure client with telegram cert and get the token
uTLGBot::uTLGBot(const char* token)
{
    https_client_init();
    
    snprintf(_token, TOKEN_LENGTH, "%s", token);
    snprintf(_tlg_api, TELEGRAM_API_LENGTH, "/bot%s", token);
    memset(_response, '\0', HTTP_MAX_RES_LENGTH);
    _connected = false;
}

/**************************************************************************************************/

/* Public Methods */

// Connect to Telegram server
uint8_t uTLGBot::connect(void)
{
    _println(F("[Bot] Connecting to telegram server..."));

    if(https_client_is_connected())
    {
        _println(F("[Bot] Already connected to server."));
        return true;
    }

    if(!https_client_connect(TELEGRAM_HOST, HTTPS_PORT))
    {
        _println(F("[Bot] Conection fail."));
        return false;
    }

    _println(F("[Bot] Successfully connected."));

    return true;
}

// Disconnect from Telegram server
void uTLGBot::disconnect(void)
{
    _println(F("[Bot] Disconnecting from telegram server..."));

    if(!https_client_is_connected())
    {
        _println(F("[Bot] Already disconnected from server."));
        return;
    }

    https_client_disconnect();

    _println(F("[Bot] Successfully disconnected."));
}

// Check for Bot connection to server status
bool uTLGBot::is_connected(void)
{
    return https_client_is_connected();
}

// Request Bot info by sending getMe command
uint8_t uTLGBot::getMe(void)
{
    uint8_t request_result;
    bool connected;
    
    // Connect to telegram server
    connected = is_connected();
    if(!connected)
    {
        connected = connect();
        if(!connected)
            return false;
    }
    
    // Send the request
    _println(F("[Bot] Trying to send getMe request..."));
    request_result = tlg_get(API_CMD_GET_ME, _response, HTTP_MAX_RES_LENGTH);
    
    // Check if request has fail
    if(request_result == 0)
    {
        _println(F("[Bot] Command fail, no response received."));
        return false;
    }

    // Parse and check response
    _println(F("\n[Bot] Response received:"));
    _println(_response);
    _println("");

    // Disconnect from telegram server
    if(is_connected())
        disconnect();

    return true;
}

/**************************************************************************************************/

/* Telegram API GET and POST Methods */

// Make and send a HTTP GET request
uint8_t uTLGBot::tlg_get(const char* command, char* response, const size_t response_len)
{
    char uri[HTTP_MAX_URI_LENGTH];
    char reader_buff[HTTP_MAX_RES_LENGTH];
    
    // Create URI and send GET request
    snprintf_P(uri, HTTP_MAX_URI_LENGTH, PSTR("%s/%s"), _tlg_api, command);
    if(https_client_get(uri, TELEGRAM_HOST, response, response_len) > 0)
        return false;
    
    // Check and remove response header (just keep response body)
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\r\n\r\n", reader_buff, false))
    {
        // Clear response if unexpected response
        _println("[Bot] Unexpected response.");
        memset(response, '\0', response_len);
        return false;
    }

    // Check for and get request "ok" response key
    // Note: We are assumming "ok" attribute comes before "response" attribute 
    // (not preserving response buffer)
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\"ok\":", reader_buff, false))
    {
        // Clear response if unexpected response
        _println("[Bot] Unexpected response.");
        memset(response, '\0', response_len);
        return false;
    }
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, ",", reader_buff, false))
    {
        // Clear response if unexpected response
        _println("[Bot] Unexpected response.");
        memset(response, '\0', response_len);
        return false;
    }
    reader_buff[strlen(reader_buff)-1] = '\0';

    // Check if request "ok" response value is "true"
    if(strcmp(reader_buff, "true") != 0)
    {
        // Clear response due bad request response ("ok" != true)
        _println("[Bot] Bad request.");
        memset(response, '\0', response_len);
        return false;
    }

    // Remove root json response and just keep "result" attribute json value in response buffer
    // i.e. for response: {"ok":true,"result":{"id":123456789,"first_name":"esp8266_Bot"}}
    // just keep: {"id":123456789,"first_name":"esp8266_Bot"}
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\"result\":", reader_buff, false))
    {
        // Clear response if unexpected response
        _println("[Bot] Unexpected response.");
        memset(response, '\0', response_len);
        return false;
    }
    response[strlen(response)-1] = '\0';
    
    return true;
}

/**************************************************************************************************/

/* Private Methods - HAL functions (Implement network functionality for differents devices) */

// Initialize HTTPS client
void uTLGBot::https_client_init(void)
{
    #ifdef ARDUINO
        _client = new WiFiClientSecure();
        //_client->setCACert(cert_https_api_telegram_org);
    #else /* ESP-IDF */
        _tls = NULL;
        /*_tls_cfg.cacert_pem_buf = server_root_cert_pem_start;
        _tls_cfg.cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start;*/
        /*_tls_cfg.cacert_pem_buf = cert_https_api_telegram_org;
        _tls_cfg.cacert_pem_bytes = strlen((char*)cert_https_api_telegram_org);*/
        unsigned int cert_size = tlg_api_ca_pem_end - tlg_api_ca_pem_start;
        static esp_tls_cfg_t tls_cfg = {
            .alpn_protos = NULL,
            .cacert_pem_buf = tlg_api_ca_pem_start,
            .cacert_pem_bytes = cert_size,
            .non_block = false,
        };
        _tls_cfg = &tls_cfg;
    #endif
}

// Make HTTPS client connection to server
bool uTLGBot::https_client_connect(const char* host, int port)
{
    #ifdef ARDUINO
        return _client->connect(host, port);
    #else /* ESP-IDF */
        _tls = esp_tls_conn_new(host, strlen(host), port, _tls_cfg);
        if(_tls == NULL)
            _connected = false;
        else
            _connected = true;
        return _connected;
    #endif
}

// HTTPS client disconnect from server
void uTLGBot::https_client_disconnect(void)
{
    #ifdef ARDUINO
        _client->stop();
    #else
        esp_tls_conn_delete(_tls);
        _connected = false;
    #endif
}

// Check if HTTPS client is connected
bool uTLGBot::https_client_is_connected(void)
{
    #ifdef ARDUINO
        return _client->connected();
    #else /* ESP-IDF */
        // Note: ESP-IDF 3.1 doesn't has connection state in ESP-TLS
        /*if(_tls != NULL)
        {
            if(_tls->conn_state == ESP_TLS_DONE)
                return true;
        }
        return false;*/
        return _connected;
    #endif
}

size_t uTLGBot::https_client_write(const char* request)
{
    #ifdef ARDUINO
        return _client->print(request);
    #else /* ESP-IDF */
        size_t written_bytes = 0;
        int ret;
        
        do
        {
            ret = esp_tls_conn_write(_tls, request + written_bytes, strlen(request) - written_bytes);
            if(ret >= 0)
                written_bytes += ret;
            else if(ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                _printf("HTTPS client write error 0x%x\n", ret);
                break;
            }
        } while(written_bytes < strlen(request));

        return written_bytes;
    #endif
}

bool uTLGBot::https_client_read(char* response, const size_t response_len)
{
    #ifdef ARDUINO
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
    #else /* ESP-IDF */
        int ret;

        ret = esp_tls_conn_read(_tls, response, response_len);

        if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
            return false;
        if(ret < 0)
        {
            _printf("HTTPS client read error -0x%x\n", -ret);
            return false;
        }
        
        if(ret >= 0)
            return true;
        return false;
    #endif
}

// Make and send a HTTP GET request
uint8_t uTLGBot::https_client_get(const char* uri, const char* host, char* response, 
    const size_t response_len, const unsigned long response_timeout)
{
    unsigned long t0, t1;
    char request[HTTP_MAX_GET_LENGTH];

    // Clear response buffer and create request
    memset(response, '\0', response_len);
    snprintf_P(request, HTTP_MAX_GET_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
               "User-Agent: ESP32\r\nAccept: text/html,application/xml,application/json" \
               "\r\n\r\n"), uri, host);

    // Send request
    //_printf("HTTP request to send: %s\n\n", request);
    if(https_client_write(request) != strlen(request))
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
        if(https_client_read(response, response_len))
        {
            _println(F("[HTTPS] Response successfully received."));
            break;
        }
    }

    //_printf("[HTTPS] Response: %s\n\n", response);
    
    return 0;
}

/**************************************************************************************************/

/* Private Auxiliar Methods */

// Read array until a specific word. The input string (str) split his content into readed chars 
// (readed) and keep not readed ones
bool uTLGBot::cstr_read_until_word(char* str, const char* word, char* readed, const bool preserve)
{
	const uint16_t readed_len = strlen(str);
	char* read_word = new char[strlen(word) + 1]();
	char char_read = '\0';
	uint16_t a, b, i;
	bool found = false;

	a = 0;	b = 0;	i = 0;
	memset(read_word, '\0', sizeof(strlen(word) + 1));
	while(i < strlen(str) + 1)
	{
		char_read = str[i];
		//printf("%c", char_read);

		if((readed_len > 0) && (a < readed_len - 1))
		{
			if(readed != NULL)
				readed[a] = char_read;
			a = a + 1;
		}

		if(char_read == word[b])
		{
			if(b < strlen(word))
			{
				read_word[b] = char_read;
				b = b + 1;
			}

			if(b == strlen(word))
			{
				read_word[b] = '\0';
				if(strcmp(read_word, word) == 0)
				{
					if(readed != NULL)
						readed[a] = '\0';

					// Remove readed data from str
					if(!preserve)
					{
						a = 0;
						i = i + 1;
						while(i < strlen(str))
						{
							str[a] = str[i];
							a = a + 1;
							i = i + 1;
						}
						str[a] = '\0';
					}

					found = true;
					break;
				}
			}
		}
		else
		{
			b = 0;
			memset(read_word, '\0', sizeof(strlen(word) + 1));
		}

		i = i + 1;
	}

	if(readed != NULL)
		readed[readed_len] = '\0';

	return found;
}
