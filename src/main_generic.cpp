/**************************************************************************************************/
// Project: uTLGBotLib
// File: main.cpp
// Description: Project main file
// Created on: 26 apr. 2019
// Last modified date: 01 may. 2019
// Version: 0.0.1
/**************************************************************************************************/

#if !defined(ARDUINO) && !defined(ESPIDF) // Windows or Linux

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <stdio.h>
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
        // Test connection and disconnection
        printf("Connection: %d\n", Bot.is_connected());
        Bot.connect();
        printf("Connection: %d\n", Bot.is_connected());
        Bot.disconnect();
        printf("Connection: %d\n", Bot.is_connected());

        /*
        // Test Bot getMe command
        Bot.getMe();

        // Test Bot sendMessage command
        Bot.sendMessage(-244141233, "Hello world");
        Bot.sendMessage(-244141233, "<b>HTML Parse-response Test</b>", "HTML", false, false, 1046);
        
        // Test Bot getUpdate command and receive messages
        while(Bot.getUpdates())
        {
            printf("-----------------------------------------\n");
            printf("Received message.\n");

            printf("  From chat ID: %" PRIi64 "\n", Bot.received_msg.chat.id);
            printf("  From chat type: %s\n", Bot.received_msg.chat.type);
            printf("  From chat alias: %s\n", Bot.received_msg.chat.username);
            printf("  From chat name: %s %s\n", Bot.received_msg.chat.first_name, 
                Bot.received_msg.chat.last_name);
            printf("  From chat title: %s\n", Bot.received_msg.chat.title);
            if(Bot.received_msg.chat.all_members_are_administrators)
                printf("  From chat where all members are admins.\n");
            else
                printf("  From chat where not all members are admins.\n");
            
            printf("  From user ID: %" PRIi64 "\n", Bot.received_msg.from.id);
            printf("  From user alias: %s\n", Bot.received_msg.from.username);
            printf("  From user name: %s %s\n", Bot.received_msg.from.first_name, 
                Bot.received_msg.from.last_name);
            printf("  From user with language code: %s\n", Bot.received_msg.from.language_code);
            if(Bot.received_msg.from.is_bot)
                printf("  From user that is a Bot.\n");
            else
                printf("  From user that is not a Bot.\n");
            
            printf("  Message ID: %" PRIi64 "\n", Bot.received_msg.message_id);
            printf("  Message sent date (UNIX epoch time): %" PRIi32 "\n", Bot.received_msg.date);
            printf("  Text: %s\n", Bot.received_msg.text);
            printf("-----------------------------------------\n");
        }*/
        
        // Wait 10s for next iteration
        _delay(10000);
    }

    return 0;
}

/**************************************************************************************************/

#endif
