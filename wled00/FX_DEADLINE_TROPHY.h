#pragma once

#include "FX.h"
#include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"
using namespace DeadlineTrophy::FxHelpers;
using DeadlineTrophy::Vec2;
extern uint16_t mode_static(void);

//////////////////////////////////////////////////////////
//  Deadline Trophy FX
//////////////////////////////////////////////////////////
// There's a lot of useful stuff in
// FX.cpp -> see the DEV INFO, also, other FX for "inspiration" :)
// wled_math.cpp
// util.cpp
// -- you can also reinvent the wheel.
//////////////////////////////////////////////////////////
//
// Notes:
// - the if (...SEGMENT) { ... } is due to "this FX will be evaluated for every segment on its own"
//   remember, that you also have to set it for each segment to be actually active.
// - the EVERY_NTH_STEP(n) { ... } helper is great for debugging.

uint16_t mode_DeadlineTrophy(void) {
    uint32_t col = SEGCOLOR(0);
    CHSV color = rgb2hsv_approximate(CRGB(col));
    CHSV color_(color);

    float bpm = static_cast<float>(SEGMENT.speed);
    float beat = beatNow(bpm);
    int wholeBeat = static_cast<int>(beat);
    float fractBeat = fmodf(beat, 1.);
    float time = secondNow();

    if (SEGENV.call == 0) {
        // sanity check
        DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);
    }

    if (IS_LOGO_SEGMENT) {
        float width = exp2f((static_cast<float>(SEGMENT.intensity) - 192.) / 64.);
        float centerX = (static_cast<float>(SEGMENT.custom1) - 128.) / 256.;
        float centerY = (static_cast<float>(SEGMENT.custom2) - 128.) / 256.;

        for (const auto& coord : DeadlineTrophy::logoCoordinates()) {
            float dist_x = (coord.uv.x - centerX) / width;
            float dist_y = (coord.uv.y - centerY) / width;
            float intensity = exp(-(dist_x*dist_x + dist_y*dist_y));

            // NOTE: as this is a loop, we need to set color.hue, color.sat, color.val as absolute at first,
            //       from then on, relative changes are ok, but otherwise we mix different pixels; seldom useful.
            color.hue = color_.hue - 90. * (1. - intensity);
            color.sat = color_.sat;
            color.val = static_cast<uint8_t>(255.f * intensity);

            float every16Beats = fmodf(beat, 16.) / 16.;
            float d = coord.sdLine(-0.4, -0.5, +0.1, +0.5);
            intensity = exp(-5.*d);
            intensity *= exp(-10. * pow(every16Beats - 15., 2.)); // is a swell at each 15th of 16 beats
            color.sat = mix8(color.sat, 0, intensity);
            color.val = mix8(color.val, 255, intensity);

            // purple flash because hue 210 is so nicey, for reasons
            float norm = coord.uv
                .shifted(0.02, 0.02)
                .scaled(1, 0.4)
                .length();
                // <-- für Kreis-nicht-Ellipse um Zentrum des Dreiecks (nicht exakt Zentrum der LEDs bisher)
            d = sin_approx(4.*norm + time);
            float d2 = clip(abs(d));
            color.hue = mix8(color.hue, 210, d2);

            // EVERY_NTH_STEP(500) {
            //     DEBUG_PRINTF("[QM_DEBUG_TIME] %.2f %.2f |%.3f| - %.3f - %.3f - %.3f -> hue = %d\n",
            //         coord.uv.x, coord.uv.y, norm, time, d, d2, color.hue
            //     );
            // }

            setLogoHSV(coord.x, coord.y, color);
        }
    }

    if (IS_BASE_SEGMENT) {
        // "state variables" -> static
        static float phi = 0.;
        static float omega = 1.7;
        float hue = static_cast<float>(color.hue);
        for (const auto& coord : DeadlineTrophy::baseCoordinates()) {
            // wandering light with the bpm
            int whiteIndex = wholeBeat % 16;
            color.sat = (coord.indexInSide() == whiteIndex) ? 0 : 255;

            // and some hue dancing with the beat
            color.hue = static_cast<uint8_t>(hue - 50. * fractBeat);

            phi = 0.25 * beat * omega;
            Vec2 corner{1, 1};
            Vec2 end = corner.rotated(phi);
            float d = coord.sdLine(-end.x, -end.y, end.x, end.y);
            color.val = mix8(255, 100, exp(-2.*d));

            setBaseHSV(coord.x, coord.y, color);
        }

    }

    uint8_t whiteBeat = beatsin8_t(SEGMENT.speed, 0, 255);
    uint8_t annoyingBlink = fractBeat < 0.25 ? 255 : 0;

    EVERY_NTH_STEP(62) {
        int segId = strip.getCurrSegmentId();
        DEBUG_PRINTF("[QM_DEBUG_WHITE %d] %.3f / %.3f / %.3f -> white %d, annoying %d, color %d, pixelY %d\n",
            segId, time, beat, fractBeat, whiteBeat, annoyingBlink,
            RGBW32(whiteBeat, whiteBeat, whiteBeat, whiteBeat), DeadlineTrophy::logoH + segId - 2
        );
    }

    if (IS_BACK_LED) {
        setSingle(annoyingBlink);

    } else if (IS_FLOOR_LED) {
        setSingle(255);

    }

    return FRAMETIME;
}

static const char _data_FX_MODE_DEADLINE_TROPHY[] PROGMEM =
    "DEADLINE TROPHY@BPM,param1,param2,param3;!,!;!;2;sx=110";
// <-- cf. https://kno.wled.ge/interfaces/json-api/#effect-metadata
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
