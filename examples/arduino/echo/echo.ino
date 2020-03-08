/**************************************************************************************************/
// Example: echobot
// Description: 
//   Bot that response to any received text message with the same text received (echo messages).
//   It gives you a basic idea of how to receive and send messages.
// Created on: 21 apr. 2019
// Last modified date: 21 apr. 2019
// Version: 1.0.0
/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// Device libraries (Arduino ESP32/ESP8266 Cores)
#include <Arduino.h>
#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif
// Custom libraries
#include <utlgbotlib.h>

/**************************************************************************************************/

// WiFi Parameters
#define WIFI_SSID "mynet1234"
#define WIFI_PASS "password1234"
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63

// Telegram Bot Token (Get from Botfather)
#define TLG_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Enable Bot debug level (0 - None; 1 - Bot Level; 2 - Bot+HTTPS Level)
#define DEBUG_LEVEL_UTLGBOT 0

/**************************************************************************************************/

/* Functions Prototypes */

void wifi_init_stat(void);
bool wifi_handle_connection(void);

/**************************************************************************************************/

/* Globals */

// Create Bot object
uTLGBot Bot(TLG_TOKEN);

/**************************************************************************************************/

/* Main Function */

void setup(void)
{
    // Enable Bot debug
    Bot.set_debug(DEBUG_LEVEL_UTLGBOT);

    // Initialize Serial
    Serial.begin(115200);

    // Initialize WiFi station connection
    wifi_init_stat();

    // Wait WiFi connection
    Serial.println("Waiting for WiFi connection.");
    while(!wifi_handle_connection())
    {
        Serial.println(".");
        delay(1000);
    }

    // Bot getMe command
    Bot.getMe();
}

void loop()
{
    // Check if WiFi is connected
    if(!wifi_handle_connection())
    {
        // Wait 100ms and check again
        delay(100);
        return;
    }

    // Test Bot getUpdate command and receive messages
    while(Bot.getUpdates())
    {
        // Send an echo message back
        Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text);

        // Feed the Watchdog
        yield();
    }

    // Wait 1s for next iteration
    delay(1000);
}

/**************************************************************************************************/

/* Functions */

// Init WiFi interface
void wifi_init_stat(void)
{
    Serial.println("Initializing TCP-IP adapter...");
    Serial.print("Wifi connecting to SSID: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.println("TCP-IP adapter successfuly initialized.");
}

/**************************************************************************************************/

/* WiFi Change Event Handler */

bool wifi_handle_connection(void)
{
    static bool wifi_connected = false;

    // Device is not connected
    if(WiFi.status() != WL_CONNECTED)
    {
        // Was connected
        if(wifi_connected)
        {
            Serial.println("WiFi disconnected.");
            wifi_connected = false;
        }

        return false;
    }
    // Device connected
    else
    {
        // Wasn't connected
        if(!wifi_connected)
        {
            Serial.println("");
            Serial.println("WiFi connected");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            wifi_connected = true;
        }

        return true;
    }
}

/**************************************************************************************************/
