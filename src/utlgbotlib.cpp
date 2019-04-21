/**************************************************************************************************/
// Project: uTLGBotLib
// File: utlgbot.h
// Description: Lightweight Library to implement Telegram Bots.
// Created on: 19 mar. 2019
// Last modified date: 21 apr. 2019
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
    #define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)

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
    #define sscanf_P(...) do { sscanf(__VA_ARGS__); } while(0)
    
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
    memset(_json_value_str, '\0', MAX_JSON_STR_LEN);
    memset(_json_subvalue_str, '\0', MAX_JSON_SUBVAL_STR_LEN);
    memset(_json_elements, 0, MAX_JSON_ELEMENTS);
    memset(_json_subelements, 0, MAX_JSON_SUBELEMENTS);
    _connected = false;
    _last_received_msg = 1;

    // Clear message data
    clear_msg_data();
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

        // Disconnect from telegram server
        if(is_connected())
            disconnect();
            
        return false;
    }

    // Parse and check response
    _println(F("\n[Bot] Response received:"));
    _println(_response);
    _println(" ");

    // Disconnect from telegram server
    if(is_connected())
        disconnect();

    return true;
}

// Request Bot send text message to specified chat ID (The Bot should be in that Chat)
// Note: reply_markup not implemented
uint8_t uTLGBot::sendMessage(const int64_t chat_id, const char* text, const char* parse_mode, 
    bool disable_web_page_preview, bool disable_notification, uint64_t reply_to_message_id)
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
    
    // Create HTTP Body request data
    char msg[HTTP_MAX_BODY_LENGTH];
    snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("{\"chat_id\":%" PRIi64 ", \"text\":\"%s\"}"), 
        chat_id, text);
    // If parse_mode is not empty
    if(strcmp(parse_mode, "") != 0)
    {
        // If parse mode has an expected value
        if((strcmp(parse_mode, "Markdown") == 0) || (strcmp(parse_mode, "HTML") == 0))
        {
            // Remove last brace and append the new field
            msg[strlen(msg)-1] = '\0';
            snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("%s, \"parse_mode\":\"%s\"}"), msg, 
                parse_mode);
        }
        else
            _println("[Bot] Warning: Invalid parse_mode provided.");
    }
    // Remove last brace and append disable_web_page_preview value if true
    if(disable_web_page_preview)
    {
        msg[strlen(msg)-1] = '\0';
        snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("%s, \"disable_web_page_preview\":true}"), msg);
    }
    // Remove last brace and append disable_notification value if true
    if(disable_notification)
    {
        msg[strlen(msg)-1] = '\0';
        snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("%s, \"disable_notification\":true}"), msg);
    }
    // Remove last brace and append reply_to_message_id value if set
    if(reply_to_message_id != 0)
    {
        msg[strlen(msg)-1] = '\0';
        snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("%s, \"reply_to_message_id\":%" PRIu64 "}"), 
            msg, reply_to_message_id);
    }

    // Send the request
    _println(F("[Bot] Trying to send message request..."));
    request_result = tlg_post(API_CMD_SEND_MSG, msg, strlen(msg), _response, HTTP_MAX_RES_LENGTH);
    
    // Check if request has fail
    if(request_result == 0)
    {
        _println(F("[Bot] Command fail, no response received."));

        // Disconnect from telegram server
        if(is_connected())
            disconnect();
            
        return false;
    }

    // Parse and check response
    _println(F("\n[Bot] Response received:"));
    _println(_response);
    _println(" ");

    // Disconnect from telegram server
    if(is_connected())
        disconnect();

    return true;
}

