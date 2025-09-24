#pragma once

#include "FX.h"
#include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"

 ///////////////////////////////////////
 // DEV INFO: cf. DEV INFO in FX.cpp! //
 ///////////////////////////////////////

extern uint16_t mode_static(void);

 //////////////////////////
 //  Deadline Trophy FX  //
 //////////////////////////

const int DEBUG_LOG_EVERY_N_CALLS = 1000; // for printing debug output every ... steps (0 = no debug out)

#define IS_DEBUG_STEP (DEBUG_LOG_EVERY_N_CALLS > 0 && (SEGENV.call % DEBUG_LOG_EVERY_N_CALLS) == 0)

using DeadlineTrophy::logoW;
using DeadlineTrophy::logoH;
using DeadlineTrophy::baseSize;

// could do: Umrechnungsfunktionen in DeadlineTrophy.h schieben.
inline void setPixel(size_t segmentIndex, int x, int y, uint32_t color) {
    if (strip.getCurrSegmentId() != segmentIndex) {
        return;
    }
    color = color & 0x00FFFFFF; // <-- again: remove white. we don't have.
    SEGMENT.setPixelColorXY(x, y, color);
}

void setBase(size_t x, size_t y, uint32_t color) {
    if (x >= baseSize || y >= baseSize) {
        return;
    }
    setPixel(0, x, y, color);
}

void setLogo(size_t x, size_t y, uint32_t color) {
    if (x >= logoW || y >= logoH) {
        return;
    }
    setPixel(1, x, y, color);
}

void setBack(uint32_t color) {
    setPixel(2, baseSize, logoH, color);
}

void setFloor(uint32_t color) {
    setPixel(3, baseSize, logoH + 1, color);
}

void setBaseHSV(size_t x, size_t y, CHSV color) { setBase(x, y, uint32_t(CRGB(color))); }
void setLogoHSV(size_t x, size_t y, CHSV color) { setLogo(x, y, uint32_t(CRGB(color))); }

#define IS_BASE_SEGMENT (strip.getCurrSegmentId() == 0)
#define IS_LOGO_SEGMENT (strip.getCurrSegmentId() == 1)

const int baseX0[4] = {17, 16, 0, 1};
const int baseY0[4] = {1, 17, 16, 0};
const int baseDX[4] = {0, -1, 0, +1};
const int baseDY[4] = {+1, 0, -1, 0};

uint32_t float_hsv(float hue, float sat, float val) {
    // parameters in [0, 255] but as float
    uint32_t color = uint32_t(CRGB(CHSV(
        static_cast<uint8_t>(hue),
        static_cast<uint8_t>(sat),
        static_cast<uint8_t>(val)
    )));
    // remove the specific "white" that might be in the color (or shouldn't we?)
    return color & 0x00FFFFFF;
}

uint8_t mix8(uint8_t a, uint8_t b, float t) {
    return a + static_cast<uint8_t>(static_cast<float>(b - a) * t);
}

// do some measurement of efficiency
int64_t t0, t1;

