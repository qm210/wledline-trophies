#pragma once

#include "FX.h"

// bla bla bla quickficks
uint16_t mode_static(void);

////////////////////////////
//  Deadline Trophy 2024  //
////////////////////////////

const size_t nOuterLeft = 10;
const size_t nLeft = 35;
const size_t nBottom = 36;
const size_t nUpperRight = 17;
const size_t nOuterRight = 27;

// the bars in the Logo
const int barOuterLeft[nOuterLeft] = {
    8, 7, 6, 5, 4, 3, 2, 1, 0,
    9
};
const int barLeft[nLeft] = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44
};
const int barBottom[nBottom] = {
    9, 11, 30, 35, 103, 102, 97, 96, 91, 90, 85, 72, 67,
    10, 31, 34, 104, 101, 98, 95, 92, 89, 86, 71, 68,
    32, 33, 105, 100, 99, 94, 93, 88, 87, 70, 69
};
const int barUpperRight[nUpperRight] = {
    44, 46, 47, 52, 53, 58,
    45, 48, 51, 54, 57, 59,
    49, 50, 55, 56, 60
};
const int barOuterRight[nOuterRight] = {
    87, 86, 85, 84, 83, 82, 81, 80, 79,
    70, 71, 72, 73, 74, 75, 76, 77, 78,
    69, 68, 67, 66, 65, 64, 63, 62, 61
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
const int nBase = 64;
const int indexBack = 170;
const int indexFloor = 171;

int hue[nBars] = {0};
int sat[nBars] = {0};
int val[nBars] = {0};
float line_direction = 0.;

const int hueSpread = 4.;
const int valSpread = 2.;
const float satDecay = 0.5;
const float satSpawnChance = 0.001;

// copy&paste from setupDeadlineTrophy(), is ok for now.
const int logoW = 27;
const int logoH = 21;

void setBase(size_t index, uint32_t color) {
    if (strip.getCurrSegmentId() != 1) {
        return;
    }
    // directly go for the LED busses, cause at least we understand these...
    busses.getBus(1)->setPixelColor(index % nBase, color);
}

void setLogo(size_t x, size_t y, uint32_t color) {
    if (strip.getCurrSegmentId() != 0) {
        return;
    }
    // is not that much more computational expensive, because the logo is at the first Bus.
    strip.setPixelColor(x + logoW * y, color);
}

void setSingleWhite(bool isFloor, uint32_t color) {
    auto index = isFloor ? 3 : 2;
    if (strip.getCurrSegmentId() != index) {
        return;
    }
    busses.getBus(index)->setPixelColor(0, color);
}
void setBack(uint32_t color) {
    setSingleWhite(false, color);
}
void setFloor(uint32_t color) {
    setSingleWhite(true, color);
}


uint32_t float_hsv(float hue, float sat, float val) {
    // parameters in [0, 255] but as float
    return uint32_t(
        CRGB(
            CHSV(
                static_cast<uint8_t>(hue),
                static_cast<uint8_t>(sat),
                static_cast<uint8_t>(val)
            )
        )
    );
}

const int DEBUG_STEPS = 5;
int DEBUG_COUNTER = 0;

uint16_t mode_DeadlineTrophy2024(void) {
  if (!strip.isDeadlineTrophy) return mode_static();

  auto isLogo = strip.getCurrSegmentId() == 0;

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  size_t b = 0;
  if (SEGENV.call == 0) {
    // very first call only, i.e. init code

    SEGMENT.fill(BLACK);
    for (b = 0; b < nBars; b++) {
        hue[b] = random(210, 330);
        val[b] = random(0, 255);
        sat[b] = 255;
    }

    line_direction = radians(random(0, 360));
  }

    size_t i, x, y;
    CHSV color;

    if (isLogo) {
        // circling piece of shit
        float phi = TWO_PI * fmod_t(0.0005 * strip.now, 1.);

        float center_x = (0.6 + 0.2 * sin_t(phi)) * logoW;
        float center_y = (0.5 + 0.2 * cos_t(phi)) * logoH;
        float size = 6.;

        for (x = 0; x < logoW; x++) {
            for (y = 0; y < logoH; y++) {
                // OVERWRITE: just a line sweep
                center_x = fmod_t(0.0005 * (strip.now % 2000), 1.) * (logoW + size);

                // float dist_x = float(x) - center_x;
                float dist_x = float(x) - center_x;
                float dist_y = 0.;
                float intensity = exp(- (dist_x*dist_x)/size - (dist_y*dist_y)/size);

                intensity = dist_x < 0 ? 0. : exp(-dist_x / size);

                setLogo(
                    x, y, float_hsv(210. - 50. * intensity, 255., 60. + 180. * intensity * intensity * intensity)
                );
            }
        }

        if (DEBUG_COUNTER <= 0) {
            DEBUG_COUNTER = DEBUG_STEPS;
        } else {
            DEBUG_COUNTER--;
        }
    }
    /*

    // LOGO
    if (strip.getCurrSegmentId() == 0) {
        for (b = 0; b < nBars; b++) {
            hue[b] = (hue[b] + random(-hueSpread, hueSpread)) % 255;
            val[b] = MAX(val[b] + random(-valSpread, valSpread), 255);
            sat[b] = static_cast<int>(satDecay * sat[b]);

            if (0.001 * random(0, 1000) < satSpawnChance) {
                sat[b] = 255;
            }

            // for (i = 0; i < nInBar[i]; i++) {
            //     auto color = CHSV(hue[b], sat[b], val[b]);
            //     SEGMENT.setPixelColor(indexBars[b][i], color);
            // }
        }

    }

    */

    for (int s = 0; s < 4; s++) {
        for (i = 0; i < 16; i++) {
            // strip.now is millisec uint32_t, so this will overflow ~ every 49 days. who shits a give.
            float wave = sin_t(PI / 15. * (static_cast<float>(i) - 0.007 * strip.now));
            float abs_wave = (wave > 0. ? wave : -wave);
            float slow_wave = 0.7 + 0.3 * sin_t(TWO_PI / 10000. * strip.now);
            setBase(
                16 * s + i,
                float_hsv(160. + 70. * wave * abs_wave, 255., 255. * wave * abs_wave * slow_wave)
            );
        }
    }

    auto stepTime = fmod_t(strip.now, 2.0);
    setBack(stepTime < 1.0 ? WHITE : BLACK);
    setFloor(stepTime < 1.0 ? BLACK : WHITE);

    return FRAMETIME;
} // mode_DeadlineTrophy2024


static const char _data_FX_MODE_DEADLINE_TROPHY_2024[] PROGMEM =
    "Deadline'24 Trophy@;;!;1";
    // <-- find out what the cryptic @... parameters mean. default is "name@;;!;1"
