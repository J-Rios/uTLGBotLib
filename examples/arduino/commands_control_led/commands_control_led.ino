/**************************************************************************************************/
// Example: echobot
// Description: 
//   Bot to control a LED through telegram commands.
//   It gives you an idea of how to detect specific words from user message and response to it.
//   Commands implemented are /start /help /ledon /ledoff /ledstatus
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

// Board Led Pin
#define PIN_LED 13

// Telegram Bot /start text message
const char TEXT_START[] = 
    "Hello, im a Bot running in an ESP microcontroller that let you turn on/off a LED/light.\n"
    "\n"
    "Check /help command to see how to use me.";

// Telegram Bot /help text message
const char TEXT_HELP[] = 
    "Available Commands:\n"
    "\n"
    "/start - Show start text.\n"
    "/help - Show actual text.\n"
    "/ledon - Turn on the LED.\n"
    "/ledoff - Turn off the LED.\n"
    "/ledstatus - Show actual LED status.";

/**************************************************************************************************/

/* Functions Prototypes */

void wifi_init_stat(void);
bool wifi_handle_connection(void);

/**************************************************************************************************/

/* Globals */

// Create Bot object
uTLGBot Bot(TLG_TOKEN);

// LED status
uint8_t led_status;

/**************************************************************************************************/

/* Main Function */

void setup(void)
{
    // Enable Bot debug
    Bot.set_debug(DEBUG_LEVEL_UTLGBOT);

    // Initialize Serial
    Serial.begin(115200);

    // Initialize LED pin as digital output
    digitalWrite(PIN_LED, LOW);
    pinMode(PIN_LED, OUTPUT);
    led_status = 0;

    // Initialize WiFi station connection
    wifi_init_stat();

    // Wait for WiFi connection
    Serial.println("Waiting for WiFi connection.");
    while(!wifi_handle_connection())
    {
        Serial.print(".");
        delay(500);
    }

    // Get Bot account info
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

    // Check for Bot received messages
    while(Bot.getUpdates())
    {
        // Show received message text
        Serial.println("");
        Serial.println("Received message:");
        Serial.println(Bot.received_msg.text);
        Serial.println("");

        // If /start command was received
        if(strncmp(Bot.received_msg.text, "/start", strlen("/start")) == 0)
        {
            // Send a Telegram message for start
            Bot.sendMessage(Bot.received_msg.chat.id, TEXT_START);
        }

        // If /help command was received
        else if(strncmp(Bot.received_msg.text, "/help", strlen("/help")) == 0)
        {
            // Send a Telegram message for start
            Bot.sendMessage(Bot.received_msg.chat.id, TEXT_HELP);
        }

        // If /ledon command was received
        else if(strncmp(Bot.received_msg.text, "/ledon", strlen("/ledon")) == 0)
        {
            // Turn on LED
            led_status = 1;
            digitalWrite(PIN_LED, HIGH);

            // Show command reception through Serial
            Serial.println("Command /ledon received.");
            Serial.println("Turning on the LED.");

            // Send a Telegram message to notify that the LED has been turned on
            Bot.sendMessage(Bot.received_msg.chat.id, "Led turned on.");
        }

        // If /ledoff command was received
        else if(strncmp(Bot.received_msg.text, "/ledoff", strlen("/ledoff")) == 0)
        {
            // Turn off LED
            led_status = 0;
            digitalWrite(PIN_LED, LOW);

            // Show command reception through Serial
            Serial.println("Command /ledoff received.");
            Serial.println("Turning off the LED.");

            // Send a Telegram message to notify that the LED has been turned off
            Bot.sendMessage(Bot.received_msg.chat.id, "Led turned off.");
        }

        // If /ledstatus command was received
        else if(strncmp(Bot.received_msg.text, "/ledstatus", strlen("/ledstatus")) == 0)
        {
            // Send a Telegram message to notify actual LED status
            if(led_status)
                Bot.sendMessage(Bot.received_msg.chat.id, "The LED is on.");
            else
                Bot.sendMessage(Bot.received_msg.chat.id, "The LED is off.");
        }

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