uint16_t mode_DeadlineTrophy(void) {
    size_t i, x, y;
    uint32_t col = SEGCOLOR(0);
    CHSV color = rgb2hsv_approximate(CRGB(col));
    CHSV color_(color);

    bool measurePerformance = SEGENV.call % 9999 == 0;

    if (SEGENV.call == 0) {
        DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);
    }

    if (IS_DEBUG_STEP) {
        DEBUG_PRINTF("[QM_DEBUG] Segment %d, color %d (rgb %d, %d, %d, hsv %d, %d, %d)\n",
            strip.getCurrSegmentId(), col, R(col), G(col), B(col), color.hue, color.sat, color.val
        );
    }

    float bpm = static_cast<float>(SEGMENT.speed);
    float beat = bpm/60. * static_cast<float>(strip.now) / 1000.;

    if (measurePerformance) {
        t0 = esp_timer_get_time();
    }

    if (IS_LOGO_SEGMENT) {
        float width = exp2f((static_cast<float>(SEGMENT.intensity) - 192.) / 64.);
        float centerX = (static_cast<float>(SEGMENT.custom1) - 128.) / 256.;
        float centerY = (static_cast<float>(SEGMENT.custom2) - 128.) / 256.;

        if (IS_DEBUG_STEP) {
            DEBUG_PRINTF("[QM_DEBUG_LOGO] params %d %d %d %d -> w=%.3f, x=%.3f, y=%.3f\n",
                SEGMENT.speed, SEGMENT.intensity, SEGMENT.custom1, SEGMENT.custom2,
                width, centerX, centerY
            );
        }

        for (const auto& coord : DeadlineTrophy::logoCoordinates()) {
            float dist_x = (coord.uvX - centerX) / width;
            float dist_y = (coord.uvY - centerY) / width;
            float intensity = exp(-(dist_x*dist_x + dist_y*dist_y));

            color.hue = color_.hue - 90. * (1. - intensity);
            color.sat = color_.sat;
            color.val = static_cast<uint8_t>(255.f * intensity);

            if (IS_DEBUG_STEP) {
                DEBUG_PRINTF("[QM_DEBUG_LOGO_COORD] %.3f/%.3f = %d/%d -- %.3f x %.3f -> %.3f [h=%d, s=%d, v=%d]\n",
                    coord.uvX, coord.uvY, coord.x, coord.y, dist_x, dist_y, intensity, color.hue, color.sat, color.val
                );
            }

            float d = coord.sdLine(-0.5, -0.5, +0.5, +0.5);
            intensity = exp(-5.*d);
            color.sat = mix8(color.sat, 0, intensity);
            color.val = mix8(color.val, 255, intensity);

            setLogoHSV(coord.x, coord.y, color);
        }

        if (measurePerformance) {
            t1 = esp_timer_get_time();
            DEBUG_PRINTF("[QM_DEBUG_PERFORMANCE] Logo: took %" PRId64 "µs\n", t1-t0);
            t0 = esp_timer_get_time();
        }
    }

    if (IS_BASE_SEGMENT) {
        for (int s = 0; s < 4; s++)
        for (i = 0; i < 16; i++) {
            x = baseX0[s] + baseDX[s] * i;
            y = baseY0[s] + baseDY[s] * i;

            // strip.now is millisec uint32_t, so this will overflow ~ every 49 days. who shits a give.
            float wave = sin_t(PI / 15. * (static_cast<float>(i) - 0.007 * strip.now));
            float abs_wave = (wave > 0. ? wave : -wave);

            color.hue = color_.hue - 20. * wave * abs_wave;

            color.hue = s == 0 ? 0 : s == 1 ? 90 : s == 2 ? 180 : 210;

            if (i == 0) {
                // first is always white
                color.sat = 0;
            } else {
                color.sat = 100;
                // try something with the "speed" parameter as BPM:
                float shape = exp(-2.10 * fmodf(beat, 1.));
                color.sat = 200 - static_cast<int>(200. * shape);
            }

            color.val = ((strip.now % 16000) > (i * 1000)) ? 255 : 20;

            setBaseHSV(x, y, color);
        }

        if (measurePerformance) {
            t1 = esp_timer_get_time();
            DEBUG_PRINTF("[QM_DEBUG_PERFORMANCE] Base: took %" PRId64 "µs\n", t1-t0);
        }
    }

    auto stepTime = fmod_t(strip.now, 2.0);
    setBack(stepTime < 1.0 ? WHITE : DARKSLATEGRAY);
    setFloor(stepTime < 1.0 ? DARKSLATEGRAY : WHITE);

    return FRAMETIME;
} // mode_DeadlineTrophy


static const char _data_FX_MODE_DEADLINE_TROPHY[] PROGMEM =
    "DEADLINE TROPHY@BPM,param1,param2,param3;!,!;!;2;sx=110";
// cf. https://kno.wled.ge/interfaces/json-api/#effect-metadata
// <Effect parameters>;<Colors>;<Palette>;<Flags>;<Defaults>
// <Colors> = !,! = Two Defaults
// <Palette> = ! = Default enabled (Empty would disable palette selection)
// <Flags> = 2 for "it needs the 2D matrix", add "v" and/or "f" for AudioReactive volume and frequency.
// <Defaults>: <ParameterCode>=<Value>
//             si=0 is "sound interaction" (for AudioReactive),
