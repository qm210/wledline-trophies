#pragma once

#include "FX.h"
#include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"

extern uint16_t mode_static(void);

//////////////////////////////////////////////////////////
//  Deadline Trophy FX
//////////////////////////////////////////////////////////
// There's a lot of useful stuff in
// FX.cpp -> see the DEV INFO, also, other FX for "inspiration" :)
// wled_math.cpp
// util.cpp
// -- you can also reinvent the wheel.
// I have never tested how efficient we really _HAVE_ to be,
// thus all the helper functions. ..qm
//////////////////////////////////////////////////////////
//
// Notes:
// - the if (...SEGMENT) { ... } is due to "this FX will be evaluated for every segment on its own"
//   remember, that you also have to set it for each segment to be actually active.
// - the EVERY_NTH_CALL(n) { ... } helper is great for debugging.

uint16_t mode_DeadlineTrophy(void) {
    using namespace DeadlineTrophy;
    using namespace DeadlineTrophy::FxHelpers;

    CHSV color = rgb2hsv_approximate(CRGB(SEGCOLOR(0)));
    CHSV color_(color);

    float bpm = static_cast<float>(SEGMENT.speed);
    float beat = beatNow(bpm);
    int wholeBeat = static_cast<int>(beat);
    float fractBeat = fmodf(beat, 1.);
    float time = secondNow();

    if (SEGENV.call == 0) {
        // DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);
        // we use SEGENV.aux0 as a internal store for the random-flashes, but for no particular reason - stop me if you care
        SEGMENT.aux0 = 0;
    }

    // Idea is, that every 4 beats random numbers are drawn, then maybe some flashing effect shown.
    // (this demonstrates random numbers and how to use the palette).
    static float randomTakenAtBeat = 0.;
    CRGBW lastRandomColor;
    if (beat > randomTakenAtBeat + 4.) {
        SEGMENT.aux0 = hw_random8();
        randomTakenAtBeat = floor_t(beat);
        lastRandomColor = ColorFromPalette(SEGPALETTE, hw_random8(), 255, LINEARBLEND_NOWRAP);
    }

    // just example for using measureMicros(), it's the same function for start and end.
    measureMicros();

    if (IS_LOGO_SEGMENT) {
        // QM: that with is useful for a point with exp(-r*r/w/w), values go from [1/8..1]
        // float width = exp2f((static_cast<float>(SEGMENT.intensity) - 192.) / 64.);
        // mapping 0..255 to [1/4 .. 4] with centered 128 = 1:
        float k = exp2f((static_cast<float>(SEGMENT.intensity) - 128.) / 32.);
        // this is useful to get a point in there:
        auto center = Vec2::fromParameters(SEGMENT.custom1, SEGMENT.custom2);
        // custom3 is only 5bit (0..31)
        float w = exp2f((static_cast<float>(SEGMENT.custom3) - 16.) / 8.);

        EVERY_NTH_CALL(2100) {
            DEBUG_PRINTF("[QM_DEBUG_FX] Params %.3f (%.3f %.3f) %.3f / Raw sx=%d ix=%d c1=%d c2=%d c3=%d\n",
                k, center.x, center.y, w, SEGMENT.speed, SEGMENT.intensity, SEGMENT.custom1, SEGMENT.custom2, SEGMENT.custom3
            );
        }

        for (const auto& coord : logoCoordinates()) {
            float d = coord.uv.length();
            d = sin_t(M_TWOPI * (d * k - w * beat));
            d *= d;
            d = 0.02 / d;

            // nach Polarwinkel unterdrücken...
            float theta = coord.uv.polarAngleFrom({0, 0}, 0.25 * beat * M_TWOPI);
            // aber reicht alle 8 Takte mal.
            float every8Beats = fmodf(beat, 8.) / 8.;
            float gaussCurveExponent = (every8Beats - 6.) / 1.5;
            float swirlIntensity = exp(-gaussCurveExponent*gaussCurveExponent);
            d *= exp(-theta * swirlIntensity);

            color.hue = color_.hue;
            color.sat = color_.sat;
            color.val = static_cast<uint8_t>(255. * clip(d));

            // EVERY_NTH_CALL(990) {
            //     if (abs(coord.uv.y) < 0.05) {
            //         DEBUG_PRINTF("[QM_DEBUG_VEC2] %.3f %.3f vs. %.2f %.2f| %.3f -> (%.3f,%.3f) - %.3f - %.3f > val = %d\n",
            //             coord.uv.x, coord.uv.y, center.x, center.y, k, distance.x, distance.y, r_, sine, sintensity
            //         );
            //     }
            // }

            /*
            float every16Beats = fmodf(beat, 16.) / 16.;
            d = coord.sdLine({-0.4, -0.5}, {+0.1, +0.5});
            d = exp(-5.*d);
            d *= exp(-10. * pow(every16Beats - 15., 2.)); // is a swell at each 15th of 16 beats
            color.sat = mix8(color.sat, 0, d);
            color.val = mix8(color.val, 255, d);
            */

            // purple flash because hue 210 is so nicey, for reasons
            /*
            float norm = coord.uv.length();
            d = sin_t(-4.*norm + time);
            float d2 = clip(abs(d));
            color.hue = mix8(color.hue, 210, d2);
            */

            // as an example of controlling the parameters. draws a gauss curve / circle around teh given point, white.
            uint8_t current_val = color.val;
            uint8_t current_sat = color.sat;
            d = 255. * coord.gaussAt(center, 0.01);
            color.val = max(color.val, static_cast<uint8_t>(d));
            color.sat = min(color.sat, static_cast<uint8_t>(255. - d));

            EVERY_NTH_CALL(831) {
                DEBUG_PRINTF("[QM_DEBUG_LOGO] %0.3f %0.3f vs %.3f %.3f: d=%.4f made S go %d->%d and V %d->%d\n",
                    coord.x, coord.y, center.x, center.y, d, current_sat, color.sat, current_val, color.val
                );
            }

            setLogo(coord.x, coord.y, color);
        }

        // Example of using one of the Logo::... areas (array of pixel indices)
        /*
        auto area = Logo::InnerTriangle;
        for (int i = 0; i < area.size(); i++) {
            auto coord = Logo::coord(area[i]);
            // could also use coord.uv for some calculation, of course.
            setLogo(coord.x, coord.y, CRGB(150, 250, 50));
        }
        */

        // now the reacting to the random draws from above
        if (SEGMENT.aux0 > 120) {
        float lastExpPeakAt = fractBeat < 0.125 ? 0. : 0.125;
        float exponent = fractBeat < 0.125 ? 5. : 1.4;
        /*
        float randomFlashIntensity = // supposed to be two exp-decay directly after each other
            exp(-exponent * (fractBeat-lastExpPeakat) * static_cast<float>(fractbeat >= lastExpPeakAt));
        */
        }


        EVERY_NTH_CALL(1111) {
            DEBUG_PRINTF("[QM_DEBUG_FX] Logo took %ld µs to compute.\n", measureMicros());
        }
    }

    if (IS_BASE_SEGMENT) {
        // "state variables" -> static (can also use the uint16 seg.aux0 & seg.aux1)
        static float phi = 0.;
        static float omega = 1.7;
        float hue = static_cast<float>(color.hue);

        SEGMENT.fadeToBlackBy(20);

        for (const auto& coord : baseCoordinates()) {
            FloatRgb col0 = cosinePalette(
                0.2 * time,
                {0.29, 0.22, 0.5},
                {0.43, 0.74, 0.74},
                {1.3, 0.7, 1.2},
                {0.85, 0.14, 0.83}
            );
            FloatRgb col1 = col0;
            // read the "uvS" coordinate as s == 0 in the middle of every side, becoming (-)0.5 towards the corners
            float uvs = min(abs(coord.uv.x), abs(coord.uv.y));
            /*
            my shader was:
            float uvm = min(abs(uv.x), abs(uv.y));
            col *= (1. - cos(pi * 5. * uvm)) * (0.6 + 0.4 * sin(0.2*iTime));
            col = pow(col, vec3(0.8));
            col *= (1. - cos(pi * uvm));
            */
            float d = (1. - cos_t(M_PI * 5. * uvs)) * (0.6 + 0.4 * sin_t(M_TWOPI * 0.125 * beat));
            col0.scale(d); // maybe d*d?
            col0.grade(0.8);
            float focusCorners = 1. - 0.8 * cos(M_PI * uvs);
            col0.scale(focusCorners);

            EVERY_NTH_CALL(431) {
                DEBUG_PRINTF("[QM_DEBUG_BASE] %d @ %.3f, %.3f => uvS=%.3f - t=%.3f; col1 (%.2f/%.2f/%.2f), d=%.4f, corner=%.3f - col0 (%.2f/%.2f/%.2f)\n",
                    coord.index, coord.uv.x, coord.uv.y, uvs, beat, col1.r, col1.g, col1.b, d, focusCorners, col0.r, col0.g, col0.b
                );
            }

            // now add a rotating line, showing the use of SDF geometry
            phi = 0.25 * beat * omega;
            Vec2 end{1, 1};
            end.rotate(phi);
            d = coord.sdLine(-end, end);
            d = exp(-2.5*d);

            color = col0.toCHSV();
            color.sat = mix8(255, 20, d);
            color.val = max(color.val, static_cast<uint8_t>(255.*d));
            setBase(coord.x, coord.y, color);
        }

        EVERY_NTH_CALL(1111) {
            DEBUG_PRINTF("[QM_DEBUG_FX] Base took %ld µs to compute.\n", measureMicros());
        }
    }

    uint8_t whiteBeat = beatsin8_t(SEGMENT.speed, 0, 255);
    uint8_t annoyingBlink = mix8(255, 0, exp(-fractBeat));

    if (IS_BACK_LED) {
        setSingle(annoyingBlink);

    } else if (IS_FLOOR_LED) {
        setSingle(whiteBeat);

    }

    return FRAMETIME;
}

static const char _data_FX_MODE_DEADLINE_TROPHY[] PROGMEM =
    "DEADLINE Trophy@BPM,SpotWidth,SpotX,SpotY,SpecialK;!,!;!;2;sx=110";
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
