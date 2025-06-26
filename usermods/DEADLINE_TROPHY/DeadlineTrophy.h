#pragma once

#include "wled.h"

const int PIN_LOGO_DATA = 21;
const int PIN_LOGO_CLOCK = 3;
const int PIN_BASE_DATA = 19;
const int PIN_BASE_CLOCK = 18;
const int PIN_BACK_SPOT = 26;
const int PIN_FLOOR_SPOT = 27;

const int LED_TYPE_HD107S = TYPE_APA102;
const int LED_SINGLE_WHITE = TYPE_ANALOG_MIN;

namespace DeadlineTrophy {

    const int N_LEDS_LOGO = 106;
    const int N_LEDS_BASE = 64;
    const int N_LEDS_TOTAL = N_LEDS_LOGO + N_LEDS_BASE + 2;

    uint16_t maxMilliAmps(uint16_t nLeds);
    BusConfig createBus(uint8_t type, uint16_t count, uint16_t start, uint8_t pin1, uint8_t pin2 = 255);

    // Usermods usually only care about their own stuff, but:
    // "You're remembered for the rules you break" - Stockton Rush (OceanGate CEO)
    void overwriteConfig();

    const int logoW = 27;
    const int logoH = 21;
    const int baseEdge = 18; // edge length 16 + 2 of the adjacent edges

    const int mappingSize = (logoW) * (logoH + baseEdge);
    const uint16_t __ = (uint16_t)-1;
    // <-- this masks the gaps for easier debugging / reading the code below

    // this Matrix-Linear-Hybrid thing is not great, with the single LEDs shoe-horned in there, but that's what we have.
    // (the empty lines are due to the actual non-equidistant lines in the logo)
    const uint16_t mappingTable[mappingSize] = {
        __, __, __, 97, __, 98, __, 99, __,100, __,101, __,102, __,103, __,104, __,105, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, 96, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, 95, __, 94, __, 93, __, 92, __, 91, __, 90, __, 89, __, 88, __, 87, __, 86, __, 85, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        73, __, 74, __, 75, __, 76, __, 77, __, 78, __, 79, __, 80, __, 81, __, 82, __, 83, __, 84, __, __, __, __,
        __, 72, __, 71, __, 70, __, 69, __, 68, __, 67, __, 66, __, 65, __, 64, __, 63, __, 62, __, 61, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __,  0, __,  1, __,  2, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 59, __, 60, __, __,
        __, __, __,  5, __,  4, __,  3, __, __, __, __, __, __, __, __, __, __, __, __, __, 58, __, 57, __, 56, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __,  6, __,  7, __,  8, __, __, __, __, __, __, __, __, __, __, __, 53, __, 54, __, 55, __, __,
        __, __, __, __, __, 11, __, 10, __,  9, __, __, __, __, __, __, __, __, __, 52, __, 51, __, 50, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, 12, __, 13, __, 14, __, __, __, __, __, __, __, 47, __, 48, __, 49, __, __, __, __,
        __, __, __, __, __, __, __, 17, __, 16, __, 15, __, __, __, __, __, __, __, 46, __, 45, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, 18, __, 19, __, 20, __, 21, __, 22, __, 23, __, 24, __, 25, __, 26, __, __,
        __, __, __, __, __, __, __, __, __, 35, __, 34, __, 33, __, 32, __, 31, __, 30, __, 29, __, 28, __, 27, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, 36, __, 37, __, 38, __, 39, __, 40, __, 41, __, 42, __, 43, __, 44,
        __,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121, __,170, __, __, __, __, __, __, __, __,
        122, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,123,171, __, __, __, __, __, __, __, __,
        124, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,125, __, __, __, __, __, __, __, __, __,
        126, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,127, __, __, __, __, __, __, __, __, __,
        128, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,129, __, __, __, __, __, __, __, __, __,
        130, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,131, __, __, __, __, __, __, __, __, __,
        132, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,133, __, __, __, __, __, __, __, __, __,
        134, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,135, __, __, __, __, __, __, __, __, __,
        136, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,137, __, __, __, __, __, __, __, __, __,
        138, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,139, __, __, __, __, __, __, __, __, __,
        140, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,141, __, __, __, __, __, __, __, __, __,
        142, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,143, __, __, __, __, __, __, __, __, __,
        144, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,145, __, __, __, __, __, __, __, __, __,
        146, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,147, __, __, __, __, __, __, __, __, __,
        148, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,149, __, __, __, __, __, __, __, __, __,
        150, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,151, __, __, __, __, __, __, __, __, __,
        152, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,153, __, __, __, __, __, __, __, __, __,
            __,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169, __, __, __, __, __, __, __, __, __, __,
    };

    static const size_t N_SEGMENTS = 4;
    extern const char* segmentName[N_SEGMENTS];
    extern const Segment segment[N_SEGMENTS];
    extern const uint8_t segmentCapabilities[N_SEGMENTS];

}

// Fixed Compiler Variables / Flags

#ifndef USERMOD_DEADLINE_TROPHY
#define USERMOD_DEADLINE_TROPHY
#endif

#ifndef DEADLINE_MAX_AMPERE
#define DEADLINE_MAX_AMPERE 1200
#endif

// These Limits are from the wled_cfg(2).json that Topy sent me on 2024/10/20 (via Signal)
#ifdef ABL_MILLIAMPS_DEFAULT
#undef ABL_MILLIAMPS_DEFAULT
#endif
#define ABL_MILLIAMPS_DEFAULT 1200

#ifdef LED_MILLIAMPS_DEFAULT
#undef LED_MILLIAMPS_DEFAULT
#endif
#define LED_MILLIAMPS_DEFAULT 255

// Brightness should be default at max, then the current needs to be limited still
#ifndef DEADLINE_INIT_BRIGHTNESS
#define DEADLINE_INIT_BRIGHTNESS 100
#endif

// Network stuff -- mDNS might be irrelevant, as Scotty told me it isn't enabled (waste of power)

#ifndef MDNS_NAME
#define MDNS_NAME "deadline"
#endif

// forcing  uniqueness make sense for many trophies at the same place

#ifndef FORCE_MDNS_PREFIX_UNIQUE
#define FORCE_MDNS_PREFIX_UNIQUE
#endif

#ifndef WLED_AP_SSID_UNIQUE
#define WLED_AP_SSID_UNIQUE
#endif

// Audioreactive Usermod (Sound-To-Light)
#ifndef UM_AUDIOREACTIVE_ENABLE
#define UM_AUDIOREACTIVE_ENABLE
#endif

#ifdef I2S_SDPIN
#undef I2S_SDPIN
#endif
#define I2S_SDPIN 14

#ifdef I2S_WSPIN
#undef I2S_WSPIN
#endif
#define I2S_WSPIN 13

#ifdef I2S_CKPIN
#undef I2S_CKPIN
#endif
#define I2S_CKPIN 12

// this includes my_config.h for customized use (WiFi access etc.)
#ifndef WLED_USE_MY_CONFIG
#define WLED_USE_MY_CONFIG
#endif

// and some helpers for developing / performance

#ifndef WLED_DEBUG
#define WLED_DEBUG
#endif

#ifndef WLED_DEBUG_NO_VERBOSE_LOGGING
#define WLED_DEBUG_NO_VERBOSE_LOGGING
#endif

#ifndef WLED_DEBUG_NO_SLOW_WARNINGS
#define WLED_DEBUG_NO_SLOW_WARNINGS
#endif

// these are just annoying during development, and unused in the trophy anyway.
#ifdef WLED_MAX_BUTTONS
#undef WLED_MAX_BUTTONS
#endif
#define WLED_MAX_BUTTONS 0
