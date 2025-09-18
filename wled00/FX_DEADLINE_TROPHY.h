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


const int DEBUG_LOG_EVERY_N_CALLS = 0; // for printing debug output every ... steps (0 = no debug out)

#define IS_DEBUG_STEP (DEBUG_LOG_EVERY_N_CALLS > 0 && (SEGENV.call % DEBUG_LOG_EVERY_N_CALLS) == 0)

const size_t nOuterLeft = 10;
const size_t nLeft = 35;
const size_t nBottom = 36;
const size_t nUpperRight = 17;
const size_t nOuterRight = 27;

const int hueSpread = 4.;
const int valSpread = 2.;
const float satDecay = 0.5;
const float satSpawnChance = 0.001;

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

uint16_t mode_DeadlineTrophy(void) {
    // official way to get the AudioReactive data, but we try our UDP route in our own Usermod
    // um_data_t *um_data = getAudioData();
    // uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

    size_t i, x, y;
    uint32_t col = SEGCOLOR(0);
    CHSV color = rgb2hsv_approximate(CRGB(col));
    CHSV color_(color);

    if (SEGENV.call == 0) {
        DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);
    }

    if (IS_DEBUG_STEP) {
        DEBUG_PRINTF("[QM_DEBUG] Segment %d, color %d (rgb %d, %d, %d, hsv %d, %d, %d)\n",
            strip.getCurrSegmentId(), col, R(col), G(col), B(col), color.hue, color.sat, color.val
        );
    }

    if (IS_LOGO_SEGMENT) {
        // circling piece of shit
        float phi = TWO_PI * fmod_t(0.0005 * strip.now, 1.);

        float center_x = (0.6 + 0.2 * sin_t(phi)) * logoW;
        float center_y = (0.5 + 0.2 * cos_t(phi)) * logoH;
        const float size = 13.;

        for (const auto& coord : DeadlineTrophy::logoCoordinates()) {
            x = coord.x;
            y = coord.y;

            //just a line sweep
            center_x = fmod_t(0.0005 * (strip.now % 2000), 1.) * (logoW + 2. * size) - size;
            center_x = logoW - center_x;

            float dist_x = float(x) - center_x;
            float dist_y = 0.;
            float intensity = exp(- (dist_x*dist_x)/size - (dist_y*dist_y)/size);

            intensity = dist_x > 0 ? exp(-dist_x / size) : 0.;
            color.hue = color_.hue - 90. * intensity;
            color.sat = color_.sat * intensity;
            // some annoying blinking, for now.
            color.val = static_cast<uint8_t>(255.f * exp(-0.003 * (strip.now % 2000)));

            setLogoHSV(x, y, color);
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
            float slow_wave = 0.7 + 0.3 * sin_t(TWO_PI / 10000. * strip.now);

            color.hue = color_.hue - 20. * wave * abs_wave;
            // color.val = color_.val * abs_wave * slow_wave;

            color.hue = s == 0 ? 0 : s == 1 ? 90 : s == 2 ? 180 : 210;

            if (i == 0) {
                // first is always white
                color.sat = 0;
            } else {
                color.sat = 100;
            }

            color.val = ((strip.now % 16000) > (i * 1000)) ? 255 : 20;

            setBaseHSV(x, y, color);
        }
    }

    auto stepTime = fmod_t(strip.now, 2.0);
    setBack(stepTime < 1.0 ? WHITE : BLACK);
    setFloor(stepTime < 1.0 ? BLACK : WHITE);

    return FRAMETIME;
} // mode_DeadlineTrophy


static const char _data_FX_MODE_DEADLINE_TROPHY[] PROGMEM =
    "DEADLINE TROPHY@;;!;1";
    // <-- TODO: find out how the parameters are encoded here. default is "name@;;!;1"

