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

// Custom libraries
#include "utlgbotlib.h"

/**************************************************************************************************/

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
