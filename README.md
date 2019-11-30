# uTLGBotLib
Micro Telegram Bot Library is a lightweight C++ library implementation that use Telegram Bot API to create Bots. The library goal is to be compatible with multiples devices, from embedded microcontrollers, to Servers and PCs (Windows and Linux). Inside microcontrollers world, uTLGBotLib focus on Espressif ESP32 (support for ESP-IDF and Arduino frameworks) and ESP8266 (support for Arduino) microcontrollers.

## Notes

- Library developed using Visual Studo Code and Platformio project.

- To build for ESP32 with ESPIDF or Arduino, uncomment "env:esp32_espidf" or "env:esp32_arduino" section of platformio.ini (comment other sections). Then use Platformio integrated buttons for compile and flash.

- For ESPIDF build, you must provide -DESP_IDF flag (see platformio.init file).

- To build for ESP8266 with Arduino, uncomment "env:esp8266_arduino" section of platformio.ini (comment other sections). Then use Platformio integrated buttons for compile and flash.

- To build for generic (Native) devices for Windows or Linux, uncomment "env:windows" or "env:linux" section of platformio.ini (comment other sections). Then use "platformio run --target clean", "platformio run" and ".pioenvs/native/program" to clean, build and run, respectively.

- For Windows build, you need an installed mingw environment in your system and ".../mingw/bin" setup in system PATH environment variable.

- Due library target embedded devices such microcontrollers, it MUST be take a lot of care in memory usage, from library size to safety use.

- To avoid ram memory fragmentation and stack-heap collisions, the library doesn't use dynamic memory in non Native (Windows/Linux) platforms (ESP32).

- The library uses [jsmn library](https://github.com/zserge/jsmn) to parse JSON text in the safest (memory) way possible, because it just get a string and return the indexes where each json element ("token") start and end.

- The library uses [mbedtls library](https://github.com/ARMmbed/mbedtls) to handle HTTPS requests in Native (Windows and Linux) systems.

- The jsmn and mbedtls libraries are included as git submodules, so remember that to clone full repository including it, you should go:
```
git clone --recurse-submodules https://github.com/J-Rios/uTLGBotLib
```
