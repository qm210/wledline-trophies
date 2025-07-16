#pragma once

// Fixed Compiler Variables / Flags

#ifndef USERMOD_DEADLINE_TROPHY
#define USERMOD_DEADLINE_TROPHY
#endif

// for the Simulator Communication
// (there is also DEADLINE_UDP_SENDER_IP but that works without default)

#ifndef DEADLINE_UDP_SENDER_PORT
#define DEADLINE_UDP_SENDER_PORT 3413
#endif

// These Limits are from the wled_cfg(2).json that Topy sent me on 2024/10/20 (via Signal)
#ifdef ABL_MILLIAMPS_DEFAULT
#undef ABL_MILLIAMPS_DEFAULT
#endif
#define ABL_MILLIAMPS_DEFAULT 1200

// Brightness should be default at max, then the current needs to be limited still
#ifndef DEADLINE_INIT_BRIGHTNESS
#define DEADLINE_INIT_BRIGHTNESS 100
#endif

// Network stuff -- mDNS might be irrelevant, as Scotty told me it isn't enabled (waste of power)

#ifndef MDNS_NAME
#define MDNS_NAME "deadline"
#endif

#ifndef WLED_AP_SSID
#define WLED_AP_SSID "DL-TROPHY-AP"
#endif

#ifndef WLED_AP_PASS
#define WLED_AP_PASS "deadline"
#endif

#ifndef WLED_OTA_PASS
#define WLED_OTA_PASS "deadlota"
#endif

// forcing  uniqueness make sense for many trophies at the same place

// #ifndef FORCE_MDNS_PREFIX_UNIQUE
// #define FORCE_MDNS_PREFIX_UNIQUE
// #endif

// #ifndef WLED_AP_SSID_UNIQUE
// #define WLED_AP_SSID_UNIQUE
// #endif

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

#ifndef WLED_ENABLE_WEBSOCKETS
#define WLED_ENABLE_WEBSOCKETS
#endif

#ifndef WLED_DEBUG
#define WLED_DEBUG
#endif

#ifndef WLED_DEBUG_NO_VERBOSE_LOGGING
#define WLED_DEBUG_NO_VERBOSE_LOGGING
#endif

#ifndef WLED_DEBUG_NO_JSON_LOCKS
#define WLED_DEBUG_NO_JSON_LOCKS
#endif

#ifndef WLED_DEBUG_NO_SLOW_WARNINGS
#define WLED_DEBUG_NO_SLOW_WARNINGS
#endif

// these are just annoying during development, and unused in the trophy anyway.
#ifdef WLED_MAX_BUTTONS
#undef WLED_MAX_BUTTONS
#endif
#define WLED_MAX_BUTTONS 0

// found this, I guess the ESP32 favours it. It is noted with, at least:
// - network: this will save us 2 bytes of RAM while increasing code by ~400 bytes
// - notifications: this will save us 8 bytes of RAM while increasing code by ~400 bytes
// - ...
#ifndef WLED_SAVE_RAM
#define WLED_SAVE_RAM
#endif