/**************************************************************************************************/
// File: multihttpsclient.h
// Description: Basic Multiplatform HTTPS Client (Implement network HALs for differents devices).
// Created on: 04 may. 2019
// Last modified date: 11 may. 2019
// Version: 0.0.1
/**************************************************************************************************/

/* Include Guard */

#ifndef MULTIHTTPSCLIENT_H_
#define MULTIHTTPSCLIENT_H_

/**************************************************************************************************/

/* Check Build System */

#if !defined(ARDUINO) && !defined(ESP_IDF) && !defined(WIN32) && !defined(_WIN32) && \
!defined(__linux__)
    #error Unsupported system (Supported: Windows, Linux and ESP32)
#endif

/**************************************************************************************************/

/* Libraries Configurations */

// Integer types macros
//#define __STDC_LIMIT_MACROS // Could be needed for C++, and it must be before inttypes include
//#define __STDC_CONSTANT_MACROS // Could be needed for C++, and it must be before inttypes include
#define __STDC_FORMAT_MACROS  // Could be needed for C++, and it must be before inttypes include

/**************************************************************************************************/

/* Use Specific HAL for build system */

#if defined(ARDUINO)
    #include "multihttpsclient_hals/arduino/multihttpsclient_arduino.h"
#elif defined(ESP_IDF)
    #include "multihttpsclient_hals/espidf/multihttpsclient_espidf.h"
#else
    #include "multihttpsclient_hals/generic/multihttpsclient_generic.h"
#endif

/**************************************************************************************************/

#endif
