/**************************************************************************************************/
// Project: uTLGBotLib
// File: main.cpp
// Description: Project main file
// Created on: 19 mar. 2019
// Last modified date: 19 mar. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Multiples Devices Build Support */

#if defined(ARDUINO) // ESP32 Arduino Framework
    #include "main_arduino.h"
#elif defined(IDF_VER) // ESP32 ESPIDF Framework
    #include "main_esp32.h"
#else // Generic devices (intel, amd, arm) and OS (windows, Linux)
    #include "main_generic.h"
#endif