// Request for check how many availables messages are waiting to be received
uint8_t uTLGBot::getUpdates(void)
{
    uint8_t request_result;
    bool connected;
    
    // Connect to telegram server
    connected = is_connected();
    if(!connected)
    {
        connected = connect();
        if(!connected)
            return 0;
    }

    // Create HTTP Body request data (Note that we limit messages to 1 and just allow text messages)
    char msg[HTTP_MAX_BODY_LENGTH];
    snprintf_P(msg, HTTP_MAX_BODY_LENGTH, PSTR("{\"offset\":%zu, \"limit\":1, " \
        "\"timeout\":%" PRIu64 ", \"allowed_updates\":[\"message\"]}"), _last_received_msg, 
        (uint64_t)TLG_LONG_POLL);

    // Send the request
    _println(F("[Bot] Trying to send getUpdates request..."));
    request_result = tlg_post(API_CMD_GET_UPDATES, msg, strlen(msg), _response, 
        HTTP_MAX_RES_LENGTH, HTTP_WAIT_RESPONSE_TIMEOUT+(TLG_LONG_POLL*1000));
    
    // Check if request has fail
    if(request_result == 0)
    {
        _println(F("[Bot] Command fail, no response received."));

        // Disconnect from telegram server
        if(is_connected())
            disconnect();

        return 0;
    }

    // Remove start and end list characters ('[' and ']') from response and just keep json structure
    char* ptr_response = &_response[0];
    if(strlen(ptr_response) >= 2)
    {
        ptr_response[strlen(ptr_response)-1] = '\0';
        ptr_response[0] = '\0';
        ptr_response = ptr_response + 1;
    }

    _println(F("\n[Bot] Response received:"));
    _println(ptr_response);
    _println(" ");

    // Check if response is empty (there is no message)
    if(strlen(ptr_response) == 0)
    {
        _println(F("[Bot] There is not new message."));

        // Disconnect from telegram server
        if(is_connected())
            disconnect();

        return 0;
    }

    // A new message received, so lets clear all message data
    clear_msg_data();

    /**********************************************************************************************/

    /* Response JSON Parse */

    uint32_t num_elements, num_subelements;
    uint32_t key_position;

    // Clear json elements objects
    memset(_json_elements, 0, MAX_JSON_ELEMENTS);
    memset(_json_subelements, 0, MAX_JSON_SUBELEMENTS);

    // Parse message string as JSON and get each element
    num_elements = json_parse_str(ptr_response, strlen(ptr_response), _json_elements, 
        MAX_JSON_ELEMENTS);
    if(num_elements == 0)
    {
        _println(F("[Bot] Error: Bad JSON sintax from received response."));

        // Disconnect from telegram server
        if(is_connected())
            disconnect();
        
        return 0;
    }

    // Check and get value of key: update_id
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "update_id");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Save value in variable
        sscanf_P(_json_value_str, PSTR("%zu"), &_last_received_msg);
        
        // Prepare variable to next update message request (offset)
        _last_received_msg = _last_received_msg + 1;
    }

    // Check and get value of key: message_id
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "message_id");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Save value in variable
        //sscanf_P(_json_value_str, PSTR(SCNd64), &received_msg.message_id); // Not compile
        sscanf_P(_json_value_str, PSTR("%lld"), &received_msg.message_id);
    }

    // Check and get value of key: date
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "date");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Save value in variable
        //sscanf_P(_json_value_str, PSTR(SCNu32), &received_msg.date); // Not compile
        sscanf_P(_json_value_str, PSTR("%ul"), &received_msg.date);
    }

    // Check and get value of key: text
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "text");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Save value in variable
        snprintf_P(received_msg.text, MAX_TEXT_LENGTH, PSTR("%s"), _json_value_str);
    }

    // Check and get value of key: from
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "from");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Parse string "from" content as JSON and get each element
        num_subelements = json_parse_str(_json_value_str, strlen(_json_value_str), _json_subelements, 
            MAX_JSON_SUBELEMENTS);
        if(num_subelements == 0)
            _println(F("[Bot] Error: Bad JSON sintax in \"from\" element."));
        else
        {
            // Check and get value of key: id
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, "id");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                //sscanf_P(_json_subvalue_str, PSTR(SCNd64), &received_msg.from.id); // Not compile
                sscanf_P(_json_subvalue_str, PSTR("%lld"), &received_msg.from.id);
            }

            // Check and get value of key: is_bot
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "is_bot");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                if(strcmp(_json_subvalue_str, "true") == 0)
                    received_msg.from.is_bot = true;
                else
                    received_msg.from.is_bot = false;
            }

            // Check and get value of key: first_name
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "first_name");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.from.first_name, MAX_USER_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: last_name
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "last_name");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.from.last_name, MAX_USER_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: username
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "username");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.from.username, MAX_USERNAME_LENGTH, PSTR("@%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: language_code
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "language_code");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.from.language_code, MAX_LANGUAGE_CODE_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }
        }
    }

    // Check and get value of key: chat
    key_position = json_has_key(ptr_response, _json_elements, num_elements, "chat");
    if(key_position != 0)
    {
        // Get json element string
        json_get_element_string(ptr_response, &_json_elements[key_position+1], _json_value_str, 
            MAX_JSON_STR_LEN);

        // Parse string "from" content as JSON and get each element
        num_subelements = json_parse_str(_json_value_str, strlen(_json_value_str), _json_subelements, 
            MAX_JSON_ELEMENTS);
        if(num_subelements == 0)
            _println(F("[Bot] Error: Bad JSON sintax in \"from\" element."));
        else
        {
            // Check and get value of key: id
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, "id");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                //sscanf_P(_json_subvalue_str, PSTR(SCNd64), &received_msg.chat.id); // Not compile
                sscanf_P(_json_subvalue_str, PSTR("%lld"), &received_msg.chat.id);
            }

            // Check and get value of key: type
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, "type");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.chat.type, MAX_CHAT_TYPE_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: title
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, "title");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.chat.title, MAX_CHAT_TITLE_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: username
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "username");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.chat.username, MAX_USERNAME_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: first_name
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "first_name");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.chat.first_name, MAX_USER_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: last_name
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "last_name");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                snprintf_P(received_msg.chat.last_name, MAX_USER_LENGTH, PSTR("%s"), 
                    _json_subvalue_str);
            }

            // Check and get value of key: is_bot
            key_position = json_has_key(_json_value_str, _json_subelements, num_subelements, 
                "all_members_are_administrators");
            if(key_position != 0)
            {
                // Get json element string
                json_get_element_string(_json_value_str, &_json_subelements[key_position+1], 
                    _json_subvalue_str, MAX_JSON_SUBVAL_STR_LEN);

                // Save value in variable
                if(strcmp(_json_subvalue_str, "true") == 0)
                    received_msg.chat.all_members_are_administrators = true;
                else
                    received_msg.chat.all_members_are_administrators = false;
            }
        }
    }

    // Disconnect from telegram server
    if(is_connected())
        disconnect();
    
    return 1;
}

