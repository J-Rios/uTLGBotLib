/**************************************************************************************************/
// Project: uTLGBotLib
// File: echobot.ino
// Description: Project main file
// Created on: 21 apr. 2019
// Last modified date: 21 apr. 2019
// Version: 1.0.0
/**************************************************************************************************/

#ifdef ARDUINO

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

// Device libraries (Arduino ESP32 Core)
#include <Arduino.h>
#include <WiFi.h>

// Custom libraries
#include "utlgbotlib.h"

/**************************************************************************************************/

// WiFi Parameters
#define WIFI_SSID "mynet1234"
#define WIFI_PASS "password1234"
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63

// Telegram Bot Token (Get from Botfather)
#define TLG_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

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
    Serial.begin(115200);
    
    // Initialize WiFi station connection
    wifi_init_stat();
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

    // Check and handle any received message
    while(Bot.getUpdates())
    {
        Serial.printf("Message received from %s at %s, sending it back.\n", 
            Bot.received_msg.from.first_name, Bot.received_msg.chat.title);
        Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text);
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

    printf("TCP-IP adapter successfuly initialized.\n");
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

#endif
