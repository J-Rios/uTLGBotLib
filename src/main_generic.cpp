/**************************************************************************************************/
// Project: uTLGBotLib
// File: main.cpp
// Description: Project main file
// Created on: 26 apr. 2019
// Last modified date: 02 dec. 2019
// Version: 1.0.1
/**************************************************************************************************/

#if defined(WIN32) || defined(_WIN32) || defined(__linux__) // Windows or Linux

/**************************************************************************************************/

/* Libraries */

// Standard C/C++ libraries
#include <stdio.h>
#include <string.h>

// Custom libraries
#include "utlgbotlib.h"

/**************************************************************************************************/

// Telegram Bot Token (Get from Botfather)
#define TLG_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Enable Bot debug level
#define DEBUG_LEVEL_UTLGBOT 0

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

    // Enable Bot debug
    Bot.set_debug(DEBUG_LEVEL_UTLGBOT);

#if defined(WIN32) || defined(_WIN32)
    printf("\nRunning test program in Windows.\n\n");
#elif defined(__linux__)
    printf("\nRunning test program in Linux.\n\n");
#endif

    // Bot getMe command
    Bot.getMe();

    // Bot sendMessage command
    //Bot.sendMessage("-1001162829058", "<b>Hello World</b>", "HTML", false, false);

    // Main loop
    while(1)
    {
        // Bot getUpdate command and receive messages
        while(Bot.getUpdates())
        {
            printf("\n-----------------------------------------\n");
            printf("Received message.\n");

            printf("  From chat ID: %s\n", Bot.received_msg.chat.id);
            printf("  From chat type: %s\n", Bot.received_msg.chat.type);
            printf("  From chat alias: %s\n", Bot.received_msg.chat.username);
            printf("  From chat name: %s %s\n", Bot.received_msg.chat.first_name, 
                Bot.received_msg.chat.last_name);
            printf("  From chat title: %s\n", Bot.received_msg.chat.title);
            if(Bot.received_msg.chat.all_members_are_administrators)
                printf("  From chat where all members are admins.\n");
            else
                printf("  From chat where not all members are admins.\n");
            
            printf("  From user ID: %s\n", Bot.received_msg.from.id);
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

            // Send echo message back
            Bot.sendMessage(Bot.received_msg.chat.id, Bot.received_msg.text);
        }

        // Wait 1s for next iteration
        _delay(1000);
    }

    return 0;
}

/**************************************************************************************************/

#endif