/**************************************************************************************************/

/* Telegram API GET and POST Methods */

// Make and send a HTTP GET request
uint8_t uTLGBot::tlg_get(const char* command, char* response, const size_t response_len, 
    const unsigned long response_timeout)
{
    char uri[HTTP_MAX_URI_LENGTH];
    char reader_buff[HTTP_MAX_RES_LENGTH];
    
    // Create URI and send GET request
    snprintf_P(uri, HTTP_MAX_URI_LENGTH, PSTR("%s/%s"), _tlg_api, command);
    if(https_client_get(uri, TELEGRAM_HOST, response, response_len, response_timeout) > 0)
        return false;
    
    // Check and remove response header (just keep response body)
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\r\n\r\n", reader_buff, false))
    {
        // Clear response if unexpected response
        _println(F("[Bot] Unexpected response."));
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
        _println(F("[Bot] Unexpected response."));
        memset(response, '\0', response_len);
        return false;
    }
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, ",", reader_buff, false))
    {
        // Clear response if unexpected response
        _println(F("[Bot] Unexpected response."));
        memset(response, '\0', response_len);
        return false;
    }
    reader_buff[strlen(reader_buff)-1] = '\0';

    // Check if request "ok" response value is "true"
    if(strcmp(reader_buff, "true") != 0)
    {
        // Clear response due bad request response ("ok" != true)
        _println(F("[Bot] Bad request."));
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
        _println(F("[Bot] Unexpected response."));
        memset(response, '\0', response_len);
        return false;
    }
    response[strlen(response)-1] = '\0';
    
    return true;
}

// Make and send a HTTP GET request
uint8_t uTLGBot::tlg_post(const char* command, const char* body, const size_t body_len, 
    char* response, const size_t response_len, const unsigned long response_timeout)
{
    char uri[HTTP_MAX_URI_LENGTH];
    char reader_buff[HTTP_MAX_RES_LENGTH];
    
    // Create URI and send POST request
    snprintf_P(uri, HTTP_MAX_URI_LENGTH, PSTR("%s/%s"), _tlg_api, command);
    if(https_client_post(uri, TELEGRAM_HOST, body, body_len, response, response_len, 
        response_timeout) > 0)
    {
        return false;
    }
    
    // Check and remove response header (just keep response body)
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\r\n\r\n", reader_buff, false))
    {
        // Clear response if unexpected response
        _println(F("[Bot] Unexpected response."));
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
        _println(F("[Bot] Unexpected response."));
        memset(response, '\0', response_len);
        return false;
    }
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, ",", reader_buff, false))
    {
        // Clear response if unexpected response
        _println(F("[Bot] Unexpected response."));
        memset(response, '\0', response_len);
        return false;
    }
    reader_buff[strlen(reader_buff)-1] = '\0';

    // Check if request "ok" response value is "true"
    if(strcmp(reader_buff, "true") != 0)
    {
        // Clear response due bad request response ("ok" != true)
        _println(F("[Bot] Bad request."));
        memset(response, '\0', response_len);
        return false;
    }

    // Remove root json response and just keep "result" attribute json value in response buffer
    // i.e. for response: {"ok":true,"result":{"id":123456789,"first_name":"esp32_Bot"}}
    // just keep: {"id":123456789,"first_name":"esp32_Bot"}
    memset(reader_buff, '\0', HTTP_MAX_RES_LENGTH);
    if(!cstr_read_until_word(response, "\"result\":", reader_buff, false))
    {
        // Clear response if unexpected response
        _println(F("[Bot] Unexpected response."));
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
        if(_tls != NULL)
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
                _printf(F("[HTTPS] Client write error 0x%x\n"), ret);
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
            _printf(F("[HTTPS] Client read error -0x%x\n"), -ret);
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
    // Note that we use specific header values for Telegram requests
    memset(response, '\0', response_len);
    snprintf_P(request, HTTP_MAX_GET_LENGTH, PSTR("GET %s HTTP/1.1\r\nHost: %s\r\n" \
               "User-Agent: ESP32\r\nAccept: text/html,application/xml,application/json" \
               "\r\n\r\n"), uri, host);

    // Send request
    _print(F("HTTP request to send: "));
    _println(request);
    _println();
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

    //_printf(F("[HTTPS] Response: %s\n\n"), response);
    
    return 0;
}

