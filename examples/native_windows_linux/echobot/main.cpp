/**************************************************************************************************/
// Project: uTLGBotLib
// File: echobot.cpp
// Description: Project main file
// Created on: 11 may. 2019
// Last modified date: 11 may. 2019
// Version: 1.0.0
/**************************************************************************************************/

#if defined(WIN32) || defined(_WIN32) || defined(__linux__)

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <string.h>

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

#if defined(WIN32) || defined(_WIN32) // Windows
    #define _delay(x) do { Sleep(x); } while(0)
#elif defined(__linux__)
    #define _delay(x) do { usleep(x*1000); } while(0)
#endif

/**************************************************************************************************/

/* Main Function */

int main(void)
{
    // Create Bot object
    uTLGBot Bot(TLG_TOKEN);
    
    // Main loop
    while(1)
    {
        // Check and handle any received message
        while(Bot.getUpdates())
        {
            printf("Message received from %s at %s, sending it back.\n", 
                Bot.received_msg.from.first_name, Bot.received_msg.chat.title);
            Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text);
        }
        
        // Wait 1s for next iteration
        _delay(1000);
    }
}

/**************************************************************************************************/

#endif
