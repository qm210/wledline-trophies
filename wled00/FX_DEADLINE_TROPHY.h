#pragma once

#include "FX.h"
#include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"
using namespace DeadlineTrophy::FxHelpers;
using DeadlineTrophy::Vec2;
extern uint16_t mode_static(void);

 //////////////////////////
 //  Deadline Trophy FX  //
 //////////////////////////
 // There's a lot of useful stuff in
 // FX.cpp -> see the DEV INFO, also, other FX for "inspiration" :)
 // wled_math.cpp
 // util.cpp
 // -- you can also reinvent the wheel, but don't always have to.
 //////////////////////////

// to do some measurement of efficiency
int64_t t0, t1;

uint16_t mode_DeadlineTrophy(void) {
    uint32_t col = SEGCOLOR(0);
    float bpm = static_cast<float>(SEGMENT.speed);
    CHSV color = rgb2hsv_approximate(CRGB(col));
    CHSV color_(color);

    bool measurePerformance = SEGENV.call % 999 == 0;

    if (SEGENV.call == 0) {
        DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);
    }

    if (IS_DEBUG_STEP) {
        DEBUG_PRINTF("[QM_DEBUG] Segment %d, color %d (rgb %d, %d, %d, hsv %d, %d, %d)\n",
            strip.getCurrSegmentId(), col, R(col), G(col), B(col), color.hue, color.sat, color.val
        );
    }

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
            float dist_x = (coord.uv.x - centerX) / width;
            float dist_y = (coord.uv.y - centerY) / width;
            float intensity = exp(-(dist_x*dist_x + dist_y*dist_y));

            color.hue = color_.hue - 90. * (1. - intensity);
            color.sat = color_.sat;
            color.val = static_cast<uint8_t>(255.f * intensity);

            // if (IS_DEBUG_STEP) {
            //     DEBUG_PRINTF("[QM_DEBUG_LOGO_COORD] %.3f/%.3f = %d/%d -- %.3f x %.3f -> %.3f [h=%d, s=%d, v=%d]\n",
            //         coord.uv.x, coord.uv.y, coord.x, coord.y, dist_x, dist_y, intensity, color.hue, color.sat, color.val
            //     );
            // }

            float d = coord.sdLine(-0.4, -0.5, +0.1, +0.5);
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
        // "state variables"
        static float phi = 0.;
        static float omega = 1.7;
        float beat = beatNow(bpm);
        int wholeBeat = static_cast<int>(beat);
        float fractBeat = fmodf(beat, 1.);
        float hue = static_cast<float>(color.hue);
        for (const auto& coord : DeadlineTrophy::baseCoordinates()) {
            // wandering light with the bpm
            int whiteIndex = wholeBeat % 16;
            color.sat = (coord.indexInSide() == whiteIndex) ? 0 : 255;

            // and some hue dancing with the beat
            color.hue = static_cast<uint8_t>(hue + 30. * fractBeat);

            phi = 0.25 * beat * omega;
            Vec2 corner{1, 1};
            Vec2 end = corner.rotated(phi);
            float d = coord.sdLine(-end.x, -end.y, end.x, end.y);
            color.val = mix8(255, 100, exp(-d*3.2));

            // if (IS_DEBUG_STEP) {
            //     DEBUG_PRINTF("[QM_DEBUG_BASE] coord %d (%d,%d) (%.3f,%.3f) [%d,%d] = H%d|S%d|V%d\n",
            //         coord.index, coord.x, coord.y, coord.uv.x, coord.uv.y,
            //         s, i, color.hue, color.sat, color.val
            //     );
            // }

            setBaseHSV(coord.x, coord.y, color);
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
// <EffectParameters>;<Colors>;<Palette>;<Flags>;<Defaults>
// <Colors> = !,! = Two Defaults
// <Palette> = ! = Default enabled (Empty would disable palette selection)
// <Flags> = 2 for "it needs the 2D matrix", add "v" and/or "f" for AudioReactive volume and frequency.
// <Defaults>: <ParameterCode>=<Value>
//             si=0 is "sound interaction" (for AudioReactive),
// <EffectParameters>: These are then read by (if defined, comma-separated)
//   sx = SEGMENT.speed (int 0-255)
//   ix = SEGMENT.intensity (int 0-255)
//   c1 = SEGMENT.custom1 (int 0-255)
//   c2 = SEGMENT.custom2 (int 0-255)
//   c3 = SEGMENT.custom3 (int 0-31)
//   o1 = SEGMENT.check1 (bool)
//   o2 = SEGMENT.check2 (bool)
//   o3 = SEGMENT.check3 (bool)
