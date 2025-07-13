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

const size_t nOuterLeft = 10;
const size_t nLeft = 35;
const size_t nBottom = 36;
const size_t nUpperRight = 17;
const size_t nOuterRight = 27;

// the bars in the Logo
const int barOuterLeft[nOuterLeft] = {
    161, 162, 163, 164, 165, 166, 167, 168, 169,
    160
};
const int barLeft[nLeft] = {
    156, 155, 154, 153, 152, 151, 150, 149,
    140, 141, 142, 143, 144, 145, 146, 147, 148,
    133, 132, 131, 130, 129, 128, 127, 126
};
const int barBottom[nBottom] = {
    137, 136, 64, 69, 79, 70, 75, 76, 81, 82, 99, 100,
    159, 138, 135, 65, 68, 71, 74, 77, 80, 83, 98, 101,
    158, 139, 134, 66, 67, 72, 73, 78, 79, 84, 97, 102
};
const int barUpperRight[nUpperRight] = {
    111, 116, 117, 122, 123, 125,
    110, 112, 115, 118, 121, 124,
    109, 113, 114, 119, 120
};
const int barOuterRight[nOuterRight] = {
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
const int* indexBars[] = {
    barOuterLeft,
    barLeft,
    barBottom,
    barUpperRight,
    barOuterRight
};

int hue[nBars] = {0};
int sat[nBars] = {0};
int val[nBars] = {0};
float line_direction = 0.;

const int hueSpread = 4.;
const int valSpread = 2.;
const float satDecay = 0.5;
const float satSpawnChance = 0.001;

const int logoW = DeadlineTrophy::logoW;
const int logoH = DeadlineTrophy::logoH;
const int baseSize = DeadlineTrophy::baseEdge;

// TODO: Umrechnungsfunktionen in DeadlineTrophy.h schieben.
inline void setPixel(size_t segmentIndex, int x, int y, uint32_t color) {
    if (strip.getCurrSegmentId() != segmentIndex) {
        return;
    }
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
    // TODO: 30°-Drehung
    // TODO: sinnieren über floatzahlige Koordinaten - evtl sinnvoll, evtl wumpe
    setPixel(1, x, y, color);
}

void setBack(uint32_t color) {
    setPixel(2, baseSize, logoH, color);
}

void setFloor(uint32_t color) {
    setPixel(3, baseSize, logoH + 1, color);
}

#define IS_BASE_SEGMENT (strip.getCurrSegmentId() == 0)
#define IS_LOGO_SEGMENT (strip.getCurrSegmentId() == 1)

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

const int DEBUG_LOG_EVERY_N_CALLS = 500; // for printing debug output every ... steps (0 = no debug out)

#define IS_DEBUG_STEP (DEBUG_LOG_EVERY_N_CALLS > 0 && (SEGENV.call % DEBUG_LOG_EVERY_N_CALLS) == 0)

uint16_t mode_DeadlineTrophy(void) {
    um_data_t *um_data;
    if (!UsermodManager::getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
        // add support for no audio
        um_data = simulateSound(SEGMENT.soundSim);
    }
    uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

    size_t i, x, y;
    uint32_t col = SEGCOLOR(0);
    CHSV color = rgb2hsv_approximate(CRGB(col));

    if (SEGENV.call == 0) {
        DEBUG_PRINTF("[DEADLINE_TROPHY] FX was called, now initialized for segment %d (%s) :)\n", strip.getCurrSegmentId(), SEGMENT.name);
        SEGMENT.fill(BLACK);

        line_direction = radians(random(0, 360));
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
            color.hue -= 90. * intensity;
            color.value *= intensity * intensity * intensity;

            col = uint32_t(CRGB(color));
            setLogo(x, y, col);
        }
    }

    if (IS_BASE_SEGMENT) {
        for (int s = 0; s < 4; s++)
        for (i = 0; i < 16; i++) {
            // strip.now is millisec uint32_t, so this will overflow ~ every 49 days. who shits a give.
            float wave = sin_t(PI / 15. * (static_cast<float>(i) - 0.007 * strip.now));
            float abs_wave = (wave > 0. ? wave : -wave);
            float slow_wave = 0.7 + 0.3 * sin_t(TWO_PI / 10000. * strip.now);
            color.hue -= 20. * wave * abs_wave;
            color.val *= wave * abs_wave * slow_wave;

            if (s == 0) {
                x = 1 + i;
                y = 0;
            }
            else if (s == 1) {
                x = 0;
                y = 16 - i;
            }
            else if (s == 2) {
                x = 17;
                y = 1 + i;
            }
            else if (s == 3) {
                x = 16 - i;
                y = 17;
            }

            col = uint32_t(CRGB(color));
            setBase(x, y, col);
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

