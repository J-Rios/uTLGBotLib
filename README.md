# uTLGBotLib
Universal Telegram Bot library for Arduino, ESP-IDF and Native (Windows and Linux) devices, that let you create Telegram Bots. You can use it with ESP8266 and ESP32 microcontrollers.

Micro Telegram Bot Library is a lightweight C++ library implementation that uses Telegram Bot API to create Bots. The library goal is to be compatible with multiples devices, from embedded microcontrollers, to Servers and PCs (Windows and Linux). Inside microcontrollers world, uTLGBotLib focus on Espressif ESP32 (support for ESP-IDF and Arduino frameworks) and ESP8266 (support for Arduino) microcontrollers, but new devices could be supported.

## Notes

- Due library target embedded devices such microcontrollers, it MUST be take a lot of care in memory usage, from library size to safety use.

- To avoid ram memory fragmentation and stack-heap collisions, the library doesn't use dynamic memory in non Native (Windows/Linux) platforms (ESP8266 and ESP32).

- The library uses [multihttpsclient](https://github.com/J-Rios/multihttpsclient) to implement all low level HTTP request specific for each device/system.

- The library uses [jsmn library](https://github.com/zserge/jsmn) to parse JSON text in the safest (memory) way possible, because it just get a string and return the indexes where each json element ("token") start and end.

- Sub-library multihttpsclient uses [mbedtls library](https://github.com/ARMmbed/mbedtls) to handle HTTPS requests in Native (Windows and Linux) systems.

- uTLGBotLib is a generic library, for that reason, to add support of a new device/system, you just need to specify the expected print() macros in utlgbotlib.cpp and create specific files in multihttpsclient library to implement the HTTP requests for this device/system.

- You can set debug levels from 0 to 2:
```
Bot.set_debug(0); // No debug msgs
Bot.set_debug(1); // Bot debug msgs
Bot.set_debug(2); // Bot+HTTPS debug msgs
```

- Global define "UTLGBOT_NO_DEBUG" to disable build debug prints and save some flash and sram memory usage.

- Global define "UTLGBOT_MEMORY_LEVEL" with values 0 to 5, to set library build memory usage level. It allows to reduce library flash and sram memory needs by reducing HTTPS response buffer length and maximum telegram text messages length buffer. 
```
-DUTLGBOT_MEMORY_LEVEL=0 // Max TLG msgs:  128 chars
-DUTLGBOT_MEMORY_LEVEL=1 // Max TLG msgs:  256 chars
-DUTLGBOT_MEMORY_LEVEL=2 // Max TLG msgs:  512 chars
-DUTLGBOT_MEMORY_LEVEL=3 // Max TLG msgs: 1024 chars
-DUTLGBOT_MEMORY_LEVEL=4 // Max TLG msgs: 2048 chars
-DUTLGBOT_MEMORY_LEVEL=5 // Max TLG msgs: 4097 chars (telegram max msg length)
```

- Defines must be passed to compiler by flag (-DUTLGBOT_NO_DEBUG -DUTLGBOT_MEMORY_LEVEL=2). Note that define in source code won't work as expected due utlgbot.cpp is compiled independent of main.cpp and that cause different definitions of memory levels from each file compiled.
