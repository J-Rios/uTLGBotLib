# uTLGBotLib
Universal Telegram Bot library for Arduino, ESP-IDF and Native (Windows and Linux) devices, that let you create Telegram Bots. You can use it with ESP8266 and ESP32 microcontrollers.

Micro Telegram Bot Library is a lightweight C++ library implementation that use Telegram Bot API to create Bots. The library goal is to be compatible with multiples devices, from embedded microcontrollers, to Servers and PCs (Windows and Linux). Inside microcontrollers world, uTLGBotLib focus on Espressif ESP32 (support for ESP-IDF and Arduino frameworks) and ESP8266 (support for Arduino) microcontrollers.

## Notes

- Due library target embedded devices such microcontrollers, it MUST be take a lot of care in memory usage, from library size to safety use.

- To avoid ram memory fragmentation and stack-heap collisions, the library doesn't use dynamic memory in non Native (Windows/Linux) platforms (ESP8266 and ESP32).

- The library uses [multihttpsclient](https://github.com/J-Rios/multihttpsclient) to implement all low level HTTP request specific for each device/system.

- The library uses [jsmn library](https://github.com/zserge/jsmn) to parse JSON text in the safest (memory) way possible, because it just get a string and return the indexes where each json element ("token") start and end.

- Sub-library multihttpsclient uses [mbedtls library](https://github.com/ARMmbed/mbedtls) to handle HTTPS requests in Native (Windows and Linux) systems.

- uTLGBotLib is a generic library, for that reason, to add support of a new device/system, you just need to specify the expected print() macros in utlgbotlib.cpp and create specific files in multihttpsclient library to implement the HTTP requests for this device/system.
