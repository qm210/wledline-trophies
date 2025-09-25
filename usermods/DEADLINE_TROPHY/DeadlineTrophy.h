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
    // "You're remembered for the rules you break" - Stockton Rush
    void overwriteConfig();

    static const size_t N_SEGMENTS = 4;
    extern const char* segmentName[N_SEGMENTS];
    extern const Segment segment[N_SEGMENTS];
    extern const uint8_t segmentCapabilities[N_SEGMENTS];

    const int logoW = 26;
    const int logoH = 12;
    const int baseSize = 18;
    const int mappingSize = (logoW) * (logoH + baseSize);
    const uint16_t __ = (uint16_t)-1;
    const uint16_t mappingTable[mappingSize] = {
        __, __, __, __, __, __, __, __,169, __, __,149, __,148,125, __,124,120, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __,168, __, __,150, __,147,126, __,123,121, __,119, __, __, __, __, __, __, __,
        __, __, __, __, __, __,167, __, __,151, __,146,127, __, __,122, __,118,114, __, __, __, __, __, __, __,
        __, __, __, __, __,166, __, __,152, __,145,128, __, __, __, __,117,115, __,113, __, __, 90, 91, __,108,
        __, __, __, __,165, __, __,153, __,144,129, __, __, __, __, __,116, __,112,109, __, 89, 92, __,107, __,
        __, __, __,164, __, __,154, __,143,130, __, __, __, __, __, __, __,111,110, __, 88, 93, __,106, __, __,
        __, __,163, __, __,155, __,142,131, __, __, __, __, __, __, __, __, __, __, 87, 94, __,105, __, __, __,
        __,162, __, __,156, __,141,132, __, __, __, __, __, __, __, __, __, __, 86, 95, __,104, __, __, __, __,
       161, __, __,157, __,140,133, __, __, __, __, __, __, __, __, __, __, 85, 96, __,103, __, __, __, __, __,
        __,160,158, __,139,134, __, 66, 67, __, 72, 73, __, 78, 79, __, 84, 97, __,102, __, __, __, __, __, __,
        __,159, __,138,135, __, 65, 68, __, 71, 74, __, 77, 80, __, 83, 98, __,101, __, __, __, __, __, __, __,
        __, __,137,136, __, 64, 69, __, 70, 75, __, 76, 81, __, 82, 99, __,100, __, __, __, __, __, __, __, __,
        __, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, __,170, __, __, __, __, __, __, __,
        47, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  0,171, __, __, __, __, __, __, __,
        46, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  1, __, __, __, __, __, __, __, __,
        45, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  2, __, __, __, __, __, __, __, __,
        44, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  3, __, __, __, __, __, __, __, __,
        43, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  4, __, __, __, __, __, __, __, __,
        42, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  5, __, __, __, __, __, __, __, __,
        41, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  6, __, __, __, __, __, __, __, __,
        40, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  7, __, __, __, __, __, __, __, __,
        39, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  8, __, __, __, __, __, __, __, __,
        38, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,  9, __, __, __, __, __, __, __, __,
        37, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 10, __, __, __, __, __, __, __, __,
        36, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 11, __, __, __, __, __, __, __, __,
        35, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 12, __, __, __, __, __, __, __, __,
        34, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 13, __, __, __, __, __, __, __, __,
        33, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 14, __, __, __, __, __, __, __, __,
        32, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 15, __, __, __, __, __, __, __, __,
        __, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, __, __, __, __, __, __, __, __, __,
    };

    struct Vec2 {
        float x;
        float y;

        Vec2 operator-(const Vec2& other) const;
        Vec2 operator+(const Vec2& other) const;
        Vec2 operator*(float factor) const;
        static float dot(const Vec2& a, const Vec2 b);
        float length() const;
        Vec2& shift(float dx, float dy);
        Vec2& rotate(float phi);
        Vec2& scale(float factorX, float factorY);
        float distance(const Vec2& other) const;

        static Vec2 fromParameters(uint8_t x, uint8_t y);
    };

    struct Coord {
        // indices for matrix
        uint8_t x;
        uint8_t y;
        // normalized coordinates
        Vec2 uv;
        // for reference, pixel index within its segment
        int index;

        float sdLine(float x1, float y1, float x2, float y2) const;
        float sdLine(Vec2 p1, Vec2 p2) const;

        // helpers for the base (don't make any sense for the Logo)
        // side 0=right, 1=front, 2=left, 3=back
        inline int indexOfSide() const { return index / 16; }
        inline int indexInSide() const { return index % 16; }
    };

    std::array<Coord, N_LEDS_LOGO>& logoCoordinates();
    std::array<Coord, N_LEDS_BASE>& baseCoordinates();

    namespace FxHelpers {

        inline void setPixel(size_t segmentIndex, int x, int y, uint32_t color) {
            // QM TO-MAYBE-DO: check whether it is worth removing these sanity checks for speed
            if (strip.getCurrSegmentId() != segmentIndex) {
                return;
            }
            color = color & 0x00FFFFFF; // <-- remove white. Trophy has that not.
            SEGMENT.setPixelColorXY(x, y, color);
        }

        inline void setBase(size_t x, size_t y, uint32_t color) {
            if (x >= baseSize || y >= baseSize) {
                return;
            }
            setPixel(0, x, y, color);
        }

        inline void setLogo(size_t x, size_t y, uint32_t color) {
            if (x >= logoW || y >= logoH) {
                return;
            }
            setPixel(1, x, y, color);
        }

        inline void setSingle(uint8_t white) {
            uint32_t color = RGBW32(white, white, white, white);
            SEGMENT.setPixelColorXY(0, 0, color);
        }

        inline void setBaseHSV(size_t x, size_t y, CHSV color) { setBase(x, y, uint32_t(CRGB(color))); }
        inline void setLogoHSV(size_t x, size_t y, CHSV color) { setLogo(x, y, uint32_t(CRGB(color))); }

        inline float secondNow() {
            return static_cast<float>(strip.now) * 1e-3;
        }

        inline float beatNow(float bpm) {
            return bpm/60. * secondNow();
        }

        inline float clip(float x) {
            return constrain(x, 0., 1.);
        }

        uint32_t floatHSV(float hue, float sat, float val);

        CRGB paletteRGB(float t,
            float aR, float aG, float aB, float bR, float bG, float bB,
            float cR, float cG, float cB, float dR, float dG, float dB
        ); // <-- usually given as four vec3, but we don't have these yet

        uint8_t mix8(uint8_t a, uint8_t b, float t);

        float invSqrt(float x);
    }

    namespace LogoBars {

        // helpers for larger bars of the logo
        const size_t nOuterLeft = 10;
        const size_t nLeft = 35;
        const size_t nBottom = 36;
        const size_t nUpperRight = 17;
        const size_t nOuterRight = 27;

        // the bars in the Logo
        const int OuterLeft[nOuterLeft] = {
            161, 162, 163, 164, 165, 166, 167, 168, 169,
            160
        };
        const int Left[nLeft] = {
            156, 155, 154, 153, 152, 151, 150, 149,
            140, 141, 142, 143, 144, 145, 146, 147, 148,
            133, 132, 131, 130, 129, 128, 127, 126
        };
        const int Bottom[nBottom] = {
            137, 136, 64, 69, 79, 70, 75, 76, 81, 82, 99, 100,
            159, 138, 135, 65, 68, 71, 74, 77, 80, 83, 98, 101,
            158, 139, 134, 66, 67, 72, 73, 78, 79, 84, 97, 102
        };
        const int UpperRight[nUpperRight] = {
            111, 116, 117, 122, 123, 125,
            110, 112, 115, 118, 121, 124,
            109, 113, 114, 119, 120
        };
        const int OuterRight[nOuterRight] = {
            85, 86, 87, 88, 89, 90,
            96, 95, 94, 93, 92, 91,
            103, 104, 105, 106, 107, 108
        };

        const size_t nBars = 5;
        const int nInBar[] = {
            nOuterLeft,
            nLeft,
            nBottom,
            nUpperRight,
            nOuterRight
        };
    }

}

#define IS_BASE_SEGMENT (strip.getCurrSegmentId() == 0)
#define IS_LOGO_SEGMENT (strip.getCurrSegmentId() == 1)
#define IS_BACK_LED     (strip.getCurrSegmentId() == 2)
#define IS_FLOOR_LED    (strip.getCurrSegmentId() == 3)

#define EVERY_NTH_STEP(x) if (SEGENV.call % x == 0)