// Make and send a HTTP POST request
uint8_t uTLGBot::https_client_post(const char* uri, const char* host, const char* body, 
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
    _print(F("HTTP request to send: "));
    _println(request);
    _println();
    if(https_client_write(request) != strlen(request))
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
        if(https_client_read(response, response_len))
        {
            _println(F("[HTTPS] Response successfully received."));
            break;
        }
    }

    //_printf(F("[HTTPS] Response: %s\n\n"), response);
    
    return 0;
}

/**************************************************************************************************/

/* Private Auxiliar Methods */

// Clear and set all received message data to default values
void uTLGBot::clear_msg_data(void)
{
    received_msg.message_id = 0;
    received_msg.date = 0;
    received_msg.text[0] = '\0';
    received_msg.from.id = 0;
    received_msg.from.is_bot = false;
    received_msg.from.first_name[0] = '\0';
    received_msg.from.last_name[0] = '\0';
    received_msg.from.username[0] = '\0';
    received_msg.from.language_code[0] = '\0';
    received_msg.chat.id = 0;
    received_msg.chat.type[0] = '\0';
    received_msg.chat.title[0] = '\0';
    received_msg.chat.username[0] = '\0';
    received_msg.chat.first_name[0] ='\0';
    received_msg.chat.last_name[0] = '\0';
    received_msg.chat.all_members_are_administrators = false;
}

// Parse and get each json elements from provided json format string
uint32_t uTLGBot::json_parse_str(const char* json_str, const size_t json_str_len, 
    jsmntok_t* json_tokens, const uint32_t json_tokens_len)
{
    jsmn_parser json_parser;
    int num_elements;

    jsmn_init(&json_parser);
    num_elements = jsmn_parse(&json_parser, json_str, json_str_len, json_tokens, json_tokens_len);
    if(num_elements < 0)
    {
        _print(F("Can't parse JSON data. Code "));
        _println(num_elements);
        _println();
        return 0;
    }
    if((num_elements == 0) || (json_tokens[0].type != JSMN_OBJECT))
    {
        _println(F("Can't parse JSON data (invalid sintax?)."));
        return 0;
    }

    return num_elements;
}

// Check if given json object contains the provided key
uint32_t uTLGBot::json_has_key(const char* json_str, jsmntok_t* json_tokens, 
    const uint32_t num_tokens, const char* key)
{
    for(uint32_t i = 0; i < num_tokens; i++)
    {
        // Continue to next iteration if json element is not a string
        if(json_tokens[i].type != JSMN_STRING)
            continue;

        // Continue to next iteration if key and json elements lengths are different
        if(strlen(key) != (unsigned int)(json_tokens[i].end-json_tokens[i].start))
            continue;

        // Check if key and json element string are the same
        if(strncmp(json_str + json_tokens[i].start, key, 
            json_tokens[i].end - json_tokens[i].start) == 0)
        {
            return i;
        }
    }
    return 0;
}

// Get the corresponding string of given json element (token)
void uTLGBot::json_get_element_string(const char* json_str, jsmntok_t* token, char* converted_str, 
    const uint32_t converted_str_len)
{
    uint32_t value_len = token->end - token->start;
    const char* value = json_str + token->start;

    memset(converted_str, '\0', converted_str_len);
    for(uint32_t i = 0; i < value_len; i++) // Dont trust memcpy...
        converted_str[i] = value[i];
}

// Get the corresponding string value of given json key
uint8_t uTLGBot::json_get_key_value(const char* key, const char* json_str, jsmntok_t* tokens, 
    const uint32_t num_tokens, char* converted_str, const uint32_t converted_str_len)
{
    // Check for key
    size_t key_position = json_has_key(json_str, tokens, num_tokens, key);
    if(key_position == 0)
    {
        _println(F("No key found inside json."));
        return false;
    }
    else
    {
        json_get_element_string(json_str, &tokens[key_position+1], converted_str, 
            converted_str_len);
    }
    
    return true;
}


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
