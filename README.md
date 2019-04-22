# uTLGBotLib
Micro Telegram Bot Library is a lightweight C++ library implementation that use Telegram Bot API and allow create Bot logic. Library goal is to be compatible with multiples devices, from embedded microcontrollers, to servers and PCs (WIP). Inside microcontrollers world, the library focus Espressif ESP32 microcontroller using ESP-IDF SDK, however, Arduino support is also available.

## Notes

- Library developed using Visual Studo Code and Platformio project.

- To build for ESP32 for ESPIDF or Arduino, uncomment "env:esp32dev" section of platformio.ini (comment other sections) and set framework attributte (framework = arduino or framework = espidf). Then use Platformio integrated buttons for compile and flash.

- To build for generic (native) devices for Windows or Linux, uncomment "env:native" section of platformio.ini (comment other sections). Then use "platformio run --target clean", "platformio run" and ".pioenvs/native/program" to clean, build and run, respectively.

- Due library target embedded devices such microcontrollers, it MUST be take a lot of care in memory usage, from library size to safety use.

- To avoid ram memory fragmentation and stack-heap collisions, the library doesn't use dynamic memory.

- The library uses [jsmn library](https://github.com/zserge/jsmn) to parse JSON text in the safest (memory) way possible, because it just get a string and return the indexes where each json element ("token") start and end.

- The jsmn library is included as git submodule, so remember that to clone full repository including it, you should go:
```
git clone --recurse-submodules https://github.com/J-Rios/uTLGBotLib
```
