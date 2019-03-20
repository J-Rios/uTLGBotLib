/**************************************************************************************************/
// Project: uTLGBotLib
// File: main.cpp
// Description: Project main file
// Created on: 19 mar. 2019
// Last modified date: 19 mar. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Multiples Devices Build Support */

#ifdef ARDUINO
    #include "main_arduino.h"
#else
    #include "main_esp32.h"
#endif
